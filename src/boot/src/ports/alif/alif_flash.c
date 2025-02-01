/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
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
 * Alif Flash driver.
 */
#include <string.h>
#include ALIF_CMSIS_H

#include "omv_boardconfig.h"
#include "omv_bootconfig.h"
#include "mram.h"

#if OMV_BOOT_AXI_FLASH_ENABLE

int axi_flash_read(uint32_t addr, uint8_t *buf, size_t size) {
    memcpy(buf, (void *) addr, size);
    return 0;
}

int axi_flash_write(uint32_t addr, const uint8_t *buf, size_t size) {
    // Note TinyUSB buffers are aligned to a multiple of the MRAM sector size.
    if ((addr % MRAM_SECTOR_SIZE) || ((uint32_t) buf % MRAM_SECTOR_SIZE)) {
        return -1;
    }

    // Round up to multiple of the MRAM sector size.
    // Note TinyUSB buffers are much bigger than the MRAM sector size, so the
    // writes can be rounded up, and we can read more than the specified size.
    if (size % MRAM_SECTOR_SIZE) {
        size = (size + (MRAM_SECTOR_SIZE - 1)) & ~(MRAM_SECTOR_SIZE - 1);
    }

    // Note writes must be 16 bytes aligned and a multiple of 16 bytes.
    for (size_t i = 0; i < size / sizeof(uint64_t); i += 2) {
        ((volatile uint64_t *) addr)[i + 0] = ((volatile uint64_t *) buf)[i + 0];
        ((volatile uint64_t *) addr)[i + 1] = ((volatile uint64_t *) buf)[i + 1];
    }
    return 0;
}
#endif // OMV_BOOT_AXI_FLASH_ENABLE
