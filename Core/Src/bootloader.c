#include <stdio.h>
#include <stdbool.h>

#include "gw_buttons.h"
#include "gw_sdcard.h"
#include "gw_flash.h"
#include "gw_lcd.h"
#include "gw_gui.h"
#include "main.h"
#include "gittag.h"
#include "ff.h"
#include "diskio.h"
#include "bootloader.h"

#define FIRMWARE_UPDATE_FILE "/retro-go_update.bin"
/* Physical drive number used by FatFs for the default volume (logical drive 0) */
#define SD_PDRV 0
#define RAM_START D1_AXISRAM_BASE /* 0x24000000 */
#define MAX_FILE_SIZE (1024 * 1024) /* 1MB of SRAM */

extern sdcard_hw_type_t sdcard_hw_type;

static FATFS FatFs; // Fatfs handle

void sdcard_hw_detect()
{
    FRESULT cause;
    DSTATUS disk_status;

    // Check if SD Card is connected to SPI1: detect by SD commands first, then mount FS
    sdcard_init_spi1();
    sdcard_hw_type = SDCARD_HW_SPI1;
    disk_status = disk_initialize(SD_PDRV);
    if (disk_status == 0)
    {
        cause = f_mount(&FatFs, (const TCHAR *)"", 1);
        if (cause == FR_OK)
        {
            fs_mounted = true;
            return;
        }
        /* Card responds but no FAT/exFAT volume FatFs can mount (e.g. NTFS, ext4). */
        fs_mounted = false;
        sdcard_hw_type = SDCARD_HW_SPI1_UNSUPPORTED_FS;
        return;
    }
    sdcard_deinit_spi1();

    // Check if SD Card is connected over OSPI1
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET); // Enable 3.3V for OSPI1
    sdcard_init_ospi1();
    sdcard_hw_type = SDCARD_HW_OSPI1;
    disk_status = disk_initialize(SD_PDRV);
    if (disk_status == 0)
    {
        cause = f_mount(&FatFs, (const TCHAR *)"", 1);
        if (cause == FR_OK)
        {
            fs_mounted = true;
            return;
        }
        fs_mounted = false;
        sdcard_hw_type = SDCARD_HW_OSPI1_UNSUPPORTED_FS;
        return;
    }
    sdcard_deinit_ospi1();
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET); // Disable 3.3V

    // No SD Card detected (no response to SD commands on either interface)
    sdcard_hw_type = SDCARD_HW_NO_SD_FOUND;
}

void enable_screen()
{
    static bool enabled = false;

    if (!enabled)
    {
        lcd_backlight_off();

        /* Power off LCD and external Flash */
        lcd_deinit(&hspi2);

        // Keep this
        // at least 8 frames at the end of power down (lcd_deinit())
        // 4 x 50 ms => 200ms
        for (int i = 0; i < 4; i++) {
            wdog_refresh();
            HAL_Delay(50);
        }

        /* Power on LCD and external Flash */
        lcd_init(&hspi2, &hltdc);

        // Keep this
        for (int i = 0; i < 4; i++) {
            wdog_refresh();
            HAL_Delay(50);
        }

        gw_gui_fill(0x0000);
        lcd_backlight_set(180);
        enabled = true;
    }
}

bool file_exists(const char *path) {
    FILINFO fileInfo;
    FRESULT result;

    result = f_stat(path, &fileInfo);
    return (result == FR_OK);
}

bool load_file_to_ram(const char *file_path, uint32_t ram_address) {
    FIL file;
    FRESULT res;
    UINT bytes_read;
    uint8_t *ram_ptr = (uint8_t *)ram_address;
    size_t file_size;
    size_t remaining;

    res = f_open(&file, file_path, FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file: %s (Error: %d)\n", file_path, res);
        return false;
    }

    if (f_size(&file) > MAX_FILE_SIZE + sizeof(uint32_t)) {
        // file is including update archive and size in stored after the 1MB limit
        f_lseek(&file, MAX_FILE_SIZE);
        res = f_read(&file, &file_size, sizeof(file_size), &bytes_read);
        if (res != FR_OK || bytes_read != sizeof(file_size)) {
            printf("Failed to read file size from the end of the file (Error: %d)\n", res);
            f_close(&file);
            return false;
        }
        f_lseek(&file, 0);
        if (file_size > MAX_FILE_SIZE) {
            printf("File size exceeds available RAM: %u bytes\n", file_size);
            f_close(&file);
            return false;
        }
    } else {
        file_size = f_size(&file);
    }

    remaining = file_size;

    while (remaining > 0) {
        size_t to_read = (remaining > 512) ? 512 : remaining;
        res = f_read(&file, ram_ptr, to_read, &bytes_read);
        if (res != FR_OK) {
            printf("Failed to read file (Error: %d)\n", res);
            f_close(&file);
            return false;
        }

        if (bytes_read == 0) {
            break;
        }

        ram_ptr += bytes_read;
        remaining -= bytes_read;
    }

    f_close(&file);

    printf("File successfully loaded to RAM at 0x%08lX size %d\n", ram_address, file_size);
    return true;
}

