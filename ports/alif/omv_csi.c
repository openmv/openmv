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
 * Alif CSI driver.
 */
#if MICROPY_PY_CSI
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "py/mphal.h"
#include "py/runtime.h"

#include "alif_hal.h"
#include "alif_isp.h"
#include "cpi.h"
#include "sys_ctrl_cpi.h"
#include "sys_clocks.h"
#include "sys_utils.h"

#include "board_config.h"
#if defined(OMV_CSI_MIPI_LANES)
#include "csi.h"
#include "sys_ctrl_csi.h"
#include "DPHY_CSI2.h"
#endif
#include "omv_gpio.h"
#include "omv_gpu.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "memcpy.h"

// Bits missing from cpi.h
#define CAM_CFG_INTERFACE_Pos       (0U)
#define CAM_CFG_CSI_HALT_EN_Pos     (1U)
#define CAM_CFG_RW_ROUNDUP_Pos      (8U)
#define CAM_CFG_PXCLK_POL_Pos       (12U)
#define CAM_CFG_ENDIANNESS_Pos      (20U)

#define CPI_VSYNC_MODE_DISABLE      (0)
#define CPI_VSYNC_MODE_ENABLE       (1)

#define CPI_VSYNC_WAIT_DISABLE      (0)
#define CPI_VSYNC_WAIT_ENABLE       (1)

#define CPI_IRQ_FLAGS   (CAM_INTR_STOP | CAM_INTR_VSYNC | CAM_INTR_HSYNC |    \
                         CAM_INTR_INFIFO_OVERRUN | CAM_INTR_OUTFIFO_OVERRUN | \
                         CAM_INTR_BRESP_ERR)

#define CPI_ERROR_FLAGS (CAM_INTR_INFIFO_OVERRUN |  \
                         CAM_INTR_OUTFIFO_OVERRUN | \
                         CAM_INTR_BRESP_ERR)

// Longest wait for a capture in progress to finish before the CPI is reset.
#define OMV_CSI_ABORT_TIMEOUT_MS    (200)

// The CPI pixel clock source selected by CLK_SEL.
#if defined(SOC_FEAT_PIX_CLKSEL_PLL_HZ)
#define CPI_PIX_CLKSEL_PLL_HZ       SOC_FEAT_PIX_CLKSEL_PLL_HZ
#else
#define CPI_PIX_CLKSEL_PLL_HZ       (480000000)
#endif

#if OMV_CSI_ISP_ENABLE && !defined(OMV_CSI_MIPI_LANES)
#error "The ISP is only supported with MIPI CSI-2 sensors."
#endif

#if defined(OMV_CSI_MIPI_LANES)
// VBAT power control bits for the MIPI RX D-PHY.
#define VBAT_PWR_CTRL_RX_DPHY_PWR_MASK  (1U << 4)
#define VBAT_PWR_CTRL_RX_DPHY_ISO       (1U << 5)
#define VBAT_PWR_CTRL_DPHY_VPH_1P8_VAL  (1U << 13)

// The MIPI data is RAW10, stored as RAW8 by the CPI.
#define CSI_MIPI_DATA_TYPE          (CSI_DT_RAW10)
#define CSI_MIPI_BPP                (10)

static bool alif_mipi_dphy_ready;
#endif

static uint32_t omv_csi_get_fb_offset(omv_csi_t *csi);

