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
 * Bootloader flash interface.
 */
#ifndef __BOOT_FLASH_H__
#define __BOOT_FLASH_H__
int axi_flash_read(uint32_t addr, uint8_t *buf, size_t size);
int axi_flash_write(uint32_t addr, const uint8_t *buf, size_t size);

int spi_flash_deinit();
int spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t size);
int spi_flash_write(uint32_t addr, const uint8_t *buf, uint32_t size);
int spi_flash_memory_map();

static inline int flash_read(uint32_t ptype, uint32_t addr, uint8_t *buf, uint32_t size) {
    #if OMV_BOOT_AXI_FLASH_ENABLE
    if (ptype == PTYPE_AXI_FLASH) {
        return axi_flash_read(addr, buf, size);
    }
    #endif
    #if OMV_BOOT_SPI_FLASH_ENABLE
    if (ptype == PTYPE_SPI_FLASH) {
        return spi_flash_read(addr, buf, size);
    }
    #endif
    return -1;
}

static inline int flash_write(uint32_t ptype, uint32_t addr, const uint8_t *buf, uint32_t size) {
    #if OMV_BOOT_AXI_FLASH_ENABLE
    if (ptype == PTYPE_AXI_FLASH) {
        return axi_flash_write(addr, buf, size);
    }
    #endif
    #if OMV_BOOT_SPI_FLASH_ENABLE
    if (ptype == PTYPE_SPI_FLASH) {
        return spi_flash_write(addr, buf, size);
    }
    #endif
    return -1;
}
#endif //__BOOT_FLASH_H__
