/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer for nRF port.
 */
#if MICROPY_PY_SENSOR
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "fsl_csi.h"
#include "mimxrt_hal.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "sensor.h"
#include "framebuffer.h"
#include "unaligned_memcpy.h"

// Sensor struct.
sensor_t sensor = {};
extern uint8_t _line_buf[OMV_LINE_BUF_SIZE];

#define CSI_IRQ_FLAGS    (CSI_CR1_SOF_INTEN_MASK            \
                          | CSI_CR1_FB2_DMA_DONE_INTEN_MASK \
                          | CSI_CR1_FB1_DMA_DONE_INTEN_MASK)
//CSI_CR1_RF_OR_INTEN_MASK

#define copy_line(dstp, srcp)                              \
    for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) { \
        *dstp = *srcp++;                                   \
        dstp += h;                                         \
    }

#define copy_line_rev(dstp, srcp)                          \
    for (int i = MAIN_FB()->u, h = MAIN_FB()->v; i; i--) { \
        *dstp = __REV16(*srcp++);                          \
        dstp += h;                                         \
    }

void sensor_init0() {
    sensor_abort();

    // Re-init I2C to reset the bus state after soft reset, which
    // could have interrupted the bus in the middle of a transfer.
    if (sensor.i2c_bus.initialized) {
        // Reinitialize the bus using the last used id and speed.
        omv_i2c_init(&sensor.i2c_bus, sensor.i2c_bus.id, sensor.i2c_bus.speed);
    }

    sensor.disable_delays = false;

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Disable Frame callback.
    sensor_set_frame_callback(NULL);
}

int sensor_init() {
    int init_ret = 0;

    mimxrt_hal_csi_init(CSI);

    #if defined(DCMI_POWER_PIN)
    omv_gpio_write(DCMI_POWER_PIN, 1);
    #endif

    #if defined(DCMI_RESET_PIN)
    omv_gpio_write(DCMI_RESET_PIN, 1);
    #endif

    // Reset the sensor state
    memset(&sensor, 0, sizeof(sensor_t));

    // Set default snapshot function.
    // Some sensors need to call snapshot from init.
    sensor.snapshot = sensor_snapshot;

    // Configure the sensor external clock (XCLK).
    if (sensor_set_xclk_frequency(OMV_XCLK_FREQUENCY) != 0) {
        // Failed to initialize the sensor clock.
        return SENSOR_ERROR_TIM_INIT_FAILED;
    }

    // Detect and initialize the image sensor.
    if ((init_ret = sensor_probe_init(ISC_I2C_ID, ISC_I2C_SPEED)) != 0) {
        // Sensor probe/init failed.
        return init_ret;
    }

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Disable Frame callback.
    sensor_set_frame_callback(NULL);

    // All good!
    sensor.detected = true;

    return 0;
}

int sensor_dcmi_config(uint32_t pixformat) {
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
    CSI_REG_CR1(CSI) |= !sensor.hw_flags.vsync ? CSI_CR1_SOF_POL_MASK    : 0;
    CSI_REG_CR1(CSI) |= !sensor.hw_flags.hsync ? CSI_CR1_HSYNC_POL_MASK  : 0;
    CSI_REG_CR1(CSI) |= sensor.hw_flags.pixck ? CSI_CR1_REDGE_MASK      : 0;

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
    return 0;
}

int sensor_abort() {
    NVIC_DisableIRQ(CSI_IRQn);
    CSI_DisableInterrupts(CSI, CSI_IRQ_FLAGS);
    CSI_REG_CR18(CSI) &= ~(CSI_CR18_CSI_ENABLE_MASK | CSI_CR3_DMA_REQ_EN_RFF_MASK);
    framebuffer_reset_buffers();
    return 0;
}

