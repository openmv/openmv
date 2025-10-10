/*
 * Copyright (C) 2023-2025 OpenMV, LLC.
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
 * QEMU dummy CSI driver.
 */
#if MICROPY_PY_CSI
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "py/mphal.h"
#include "py/runtime.h"

#include "omv_boardconfig.h"
#include "omv_i2c.h"
#include "omv_csi.h"

static int qemu_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    // Dummy implementation - accept all configurations
    return 0;
}

static int qemu_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    // Dummy implementation
    return 0;
}

static uint32_t qemu_clk_get_frequency(omv_clk_t *clk) {
    // Dummy implementation - return nominal frequency
    return clk->freq;
}

static int qemu_clk_set_frequency(omv_clk_t *clk, uint32_t frequency) {
    // Dummy implementation - accept any frequency
    clk->freq = frequency;
    return 0;
}

static int qemu_csi_snapshot(omv_csi_t *csi, image_t *dst_image, uint32_t flags) {
    // Dummy implementation - return timeout error since there's no camera
    return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
}

int omv_csi_ops_init(omv_csi_t *csi) {
    // Set CSI ops to dummy implementations
    csi->abort = qemu_csi_abort;
    csi->config = qemu_csi_config;
    csi->snapshot = qemu_csi_snapshot;

    // Set CSI clock ops
    csi->clk->freq = OMV_CSI_CLK_FREQUENCY;
    csi->clk->set_freq = qemu_clk_set_frequency;
    csi->clk->get_freq = qemu_clk_get_frequency;

    return 0;
}
#endif // MICROPY_PY_CSI
