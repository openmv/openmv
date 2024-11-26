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
 * MIMXRT CSI driver.
 */
#if MICROPY_PY_CSI
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "fsl_csi.h"
#include "mimxrt_hal.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "framebuffer.h"
#include "unaligned_memcpy.h"

#define DMA_LENGTH_ALIGNMENT    (8)
#define MIN_EDMA_DST_INC        (4)

// Sensor struct.
omv_csi_t csi = {};
extern uint8_t _line_buf[OMV_LINE_BUF_SIZE];

#define CSI_IRQ_FLAGS    (CSI_CR1_SOF_INTEN_MASK            \
                          | CSI_CR1_FB2_DMA_DONE_INTEN_MASK \
                          | CSI_CR1_FB1_DMA_DONE_INTEN_MASK)

void omv_csi_init0() {
    omv_csi_abort(true, false);

    // Re-init I2C to reset the bus state after soft reset, which
    // could have interrupted the bus in the middle of a transfer.
    if (csi.i2c_bus.initialized) {
        // Reinitialize the bus using the last used id and speed.
        omv_i2c_init(&csi.i2c_bus, csi.i2c_bus.id, csi.i2c_bus.speed);
    }

    csi.disable_delays = false;

    // Disable VSYNC IRQ and callback
    omv_csi_set_vsync_callback(NULL);

    // Disable Frame callback.
    omv_csi_set_frame_callback(NULL);
}

int omv_csi_init() {
    int init_ret = 0;

    mimxrt_hal_csi_init(CSI);

    #if defined(OMV_CSI_POWER_PIN)
    omv_gpio_write(OMV_CSI_POWER_PIN, 1);
    #endif

    #if defined(OMV_CSI_RESET_PIN)
    omv_gpio_write(OMV_CSI_RESET_PIN, 1);
    #endif

    // Reset the csi state
    memset(&csi, 0, sizeof(omv_csi_t));

    // Set default snapshot function.
    // Some sensors need to call snapshot from init.
    csi.snapshot = omv_csi_snapshot;

    // Configure the csi external clock (XCLK).
    if (omv_csi_set_clk_frequency(OMV_CSI_CLK_FREQUENCY) != 0) {
        // Failed to initialize the csi clock.
        return OMV_CSI_ERROR_TIM_INIT_FAILED;
    }

    // Detect and initialize the image sensor.
    if ((init_ret = omv_csi_probe_init(OMV_CSI_I2C_ID, OMV_CSI_I2C_SPEED)) != 0) {
        // Sensor probe/init failed.
        return init_ret;
    }

    // Configure the CSI interface.
    if (omv_csi_config(OMV_CSI_CONFIG_INIT) != 0) {
        // CSI config failed
        return OMV_CSI_ERROR_CSI_INIT_FAILED;
    }

    // Set default color palette.
    csi.color_palette = rainbow_table;

    // Disable VSYNC IRQ and callback
    omv_csi_set_vsync_callback(NULL);

    // Disable Frame callback.
    omv_csi_set_frame_callback(NULL);

    // All good!
    csi.detected = true;

    return 0;
}

int omv_csi_config(omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        CSI_Reset(CSI);
        NVIC_DisableIRQ(CSI_IRQn);

        // CSI_Reset does not zero CR1.
        CSI_REG_CR1(CSI) = 0;
        // CSI mode: HSYNC, VSYNC, and PIXCLK signals are used.
        CSI_REG_CR1(CSI) |= CSI_CR1_GCLK_MODE(1U);
        // Synchronous FIFO clear.
        // RXFIFO and STATFIFO are cleared on every SOF.
        CSI_REG_CR1(CSI) |= CSI_CR1_FCC_MASK;

        // Configure VSYNC, HSYNC and PIXCLK signals.
        CSI_REG_CR1(CSI) |= CSI_CR1_EXT_VSYNC_MASK;
        CSI_REG_CR1(CSI) |= !csi.vsync_pol ? CSI_CR1_SOF_POL_MASK    : 0;
        CSI_REG_CR1(CSI) |= !csi.hsync_pol ? CSI_CR1_HSYNC_POL_MASK  : 0;
        CSI_REG_CR1(CSI) |= csi.pixck_pol ? CSI_CR1_REDGE_MASK      : 0;

        // Stride config: No stride.
        CSI_REG_FBUF_PARA(CSI) = 0;
        // Reset frame counter
        CSI_REG_CR3(CSI) |= CSI_CR3_FRMCNT_RST_MASK;

        // Configure CSI FIFO depth and DMA burst size.
        CSI_REG_CR2(CSI) |= CSI_CR2_DMA_BURST_TYPE_RFF(3U);
        CSI_REG_CR3(CSI) |= 7U << CSI_CR3_RxFF_LEVEL_SHIFT;

        // Configure DMA buffers.
        CSI_REG_DMASA_FB1(CSI) = (uint32_t) (&_line_buf[OMV_LINE_BUF_SIZE * 0]);
        CSI_REG_DMASA_FB2(CSI) = (uint32_t) (&_line_buf[OMV_LINE_BUF_SIZE / 2]);

        // Write to memory from first completed frame.
        // DMA CSI addr switch at dma transfer done.
        CSI_REG_CR18(CSI) |= CSI_CR18_MASK_OPTION(0);
    }
    return 0;
}

