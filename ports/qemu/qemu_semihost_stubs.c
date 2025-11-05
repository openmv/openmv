/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
 *
 * Minimal system call stubs for QEMU with semihosting.
 * QEMU uses ARM semihosting for I/O, but libc still needs these stubs.
 */
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "py/gc.h"
#include "py/runtime.h"

// QEMU uses MicroPython's GC for heap allocation, not sbrk
void *_sbrk(int incr) {
    errno = ENOMEM;
    return (void *) -1;
}

// Provide malloc/calloc/free using MicroPython's GC
// This is needed for drivers like softcsi that use standard C allocation
void *malloc(size_t size) {
    void *ptr = gc_alloc(size, false);
    return ptr;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void *ptr = gc_alloc(total_size, false);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        // Note: Not calling free(ptr) since GC will collect it
        return NULL;
    }
    void *new_ptr = gc_realloc(ptr, size, true);
    return new_ptr;
}

void free(void *ptr) {
    // GC will collect unused memory automatically
    // We can explicitly mark as free if needed
    if (ptr) {
        gc_free(ptr);
    }
}

int _close(int file) {
    errno = EBADF;
    return -1;
}

int _fstat(int file, struct stat *st) {
    errno = EBADF;
    return -1;
}

int _isatty(int file) {
    return 0;
}

int _lseek(int file, int ptr, int dir) {
    errno = EBADF;
    return -1;
}

int _read(int file, char *ptr, int len) {
    errno = EBADF;
    return -1;
}

int _write(int file, char *ptr, int len) {
    errno = EBADF;
    return -1;
}