void CAM_IRQHandler(void) {
    uint32_t mask = 0;
    omv_csi_t *csi = omv_csi_get(-1);
    CPI_Type *cpi = csi->base;

    uint32_t status = cpi_get_interrupt_status(cpi);

    if (status & CAM_INTR_VSYNC) {
        mask |= CAM_INTR_VSYNC;
    }

    if (status & CAM_INTR_HSYNC) {
        mask |= CAM_INTR_HSYNC;
    }

    // The capture can't be stopped mid-frame (see alif_csi_abort), so mark
    // the frame invalid and drop it at STOP.
    if (status & CPI_ERROR_FLAGS) {
        mask |= (status & CPI_ERROR_FLAGS);
        vbuffer_t *buffer = framebuffer_acquire(csi->fb, FB_FLAG_FREE | FB_FLAG_PEEK);
        if (buffer != NULL) {
            buffer->flags |= VB_FLAG_INVALID;
        }
    }

    if (status & CAM_INTR_STOP) {
        mask |= CAM_INTR_STOP;
        cpi->CAM_CTRL = 0;

        // Get the buffer that was just captured.
        vbuffer_t *buffer = framebuffer_acquire(csi->fb, FB_FLAG_FREE | FB_FLAG_PEEK);
        bool frame_ok = (buffer != NULL) && !(buffer->flags & VB_FLAG_INVALID);

        if (frame_ok) {
            // Release the buffer from free queue -> used queue.
            framebuffer_release(csi->fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
        } else if (buffer != NULL) {
            // Drop the frame and reuse the buffer.
            buffer->flags &= ~VB_FLAG_INVALID;
        }

        // Acquire a buffer from the free queue.
        buffer = framebuffer_acquire(csi->fb, FB_FLAG_FREE | FB_FLAG_PEEK);

        if (buffer != NULL) {
            cpi->CAM_CTRL = 0;
            cpi->CAM_CTRL = CAM_CTRL_SW_RESET;
            cpi->CAM_FRAME_ADDR = LocalToGlobal(buffer->data + omv_csi_get_fb_offset(csi));
            cpi_irq_handler_clear_intr_status(cpi, mask);
            cpi->CAM_CTRL = (CAM_CTRL_SNAPSHOT | CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
        }

        if (csi->frame_cb.fun && frame_ok) {
            csi->frame_cb.fun(csi->frame_cb.arg);
        }
    }

    // Clear interrupts.
    cpi_irq_handler_clear_intr_status(cpi, mask);
}

static bool alif_csi_is_active(omv_csi_t *csi) {
    #if OMV_CSI_ISP_ENABLE
    // The CPI runs continuously while the ISP is streaming.
    return alif_isp_is_active(csi);
    #else
    CPI_Type *cpi = csi->base;

    return (cpi->CAM_CTRL & CAM_CTRL_BUSY);
    #endif
}

#if defined(OMV_CSI_MIPI_LANES)
// Initialize the MIPI RX D-PHY. The D-PHY waits for all lanes to enter the
// stop state, so the sensor's output is suspended while it's initialized.
static int alif_mipi_dphy_init(omv_csi_t *csi) {
    if (alif_mipi_dphy_ready) {
        return 0;
    }

    // Power up the MIPI RX D-PHY and its 1.8V supply.
    VBAT->PWR_CTRL |= VBAT_PWR_CTRL_DPHY_VPH_1P8_VAL;
    VBAT->PWR_CTRL &= ~(VBAT_PWR_CTRL_RX_DPHY_PWR_MASK | VBAT_PWR_CTRL_RX_DPHY_ISO);

    if (csi->sleep != NULL) {
        csi->sleep(csi, true);
        mp_hal_delay_ms(10);
    }

    // The D-PHY frequency is the clock lane frequency (half the bit rate).
    int32_t ret = CSI2_DPHY_Initialize(csi->mipi_brate * 1000000 / 2, OMV_CSI_MIPI_LANES);

    if (csi->sleep != NULL) {
        csi->sleep(csi, false);
    }

    if (ret != 0) {
        return -1;
    }

    alif_mipi_dphy_ready = true;
    return 0;
}

// Configure the CSI-2 host IPI for the current frame size, following the
// DFP's camera timing calculations, and set the CPI to receive from it.
// The CPI passes the RAW10 data to the ISP, if enabled, or stores it as RAW8.
static void alif_mipi_config(omv_csi_t *csi) {
    CSI_Type *csi2 = (CSI_Type *) CSI_BASE;
    CPI_Type *cpi = csi->base;
    uint32_t width = csi->resolution[csi->framesize][0];
    float frequency = csi->mipi_brate * 1000000.0f / 2.0f;

    // Make the IPI bandwidth 20% higher than the D-PHY bandwidth.
    float pixclk = (frequency * OMV_CSI_MIPI_LANES * 2 * 1.2f) / CSI_MIPI_BPP;
    float aclk = (float) GetSystemAXIClock();
    uint32_t div = (uint32_t) ((aclk / pixclk) + 0.5f);
    set_csi_pixel_clk(CSI_PIX_CLKSEL_AXI, div);

    float time_ppi_ns = (8.0f / frequency) * 1000000000.0f;
    float time_ipi_ns = (div / aclk) * 1000000000.0f;
    uint32_t bytes = (width * CSI_MIPI_BPP) / 8;

    int32_t hsa = CSI2_HSA_MIN + 1;
    int32_t hbp = ceilf(((2 * CSI2_SHORT_PKT_BYTES / OMV_CSI_MIPI_LANES + 2) *
                         time_ppi_ns / CSI2_BYTES_PER_HS_CLK) / time_ipi_ns) - hsa + 1;
    hbp = IM_MAX(hbp, CSI2_HBP_MIN);
    int32_t hsd = ceilf((((CSI2_LONG_PKT_BYTES + bytes) / OMV_CSI_MIPI_LANES) *
                         time_ppi_ns / CSI2_BYTES_PER_HS_CLK) / time_ipi_ns) -
                  (hsa + hbp + width) + 1;
    hsd = IM_MAX(hsd, CSI2_HSD_MIN);

    csi_disable_ipi_mode(csi2);
    csi_enable_software_reset_state(csi2);
    csi_set_ipi_mode(csi2, CSI_IPI_MODE_CAM_TIMIMG);
    csi_set_ipi_color_cop(csi2, CSI_IPI_COLOR_COM_TYPE_COLOR16);
    csi_enable_ipi_mem_flush_auto(csi2);
    csi_set_ipi_vc_id(csi2, 0);
    csi_set_ipi_data_type(csi2, CSI_MIPI_DATA_TYPE);
    csi_set_ipi_sync_event_type(csi2, CSI_IPI_SYNC_EVENT_FSN);
    csi_set_ipi_line_event_selection(csi2, CSI_IPI_LINE_EVENT_SELECT_PROG);
    csi_set_packet_configuration(csi2, CSI_IPI_EVENT_SELECTION_EN_VIDEO);
    csi_disable_ipi_dt_overwrite(csi2);
    csi_set_horizontal_timing(csi2, hsa, hbp, hsd, hsa + hbp + hsd);
    csi_set_vertical_timing(csi2, 0, 0, 0, 0);
    csi_disable_software_reset_state(csi2);
    csi_enable_ipi_mode(csi2);

    cpi->CAM_CFG |= (CPI_INTERFACE_MIPI_CSI << CAM_CFG_INTERFACE_Pos);
    cpi->CAM_CFG &= ~(CAM_CFG_DATA_MODE_Msk | (1U << CAM_CFG_AXI_PORT_Pos) | (1U << CAM_CFG_ISP_PORT_Pos));

    #if OMV_CSI_ISP_ENABLE
    // More than 8 bits per sample needs the 16-bit data mode.
    cpi->CAM_CFG |= (1U << CAM_CFG_ISP_PORT_Pos);
    cpi->CAM_CFG |= (CPI_DATA_MODE_BIT_16 << CAM_CFG_DATA_MODE_Pos);
    cpi->CAM_CSI_CMCFG = CPI_COLOR_MODE_CONFIG_IPI16_RAW10;
    #else
    cpi->CAM_CFG |= (1U << CAM_CFG_AXI_PORT_Pos);
    cpi->CAM_CFG |= (CPI_DATA_MODE_BIT_8 << CAM_CFG_DATA_MODE_Pos);
    cpi->CAM_CSI_CMCFG = CPI_COLOR_MODE_CONFIG_IPI16_RAW8;
    #endif
}
#endif

int alif_csi_isp_reset(omv_csi_t *csi) {
    #ifdef IMLIB_ENABLE_GAMMA_LUT
    imlib_update_gamma_table(-0.2f, 1.0f, 2.2f);
    #endif
    return 0;
}

int alif_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        CPI_Type *cpi = csi->base;

        // Configure the FIFO.
        cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_RD_WMARK_Msk;
        cpi->CAM_FIFO_CTRL = 0x08;

        cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_WR_WMARK_Msk;
        cpi->CAM_FIFO_CTRL |= (0x18 << CAM_FIFO_CTRL_WR_WMARK_Pos);

        cpi->CAM_CFG = 0;
        // Configure the capture interface (CPI, LPCPI or CSI).
        cpi->CAM_CFG |= (CPI_INTERFACE_PARALLEL << CAM_CFG_INTERFACE_Pos);
        #if SOC_FEAT_HAS_ISP
        // On parts with an ISP, the output must be routed to the AXI port.
        cpi->CAM_CFG |= (1U << CAM_CFG_AXI_PORT_Pos);
        #endif
        cpi->CAM_CFG |= (CPI_VSYNC_MODE_DISABLE << CAM_CFG_VSYNC_MODE_Pos);
        cpi->CAM_CFG |= (CPI_VSYNC_WAIT_DISABLE << CAM_CFG_VSYNC_WAIT_Pos);

        // Set VSYNC, HSYNC and PIXCLK polarities.
        cpi->CAM_CFG |= (csi->vsync_pol << CAM_CFG_VSYNC_POL_Pos);
        cpi->CAM_CFG |= (csi->hsync_pol << CAM_CFG_HSYNC_POL_Pos);
        cpi->CAM_CFG |= (!csi->pixck_pol << CAM_CFG_PXCLK_POL_Pos);

        // Configure the data bus width, mode, endianness.
        cpi->CAM_CFG |= (CPI_ROW_ROUNDUP_DISABLE << CAM_CFG_RW_ROUNDUP_Pos);
        cpi->CAM_CFG |= (CPI_DATA_MODE_BIT_8 << CAM_CFG_DATA_MODE_Pos);
        cpi->CAM_CFG |= (CPI_CODE10ON8_CODING_DISABLE << CAM_CFG_CODE10ON8_Pos);
        cpi->CAM_CFG |= (CPI_DATA_ENDIANNESS_LSB_FIRST << CAM_CFG_ENDIANNESS_Pos);

        // Configure the data mask (for 16-bits mode only).
        cpi->CAM_CFG &= ~CAM_CFG_DATA_MASK_Msk;
        cpi->CAM_CFG |= (0 << CAM_CFG_DATA_MASK_Pos);

        // Configure IPI color mode (for CSI mode only).
        cpi->CAM_CSI_CMCFG = CPI_COLOR_MODE_CONFIG_IPI48_RGB565;
    }
    return 0;
}

