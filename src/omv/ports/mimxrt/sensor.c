/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor driver for mimxrt port.
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

void sensor_init0() {
    sensor_abort(true, false);

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

int sensor_abort(bool fifo_flush, bool in_irq) {
    NVIC_DisableIRQ(CSI_IRQn);
    CSI_DisableInterrupts(CSI, CSI_IRQ_FLAGS);
    CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
    CSI_REG_CR18(CSI) &= ~CSI_CR18_CSI_ENABLE_MASK;
    sensor.first_line = false;
    sensor.drop_frame = false;
    sensor.last_frame_ms = 0;
    sensor.last_frame_ms_valid = false;
    if (fifo_flush) {
        framebuffer_flush_buffers(true);
    } else if (!sensor.disable_full_flush) {
        framebuffer_flush_buffers(false);
    }
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
    sensor.first_line = false;
    sensor.drop_frame = false;
    // Get current framebuffer.
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
    if (buffer == NULL) {
        sensor_abort(false, true);
    } else if (buffer->offset < resolution[sensor.framesize][1]) {
        // Missed a few lines, reset buffer state and continue.
        buffer->reset_state = true;
    }
}

void sensor_line_callback(uint32_t addr) {
    // Throttle frames to match the current frame rate.
    sensor_throttle_framerate();

    // Get current framebuffer.
    vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        if (sensor.drop_frame) {
            return;
        }
        bool jpeg_end = false;
        if (sensor.hw_flags.jpeg_mode == 4) {
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
        } else if (sensor.hw_flags.jpeg_mode == 3) {
            // OV2640 JPEG TODO
        }
        // In JPEG mode the camera sensor will output some number of lines that doesn't match the
        // the current framesize. Since we don't have an end-of-frame interrupt on the mimxrt we
        // detect the end of the frame when there's no more jpeg data.
        if (jpeg_end) {
            // Release the current framebuffer.
            framebuffer_get_tail(FB_NO_FLAGS);
            CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
            if (sensor.frame_callback) {
                sensor.frame_callback();
            }
            sensor.drop_frame = true;
        }
        return;
    }

    if (sensor.drop_frame) {
        if (++buffer->offset == resolution[sensor.framesize][1]) {
            buffer->offset = 0;
            CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
        }
        return;
    }

    if ((MAIN_FB()->y <= buffer->offset) && (buffer->offset < (MAIN_FB()->y + MAIN_FB()->v))) {
        // Copy from DMA buffer to framebuffer.
        uint32_t bytes_per_pixel = sensor_get_src_bpp();
        uint8_t *src = ((uint8_t *) addr) + (MAIN_FB()->x * bytes_per_pixel);
        uint8_t *dst = buffer->data;

        // Adjust BPP for Grayscale.
        if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
            bytes_per_pixel = 1;
        }

        if (sensor.transpose) {
            dst += bytes_per_pixel * (buffer->offset - MAIN_FB()->y);
        } else {
            dst += MAIN_FB()->u * bytes_per_pixel * (buffer->offset - MAIN_FB()->y);
        }

        // Copy line to frame buffer.
        sensor_copy_line(NULL, src, dst);
    }

    if (++buffer->offset == resolution[sensor.framesize][1]) {
        // Release the current framebuffer.
        framebuffer_get_tail(FB_NO_FLAGS);
        CSI_REG_CR3(CSI) &= ~CSI_CR3_DMA_REQ_EN_RFF_MASK;
        if (sensor.frame_callback) {
            sensor.frame_callback();
        }
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
            sensor_abort(true, false);

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

    // The JPEG in the frame buffer is actually invalid.
    if (buffer->jpeg_buffer_overflow) {
        return SENSOR_ERROR_JPEG_OVERFLOW;
    }

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
            MAIN_FB()->pixfmt = imlib_bayer_shift(MAIN_FB()->pixfmt, MAIN_FB()->x, MAIN_FB()->y, sensor->transpose);
            break;
        case PIXFORMAT_YUV422: {
            bool yuv_order = sensor->hw_flags.yuv_order == SENSOR_HW_FLAGS_YUV422;
            int even = yuv_order ? PIXFORMAT_YUV422 : PIXFORMAT_YVU422;
            int odd = yuv_order ? PIXFORMAT_YVU422 : PIXFORMAT_YUV422;
            MAIN_FB()->pixfmt = (MAIN_FB()->x % 2) ? odd : even;
            break;
        }
        case PIXFORMAT_JPEG: {
            int32_t size = 0;
            if (sensor->chip_id == OV5640_ID) {
                // Offset contains the sum of all the bytes transferred from the offset buffers
                // while in sensor_line_callback().
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
