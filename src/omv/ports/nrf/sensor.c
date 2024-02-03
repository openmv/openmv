/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor driver for nRF port.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "omv_i2c.h"
#include "sensor.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "nrf_i2s.h"
#include "hal/nrf_gpio.h"

// Sensor struct.
sensor_t sensor = {};

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

int sensor_init() {
    int init_ret = 0;

    #if defined(DCMI_POWER_PIN)
    nrf_gpio_cfg_output(DCMI_POWER_PIN);
    nrf_gpio_pin_write(DCMI_POWER_PIN, 1);
    #endif

    #if defined(DCMI_RESET_PIN)
    nrf_gpio_cfg_output(DCMI_RESET_PIN);
    nrf_gpio_pin_write(DCMI_RESET_PIN, 1);
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


    // Configure the DCMI interface.
    if (sensor_dcmi_config(PIXFORMAT_INVALID) != 0) {
        // DCMI config failed
        return SENSOR_ERROR_DCMI_INIT_FAILED;
    }

    // Clear fb_enabled flag
    // This is executed only once to initialize the FB enabled flag.
    //JPEG_FB()->enabled = 0;

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    sensor.detected = true;

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    /* All good! */
    return 0;
}

int sensor_dcmi_config(uint32_t pixformat) {
    uint32_t dcmi_pins[] = {
        DCMI_D0_PIN,
        DCMI_D1_PIN,
        DCMI_D2_PIN,
        DCMI_D3_PIN,
        DCMI_D4_PIN,
        DCMI_D5_PIN,
        DCMI_D6_PIN,
        DCMI_D7_PIN,
        DCMI_VSYNC_PIN,
        DCMI_HSYNC_PIN,
        DCMI_PXCLK_PIN,
    };

    // Configure DCMI input pins
    for (int i = 0; i < sizeof(dcmi_pins) / sizeof(dcmi_pins[0]); i++) {
        nrf_gpio_cfg_input(dcmi_pins[i], NRF_GPIO_PIN_PULLUP);
    }

    _vsyncMask = digitalPinToBitMask(DCMI_VSYNC_PIN);
    _hrefMask = digitalPinToBitMask(DCMI_HSYNC_PIN);
    _pclkMask = digitalPinToBitMask(DCMI_PXCLK_PIN);

    _vsyncPort = portInputRegister(digitalPinToPort(DCMI_VSYNC_PIN));
    _hrefPort = portInputRegister(digitalPinToPort(DCMI_HSYNC_PIN));
    _pclkPort = portInputRegister(digitalPinToPort(DCMI_PXCLK_PIN));

    return 0;
}

uint32_t sensor_get_xclk_frequency() {
    return OMV_XCLK_FREQUENCY;
}

int sensor_set_xclk_frequency(uint32_t frequency) {
    nrf_gpio_cfg_output(DCMI_XCLK_PIN);

    // Generates 16 MHz signal using I2S peripheral
    NRF_I2S->CONFIG.MCKEN = (I2S_CONFIG_MCKEN_MCKEN_ENABLE << I2S_CONFIG_MCKEN_MCKEN_Pos);
    NRF_I2S->CONFIG.MCKFREQ = I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV2 << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
    NRF_I2S->CONFIG.MODE = I2S_CONFIG_MODE_MODE_MASTER << I2S_CONFIG_MODE_MODE_Pos;

    NRF_I2S->PSEL.MCK = (DCMI_XCLK_PIN << I2S_PSEL_MCK_PIN_Pos);

    NRF_I2S->ENABLE = 1;
    NRF_I2S->TASKS_START = 1;

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h) {
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

// This is the default snapshot function, which can be replaced in sensor_init functions.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags) {
    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    framebuffer_update_jpeg_buffer();

    // This driver supports a single buffer.
    if (MAIN_FB()->n_buffers != 1) {
        framebuffer_set_buffers(1);
    }

    if (sensor_check_framebuffer_size() != 0) {
        return SENSOR_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    framebuffer_free_current_buffer();
    framebuffer_setup_buffers();
    vbuffer_t *buffer = framebuffer_get_tail(FB_NO_FLAGS);

    if (!buffer) {
        return SENSOR_ERROR_FRAMEBUFFER_ERROR;
    }

    uint8_t *b = buffer->data;
    uint32_t _width = MAIN_FB()->w;
    uint32_t _height = MAIN_FB()->h;
    int bytesPerRow = _width * 2;  // Always read 2 BPP
    bool _grayscale = (sensor->pixformat == PIXFORMAT_GRAYSCALE);

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
    if (sensor->frame_callback) {
        sensor->frame_callback();
    }

    // Set framebuffer pixel format.
    MAIN_FB()->pixfmt = sensor->pixformat;

    // Swap bytes if set.
    if ((MAIN_FB()->pixfmt == PIXFORMAT_RGB565 && sensor->hw_flags.rgb_swap) ||
        (MAIN_FB()->pixfmt == PIXFORMAT_YUV422 && sensor->hw_flags.yuv_swap)) {
        unaligned_memcpy_rev16(buffer->data, buffer->data, _width * _height);
    }

    // Set the user image.
    framebuffer_init_image(image);
    return 0;
}