int alif_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    CPI_Type *cpi = csi->base;

    // Disable IRQs first so the STOP handler can't restart the capture.
    NVIC_DisableIRQ(CAM_IRQ_IRQn);
    cpi_disable_interrupt(cpi, CPI_IRQ_FLAGS);

    #if OMV_CSI_ISP_ENABLE
    // Stop the ISP, and the continuous capture after the current frame.
    if (!in_irq) {
        alif_isp_stop(csi);
        cpi->CAM_CTRL = 0;
    }
    #endif

    // The CPI always finishes the current frame, and writing CAM_CTRL while
    // it's busy is undefined. Wait for the frame to end, then reset the CPI.
    // In an IRQ, leave it to the next start or abort.
    if (!in_irq) {
        for (mp_uint_t start = mp_hal_ticks_ms(); cpi->CAM_CTRL & CAM_CTRL_BUSY; ) {
            if ((mp_hal_ticks_ms() - start) > OMV_CSI_ABORT_TIMEOUT_MS) {
                break;
            }
        }

        cpi->CAM_CTRL = 0;
        cpi->CAM_CTRL = CAM_CTRL_SW_RESET;
        cpi->CAM_CTRL = 0;
    }

    cpi_irq_handler_clear_intr_status(cpi, CPI_IRQ_FLAGS);
    return 0;
}

