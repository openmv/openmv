/*
 * Copyright (C) 2026 OpenMV, LLC.
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
 * Alif ISP driver (VeriSilicon ISP library glue).
 *
 * The CPI feeds the sensor's raw data to the ISP through its ISP port, and the
 * ISP's memory interface (MI) writes the processed frames to the frame buffers.
 */
#include "board_config.h"

#if MICROPY_PY_CSI && OMV_CSI_ISP_ENABLE
#include <string.h>
#include "py/mphal.h"

#include "sys_utils.h"
#include "isp.h"
// mpi_isp.h depends on these.
#include "vsi_comm_video.h"
#include "vsi_comm_isp.h"
#include "vsi_comm_sns.h"
#include "vsi_comm_calib.h"
#include "mpi_isp.h"
#include "mpi_isp_bls.h"
#include "mpi_isp_csm.h"
#include "mpi_isp_dmsc.h"
#include "mpi_isp_flt.h"
#include "mpi_isp_gamma_out.h"
#include "mpi_isp_wb.h"
#include "vsios_isp.h"

#include "omv_csi.h"
#include "framebuffer.h"
#include "alif_isp.h"

// ISP digital gain (white balance) registers.
#define ISP_DGAIN_RB                (0x800)
#define ISP_DGAIN_G                 (0x804)

// ISP memory interface (MI) registers.
#define ISP_MI_INIT                 (0xE04)
#define ISP_MI_INIT_SKIP            (1U << 2)
#define ISP_MI_INIT_CFG_UPD         (1U << 4)
#define ISP_MI_MP_Y_BASE_AD_INIT    (0xE08)

static const ISP_DEV isp_dev = 0;
static const ISP_PORT isp_port = { .devId = 0, .portId = 0 };
static const ISP_CHN isp_chn = { .devId = 0, .portId = 0, .chnId = CHN_ID_MP };

static bool isp_initialized;
static bool isp_streaming;
// The frame being written goes to the buffer at the head of the free queue.
static volatile bool isp_frame_valid;
// An error was reported while the current frame was being processed.
static volatile bool isp_frame_error;

// The library calls these sensor hooks, but doesn't define them. The sensor
// is controlled by its OpenMV driver, so there's nothing for them to do.
int VSI_ISP_SnsRegCallBack(void) {
    return 0;
}

int VSI_ISP_SnsUnRegCallBack(void) {
    return 0;
}

int VSI_ISP_SnsCfgIsBusy(void) {
    return 0;
}

int VSI_ISP_SnsStreamStatus(void) {
    return 0;
}

static int alif_isp_get_format(omv_csi_t *csi, PIXEL_FORMAT_E *format) {
    switch (csi->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            // The luma (Y) plane only.
            *format = PIXEL_FORMAT_YUV400;
            return 0;
        case PIXFORMAT_BAYER:
            *format = PIXEL_FORMAT_RAW8;
            return 0;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
            // RGB565 is converted from YUV422 after capture.
            *format = PIXEL_FORMAT_YUYV;
            return 0;
        default:
            return -1;
    }
}

bool alif_isp_is_active(omv_csi_t *csi) {
    return isp_streaming;
}

// The WB gains are fixed point with 8 fractional bits, from 1.0 to about 4.0.
static vsi_u16_t alif_isp_wb_gain(float gain) {
    return IM_CLAMP((int) (gain * 256.0f + 0.5f), 256, 1023);
}

void alif_isp_set_wb_gains(float r_gain, float g_gain, float b_gain) {
    vsi_u16_t r = alif_isp_wb_gain(r_gain);
    vsi_u16_t g = alif_isp_wb_gain(g_gain);
    vsi_u16_t b = alif_isp_wb_gain(b_gain);

    ISP_WB_ATTR_S wb = {
        .enable = 1,
        .opType = OP_TYPE_MANUAL,
        .manualAttr.wbGain = { .rGain = r, .grGain = g, .gbGain = g, .bGain = b },
    };
    VSI_MPI_ISP_SetWbAttr(isp_port, &wb);

    // While the port is streaming, the library only applies the gains from its
    // frame end processing, which isn't used, so write them directly.
    if (isp_streaming) {
        vsios_isp_write_reg(isp_port.devId, ISP_DGAIN_RB, (r << 16) | b);
        vsios_isp_write_reg(isp_port.devId, ISP_DGAIN_G, (g << 16) | g);
    }
}

static int alif_isp_init(omv_csi_t *csi) {
    if (isp_initialized) {
        return 0;
    }

    if (VSI_MPI_ISP_Init(isp_dev) != VSI_SUCCESS) {
        return -1;
    }

    ISP_DEV_ATTR_S dev_attr = { .ispWorkMode = WORK_MODE_NORMAL };
    if (VSI_MPI_ISP_SetDevAttr(isp_dev, &dev_attr) != VSI_SUCCESS) {
        return -1;
    }

    isp_initialized = true;
    return 0;
}

