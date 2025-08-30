/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023-2024 OpenMV, LLC.
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
#include "crc.h"
#include "global_map.h"
#include "omv_crc.h"

#define CRC0 ((CRC_Type *)CRC0_BASE)

static bool crc_initialized = false;

void omv_crc_init(void) {
    if (crc_initialized) {
        return;
    }
    
    crc_clear_config(CRC0);
    
    crc_enable_16bit_ccitt(CRC0);
    
    crc_initialized = true;
}

uint16_t omv_crc_calc(const void *buf, size_t size) {
    omv_crc_init();
    
    if (size == 0) {
        return 0xFFFF;
    }
    
    crc_set_seed(CRC0, 0xFFFF);
    crc_enable(CRC0);
    
    uint32_t crc_output = 0;
    crc_calculate_16bit(CRC0, buf, size, &crc_output);
    
    return (uint16_t)(crc_output & 0xFFFF);
}