static uint32_t alif_clk_get_frequency(omv_clk_t *clk) {
    uint32_t div = (CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL & CAMERA_PIXCLK_CTRL_DIVISOR_Msk) >>
                   CAMERA_PIXCLK_CTRL_DIVISOR_Pos;
    if (CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL & CAMERA_PIXCLK_CTRL_CLK_SEL) {
        return CPI_PIX_CLKSEL_PLL_HZ / div;
    } else {
        return GetSystemAXIClock() / div;
    }
}

static int alif_clk_set_frequency(omv_clk_t *clk, uint32_t frequency) {
    // Configure the CPI clock source and the divider closest to 24, 12 or 6MHz.
    if (frequency >= 24000000) {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_PLL, (CPI_PIX_CLKSEL_PLL_HZ + 12000000) / 24000000);
    } else if (frequency >= 12000000) {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_PLL, (CPI_PIX_CLKSEL_PLL_HZ + 6000000) / 12000000);
    } else if (frequency >= 6000000) {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_PLL, (CPI_PIX_CLKSEL_PLL_HZ + 3000000) / 6000000);
    } else {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_AXI, (GetSystemAXIClock() + 2000000) / 4000000);
    }
    return 0;
}

static uint32_t omv_csi_get_fb_offset(omv_csi_t *csi) {
    uint32_t offset = 0;

    // With the ISP, there's no debayer buffer.
    #if !OMV_CSI_ISP_ENABLE
    uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
    uint32_t line_size_bytes = csi->resolution[csi->framesize][0] * bytes_per_pixel;

    // Offset the pixels buffer for debayering.
    if (csi->raw_output && csi->pixformat == PIXFORMAT_RGB565) {
        offset += line_size_bytes * csi->resolution[csi->framesize][1];
    }
    #endif

    return offset;
}

