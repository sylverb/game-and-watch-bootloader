#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "sdcard.h"
#include "lcd.h"
#include "main.h"
#include "rg_rtc.h"
#include "bootloader.h"
#include "buttons.h"
#include "overlay.h"
#include "stm32h7xx_hal.h"

#include "ff.h"

static FATFS FatFs;  // Fatfs handle

void sdcard_hw_detect() {
    FRESULT cause;

    // Check if SD Card is connected to SPI1
    sdcard_init_spi1();
    sdcard_hw = SDCARD_HW_1;
    cause = f_mount(&FatFs, (const TCHAR *)"", 1);
    if (cause == FR_OK) {
        f_mount(NULL, "", 0);
        return;
    } else {
        sdcard_deinit_spi1();
    }

    // Check if SD Card is connected over OSPI1
    sdcard_init_ospi1();
    sdcard_hw = SDCARD_HW_2;
    cause = f_mount(&FatFs, (const TCHAR *)"", 1);
    if (cause == FR_OK) {
        f_mount(NULL, "", 0);
        return;
    } else {
        sdcard_deinit_ospi1();
    }

    // No SD Card detected
    sdcard_hw = SDCARD_HW_NO_SD_FOUND;
}

#if 0
/**
 * @param bank - Must be 1 or 2.
 * @param offset - Must be a multiple of 8192
 * @param bytes_remaining - Must be a multiple of 8192
 */
uint32_t erase_intflash(uint8_t bank, uint32_t offset, uint32_t size){
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;

    assert(bank == 1 || bank == 2);
    assert((offset & 0x1fff) == 0);
    assert((size & 0x1fff) == 0);

    HAL_FLASH_Unlock();

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Banks = bank;  // Must be 1 or 2
    EraseInitStruct.Sector = offset >> 13;
    EraseInitStruct.NbSectors = size >> 13;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        Error_Handler();
    }

    HAL_FLASH_Lock();

    return 0;
}
#endif

static void bootloader_run(void)
{
    char msg[256];
    uint16_t offset = 0;
    wdog_refresh();
    offset += overlay_draw_text(0, offset, GW_LCD_WIDTH, "Hello GnW", 0xFFFF, 0x0000);

    uint32_t sp = *((uint32_t *)FLASH_BANK2_BASE);
    uint32_t pc = *((uint32_t *)FLASH_BANK2_BASE+0x04);

    snprintf(msg, sizeof(msg), "Bank 2 sp: 0x%08lx PC=0x%08lx",sp,pc);
    offset += overlay_draw_text(0, offset, GW_LCD_WIDTH, msg, 0xFFFF, 0x0000);
    switch (sdcard_hw) {
        case SDCARD_HW_UNDETECTED:
        case SDCARD_HW_NO_SD_FOUND:
            offset += overlay_draw_text(0, offset, GW_LCD_WIDTH, "No SD Card found", 0xF800, 0x0000);
            break;
        case SDCARD_HW_1:
            offset += overlay_draw_text(0, offset, GW_LCD_WIDTH, "SD Card found (SPI1)", 0xFFFF, 0x0000);
            break;
        case SDCARD_HW_2:
            offset += overlay_draw_text(0, offset, GW_LCD_WIDTH, "SD Card found (QSPI1)", 0xFFFF, 0x0000);
            break;
    }
}

void bootloader_main()
{
    sdcard_hw_detect();

    lcd_backlight_on();

    while (true) {
        if(buttons_get() & B_POWER) {
            HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
            lcd_backlight_off();
            HAL_PWR_EnterSTANDBYMode();
        }
        if(buttons_get() & B_PAUSE){
            // TODO : Go to app in Bank 2
        }

        bootloader_run();
    }
}
