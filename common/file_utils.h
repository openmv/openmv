/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 *
 * Filesystem helper functions using MicroPython VFS interface.
 */
#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "py/obj.h"

// File handle that wraps a MicroPython file object
// Replaces FatFS FIL type with VFS-agnostic abstraction
typedef struct {
    mp_obj_t fp;      // MicroPython file object
    uint8_t flags;    // Open mode flags (FA_READ, FA_WRITE, etc.)
} file_t;

// FatFS-compatible mode flags for backward compatibility
#define FA_READ             0x01
#define FA_WRITE            0x02
#define FA_OPEN_EXISTING    0x00
#define FA_CREATE_NEW       0x04
#define FA_CREATE_ALWAYS    0x08
#define FA_OPEN_ALWAYS      0x10
#define FA_OPEN_APPEND      0x30

// Error/exception functions
void file_raise_format(file_t *fp);
void file_raise_corrupted(file_t *fp);
void file_raise_error(file_t *fp, mp_rom_error_text_t msg);

// File buffer functions
void file_buffer_init0();
void file_buffer_on(file_t *fp);  // Calls fb_alloc_all()
void file_buffer_off(file_t *fp); // Calls fb_free()

// File operations
void file_open(file_t *fp, const char *path, bool buffered, uint32_t flags);
void file_close(file_t *fp);
void file_seek(file_t *fp, size_t offset);
void file_truncate(file_t *fp);
void file_sync(file_t *fp);
size_t file_tell(file_t *fp);
size_t file_size(file_t *fp);
bool file_eof(file_t *fp);

// Buffered read/write operations
void file_read(file_t *fp, void *data, size_t size);
void file_write(file_t *fp, const void *data, size_t size);
void file_write_byte(file_t *fp, uint8_t value);
void file_write_short(file_t *fp, uint16_t value);
void file_write_long(file_t *fp, uint32_t value);
void file_read_check(file_t *fp, const void *data, size_t size);

#endif /* __FILE_UTILS_H__ */