// The ISP crops the frame to the window if it can, otherwise it outputs the whole
// frame, and the window is cropped by the GPU. The output width must be even (whole
// YCbCr pairs), and the offset is kept even so the chroma pairs don't shift.
bool alif_isp_crops_window(omv_csi_t *csi) {
    return !(csi->fb->x % 2) && !(csi->fb->u % 2);
}

static rectangle_t alif_isp_get_output(omv_csi_t *csi) {
    if (alif_isp_crops_window(csi)) {
        return (rectangle_t) { csi->fb->x, csi->fb->y, csi->fb->u, csi->fb->v };
    }

    return (rectangle_t) { 0, 0, csi->resolution[csi->framesize][0], csi->resolution[csi->framesize][1] };
}

static int alif_isp_config(omv_csi_t *csi) {
    uint32_t w = csi->resolution[csi->framesize][0];
    uint32_t h = csi->resolution[csi->framesize][1];
    rectangle_t output = alif_isp_get_output(csi);
    PIXEL_FORMAT_E format;

    if (alif_isp_get_format(csi, &format) != 0) {
        return -1;
    }

    // Raw 10-bit Bayer input from the sensor, and the output cropped to the window or
    // the whole frame. Note the library's RECT_S fields for the output are swapped: top
    // is the horizontal offset, and left is the vertical offset (see Alif's
    // isp_param_set_crop()).
    ISP_PORT_ATTR_S port_attr = {
        .ispInputType = INPUT_TYPE_SENSOR,
        .ispMode = ISP_MODE_RAW,
        .hdrMode = HDR_MODE_LINEAR,
        // The 10-bit Bayer formats are in the same order as the CFA subformats.
        .pixelFormat = PIXEL_FORMAT_BGGR10 + csi->cfa_format,
        // The CPI's ISP port data must be sampled on the rising edge, otherwise
        // the input formatter reports a size error and drops every frame.
        .syncCfg = { .sampleEdge = SAMPLE_EDGE_RISING },
        .snsRect = { .width = w, .height = h },
        .inFormRect = { .width = w, .height = h },
        .outFormRect = {
            .top = output.x,
            .left = output.y,
            .width = output.w,
            .height = output.h,
        },
        .iSRect = { .width = w, .height = h },
    };

    if (VSI_MPI_ISP_SetPortAttr(isp_port, &port_attr) != VSI_SUCCESS) {
        return -1;
    }

    // Start with unity white balance gains.
    alif_isp_set_wb_gains(1.0f, 1.0f, 1.0f);

    // Black level subtraction is disabled until the sensor is calibrated.
    ISP_BLS_ATTR_S bls = { .enable = 0, .opType = OP_TYPE_MANUAL };
    ISP_DMSC_ATTR_S dmsc = { .enable = 1, .threshold = 10 };
    ISP_FLT_ATTR_S flt = { .enable = 0, .opType = OP_TYPE_MANUAL };
    ISP_GAMMA_OUT_ATTR_S gamma = { .enable = 0 };
    ISP_CSM_ATTR_S csm = { .type = ISP_CSM_TYPE_601, .quantization = ISP_CSM_FULL_RANGE };

    if (VSI_MPI_ISP_SetBlsAttr(isp_port, &bls) != VSI_SUCCESS ||
        VSI_MPI_ISP_SetDmscAttr(isp_port, &dmsc) != VSI_SUCCESS ||
        VSI_MPI_ISP_SetFltAttr(isp_port, &flt) != VSI_SUCCESS ||
        VSI_MPI_ISP_SetGammaOutAttr(isp_port, &gamma) != VSI_SUCCESS ||
        VSI_MPI_ISP_SetCsmAttr(isp_port, &csm) != VSI_SUCCESS) {
        return -1;
    }

    ISP_CHN_ATTR_S chn_attr = {
        .transBus = TRANS_BUS_ONLINE,
        .chnFormat = {
            .width = output.w,
            .height = output.h,
            .pixelFormat = format,
        },
    };

    if (VSI_MPI_ISP_SetChnAttr(isp_chn, &chn_attr) != VSI_SUCCESS) {
        return -1;
    }

    return 0;
}

// The library's buffer queue expects at least two buffers (one being written,
// and the next one), but only has room for one. So the buffers are managed here:
// the first one is queued to start streaming, and after that the MI frame end
// handler programs the next buffer's address directly.
static int alif_isp_queue_buffer(omv_csi_t *csi, vbuffer_t *buffer) {
    rectangle_t output = alif_isp_get_output(csi);
    VIDEO_BUF_S vbuf = {
        .index = 0,
        .numPlanes = 1,
        .planes[0] = {
            .dmaPhyAddr = LocalToGlobal(buffer->data),
            .size = output.w * output.h * omv_csi_get_dst_bpp(csi),
            .pUserAddr = buffer->data,
        },
    };

    return (VSI_MPI_ISP_QBUF(isp_chn, &vbuf) == VSI_SUCCESS) ? 0 : -1;
}