int sensor_set_xclk_frequency(uint32_t frequency) {
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

uint32_t sensor_get_xclk_frequency() {
    return 24000000 / (CLOCK_GetDiv(kCLOCK_CsiDiv) + 1);
}

void sensor_sof_callback() {
    // Get current framebuffer.
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
    if (buffer == NULL) {
        sensor_abort();
    }
    if (buffer->offset < MAIN_FB()->v) {
        // Missed a few lines, reset buffer state and continue.
        buffer->reset_state = true;
    }
}

void sensor_line_callback(uint32_t addr) {
    // Get current framebuffer.
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);

    // Copy from DMA buffer to framebuffer.
    uint32_t bytes_per_pixel = sensor_get_src_bpp();
    uint8_t *src = ((uint8_t *) addr) + (MAIN_FB()->x * bytes_per_pixel);
    uint8_t *dst = buffer->data;

    // Adjust BPP for Grayscale.
    if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
        bytes_per_pixel = 1;
    }

    if (sensor.transpose) {
        dst += bytes_per_pixel * buffer->offset;
    } else {
        dst += MAIN_FB()->u * bytes_per_pixel * buffer->offset;
    }

    // Implement per line, per pixel cropping, and image transposing (for image rotation) in
    // in software using the CPU to transfer the image from the line buffers to the frame buffer.
    uint16_t *src16 = (uint16_t *) src;
    uint16_t *dst16 = (uint16_t *) dst;

    switch (sensor.pixformat) {
        case PIXFORMAT_BAYER:
            #if (OMV_ENABLE_SENSOR_EDMA == 1)
            edma_memcpy(buffer, dst, src, sizeof(uint8_t), sensor.transpose);
            #else
            if (!sensor.transpose) {
                unaligned_memcpy(dst, src, MAIN_FB()->u);
            } else {
                copy_line(dst, src);
            }
            #endif
            break;
        case PIXFORMAT_GRAYSCALE:
            #if (OMV_ENABLE_SENSOR_EDMA == 1)
            edma_memcpy(buffer, dst, src, sizeof(uint8_t), sensor.transpose);
            #else
            if (sensor.hw_flags.gs_bpp == 1) {
                // 1BPP GRAYSCALE.
                if (!sensor.transpose) {
                    unaligned_memcpy(dst, src, MAIN_FB()->u);
                } else {
                    copy_line(dst, src);
                }
            } else {
                // Extract Y channel from YUV.
                if (!sensor.transpose) {
                    unaligned_2_to_1_memcpy(dst, src16, MAIN_FB()->u);
                } else {
                    copy_line(dst, src16);
                }
            }
            #endif
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            #if (OMV_ENABLE_SENSOR_EDMA == 1)
            edma_memcpy(buffer, dst16, src16, sizeof(uint16_t), sensor.transpose);
            #else
            if ((sensor.pixformat == PIXFORMAT_RGB565 && sensor.hw_flags.rgb_swap)
                || (sensor.pixformat == PIXFORMAT_YUV422 && sensor.hw_flags.yuv_swap)) {
                if (!sensor.transpose) {
                    unaligned_memcpy_rev16(dst16, src16, MAIN_FB()->u);
                } else {
                    copy_line_rev(dst16, src16);
                }
            } else {
                if (!sensor.transpose) {
                    unaligned_memcpy(dst16, src16, MAIN_FB()->u * sizeof(uint16_t));
                } else {
                    copy_line(dst16, src16);
                }
            }
            #endif
            break;
        default:
            break;
    }

    if (++buffer->offset == MAIN_FB()->v) {
        // Release the current framebuffer.
        framebuffer_get_tail(FB_NO_FLAGS);
        CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
    }
}

// This is the default snapshot function, which can be replaced in sensor_init functions.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags) {
    // Used to restore MAIN_FB's width and height.
    uint32_t w = MAIN_FB()->u;
    uint32_t h = MAIN_FB()->v;

    if (sensor->pixformat == PIXFORMAT_INVALID) {
        return SENSOR_ERROR_INVALID_PIXFORMAT;
    }

    if (sensor->framesize == FRAMESIZE_INVALID) {
        return SENSOR_ERROR_INVALID_FRAMESIZE;
    }

    if (sensor_check_framebuffer_size() != 0) {
        return SENSOR_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    // Compress the framebuffer for the IDE preview.
    framebuffer_update_jpeg_buffer();

    // Free the current FB head.
    framebuffer_free_current_buffer();

    // If the DMA is Not currently transferring a new buffer,
    // reconfigure and restart the CSI transfer.
    if (!(CSI->CR18 & CSI_CR18_CSI_ENABLE_MASK)) {
        framebuffer_setup_buffers();

        vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer == NULL) {
            return SENSOR_ERROR_FRAMEBUFFER_ERROR;
        }
        // Re/configure and re/start the CSI.
        uint32_t bytes_per_pixel = sensor_get_src_bpp();
        uint32_t dma_line_bytes = resolution[sensor->framesize][0] * bytes_per_pixel;
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
    #if defined(DCMI_FSYNC_PIN)
    if (sensor->hw_flags.fsync) {
        omv_gpio_write(DCMI_FSYNC_PIN, 1);
    }
    #endif

    vbuffer_t *buffer = framebuffer_get_head(FB_NO_FLAGS);
    // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        MICROPY_EVENT_POLL_HOOK
        if ((mp_hal_ticks_ms() - ticks) > 3000) {
            sensor_abort();

            #if defined(DCMI_FSYNC_PIN)
            if (sensor->hw_flags.fsync) {
                omv_gpio_write(DCMI_FSYNC_PIN, 0);
            }
            #endif

            return SENSOR_ERROR_CAPTURE_TIMEOUT;
        }
        buffer = framebuffer_get_head(FB_NO_FLAGS);
    }

    // We're done receiving data.
    #if defined(DCMI_FSYNC_PIN)
    if (sensor->hw_flags.fsync) {
        omv_gpio_write(DCMI_FSYNC_PIN, 0);
    }
    #endif

    if (!sensor->transpose) {
        MAIN_FB()->w = w;
        MAIN_FB()->h = h;
    } else {
        MAIN_FB()->w = h;
        MAIN_FB()->h = w;
    }

    // Fix the BPP.
    switch (sensor->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->pixfmt = PIXFORMAT_GRAYSCALE;
            break;
        case PIXFORMAT_RGB565:
            MAIN_FB()->pixfmt = PIXFORMAT_RGB565;
            break;
        case PIXFORMAT_BAYER:
            MAIN_FB()->pixfmt = PIXFORMAT_BAYER;
            MAIN_FB()->subfmt_id = sensor->hw_flags.bayer;
            break;
        case PIXFORMAT_YUV422: {
            bool yuv_order = sensor->hw_flags.yuv_order == SENSOR_HW_FLAGS_YUV422;
            int even = yuv_order ? PIXFORMAT_YUV422 : PIXFORMAT_YVU422;
            int odd = yuv_order ? PIXFORMAT_YVU422 : PIXFORMAT_YUV422;
            MAIN_FB()->pixfmt = (MAIN_FB()->x % 2) ? odd : even;
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
