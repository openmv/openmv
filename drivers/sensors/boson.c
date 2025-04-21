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
 * Boson driver.
 */
#include "omv_boardconfig.h"
#if (OMV_BOSON_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "py/mphal.h"
#include "framebuffer.h"
#include "omv_csi.h"

#include "Client_API.h"
#include "UART_Connector.h"

#define FLIR_BOSON_BOOT_TRY_COUNT (10)
#define FLIR_BOSON_BOOT_TIME_MS (1000)

static int boson_framesize = 0;

static int reset(omv_csi_t *csi) {
    csi->color_palette = NULL;

    int i = 0;
    FLR_BOSON_PARTNUMBER_T part;

    // Older FLIR Boson (< IDD 4.x) cameras take forever to boot up.
    for (; i < FLIR_BOSON_BOOT_TRY_COUNT; i++) {
        if (i > 1) {
            // Print something to prevent the user from thinking the camera is stuck.
            mp_printf(MP_PYTHON_PRINTER,
                      "CSI: FLIR Boson not ready, retrying (%d/%d)...\n",
                      i, FLIR_BOSON_BOOT_TRY_COUNT - 1);
        }

        // Give the camera time to boot.
        mp_hal_delay_ms(FLIR_BOSON_BOOT_TIME_MS);

        // Turn the com port on.
        Initialize();

        if (bosonGetCameraPN(&part) == FLR_OK) {
            break;
        }
    }

    if (i == FLIR_BOSON_BOOT_TRY_COUNT) {
        return -1;
    }

    if (!strncmp((char *) (part.value + 2), "640", 3)) {
        boson_framesize = OMV_CSI_FRAMESIZE_VGA;
    } else if (!strncmp((char *) (part.value + 2), "320", 3)) {
        boson_framesize = OMV_CSI_FRAMESIZE_QVGA;
    } else {
        return -1;
    }

    // Always restore factory defaults to ensure the camera is in a known state.
    FLR_RESULT ret = bosonRestoreFactoryDefaultsFromFlash();

    // FLIR BOSON may glitch after restoring factory defaults.
    if (ret != FLR_OK && ret != FLR_COMM_ERROR_READING_COMM) {
        return -1;
    }

    if (dvoSetOutputFormat(FLR_DVO_DEFAULT_FORMAT) != FLR_OK) {
        return -1;
    }

    if (dvoSetType(FLR_DVO_TYPE_MONO8) != FLR_OK) {
        return -1;
    }

    if (dvoApplyCustomSettings() != FLR_OK) {
        return -1;
    }

    if (telemetrySetState(FLR_DISABLE) != FLR_OK) {
        return -1;
    }

    return 0;
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    return (pixformat == PIXFORMAT_GRAYSCALE) ? 0 : -1;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    return (framesize == boson_framesize) ? 0 : -1;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    if (gaoSetTestRampState(enable ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
        return -1;
    }

    if (testRampSetType(0, FLR_TESTRAMP_VERT_SHADE) != FLR_OK) {
        return -1;
    }

    return 0;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    int ret = omv_csi_snapshot(csi, image, flags);

    if (ret < 0) {
        return ret;
    }

    int num_pixels = resolution[boson_framesize][0] * resolution[boson_framesize][1];

    if (csi->color_palette && (framebuffer_get_buffer_size(csi->fb) >= (num_pixels * sizeof(uint16_t)))) {
        for (int32_t i = num_pixels - 1; i >= 0; i--) {
            ((uint16_t *) image->data)[i] = csi->color_palette[image->data[i]];
        }

        image->pixfmt = PIXFORMAT_RGB565;
        csi->fb->pixfmt = PIXFORMAT_RGB565;
    }

    return ret;
}

int boson_init(omv_csi_t *csi) {
    // Initialize csi structure
    csi->reset = reset;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_colorbar = set_colorbar;
    csi->snapshot = snapshot;

    // Set csi flags
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->mono_bpp = sizeof(uint8_t);

    // Override standard resolutions
    resolution[OMV_CSI_FRAMESIZE_VGA][0] = 640;
    resolution[OMV_CSI_FRAMESIZE_VGA][1] = 512;

    resolution[OMV_CSI_FRAMESIZE_QVGA][0] = 320;
    resolution[OMV_CSI_FRAMESIZE_QVGA][1] = 256;

    if (reset(csi) != 0) {
        return -1;
    }

    csi->chip_id = (boson_framesize == OMV_CSI_FRAMESIZE_VGA) ? BOSON_640_ID : BOSON_320_ID;

    return 0;
}
#endif // (OMV_BOSON_ENABLE == 1)
