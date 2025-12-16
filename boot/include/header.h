/*
 * Copyright (C) 2025 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Bootloader Header.
 */
#ifndef __OMV_BOOT_HEADER_H__
#define __OMV_BOOT_HEADER_H__

#include "omv_boardconfig.h"

// Boot magic value (also used for forced boot entry).
#ifndef OMV_BOOT_MAGIC_VALUE
#define OMV_BOOT_MAGIC_VALUE            (0xB00710AD)
#endif

// Header offset from bootloader start (default 4KB, can be overridden).
#ifndef OMV_BOOT_HEADER_OFFSET
#define OMV_BOOT_HEADER_OFFSET          (0x1000)
#endif

// Bootloader version numbers.
#define OMV_BOOT_VERSION_MAJOR          (1)
#define OMV_BOOT_VERSION_MINOR          (0)
#define OMV_BOOT_VERSION_PATCH          (1)

#ifndef LINKER_SCRIPT
#include <stdint.h>

// Header struct placed at fixed offset in bootloader binary.
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t reserved[25];
} omv_boot_header_t;

_Static_assert(sizeof(omv_boot_header_t) == 32, "omv_boot_header_t must be 32 bytes");
#endif // LINKER_SCRIPT

#endif // __OMV_BOOT_HEADER_H__