int omv_csi_abort(bool fifo_flush, bool in_irq) {
    NVIC_DisableIRQ(CSI_IRQn);
    CSI_DisableInterrupts(CSI, CSI_IRQ_FLAGS);
    CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
    CSI_REG_CR18(CSI) &= ~CSI_CR18_CSI_ENABLE_MASK;
    csi.dest_inc = 0;
    csi.first_line = false;
    csi.drop_frame = false;
    csi.last_frame_ms = 0;
    csi.last_frame_ms_valid = false;
    if (fifo_flush) {
        framebuffer_flush_buffers(true);
    } else if (!csi.disable_full_flush) {
        framebuffer_flush_buffers(false);
    }
    return 0;
}

int omv_csi_set_clk_frequency(uint32_t frequency) {
    if (frequency >= 24000000) {
        CLOCK_SetDiv(kCLOCK_CsiDiv, 0);
    } else if (frequency >= 12000000) {
        CLOCK_SetDiv(kCLOCK_CsiDiv, 1);
    } else if (frequency >= 8000000) {
        CLOCK_SetDiv(kCLOCK_CsiDiv, 2);
    } else if (frequency >= 6000000) {
        CLOCK_SetDiv(kCLOCK_CsiDiv, 3);
    } else if (frequency >= 4000000) {
        CLOCK_SetDiv(kCLOCK_CsiDiv, 5);
    } else {
        CLOCK_SetDiv(kCLOCK_CsiDiv, 7);
    }
    return 0;
}

uint32_t omv_csi_get_xclk_frequency() {
    return 24000000 / (CLOCK_GetDiv(kCLOCK_CsiDiv) + 1);
}

void omv_csi_sof_callback() {
    csi.first_line = false;
    csi.drop_frame = false;
    // Get current framebuffer.
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
    if (buffer == NULL) {
        omv_csi_abort(false, true);
    } else if (buffer->offset < resolution[csi.framesize][1]) {
        // Missed a few lines, reset buffer state and continue.
        buffer->reset_state = true;
    }
}

#if defined(OMV_CSI_DMA)
int omv_csi_dma_memcpy(void *dma, void *dst, void *src, int bpp, bool transposed) {
    // EMDA will not perform burst transfers for anything less than 32-byte chunks of four 64-bit
    // beats. Additionally, the CSI hardware lacks cropping so we cannot align the source address.
    // Given this, performance will be lacking on cropped images. So much so that we do not use
    // the EDMA for anything less than 4-byte transfers otherwise you get sensor timeout errors.
    if (csi.dest_inc < MIN_EDMA_DST_INC) {
        return -1;
    }

    edma_handle_t *handle = dma;
    edma_transfer_config_t config;
    EDMA_PrepareTransferConfig(&config,
                               src, // srcAddr
                               csi.src_size, // srcWidth
                               csi.src_inc, // srcOffset
                               dst, // destAddr
                               transposed ? bpp : csi.dest_inc, // destWidth
                               transposed ? (MAIN_FB()->v * bpp) : csi.dest_inc, // destOffset
                               MAIN_FB()->u * bpp, // bytesEachRequest
                               MAIN_FB()->u * bpp); // transferBytes

    size_t retry = 3;
    status_t status = kStatus_EDMA_Busy;
    while (status == kStatus_EDMA_Busy) {
        status = EDMA_SubmitTransfer(handle, &config);
        if (status == kStatus_Success) {
            break;
        }
        if (--retry == 0) {
            // Drop the frame if EDMA is not keeping up as the image will be corrupt.
            csi.drop_frame = true;
            return 0;
        }
    }

    EDMA_TriggerChannelStart(handle->base, handle->channel);
    return 0;
}
#endif

