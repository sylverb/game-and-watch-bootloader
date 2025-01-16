#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "stm32h7xx_hal.h"
#include "ff.h"
#include "gw_intflash.h"

#define INTFLASH_2_UPDATE_FILE "/update_bank2.bin"
#define INTFLASH_2_SIZE (256 << 10) // 256KB
#define BUFFER_SIZE 256

static uint8_t buffer[BUFFER_SIZE];

/**
 * @param bank - Must be 2.
 * @param offset - Must be a multiple of 8192
 * @param size - Must be a multiple of 8192
 */
static uint32_t erase_intflash(uint8_t bank, uint32_t offset, uint32_t size)
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

static HAL_StatusTypeDef flash_write(uint32_t flash_address, uint8_t *data, uint32_t length)
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

bool update_bank2_flash(flash_progress_callback_t progress_callback)
{
    FIL file;
    UINT bytesRead;
    bool update_status = true;
    FRESULT res;

    res = f_open(&file, INTFLASH_2_UPDATE_FILE, FA_READ);
    if (res != FR_OK)
    {
        printf("No update file found\n");
        return false;
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

        if (progress_callback) {
            unsigned int percentage = (unsigned int)(((flash_address-FLASH_BANK2_BASE) * 100) / f_size(&file));
            progress_callback(percentage);
        }
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
    return update_status;
}