// Write the next frame to the buffer at the head of the free queue, or skip it
// if there are no free buffers. Called at frame end, before the next frame starts.
static void alif_isp_next_buffer(omv_csi_t *csi) {
    vbuffer_t *buffer = framebuffer_acquire(csi->fb, FB_FLAG_FREE | FB_FLAG_PEEK);
    vsi_u32_t mi_init;

    // MI_INIT also holds the output format.
    vsios_isp_read_reg(isp_port.devId, ISP_MI_INIT, &mi_init);

    if (buffer == NULL) {
        vsios_isp_write_reg(isp_port.devId, ISP_MI_INIT, mi_init | ISP_MI_INIT_SKIP);
        isp_frame_valid = false;
        return;
    }

    buffer->flags &= ~VB_FLAG_INVALID;
    vsios_isp_write_reg(isp_port.devId, ISP_MI_MP_Y_BASE_AD_INIT, LocalToGlobal(buffer->data));
    vsios_isp_write_reg(isp_port.devId, ISP_MI_INIT, mi_init | ISP_MI_INIT_CFG_UPD);
    isp_frame_valid = true;
}

int alif_isp_start(omv_csi_t *csi) {
    // Already streaming, the MI frame end handler picks up free buffers.
    if (isp_streaming) {
        return 0;
    }

    if (alif_isp_init(csi) != 0 || alif_isp_config(csi) != 0) {
        return -1;
    }

    vbuffer_t *buffer = framebuffer_acquire(csi->fb, FB_FLAG_FREE | FB_FLAG_PEEK);

    if (buffer == NULL || alif_isp_queue_buffer(csi, buffer) != 0) {
        return -1;
    }

    buffer->flags &= ~VB_FLAG_INVALID;
    isp_frame_valid = true;
    isp_frame_error = false;

    NVIC_ClearPendingIRQ(ISP_IRQ_IRQn);
    NVIC_ClearPendingIRQ(ISP_MI_IRQ_IRQn);
    NVIC_SetPriority(ISP_IRQ_IRQn, IRQ_PRI_CSI);
    NVIC_SetPriority(ISP_MI_IRQ_IRQn, IRQ_PRI_CSI);
    NVIC_EnableIRQ(ISP_IRQ_IRQn);
    NVIC_EnableIRQ(ISP_MI_IRQ_IRQn);

    if (VSI_MPI_ISP_EnableDev(isp_dev) != VSI_SUCCESS ||
        VSI_MPI_ISP_EnableChn(isp_chn) != VSI_SUCCESS ||
        VSI_MPI_ISP_EnablePort(isp_port) != VSI_SUCCESS) {
        alif_isp_stop(csi);
        return -1;
    }

    isp_streaming = true;
    return 0;
}

void alif_isp_stop(omv_csi_t *csi) {
    NVIC_DisableIRQ(ISP_IRQ_IRQn);
    NVIC_DisableIRQ(ISP_MI_IRQ_IRQn);

    if (isp_initialized) {
        VSI_MPI_ISP_DisablePort(isp_port);
        VSI_MPI_ISP_DisableChn(isp_chn);
        VSI_MPI_ISP_DisableDev(isp_dev);

        // Take back the queued buffer, if any.
        VIDEO_BUF_S vbuf;
        while (VSI_MPI_ISP_DQBUF(isp_chn, &vbuf, 0) == VSI_SUCCESS) {
        }
    }

    isp_streaming = false;
    isp_frame_valid = false;
}

void ISP_IRQHandler(void) {
    uint32_t status = isp_get_interrupt_status(isp_port);
    isp_irq_handler_clear_intr_status(isp_port, status);

    if (status & (ISP_INTR_DATALOSS | ISP_INTR_SIZE_ERR)) {
        isp_frame_error = true;
    }
}

void ISP_MI_IRQHandler(void) {
    omv_csi_t *csi = omv_csi_get(-1);
    uint32_t status = isp_mi_get_interrupt_status(isp_port);
    isp_mi_irq_handler_clear_intr_status(isp_port, status);

    if (!(status & ISP_MI_INTR_MP_FRAME_END)) {
        return;
    }

    bool frame_ok = isp_frame_valid && !isp_frame_error;
    isp_frame_error = false;

    if (frame_ok) {
        // Release the buffer from free queue -> used queue.
        framebuffer_release(csi->fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
    }

    // Program the buffer for the next frame (or the same one, if this frame
    // was dropped).
    alif_isp_next_buffer(csi);

    if (frame_ok && csi->frame_cb.fun) {
        csi->frame_cb.fun(csi->frame_cb.arg);
    }
}
#endif // MICROPY_PY_CSI && OMV_CSI_ISP_ENABLE