void omv_csi_line_callback(uint32_t addr) {
    // Throttle frames to match the current frame rate.
    omv_csi_throttle_framerate();

    // Get current framebuffer.
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);

    if (csi.pixformat == PIXFORMAT_JPEG) {
        if (csi.drop_frame) {
            return;
        }
        bool jpeg_end = false;
        if (csi.jpg_format == 4) {
            // JPEG MODE 4:
            //
            // The width and height are fixed in each frame. The first two bytes are valid data
            // length in every line, followed by valid image data. Dummy data (0xFF) may be used as
            // padding at each line end if the current valid image data is less than the line width.
            //
            // In this mode `offset` holds the size of all jpeg data transferred.
            //
            // Note: We are using this mode for the OV5640 because it allows us to use the line
            // buffers to fifo the JPEG image data input so we can handle SDRAM refresh hiccups
            // that will cause data loss if we make the DMA hardware write directly to the FB.
            //
            uint16_t size = __REV16(*((uint16_t *) addr));
            // Prevent a buffer overflow when writing the jpeg data.
            if (buffer->offset + size > framebuffer_get_buffer_size()) {
                buffer->jpeg_buffer_overflow = true;
                jpeg_end = true;
            } else {
                unaligned_memcpy(buffer->data + buffer->offset, ((uint16_t *) addr) + 1, size);
                for (int i = 0; i < size; i++) {
                    int e = buffer->offset + i;
                    int s = IM_MAX(e - 1, 0);
                    if ((buffer->data[s] == 0xFF) && (buffer->data[e] == 0xD9)) {
                        jpeg_end = true;
                        break;
                    }
                }
                buffer->offset += size;
            }
        } else if (csi.jpg_format == 3) {
            // OV2640 JPEG TODO
        }
        // In JPEG mode the camera sensor will output some number of lines that doesn't match the
        // the current framesize. Since we don't have an end-of-frame interrupt on the mimxrt we
        // detect the end of the frame when there's no more jpeg data.
        if (jpeg_end) {
            // Release the current framebuffer.
            framebuffer_get_tail(FB_NO_FLAGS);
            CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
            if (csi.frame_callback) {
                csi.frame_callback();
            }
            csi.drop_frame = true;
        }
        return;
    }

    if (csi.drop_frame) {
        if (++buffer->offset == resolution[csi.framesize][1]) {
            buffer->offset = 0;
            CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
        }
        return;
    }

    if ((MAIN_FB()->y <= buffer->offset) && (buffer->offset < (MAIN_FB()->y + MAIN_FB()->v))) {
        // Copy from DMA buffer to framebuffer.
        uint32_t bytes_per_pixel = omv_csi_get_src_bpp();
        uint8_t *src = ((uint8_t *) addr) + (MAIN_FB()->x * bytes_per_pixel);
        uint8_t *dst = buffer->data;

        // Adjust BPP for Grayscale.
        if (csi.pixformat == PIXFORMAT_GRAYSCALE) {
            bytes_per_pixel = 1;
        }

        if (csi.transpose) {
            dst += bytes_per_pixel * (buffer->offset - MAIN_FB()->y);
        } else {
            dst += MAIN_FB()->u * bytes_per_pixel * (buffer->offset - MAIN_FB()->y);
        }

        #if defined(OMV_CSI_DMA)
        // We're using multiple handles to give each channel the maximum amount of time possible to do the line
        // transfer. In most situations only one channel will be running at a time. However, if SDRAM is
        // backedup we don't have to disable the channel if it is flushing trailing data to SDRAM.
        omv_csi_copy_line(&csi.dma_channels[buffer->offset % OMV_CSI_DMA_CHANNEL_COUNT], src, dst);
        #else
        omv_csi_copy_line(NULL, src, dst);
        #endif
    }

    if (++buffer->offset == resolution[csi.framesize][1]) {
        // Release the current framebuffer.
        framebuffer_get_tail(FB_NO_FLAGS);
        CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
        if (csi.frame_callback) {
            csi.frame_callback();
        }
    }
}

