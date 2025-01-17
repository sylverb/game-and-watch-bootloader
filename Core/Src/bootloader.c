#include <stdio.h>
#include <stdbool.h>

#include "gw_sdcard.h"
#include "main.h"
#include "ff.h"
#include "bootloader.h"

#define FIRMWARE_UPDATE_FILE "/firmware_update.bin"
#define RAM_START D1_AXISRAM_BASE /* 0x24000000 */
#define MAX_FILE_SIZE (1024 * 1024) /* 1MB of SRAM */

extern sdcard_hw_type_t sdcard_hw_type;

static FATFS FatFs; // Fatfs handle

void sdcard_hw_detect()
{
    FRESULT cause;

    // Check if SD Card is connected to SPI1
    sdcard_init_spi1();
    sdcard_hw_type = SDCARD_HW_SPI1;
    cause = f_mount(&FatFs, (const TCHAR *)"", 1);
    if (cause == FR_OK)
    {
        return;
    }
    else
    {
        sdcard_deinit_spi1();
    }

    // Check if SD Card is connected over OSPI1
    sdcard_init_ospi1();
    sdcard_hw_type = SDCARD_HW_OSPI1;
    cause = f_mount(&FatFs, (const TCHAR *)"", 1);
    if (cause == FR_OK)
    {
        return;
    }
    else
    {
        sdcard_deinit_ospi1();
    }

    // No SD Card detected
    sdcard_hw_type = SDCARD_HW_NO_SD_FOUND;
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

    res = f_open(&file, file_path, FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file: %s (Error: %d)\n", file_path, res);
        return false;
    }

    if (f_size(&file) > MAX_FILE_SIZE) {
        printf("File size exceeds available RAM: %llu bytes\n", f_size(&file));
        f_close(&file);
        return false;
    }

    while (1) {
        res = f_read(&file, ram_ptr, 512, &bytes_read);
        if (res != FR_OK) {
            printf("Failed to read file (Error: %d)\n", res);
            f_close(&file);
            return false;
        }

        if (bytes_read == 0) {
            break;
        }

        ram_ptr += bytes_read;
    }

    f_close(&file);

    printf("File successfully loaded to RAM at 0x%08lX\n", ram_address);
    return true;
}

static void __attribute__((naked)) start_app(void (*const pc)(void), uint32_t sp)
{
    __asm("           \n\
          msr msp, r1 /* load r1 into MSP */\n\
          bx r0       /* branch to the address at r0 */\n\
    ");
}

void boot_bank2(void)
{
    uint32_t sp = *((uint32_t *)FLASH_BANK2_BASE);
    uint32_t pc = *((uint32_t *)FLASH_BANK2_BASE + 1);

    // Check that Bank 2 content is valid
    if ((pc > FLASH_BANK2_BASE) && (pc < FLASH_END))
    {
        __set_MSP(sp);
        __set_PSP(sp);
        HAL_MPU_Disable();
        start_app((void (*const)(void))pc, (uint32_t)sp);
    }
    else
    {
        while(1) {
            wdog_refresh();
        }
    }
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
    printf("bootloader_main()\n");

    sdcard_hw_detect();

    if (sdcard_hw_type == SDCARD_HW_NO_SD_FOUND)
    {
        printf("No SD Card found\n");
    } else {
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
    }

    // Unmount Fs and Deinit SD Card if needed
    if (fs_mounted) {
        f_unmount("");
    }
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
    while (1) {
        boot_bank2();
    }
}
