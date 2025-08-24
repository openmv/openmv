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

void omv_crc_init(void) {
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.GeneratingPolynomial = OMV_CRC_POLY;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    hcrc.Init.InitValue = OMV_CRC_INIT;
    hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;

    HAL_CRC_Init(&hcrc);
    crc_initialized = true;
}

omv_crc_t omv_crc_start(const void *buf, size_t size) {
    if (!crc_initialized) {
        omv_crc_init();
    }

    if (size == 0) {
        return OMV_CRC_INIT;
    }

    return (omv_crc_t) HAL_CRC_Calculate(&hcrc, (uint32_t *)buf, size);
}

omv_crc_t omv_crc_update(omv_crc_t crc, const void *buf, size_t size) {
    if (!crc_initialized) {
        omv_crc_init();
    }

    if (size == 0) {
        return crc;
    }

    return (omv_crc_t) HAL_CRC_Accumulate(&hcrc, (uint32_t *)buf, size);
}

#endif // STM32F7 || STM32H7 || STM32N6