#if defined(OMV_CSI_DMA)
static void edma_config(omv_csi_t *csi, uint32_t bytes_per_pixel) {
    uint32_t line_offset_bytes = MAIN_FB()->x * bytes_per_pixel;
    uint32_t line_width_bytes = MAIN_FB()->u * bytes_per_pixel;

    // YUV422 Source -> Y Destination
    if ((csi->pixformat == PIXFORMAT_GRAYSCALE) && (csi->mono_bpp == 2)) {
        line_width_bytes /= 2;
    }

    // Destination will be 32-byte aligned. So, we just need to breakup the line width into the largest
    // power of 2. Source may have an offset which further limits this to a sub power of 2.
    for (int i = 5; i >= 0; i--) {
        // 16-byte burst is not supported.
        if ((i != 4) && (!(line_width_bytes % (1 << i)))) {
            for (int j = i; j >= 0; j--) {
                // 16-byte burst is not supported.
                if ((j != 4) && (!(line_offset_bytes % (1 << j)))) {
                    csi->src_inc = csi->src_size = 1 << j;
                    break;
                }
            }

            csi->dest_inc = 1 << i;
            break;
        }
    }

    if (csi->transpose) {
        csi->dest_inc = bytes_per_pixel;
    }

    // YUV422 Source -> Y Destination
    if ((csi->pixformat == PIXFORMAT_GRAYSCALE) && (csi->mono_bpp == 2)) {
        csi->src_inc = 2;
        csi->src_size = 1;
    }
}
#endif

