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
 * DFU callbacks implementations.
 */
#include <stdint.h>
#include <string.h>
#include "omv_bootconfig.h"
#include "flash.h"

extern bool tud_dfu_detached;

// Invoked when a DFU_DETACH request is received.
void tud_dfu_detach_cb(void) {
    tud_dfu_detached = true;
}

// Invoked before tud_dfu_download_cb() (state=DFU_DNBUSY)
// or before tud_dfu_manifest_cb() (state=DFU_MANIFEST).
uint32_t tud_dfu_get_timeout_cb(uint8_t itf, uint8_t state) {
    tud_dfu_detached = false;

    if (state == DFU_DNBUSY) {
        if (itf < 3) {
            return 0;
        } else {
            return 100;
        }
    } else if (state == DFU_MANIFEST) {
        return 0;
    }
    return 0;
}

// Invoked when DFU_DNLOAD is received followed by DFU_GETSTATUS (state=DFU_DNBUSY).
void tud_dfu_download_cb(uint8_t itf, uint16_t block, uint8_t const *buf, uint16_t size) {
    const partition_t *p = &OMV_BOOT_PARTITIONS[itf];
    uint32_t addr = p->start + (block * CFG_TUD_DFU_XFER_BUFSIZE);

    // Check if partition is writable.
    if (p->rdonly) {
        return tud_dfu_finish_flashing(DFU_STATUS_ERR_WRITE);
    }

    // Allow writes to this region.
    if (block == 0) {
        port_mpu_config(p, 0, 1, 1);
    }

    if (addr + size > p->limit) {
        return tud_dfu_finish_flashing(DFU_STATUS_ERR_ADDRESS);
    }

    if (flash_write(p->type, addr, buf, size) == 0) {
        tud_dfu_finish_flashing(DFU_STATUS_OK);
    } else {
        tud_dfu_finish_flashing(DFU_STATUS_ERR_WRITE);
    }
}

// Invoked when the download process is complete, i.e.,
// DFU_DNLOAD followed by DFU_GETSTATUS (state=Manifest).
void tud_dfu_manifest_cb(uint8_t itf) {
    port_mpu_config(&OMV_BOOT_PARTITIONS[itf], 1, 1, 1);
    return tud_dfu_finish_flashing(DFU_STATUS_OK);
}

// Invoked when DFU_UPLOAD request is received.
uint16_t tud_dfu_upload_cb(uint8_t itf, uint16_t block, uint8_t *buf, uint16_t size) {
    const partition_t *p = &OMV_BOOT_PARTITIONS[itf];
    uint32_t addr = p->start + (block * CFG_TUD_DFU_XFER_BUFSIZE);

    if (addr + size > p->limit) {
        // Respond with a short frame to indicate EOF to the host.
        size = 4;
        memset(buf, 0, size);
        tud_dfu_finish_flashing(DFU_STATUS_OK);
    } else if (flash_read(p->type, addr, buf, size) != 0) {
        size = 0;
        tud_dfu_finish_flashing(DFU_STATUS_ERR_FILE);
    } else {
        tud_dfu_finish_flashing(DFU_STATUS_OK);
    }

    tud_dfu_detached = false;
    return size;
}
