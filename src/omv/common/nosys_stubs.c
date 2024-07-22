/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * stub functions for use when linking with nosys.
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
