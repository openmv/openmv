/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include STM32_HAL_H
#include "flash.h"

#if defined (MCU_SERIES_F7)
#define FLASH_FLAG_PGSERR   (FLASH_FLAG_ERSERR)
#endif

extern void __fatal_error();

void flash_erase(uint32_t sector)
{
    // unlock
    HAL_FLASH_Unlock();

    #if defined(MCU_SERIES_H7)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1 | FLASH_FLAG_ALL_ERRORS_BANK2);
    #else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    #endif

    // erase the sector(s)
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase    = TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3; // voltage range needs to be 2.7V to 3.6V
    #if defined(MCU_SERIES_H7)
    EraseInitStruct.Sector = (sector % 8);
    if (sector < 8) {
        EraseInitStruct.Banks = FLASH_BANK_1;
    } else {
        EraseInitStruct.Banks = FLASH_BANK_2;
    }
    #else
    EraseInitStruct.Sector = sector;
    #endif
    EraseInitStruct.NbSectors = 1;

    uint32_t SectorError = 0;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        // error occurred during sector erase
        HAL_FLASH_Lock(); // lock the flash
        __fatal_error();
    }

    HAL_FLASH_Lock(); // lock the flash
}

void flash_write(const uint32_t *src, uint32_t dst, uint32_t size)
{
    // Unlock flash
    HAL_FLASH_Unlock();

    #if defined(MCU_SERIES_H7)
    // Program the flash 32 bytes at a time.
    for (int i=0; i<size/32; i++) {
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, dst, (uint64_t)(uint32_t) src) != HAL_OK) {
            // error occurred during flash write
            HAL_FLASH_Lock(); // lock the flash
            __fatal_error();
        }
        src += 8;
        dst += 32;
    }
    #else
    // Program the flash 4 bytes at a time.
    for (int i=0; i<size/4; i++) {
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, dst, *src) != HAL_OK) {
            // error occurred during flash write
            HAL_FLASH_Lock(); // lock the flash
            __fatal_error();
        }
        src += 1;
        dst += 4;
    }
    #endif

    // lock the flash
    HAL_FLASH_Lock();
}
