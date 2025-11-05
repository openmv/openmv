/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
 *
 * Minimal system call stubs for QEMU with semihosting.
 * QEMU uses ARM semihosting for I/O, but libc still needs these stubs.
 */
#include <errno.h>
#include <sys/stat.h>

// QEMU uses MicroPython's GC for heap allocation, not sbrk
void *_sbrk(int incr) {
    errno = ENOMEM;
    return (void *)-1;
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