// This is the default snapshot function, which can be replaced in omv_csi_init functions.
int alif_csi_snapshot(omv_csi_t *csi, image_t *dst_image, uint32_t flags) {
    CPI_Type *cpi = csi->base;
    framebuffer_t *fb = csi->fb;
    vbuffer_t *buffer = NULL;

    // Configure and re/start the capture if it's not alrady active
    // and there are no pending buffers (from non-blocking capture).
    if (!alif_csi_is_active(csi) && !framebuffer_readable(fb)) {
        uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
        uint32_t line_size_bytes = csi->resolution[csi->framesize][0] * bytes_per_pixel;

        // Acquire a buffer from the free queue.
        buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);

        // Check if buffer is not ready or is not 64-bit aligned.
        if ((!buffer) || (LocalToGlobal(buffer->data) & 0x7)) {
            return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
        }

        // Clear an error left by an aborted capture.
        buffer->flags &= ~VB_FLAG_INVALID;

        // Ensure that the transfer size is compatible with DMA restrictions.
        if ((!line_size_bytes) ||
            (line_size_bytes % sizeof(uint64_t)) ||
            csi->transpose ||
            (csi->pixformat == PIXFORMAT_JPEG)) {
            return OMV_CSI_ERROR_INVALID_FRAMESIZE;
        }

        #if OMV_CSI_ISP_ENABLE
        bool cpi_active = cpi->CAM_CTRL & CAM_CTRL_BUSY;

        if (!cpi_active) {
            if (alif_mipi_dphy_init(csi) != 0) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
            alif_mipi_config(csi);
        }

        // Start the ISP before the CPI feeds it.
        if (alif_isp_start(csi) != 0) {
            return OMV_CSI_ERROR_CSI_INIT_FAILED;
        }

        if (!cpi_active) {
            // The CPI passes one sample per transfer to the ISP, and the row
            // size is in samples.
            cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_DATA_Msk;
            cpi->CAM_VIDEO_FCFG = csi->resolution[csi->framesize][0];
            cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_ROW_Msk;
            cpi->CAM_VIDEO_FCFG |= ((csi->resolution[csi->framesize][1] - 1) << CAM_VIDEO_FCFG_ROW_Pos);

            // Reset the CPI and start capturing continuously.
            cpi->CAM_CTRL = 0;
            cpi->CAM_CTRL = CAM_CTRL_SW_RESET;
            cpi->CAM_CTRL = (CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
        }
        #else
        #if defined(OMV_CSI_MIPI_LANES)
        if (csi->mipi_if) {
            if (alif_mipi_dphy_init(csi) != 0) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
            alif_mipi_config(csi);
        }
        #endif // defined(OMV_CSI_MIPI_LANES)

        if (!csi->raw_output &&
            ((csi->pixformat == PIXFORMAT_RGB565 && csi->rgb_swap) ||
             (csi->pixformat == PIXFORMAT_YUV422 && csi->yuv_swap))) {
            cpi->CAM_CFG |= (CPI_DATA_ENDIANNESS_MSB_FIRST << CAM_CFG_ENDIANNESS_Pos);
        } else {
            cpi->CAM_CFG &= ~(CPI_DATA_ENDIANNESS_MSB_FIRST << CAM_CFG_ENDIANNESS_Pos);
        }

        // Find maximum burst size that perfectly fits the line size.
        cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_RD_WMARK_Msk;

        for (uint32_t i = 16; i >= 4; i--) {
            if (!(line_size_bytes % (i * 8))) {
                cpi->CAM_FIFO_CTRL |= (i << CAM_FIFO_CTRL_RD_WMARK_Pos);
                break;
            }
        }

        cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_DATA_Msk;
        cpi->CAM_VIDEO_FCFG = line_size_bytes;
        cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_ROW_Msk;
        cpi->CAM_VIDEO_FCFG |= ((csi->resolution[csi->framesize][1] - 1) << CAM_VIDEO_FCFG_ROW_Pos);
        cpi->CAM_FRAME_ADDR = LocalToGlobal(buffer->data + omv_csi_get_fb_offset(csi));

        // Configure and enable CSI interrupts.
        cpi_irq_handler_clear_intr_status(cpi, CPI_IRQ_FLAGS);
        cpi_enable_interrupt(cpi, CPI_IRQ_FLAGS);
        NVIC_ClearPendingIRQ(CAM_IRQ_IRQn);
        NVIC_SetPriority(CAM_IRQ_IRQn, IRQ_PRI_CSI);
        NVIC_EnableIRQ(CAM_IRQ_IRQn);

        // Reset CSI and start the capture.
        cpi->CAM_CTRL = 0;
        cpi->CAM_CTRL = CAM_CTRL_SW_RESET;
        cpi->CAM_CTRL = (CAM_CTRL_SNAPSHOT | CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
        #endif
    }

    // One shot DMA transfers must be invalidated.
    framebuffer_flags_t fb_flags = FB_FLAG_USED | FB_FLAG_PEEK | FB_FLAG_INVALIDATE;

    // Wait for a frame to be ready.
    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        if ((buffer = framebuffer_acquire(fb, fb_flags))) {
            break;
        }

        if (flags & OMV_CSI_FLAG_NON_BLOCK) {
            return OMV_CSI_ERROR_WOULD_BLOCK;
        }

        if ((mp_hal_ticks_ms() - start) > OMV_CSI_TIMEOUT_MS) {
            omv_csi_abort(csi, true, false);
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
    }

    // Set the framebuffer width/height.
    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;

    // Set the framebuffer pixel format.
    switch (csi->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            fb->pixfmt = PIXFORMAT_GRAYSCALE;
            break;
        case PIXFORMAT_RGB565:
            fb->pixfmt = PIXFORMAT_RGB565;
            break;
        case PIXFORMAT_YUV422: {
            fb->pixfmt = PIXFORMAT_YUV;
            fb->subfmt_id = csi->yuv_format;
            fb->pixfmt = imlib_yuv_shift(fb->pixfmt, fb->x);
            break;
            case PIXFORMAT_BAYER:
                fb->pixfmt = PIXFORMAT_BAYER;
                fb->subfmt_id = csi->cfa_format;
                fb->pixfmt = imlib_bayer_shift(fb->pixfmt, fb->x, fb->y, csi->transpose);
                break;
        }
        default:
            break;
    }

    // Initialize a frame using the frame buffer.
    framebuffer_to_image(fb, dst_image);

    #if OMV_CSI_ISP_ENABLE
    if (csi->pixformat != PIXFORMAT_BAYER) {
        // Update the white balance gains from the sensor's RGB stats, for the next frames.
        if (csi->stats_enabled) {
            omv_csi_ioctl_arg_t arg = {0};
            int ret = omv_csi_ioctl(csi, OMV_CSI_IOCTL_GET_RGB_STATS, &arg);
            if (ret != 0) {
                return ret;
            }

            uint32_t r = arg.rgb_stats.r;
            uint32_t b = arg.rgb_stats.b;
            uint32_t g = (arg.rgb_stats.gb + arg.rgb_stats.gr) / 2;
            omv_csi_stats_update(csi, &r, &g, &b, mp_hal_ticks_ms());
        }

        float r_gain, g_gain, b_gain;
        omv_csi_stats_get_gains(csi, &r_gain, &g_gain, &b_gain);
        alif_isp_set_wb_gains(r_gain, g_gain, b_gain);
    }

    // The ISP crops the frame to the window if it can, otherwise it's cropped below.
    bool isp_cropped = alif_isp_crops_window(csi);

    // The ISP outputs YUV422 for RGB565. Convert it in place (each YUV422 pixel pair
    // is read before it's written). The ISP writes Y0 Cb Y1 Cr, which is imlib's
    // YVU422 byte order.
    if (csi->pixformat == PIXFORMAT_RGB565) {
        image_t yuv_image = {
            .w = isp_cropped ? fb->u : csi->resolution[csi->framesize][0],
            .h = isp_cropped ? fb->v : csi->resolution[csi->framesize][1],
            .pixfmt = PIXFORMAT_YVU422,
            .data = dst_image->data,
        };
        image_t rgb_image = yuv_image;
        rgb_image.pixfmt = PIXFORMAT_RGB565;
        imlib_deyuv_image(&rgb_image, &yuv_image);
    }
    #else
    bool isp_cropped = false;

    // Set the frame's pixel format to bayer for raw sensors.
    if (csi->raw_output && csi->pixformat != PIXFORMAT_BAYER) {
        dst_image->pixfmt = PIXFORMAT_BAYER;
        dst_image->subfmt_id = csi->cfa_format;
        dst_image->pixfmt = imlib_bayer_shift(dst_image->pixfmt, fb->x, fb->y, csi->transpose);
    }
    #endif

    // Crop first to reduce the frame size before debayering.
    if (omv_csi_get_cropped(csi) && !isp_cropped) {
        image_t src_cimage = *dst_image;
        image_t dst_cimage = *dst_image;

        src_cimage.w = csi->resolution[csi->framesize][0];
        src_cimage.h = csi->resolution[csi->framesize][1];

        // Offset the pixels buffer for the debayer code.
        if (csi->pixformat == PIXFORMAT_RGB565) {
            src_cimage.data += omv_csi_get_fb_offset(csi);
            dst_cimage.data += omv_csi_get_fb_offset(csi);
        }

        rectangle_t srect = { fb->x, fb->y, fb->u, fb->v };
        rectangle_t drect = { 0, 0, fb->u, fb->v };
        if (omv_gpu_draw_image(&src_cimage, &srect, &dst_cimage, &drect, 255, NULL, NULL, 0, NULL) != 0) {
            return OMV_CSI_ERROR_IO_ERROR;
        }
    }

    #if !OMV_CSI_ISP_ENABLE
    // Debayer the frame to match the target pixel format.
    if (csi->raw_output && csi->pixformat != PIXFORMAT_BAYER) {
        image_t src_image = *dst_image;

        // Offset the pixels buffer for the debayer code.
        if (csi->pixformat == PIXFORMAT_RGB565) {
            src_image.data += omv_csi_get_fb_offset(csi);
        }

        // Set the target pixel format before debayer.
        dst_image->pixfmt = fb->pixfmt;

        if (csi->stats_enabled) {
            uint32_t r, g, b;
            omv_csi_ioctl_arg_t arg = {0};
            int ret = omv_csi_ioctl(csi, OMV_CSI_IOCTL_GET_RGB_STATS, &arg);
            if (ret != 0) {
                return ret;
            }

            r = arg.rgb_stats.r;
            b = arg.rgb_stats.b;
            g = (arg.rgb_stats.gb + arg.rgb_stats.gr) / 2;
            omv_csi_stats_update(csi, &r, &g, &b, mp_hal_ticks_ms());
        }

        float r_gain, g_gain, b_gain;
        omv_csi_stats_get_gains(csi, &r_gain, &g_gain, &b_gain);

        // Debayer frame.
        imlib_debayer_image_awb(dst_image, &src_image, false, r_gain, g_gain, b_gain);
    }
    #endif
    return 0;
}

#if OMV_CSI_ISP_ENABLE
// The ISP outputs the target pixel format.
uint32_t omv_csi_get_src_bpp(omv_csi_t *csi) {
    return omv_csi_get_dst_bpp(csi);
}
#endif

int omv_csi_ops_init(omv_csi_t *csi) {
    // Set CPI base (LP/CPI).
    csi->base = OMV_CSI_BASE;

    // Set CSI ops.
    csi->abort = alif_csi_abort;
    csi->config = alif_csi_config;
    csi->snapshot = alif_csi_snapshot;
    csi->isp_reset = alif_csi_isp_reset;

    // Set CSI clock ops.
    csi->clk->freq = OMV_CSI_CLK_FREQUENCY;
    csi->clk->set_freq = alif_clk_set_frequency;
    csi->clk->get_freq = alif_clk_get_frequency;
    return 0;
}
#endif // MICROPY_PY_CSI
