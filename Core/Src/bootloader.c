#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "gw_sdcard.h"
#include "main.h"
#include "rg_rtc.h"
#include "bootloader.h"
#include "gw_buttons.h"
#include "gw_timer.h"
#include "main.h"
#include "stm32h7xx_hal.h"

#include "ff.h"

#define INTFLASH_2_UPDATE_FILE "/update_bank2.bin"
#define INTFLASH_2_SIZE (256 << 10) // 256KB
#define BUFFER_SIZE 256

extern sdcard_hw_type_t sdcard_hw_type;

static FATFS FatFs; // Fatfs handle
static FIL file;
UINT bytesRead;
uint8_t buffer[BUFFER_SIZE];

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

/**
 * @param bank - Must be 2.
 * @param offset - Must be a multiple of 8192
 * @param size - Must be a multiple of 8192
 */
uint32_t erase_intflash(uint8_t bank, uint32_t offset, uint32_t size)
{
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;

    assert(bank == 2);
    assert((offset & 0x1fff) == 0);
    assert((size & 0x1fff) == 0);

    HAL_FLASH_Unlock();

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Banks = bank; // Must be 2
    EraseInitStruct.Sector = offset >> 13;
    EraseInitStruct.NbSectors = size >> 13;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_FLASH_Lock();

    return 0;
}

HAL_StatusTypeDef flash_write(uint32_t flash_address, uint8_t *data, uint32_t length)
{
    // A flash word is 128bits (16 bytes)
    HAL_StatusTypeDef status;
    HAL_FLASH_Unlock();
    for (uint32_t i = 0; i < length; i += 16)
    {
        uint32_t data_address = (uint32_t)(data) + i;
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_address + i, data_address);
        if (status != HAL_OK)
        {
            printf("status %x\n", status);
            HAL_FLASH_Lock();
            return status;
        }
    }
    HAL_FLASH_Lock();
    return HAL_OK;
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

void update_bank2_flash()
{
    bool update_status = true;
    FRESULT res;

    res = f_open(&file, INTFLASH_2_UPDATE_FILE, FA_READ);
    if (res != FR_OK)
    {
        printf("No update file found\n");
        return;
    }

    uint32_t flash_address = FLASH_BANK2_BASE;

    // File is present, erase bank
    erase_intflash(2, FLASH_SECTOR_0, INTFLASH_2_SIZE);

    do
    {
        res = f_read(&file, buffer, BUFFER_SIZE, &bytesRead);
        if (res != FR_OK)
        {
            update_status = false;
            break;
        }

        // Write block in internal flash bank
        if (flash_write(flash_address, buffer, bytesRead) != HAL_OK)
        {
            update_status = false;
            printf("Flash writing error @0x%lx - %d\n", flash_address, bytesRead);
            break;
        }

        // Next block
        flash_address += bytesRead;
    } while (bytesRead == BUFFER_SIZE);

    f_close(&file);
    if (update_status)
    {
        printf("Flashing done, delete update file\n");
        f_unlink(INTFLASH_2_UPDATE_FILE);
    }
    else
    {
        printf("Flashing failed\n");
    }
}

void bootloader_main(void)
{
    printf("bootloader_main()\n");

    sdcard_hw_detect();

    if (sdcard_hw_type == SDCARD_HW_NO_SD_FOUND)
    {
        printf("No SD Card found\n");
    }
    else
    {
        update_bank2_flash();
    }

    while (1)
    {
        boot_bank2();
    }
}
