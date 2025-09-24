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

// CRC size enumeration
typedef enum {
    OMV_CRC16 = 16,
    OMV_CRC32 = 32
} omv_crc_size_t;

// HD=6 up to 114 bits
#define OMV_CRC16_POLY   (0xF94F)
#define OMV_CRC16_INIT   (0xFFFF)

// HD=6 up to 32738 bits (~4K)
#define OMV_CRC32_POLY   (0xFA567D89)
#define OMV_CRC32_INIT   (0xFFFFFFFF)

// Default software implementations
uint16_t omv_crc16_start(const void *buf, size_t len);
uint16_t omv_crc16_update(uint16_t crc, const void *buf, size_t len);

uint32_t omv_crc32_start(const void *buf, size_t len);
uint32_t omv_crc32_update(uint32_t crc, const void *buf, size_t len);

// Public API functions
static inline uint32_t omv_crc_start(omv_crc_size_t size, const void *buf, size_t len) {
    if (size == OMV_CRC16) {
        return omv_crc16_start(buf, len);
    } else if (size == OMV_CRC32) {
        return omv_crc32_start(buf, len);
    }
    return 0;
}

static inline uint32_t omv_crc_update(omv_crc_size_t size, uint32_t crc, const void *buf, size_t len) {
    if (size == OMV_CRC16) {
        return omv_crc16_update((uint16_t) crc, buf, len);
    } else if (size == OMV_CRC32) {
        return omv_crc32_update(crc, buf, len);
    }
    return crc;
}

// Read CRC value from the last bytes of buffer
static inline uint32_t omv_crc_read(omv_crc_size_t crc_size, const void *buf, size_t size) {
    const uint8_t *crc_buf = (const uint8_t *) buf + size - (crc_size == OMV_CRC16 ? 2 : 4);
    if (crc_size == OMV_CRC16) {
        return *((uint16_t *) crc_buf);
    } else {
        return *((uint32_t *) crc_buf);
    }
}

// Calculate CRC and write it to the last bytes of buffer
static inline void omv_crc_write(omv_crc_size_t crc_size, void *buf, size_t size) {
    uint32_t crc = omv_crc_start(crc_size, buf, size - (crc_size == OMV_CRC16 ? 2 : 4));
    uint8_t *crc_buf = (uint8_t *) buf + size - (crc_size == OMV_CRC16 ? 2 : 4);

    if (crc_size == OMV_CRC16) {
        crc_buf[0] = crc & 0xFF;
        crc_buf[1] = (crc >> 8) & 0xFF;
    } else {
        crc_buf[0] = crc & 0xFF;
        crc_buf[1] = (crc >> 8) & 0xFF;
        crc_buf[2] = (crc >> 16) & 0xFF;
        crc_buf[3] = (crc >> 24) & 0xFF;
    }
}

// Calculate and check if CRC matches the one stored in buffer
static inline bool omv_crc_check(omv_crc_size_t crc_size, const void *buf, size_t size) {
    return omv_crc_read(crc_size, buf, size) == omv_crc_start(crc_size, buf, size - (crc_size == OMV_CRC16 ? 2 : 4));
}
#endif // __OMV_CRC_H__
