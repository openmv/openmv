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

// There seems to be a mismatch between how the Alif HW CRC32 algorithm
// works and the other HW CRCs and the software implementation.
// Just leaving this here just in case it works on a different series.
#if 0
#define CRC0 ((CRC_Type *) CRC0_BASE)

static bool crc32_initialized = false;
static void crc_calculate(CRC_Type *crc, const void *buf, uint32_t len, uint32_t *value);

static void omv_crc32_init(void) {
    CRC0->CRC_CONTROL = CRC_32C |
                        CRC_CUSTOM_POLY |
                        CRC_ALGO_32_BIT_SIZE;
    CRC0->CRC_SEED = OMV_CRC32_INIT;
    CRC0->CRC_POLY_CUSTOM = OMV_CRC32_POLY;
    crc32_initialized = true;
}

uint32_t omv_crc32_start(const void *buf, size_t len) {
    uint32_t result = OMV_CRC32_INIT;

    if (!crc32_initialized) {
        omv_crc32_init();
    }

    if (len == 0) {
        return OMV_CRC32_INIT;
    }

    crc_calculate(CRC0, buf, len, &result);
    return result;
}

uint32_t omv_crc32_update(uint32_t crc, const void *buf, size_t len) {
    uint32_t result = crc;

    if (!crc32_initialized) {
        omv_crc32_init();
    }

    if (len == 0) {
        return crc;
    }

    crc_calculate(CRC0, buf, len, &result);
    return result;
}

static void crc_calculate(CRC_Type *crc, const void *buf, uint32_t len, uint32_t *value) {
    const uint8_t *data = (const uint8_t *) buf;

    if ((len % 4) == 0) {
        // Use hardware only if length is multiple of 4
        crc->CRC_SEED = *value;
        crc->CRC_CONTROL |= CRC_INIT_BIT;

        // Hardware path - process all data as 32-bit words
        const uint32_t *data32 = (const uint32_t *) data;
        for (uint32_t i = 0; i < len / 4; i++) {
            crc->CRC_DATA_IN_32_0 = data32[i];
        }
        *value = crc->CRC_OUT;
    } else {
        // Software path - use lookup table
        uint32_t result = *value;
        extern const uint32_t crc32_table[256];

        for (uint32_t i = 0; i < len; i++) {
            uint8_t index = (result >> 24) ^ data[i];
            result = (result << 8) ^ crc32_table[index];
        }
        *value = result;
    }
}
#endif