static void __attribute__((naked)) start_app(void (*const pc)(void), uint32_t sp)
{
    __asm("           \n\
          msr msp, r1 /* load r1 into MSP */\n\
          bx r0       /* branch to the address at r0 */\n\
    ");
}

static void show_info(bool show_press_key) {
    uint8_t line = 0;
    char text[50];
    uint32_t sp = *((uint32_t *)FLASH_BANK2_BASE);
    uint32_t pc = *((uint32_t *)FLASH_BANK2_BASE + 1);
    DWORD sector_count = 0;
    const char *fs_type_str = "Unknown";
    unsigned long size_gb = 0;
    enable_screen();
    switch_ospi_gpio(true);
    gw_gui_draw_text(10, line++ * 10, GIT_TAG, GUI_WHITE);
    gw_gui_draw_text(10, line++ * 10, "By Sylver Bruneau (2025-2026)", GUI_WHITE);
    line++;
    /* Line 1: hardware detection; line 2: filesystem / volume */
    switch (sdcard_hw_type) {
        case SDCARD_HW_NO_SD_FOUND:
            gw_gui_draw_text(10, line * 10, "SD Card detection :", GUI_RED);
            gw_gui_draw_text(170, line++ * 10, "No SD Card found", GUI_RED);
            gw_gui_draw_text(10, line++ * 10, "FS : -", GUI_WHITE);
            break;
        case SDCARD_HW_SPI1_UNSUPPORTED_FS:
            gw_gui_draw_text(10, line * 10, "SD Card detection :", GUI_GREEN);
            gw_gui_draw_text(170, line++ * 10, "OK (SPI1)", GUI_GREEN);
            gw_gui_draw_text(10, line++ * 10, "FS : not FAT/exFAT", GUI_RED);
            break;
        case SDCARD_HW_OSPI1_UNSUPPORTED_FS:
            gw_gui_draw_text(10, line * 10, "SD Card detection :", GUI_GREEN);
            gw_gui_draw_text(170, line++ * 10, "OK (OSPI1)", GUI_GREEN);
            gw_gui_draw_text(10, line++ * 10, "FS : not FAT/exFAT", GUI_RED);
            break;
        case SDCARD_HW_SPI1:
        case SDCARD_HW_OSPI1: {
            FATFS *pfs = &FatFs;

            gw_gui_draw_text(10, line * 10, "SD Card detection :", GUI_GREEN);
            if (sdcard_hw_type == SDCARD_HW_SPI1) {
                gw_gui_draw_text(170, line++ * 10, "OK (SPI1)", GUI_GREEN);
            } else {
                gw_gui_draw_text(170, line++ * 10, "OK (OSPI1)", GUI_GREEN);
            }

            if (fs_mounted) {
                if (pfs->n_fatent > 2 && pfs->csize != 0) {
                    unsigned long long total_sectors =
                        (unsigned long long)(pfs->n_fatent - 2) * (unsigned long long)pfs->csize;
                    if (total_sectors <= 0xFFFFFFFFUL)
                        sector_count = (DWORD)total_sectors;
                    else
                        sector_count = 0xFFFFFFFFUL;
                }

                switch (pfs->fs_type) {
                    case FS_EXFAT:
                        fs_type_str = "exFAT";
                        break;
                    case FS_FAT12:
                        fs_type_str = "FAT12";
                        break;
                    case FS_FAT16:
                        fs_type_str = "FAT16";
                        break;
                    case FS_FAT32:
                        fs_type_str = "FAT32";
                        break;
                    default:
                        fs_type_str = "?";
                        break;
                }

                if (sector_count != 0) {
                    unsigned long long bytes = (unsigned long long)sector_count * 512ULL;
                    size_gb = (unsigned long)(bytes / 1000000000ULL);
                    sprintf(text, "FS : %lu GB %s", size_gb, fs_type_str);
                    gw_gui_draw_text(10, line++ * 10, text, GUI_GREEN);
                } else {
                    gw_gui_draw_text(10, line++ * 10, "FS : ?", GUI_WHITE);
                }
            } else {
                gw_gui_draw_text(10, line++ * 10, "FS : ?", GUI_WHITE);
            }
            break;
        }
        default:
            gw_gui_draw_text(10, line * 10, "SD Card detection :", GUI_RED);
            gw_gui_draw_text(170, line++ * 10, "Unknown", GUI_RED);
            gw_gui_draw_text(10, line++ * 10, "FS : -", GUI_WHITE);
            break;
    }
    sprintf(text, "Bank 2 PC: 0x%08lX SP: 0x%08lX", pc, sp);
    if ((pc > FLASH_BANK2_BASE) && (pc < FLASH_BANK2_BASE + 256*1024)) {
        gw_gui_draw_text(10, line++ * 10, text, GUI_GREEN);
    } else {
        gw_gui_draw_text(10, line++ * 10, text, GUI_RED);
    }

    OSPI_DisableMemoryMappedMode();
    if (OSPI_Init(&hospi1)) {
        sprintf(text, "External Flash: %s (%ldMB)", OSPI_GetFlashName(), OSPI_GetFlashSize() / (1024 * 1024));
        gw_gui_draw_text(10, line++ * 10, text, GUI_GREEN);
    } else {
        gw_gui_draw_text(10, line++ * 10, "No external flash detected !!!", GUI_RED);
        gw_gui_draw_text(10, line++ * 10, "Bad soldering ?", GUI_RED);
    }

    line++;
    if (show_press_key) {
        gw_gui_draw_text(10, line++ * 10, "Press A/B button to continue", GUI_WHITE);
    } else {
        gw_gui_draw_text(10, line++ * 10, "Failed to boot bank2", GUI_RED);
        line++;
        gw_gui_draw_text(10, line++ * 10, "Insert SD Card with retro-go_update.bin", GUI_WHITE);
        gw_gui_draw_text(10, line++ * 10, "and reboot", GUI_WHITE);
    }
}

