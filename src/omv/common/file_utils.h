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
 * Filesystem helper functions.
 */
#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__
#include <stdint.h>
#include <stdbool.h>
#include <ff.h>
void file_raise_format(FIL *fp);
void file_raise_corrupted(FIL *fp);
void file_raise_error(FIL *fp, FRESULT res);

// These low-level functions/wrappers don't use file buffering,
// and they don't raise any exceptions, so they're safe to call
// from anywhere.
FRESULT file_ll_open(FIL *fp, const TCHAR *path, BYTE mode);
FRESULT file_ll_close(FIL *fp);
FRESULT file_ll_read(FIL *fp, void *buff, UINT btr, UINT *br);
FRESULT file_ll_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT file_ll_opendir(FF_DIR *dp, const TCHAR *path);
FRESULT file_ll_stat(const TCHAR *path, FILINFO *fno);
FRESULT file_ll_mkdir(const TCHAR *path);
FRESULT file_ll_unlink(const TCHAR *path);
FRESULT file_ll_rename(const TCHAR *path_old, const TCHAR *path_new);
FRESULT file_ll_touch(const TCHAR *path);

// File buffer functions.
void file_buffer_init0();
void file_buffer_on(FIL *fp);  // Calls fb_alloc_all()
void file_buffer_off(FIL *fp); // Calls fb_free()

void file_open(FIL *fp, const char *path, bool buffered, uint32_t flags);
void file_close(FIL *fp);
void file_seek(FIL *fp, UINT offset);
void file_truncate(FIL *fp);
void file_sync(FIL *fp);
uint32_t file_tell(FIL *fp);
uint32_t file_size(FIL *fp);
void file_read(FIL *fp, void *data, size_t size);
void file_write(FIL *fp, const void *data, size_t size);
void file_write_byte(FIL *fp, uint8_t value);
void file_write_short(FIL *fp, uint16_t value);
void file_write_long(FIL *fp, uint32_t value);
void file_read_check(FIL *fp, const void *data, size_t size);
const char *file_strerror(FRESULT res);
#endif /* __FILE_UTILS_H__ */
