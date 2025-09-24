/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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

// Only compile for STM32 families with configurable CRC hardware
#include STM32_HAL_H
#include "omv_crc.h"
#include <stdbool.h>

#if defined(STM32F7) || defined(STM32H7) || defined(STM32N6)
static CRC_HandleTypeDef hcrc = {0};
static bool crc_initialized = false;

static void omv_crc_init(void) {
    hcrc.Instance = CRC;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.Init.CRCLength = CRC_POLYLENGTH_32B;
    hcrc.Init.InitValue = OMV_CRC32_INIT;
    hcrc.Init.GeneratingPolynomial = OMV_CRC32_POLY;

    HAL_CRC_Init(&hcrc);
    crc_initialized = true;
}

uint16_t omv_crc16_start(const void *buf, size_t len) {
    if (!crc_initialized) {
        omv_crc_init();
    }

    // Set CRC16 polynomial and length directly
    WRITE_REG(hcrc.Instance->POL, OMV_CRC16_POLY);
    MODIFY_REG(hcrc.Instance->CR, CRC_CR_POLYSIZE, CRC_POLYLENGTH_16B);

    if (len == 0) {
        return OMV_CRC16_INIT;
    }

    return (uint16_t) HAL_CRC_Calculate(&hcrc, (uint32_t *) buf, len);
}

uint16_t omv_crc16_update(uint16_t crc, const void *buf, size_t len) {
    if (len == 0) {
        return crc;
    }

    return (uint16_t) HAL_CRC_Accumulate(&hcrc, (uint32_t *) buf, len);
}

uint32_t omv_crc32_start(const void *buf, size_t len) {
    if (!crc_initialized) {
        omv_crc_init();
    }

    // Set CRC32 polynomial and length directly
    WRITE_REG(hcrc.Instance->POL, OMV_CRC32_POLY);
    MODIFY_REG(hcrc.Instance->CR, CRC_CR_POLYSIZE, CRC_POLYLENGTH_32B);

    if (len == 0) {
        return OMV_CRC32_INIT;
    }

    return HAL_CRC_Calculate(&hcrc, (uint32_t *) buf, len);
}

uint32_t omv_crc32_update(uint32_t crc, const void *buf, size_t len) {
    if (len == 0) {
        return crc;
    }
    return HAL_CRC_Accumulate(&hcrc, (uint32_t *) buf, len);
}

#endif // STM32F7 || STM32H7 || STM32N6