int omv_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    // Used to restore MAIN_FB's width and height.
    uint32_t w = MAIN_FB()->u;
    uint32_t h = MAIN_FB()->v;

    if (csi->pixformat == PIXFORMAT_INVALID) {
        return OMV_CSI_ERROR_INVALID_PIXFORMAT;
    }

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    if (omv_csi_check_framebuffer_size() != 0) {
        return OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    // Compress the framebuffer for the IDE preview.
    framebuffer_update_jpeg_buffer();

    // Free the current FB head.
    framebuffer_free_current_buffer();

    // If the DMA is not active, reconfigure and restart the CSI transfer.
    if (!(CSI->CR18 & CSI_CR18_CSI_ENABLE_MASK)) {
        framebuffer_setup_buffers();

        uint32_t bytes_per_pixel = omv_csi_get_src_bpp();
        uint32_t dma_line_bytes = resolution[csi->framesize][0] * bytes_per_pixel;
        uint32_t length = dma_line_bytes * h;

        // Error out if the transfer size is not compatible with DMA transfer restrictions.
        if ((!dma_line_bytes)
            || (dma_line_bytes % sizeof(uint64_t))
            || (dma_line_bytes > (OMV_LINE_BUF_SIZE / 2))
            || (!length)
            || (length % DMA_LENGTH_ALIGNMENT)) {
            return OMV_CSI_ERROR_INVALID_FRAMESIZE;
        }

        #if defined(OMV_CSI_DMA)
        // The code below will enable EDMA data transfer from the line buffer for non-JPEG modes.
        if (csi->pixformat != PIXFORMAT_JPEG) {
            edma_config(csi, bytes_per_pixel);
            for (int i = 0; i < OMV_CSI_DMA_CHANNEL_COUNT; i++) {
                EDMA_CreateHandle(&csi->dma_channels[i], OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL_START + i);
                EDMA_DisableChannelInterrupts(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL_START + i, kEDMA_MajorInterruptEnable);
            }
        }
        #endif

        if ((csi->pixformat == PIXFORMAT_RGB565 && csi->rgb_swap) ||
            (csi->pixformat == PIXFORMAT_YUV422 && csi->yuv_swap)) {
            CSI_REG_CR1(CSI) |= CSI_CR1_SWAP16_EN_MASK | CSI_CR1_PACK_DIR_MASK;
        } else {
            CSI_REG_CR1(CSI) &= ~(CSI_CR1_SWAP16_EN_MASK | CSI_CR1_PACK_DIR_MASK);
        }

        CSI_REG_IMAG_PARA(CSI) =
            (dma_line_bytes << CSI_IMAG_PARA_IMAGE_WIDTH_SHIFT) |
            (1 << CSI_IMAG_PARA_IMAGE_HEIGHT_SHIFT);

        // Configure and enable CSI interrupts.
        CSI_EnableInterrupts(CSI, CSI_IRQ_FLAGS);
        NVIC_EnableIRQ(CSI_IRQn);

        // Enable CSI
        CSI_REG_CR18(CSI) |= CSI_CR18_CSI_ENABLE_MASK;
    }

    // Let the camera know we want to trigger it now.
    #if defined(OMV_CSI_FSYNC_PIN)
    if (csi->frame_sync) {
        omv_gpio_write(OMV_CSI_FSYNC_PIN, 1);
    }
    #endif

    framebuffer_flags_t fb_flags = FB_NO_FLAGS;

    #if defined(OMV_CSI_DMA)
    // dest_inc will be less than MIN_EDMA_DST_INC if the EDMA is not initialized or unusable.
    if (csi->dest_inc >= MIN_EDMA_DST_INC) {
        fb_flags = FB_INVALIDATE;
    }
    #endif

    vbuffer_t *buffer = framebuffer_get_head(fb_flags);
    // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        MICROPY_EVENT_POLL_HOOK
        if ((mp_hal_ticks_ms() - ticks) > OMV_CSI_TIMEOUT_MS) {
            omv_csi_abort(true, false);

            #if defined(OMV_CSI_FSYNC_PIN)
            if (csi->frame_sync) {
                omv_gpio_write(OMV_CSI_FSYNC_PIN, 0);
            }
            #endif

            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
        buffer = framebuffer_get_head(fb_flags);
    }

    // We're done receiving data.
    #if defined(OMV_CSI_FSYNC_PIN)
    if (csi->frame_sync) {
        omv_gpio_write(OMV_CSI_FSYNC_PIN, 0);
    }
    #endif

    // The JPEG in the frame buffer is actually invalid.
    if (buffer->jpeg_buffer_overflow) {
        return OMV_CSI_ERROR_JPEG_OVERFLOW;
    }

    if (!csi->transpose) {
        MAIN_FB()->w = w;
        MAIN_FB()->h = h;
    } else {
        MAIN_FB()->w = h;
        MAIN_FB()->h = w;
    }

    // Fix the BPP.
    switch (csi->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->pixfmt = PIXFORMAT_GRAYSCALE;
            break;
        case PIXFORMAT_RGB565:
            MAIN_FB()->pixfmt = PIXFORMAT_RGB565;
            break;
        case PIXFORMAT_BAYER:
            MAIN_FB()->pixfmt = PIXFORMAT_BAYER;
            MAIN_FB()->subfmt_id = csi->cfa_format;
            MAIN_FB()->pixfmt = imlib_bayer_shift(MAIN_FB()->pixfmt, MAIN_FB()->x, MAIN_FB()->y, csi->transpose);
            break;
        case PIXFORMAT_YUV422: {
            MAIN_FB()->pixfmt = PIXFORMAT_YUV;
            MAIN_FB()->subfmt_id = csi->yuv_format;
            MAIN_FB()->pixfmt = imlib_yuv_shift(MAIN_FB()->pixfmt, MAIN_FB()->x);
            break;
        }
        case PIXFORMAT_JPEG: {
            int32_t size = 0;
            if (csi->chip_id == OV5640_ID) {
                // Offset contains the sum of all the bytes transferred from the offset buffers
                // while in omv_csi_line_callback().
                size = buffer->offset;
            } else {
                // OV2640 JPEG TODO
            }
            // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
            MAIN_FB()->pixfmt = PIXFORMAT_JPEG;
            MAIN_FB()->size = jpeg_clean_trailing_bytes(size, buffer->data);
            break;
        }
        default:
            break;
    }

    // Set the user image.
    framebuffer_init_image(image);
    return 0;
}
#endif
