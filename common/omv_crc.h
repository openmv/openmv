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
#ifndef __OMV_CRC_H__
#define __OMV_CRC_H__
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// 0xBAAD is ironically good - HD=5 up to 108 bits, HD=4 up to 7985 bits
#define OMV_CRC_POLY    (0xBAAD)
#define OMV_CRC_INIT    (0xFFFF)
#define OMV_CRC_SIZE    (16)
typedef uint16_t omv_crc_t;

// Initialize CRC hardware (if available)
void omv_crc_init(void);

// Start CRC calculation for buffer data
omv_crc_t omv_crc_start(const void *buf, size_t size);

// Update CRC with additional buffer data
omv_crc_t omv_crc_update(omv_crc_t crc, const void *buf, size_t size);

// Read CRC value from the last 2 bytes of buffer
static inline omv_crc_t omv_crc_read(const void *buf, size_t size) {
    const uint8_t *crc_buf = (const uint8_t *)buf + size - 2;
    return *((omv_crc_t*) crc_buf);
}

// Calculate CRC and write it to the last 2 bytes of buffer
static inline void omv_crc_write(void *buf, size_t size) {
    omv_crc_t crc = omv_crc_start(buf, size - 2);
    uint8_t *crc_buf = (uint8_t *)buf + size - 2;
    crc_buf[0] = crc & 0xFF;
    crc_buf[1] = (crc >> 8) & 0xFF;
}

// Calculate and check if CRC matches the one stored in buffer
static inline bool omv_crc_check(const void *buf, size_t size) {
    return omv_crc_read(buf, size) == omv_crc_start(buf, size - 2);
}
#endif // __OMV_CRC_H__
