/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 OpenMV, LLC.
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
 * Stub functions for use when linking with nosys.
 */
#if OMV_NOSYS_STUBS_ENABLE
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include CMSIS_MCU_H

void _exit(int fd) {
    while (1) {
        __WFI();
    }
}

int _write(int handle, char *buffer, int size) {
    errno = ENOSYS;
    return -1;
}

int _read(int handle, char *buffer, int size) {
    errno = ENOSYS;
    return -1;
}

int _close(int f) {
    errno = ENOSYS;
    return -1;
}

int _lseek(int f, int ptr, int dir) {
    errno = ENOSYS;
    return -1;
}

int _fstat(int fs, struct stat *st) {
    errno = ENOSYS;
    return -1;
}

int _isatty(int f) {
    errno = ENOSYS;
    return 0;
}

int _stat(const char *f, struct stat *st) {
    errno = ENOSYS;
    return -1;
}

void *_sbrk(ptrdiff_t increment) {
    extern char __end__;
    static char *heap_end;

    extern char _heap_limit;
    static char *heap_limit = &_heap_limit;

    if (heap_end == 0) {
        heap_end = &__end__;
    }

    char *prev_heap_end = heap_end;

    // Check if the new break is within the limits
    if (heap_end + increment > heap_limit) {
        // Out of memory
        errno = ENOMEM;
        return (void *) -1;
    }

    heap_end += increment;
    return (void *) prev_heap_end;
}
#endif // OMV_NOSYS_STUBS_ENABLE
