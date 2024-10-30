/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * DFU bootloader SPI flash driver.
 */
#include <string.h>
#include STM32_HAL_H

#include "omv_boardconfig.h"
#include "omv_bootconfig.h"

#if OMV_BOOT_AXI_FLASH_ENABLE

#if defined(MCU_SERIES_H7)
#define FLASH_WORD_SIZE 32
#define FLASH_WORD_GET(x)   ((uint32_t) x)
#else
#define FLASH_WORD_SIZE 4
#define FLASH_WORD_GET(x)   *((uint32_t *) x)
#endif

#if defined(MCU_SERIES_F7)
#define FLASH_FLAG_PGSERR   (FLASH_FLAG_ERSERR)
#endif

#if defined(MCU_SERIES_F4)
static const uint32_t flash_sectors[] = {
    0x08000000, 0x08004000, 0x08008000, 0x0800C000,
    0x08010000, 0x08020000, 0x08040000, 0x08060000,
    0x08080000, 0x080A0000, 0x080C0000, 0x080E0000
};
#elif defined(MCU_SERIES_F7)
static const uint32_t flash_sectors[] = {
    0x08000000, 0x08008000, 0x08010000, 0x08018000,
    0x08020000, 0x08040000, 0x08080000, 0x080C0000,
    0x08100000, 0x08140000, 0x08180000, 0x081C0000
};
#elif defined(MCU_SERIES_H7)
static const uint32_t flash_sectors[] = {
    0x08000000, 0x08020000, 0x08040000, 0x08060000,
    0x08080000, 0x080A0000, 0x080C0000, 0x080E0000,
    0x08100000, 0x08120000, 0x08140000, 0x08160000,
    0x08180000, 0x081A0000, 0x081C0000, 0x081E0000
};
#else
#error MCU not specified!
#endif

#define OMV_BOOT_FLASH_SECTORS_COUNT (sizeof(flash_sectors) / sizeof(flash_sectors[0]))

static int axi_flash_erase(uint32_t sector) {
    uint32_t error = 0;

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
    #if defined(MCU_SERIES_F4) || defined(MCU_SERIES_F7)
    EraseInitStruct.Sector = sector;
    #elif defined(MCU_SERIES_H7)
    EraseInitStruct.Sector = (sector % 8);
    EraseInitStruct.Banks = (sector < 8) ? FLASH_BANK_1 : FLASH_BANK_2;
    #endif

    // Unlock flash
    HAL_FLASH_Unlock();

    #if defined(MCU_SERIES_H7)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1 | FLASH_FLAG_ALL_ERRORS_BANK2);
    #else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    #endif

    // Erase the sector
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &error) != HAL_OK) {
        HAL_FLASH_Lock();
        return -1;
    }

    // Lock the flash
    HAL_FLASH_Lock();
    return 0;
}

int axi_flash_read(uint32_t addr, uint8_t *buf, size_t size) {
    memcpy(buf, (void *) addr, size);
    return 0;
}

int axi_flash_write(uint32_t addr, const uint8_t *buf, size_t size) {
    // If at a block boundary, erase the block first.
    for (size_t i = 0; i < OMV_BOOT_FLASH_SECTORS_COUNT; i++) {
        if (addr == flash_sectors[i]) {
            if (axi_flash_erase(i) != 0) {
                return -1;
            }
            break;
        }
    }

    // Unlock flash
    HAL_FLASH_Unlock();

    for (int i = 0; i < size / FLASH_WORD_SIZE; i++, addr += FLASH_WORD_SIZE, buf += FLASH_WORD_SIZE) {
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, FLASH_WORD_GET(buf)) != HAL_OK) {
            HAL_FLASH_Lock(); // Lock the flash
            return -1;
        }
    }

    if (size % FLASH_WORD_SIZE) {
        uint8_t tmp[FLASH_WORD_SIZE] = {0};
        memcpy(tmp, buf, size % FLASH_WORD_SIZE);
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, FLASH_WORD_GET(tmp)) != HAL_OK) {
            HAL_FLASH_Lock(); // Lock the flash
            return -1;
        }
    }

    HAL_FLASH_Lock();
    return 0;
}
#endif // OMV_BOOT_AXI_FLASH_ENABLE