void boot_bank2(void)
{
    uint32_t sp = *((uint32_t *)FLASH_BANK2_BASE);
    uint32_t pc = *((uint32_t *)FLASH_BANK2_BASE + 1);

    // Check that Bank 2 content is valid
    __set_MSP(sp);
    __set_PSP(sp);
    HAL_MPU_Disable();
    start_app((void (*const)(void))pc, (uint32_t)sp);
}

void set_vtor(uint32_t address) {
    SCB->VTOR = address;
    __DSB();
    __ISB();
}

void boot_ram(void)
{
    uint32_t sp = *((uint32_t *)RAM_START);
    uint32_t pc = *((uint32_t *)RAM_START + 1);

    __set_MSP(sp);
    __set_PSP(sp);
    set_vtor(RAM_START);
    start_app((void (*const)(void))pc, (uint32_t)sp);
}

void bootloader_main(void)
{
    uint32_t pc = *((uint32_t *)FLASH_BANK2_BASE + 1);

    printf("bootloader_main()\n");

    sdcard_hw_detect();

    uint32_t boot_buttons = buttons_get();
    if (boot_buttons & B_PAUSE)
    {
        show_info(true);
        while (1) {
            boot_buttons = buttons_get();
            if (boot_buttons & B_POWER) {
                GW_EnterDeepSleep();
            } else if (boot_buttons & (B_A | B_B)) {
                break;
            }
        }
    }

    if (sdcard_hw_type == SDCARD_HW_NO_SD_FOUND)
    {
        printf("No SD Card found\n");
    } else if (fs_mounted) {
        if (load_file_to_ram(FIRMWARE_UPDATE_FILE, RAM_START)) {
            // Unmount Fs and Deinit SD Card
            f_unmount("");
            switch (sdcard_hw_type) {
                case SDCARD_HW_SPI1:
                    sdcard_deinit_spi1();
                break;
                case SDCARD_HW_OSPI1:
                    sdcard_deinit_ospi1();
                break;
                default:
                break;
            }
            boot_ram();
        }
    } else if (sdcard_hw_type == SDCARD_HW_SPI1_UNSUPPORTED_FS ||
               sdcard_hw_type == SDCARD_HW_OSPI1_UNSUPPORTED_FS) {
        printf("SD card present but filesystem not supported (FAT/exFAT only)\n");
    }

    // Unmount Fs and Deinit SD Card if needed
    if (fs_mounted) {
        f_unmount("");
    }
    switch (sdcard_hw_type) {
        case SDCARD_HW_SPI1:
        case SDCARD_HW_SPI1_UNSUPPORTED_FS:
            sdcard_deinit_spi1();
        break;
        case SDCARD_HW_OSPI1:
        case SDCARD_HW_OSPI1_UNSUPPORTED_FS:
            sdcard_deinit_ospi1();
        break;
        default:
        break;
    }

    // If bank 2 is not valid, show info screen
    if ((pc < FLASH_BANK2_BASE) || (pc >= FLASH_BANK2_BASE + 256*1024)) {
        show_info(false);
        while (1) {
            boot_buttons = buttons_get();
            if (boot_buttons & B_POWER) {
                GW_EnterDeepSleep();
            }
        }
    }
    while (1) {
        boot_bank2();
    }
}
