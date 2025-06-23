/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2020-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Sensor driver for nRF port.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "nrf_i2s.h"
#include "hal/nrf_gpio.h"

static uint32_t _vsyncMask;
static uint32_t _hrefMask;
static uint32_t _pclkMask;
static const volatile uint32_t *_vsyncPort;
static const volatile uint32_t *_hrefPort;
static const volatile uint32_t *_pclkPort;

#define interrupts()              __enable_irq()
#define noInterrupts()            __disable_irq()

#define digitalPinToPort(P)       (P / 32)

#ifndef digitalPinToBitMask
#define digitalPinToBitMask(P)    (1 << (P % 32))
#endif

#ifndef portInputRegister
#define portInputRegister(P)      ((P == 0) ? &NRF_P0->IN : &NRF_P1->IN)
#endif

#ifndef I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2
// Note this define is out of spec and has been removed from hal.
#define I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2    (0x80000000UL)  /*!< 32 MHz / 2 = 16.0 MHz */
#endif

extern void __fatal_error(const char *msg);

int omv_csi_config(omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        uint32_t csi_pins[] = {
            OMV_CSI_D0_PIN,
            OMV_CSI_D1_PIN,
            OMV_CSI_D2_PIN,
            OMV_CSI_D3_PIN,
            OMV_CSI_D4_PIN,
            OMV_CSI_D5_PIN,
            OMV_CSI_D6_PIN,
            OMV_CSI_D7_PIN,
            OMV_CSI_VSYNC_PIN,
            OMV_CSI_HSYNC_PIN,
            OMV_CSI_PXCLK_PIN,
        };

        // Configure CSI input pins
        for (int i = 0; i < sizeof(csi_pins) / sizeof(csi_pins[0]); i++) {
            nrf_gpio_cfg_input(csi_pins[i], NRF_GPIO_PIN_PULLUP);
        }

        _vsyncMask = digitalPinToBitMask(OMV_CSI_VSYNC_PIN);
        _hrefMask = digitalPinToBitMask(OMV_CSI_HSYNC_PIN);
        _pclkMask = digitalPinToBitMask(OMV_CSI_PXCLK_PIN);

        _vsyncPort = portInputRegister(digitalPinToPort(OMV_CSI_VSYNC_PIN));
        _hrefPort = portInputRegister(digitalPinToPort(OMV_CSI_HSYNC_PIN));
        _pclkPort = portInputRegister(digitalPinToPort(OMV_CSI_PXCLK_PIN));
    }

    return 0;
}

uint32_t omv_csi_get_clk_frequency() {
    return OMV_CSI_CLK_FREQUENCY;
}

int omv_csi_set_clk_frequency(uint32_t frequency) {
    nrf_gpio_cfg_output(OMV_CSI_MXCLK_PIN);

    // Generates 16 MHz signal using I2S peripheral
    NRF_I2S->CONFIG.MCKEN = (I2S_CONFIG_MCKEN_MCKEN_ENABLE << I2S_CONFIG_MCKEN_MCKEN_Pos);
    NRF_I2S->CONFIG.MCKFREQ = I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2 << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
    NRF_I2S->CONFIG.MODE = I2S_CONFIG_MODE_MODE_MASTER << I2S_CONFIG_MODE_MODE_Pos;

    NRF_I2S->PSEL.MCK = (OMV_CSI_MXCLK_PIN << I2S_PSEL_MCK_PIN_Pos);

    NRF_I2S->ENABLE = 1;
    NRF_I2S->TASKS_START = 1;

    return 0;
}

int omv_csi_set_windowing(int x, int y, int w, int h) {
    return OMV_CSI_ERROR_CTL_UNSUPPORTED;
}

// This is the default snapshot function, which can be replaced in omv_csi_init functions.
int omv_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    framebuffer_t *fb = csi->fb;

    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    framebuffer_update_jpeg_buffer(fb);

    // This driver supports a single buffer.
    if (fb->n_buffers != 1) {
        framebuffer_set_buffers(fb, 1);
    }

    if (omv_csi_check_framebuffer_size(fb) != 0) {
        return OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    framebuffer_free_current_buffer(fb);
    framebuffer_setup_buffers(fb);
    vbuffer_t *buffer = framebuffer_get_tail(fb, FB_NO_FLAGS);

    if (!buffer) {
        return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
    }

    uint8_t *b = buffer->data;
    uint32_t _width = fb->w;
    uint32_t _height = fb->h;
    int bytesPerRow = _width * 2;  // Always read 2 BPP
    bool _grayscale = (csi->pixformat == PIXFORMAT_GRAYSCALE);

    uint32_t ulPin = 32; // P1.xx set of GPIO is in 'pin' 32 and above
    NRF_GPIO_Type *port = nrf_gpio_pin_port_decode(&ulPin);

    noInterrupts();

    // Falling edge indicates start of frame
    while ((*_vsyncPort & _vsyncMask) == 0) {
        // Wait for high
    }
    while ((*_vsyncPort & _vsyncMask) != 0) {
        // Wait for low
    }
    for (int i = 0; i < _height; i++) {
        // rising edge indicates start of line
        while ((*_hrefPort & _hrefMask) == 0) {
            // Wait for high
        }
        for (int j = 0; j < bytesPerRow; j++) {
            while ((*_pclkPort & _pclkMask) != 0) {
                // Wait for low
            }
            uint32_t in = port->IN; // read all bits in parallel
            if (!_grayscale || !(j & 1)) {
                // Note D0 & D1 are swapped on the ML kit.
                *b++ = ((in >> 8) | ((in >> 3) & 1) | ((in >> 1) & 2));
            }
            while ((*_pclkPort & _pclkMask) == 0) {
                // Wait for high
            }
        }
        while ((*_hrefPort & _hrefMask) != 0) {
            // Wait for low
        }
    }

    interrupts();

    // Not useful for the NRF but must call to keep API the same.
    if (csi->frame_callback) {
        csi->frame_callback();
    }

    // Set framebuffer pixel format.
    fb->pixfmt = csi->pixformat;

    // Swap bytes if set.
    if ((fb->pixfmt == PIXFORMAT_RGB565 && csi->rgb_swap) ||
        (fb->pixfmt == PIXFORMAT_YUV422 && csi->yuv_swap)) {
        unaligned_memcpy_rev16(buffer->data, buffer->data, _width * _height);
    }

    // Set the user image.
    framebuffer_init_image(fb, image);
    return 0;
}

int omv_csi_init() {
    int init_ret = 0;

    #if defined(OMV_CSI_POWER_PIN)
    nrf_gpio_cfg_output(OMV_CSI_POWER_PIN);
    nrf_gpio_pin_write(OMV_CSI_POWER_PIN, 1);
    #endif

    #if defined(OMV_CSI_RESET_PIN)
    nrf_gpio_cfg_output(OMV_CSI_RESET_PIN);
    nrf_gpio_pin_write(OMV_CSI_RESET_PIN, 1);
    #endif

    // Reset the csi state
    memset(&csi, 0, sizeof(omv_csi_t));

    // Set default framebuffer
    csi.fb = framebuffer_get(0);

    // Set default snapshot function.
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

    // Clear fb_enabled flag
    // This is executed only once to initialize the FB enabled flag.
    //JPEG_FB()->enabled = 0;

    // Set default color palette.
    csi.color_palette = rainbow_table;

    csi.detected = true;

    // Disable VSYNC IRQ and callback
    omv_csi_set_vsync_callback(NULL);

    /* All good! */
    return 0;
}
