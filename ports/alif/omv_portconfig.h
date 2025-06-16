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
 * OpenMV MIMXRT port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__

#include <stdlib.h>
#include "clk.h"
#include "i2c.h"
#include "i3c.h"
#include "spi.h"
#include "gpio.h"
#include "pinconf.h"

#include PINS_AF_H
#include CMSIS_MCU_H

// *INDENT-OFF*
// omv_gpio_t definitions

// GPIO speeds.
#define OMV_GPIO_SPEED_LOW          ((1 << 8) | 0)
#define OMV_GPIO_SPEED_MED          ((2 << 8) | 0)
#define OMV_GPIO_SPEED_HIGH         ((3 << 8) | PADCTRL_SLEW_RATE_FAST)
#define OMV_GPIO_SPEED_MAX          ((4 << 8) | PADCTRL_SLEW_RATE_FAST)

// GPIO pull.                                    
#define OMV_GPIO_PULL_NONE          ((1 << 8) | PADCTRL_DRIVER_DISABLED_HIGH_Z)
#define OMV_GPIO_PULL_UP            ((2 << 8) | PADCTRL_DRIVER_DISABLED_PULL_UP)
#define OMV_GPIO_PULL_DOWN          ((3 << 8) | PADCTRL_DRIVER_DISABLED_PULL_DOWN)

// GPIO modes.                                   
#define OMV_GPIO_MODE_INPUT         ((1 << 8) | PADCTRL_SCHMITT_TRIGGER_ENABLE | PADCTRL_READ_ENABLE)
#define OMV_GPIO_MODE_OUTPUT        ((2 << 8) | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA)
#define OMV_GPIO_MODE_OUTPUT_OD     ((3 << 8) | PADCTRL_OUTPUT_DRIVE_STRENGTH_8MA | PADCTRL_DRIVER_OPEN_DRAIN)
#define OMV_GPIO_MODE_ALT           ((4 << 8))  // No settings for AF modes.
#define OMV_GPIO_MODE_ALT_OD        ((5 << 8))  // No settings for AF modes.

// GPIO IT modes.                                
#define OMV_GPIO_MODE_IT_FALL       ((6 << 8) | PADCTRL_SCHMITT_TRIGGER_ENABLE | PADCTRL_READ_ENABLE)
#define OMV_GPIO_MODE_IT_RISE       ((7 << 8) | PADCTRL_SCHMITT_TRIGGER_ENABLE | PADCTRL_READ_ENABLE)
#define OMV_GPIO_MODE_IT_BOTH       ((8 << 8) | PADCTRL_SCHMITT_TRIGGER_ENABLE | PADCTRL_READ_ENABLE)
// *INDENT-OFF*

typedef struct {
    uint8_t port;
    uint8_t pin;
    uint8_t af;
    uint8_t ren;    // Receiver enable.
} alif_gpio_t;

typedef const alif_gpio_t *omv_gpio_t;

// For board config files.
#if OMV_GPIO_DEFINE_PINS
#define OMV_GPIO_DEFINE(name, port, pin, af, ren) \
    const alif_gpio_t omv_pin_##name = {port, pin, port##_##pin##_AF_##af, ren};
#else
#define OMV_GPIO_DEFINE(name, port, pin, af, ren) \
    extern const alif_gpio_t omv_pin_##name;
#endif
#include "omv_pins.h"

// omv_i2c_t definitions
typedef void *omv_i2c_dev_t;
#define OMV_I2C_PORT_BITS   \
struct {                    \
    bool is_i3c;            \
    size_t cw_size;         \
    uint8_t cw_buf[256];    \
};

#define OMV_I2C_MAX_8BIT_XFER   (65536U - 16U)
#define OMV_I2C_MAX_16BIT_XFER  (65536U - 8U)

// omv_spi_t definitions
#define OMV_SPI_MODE_SLAVE      (0)
#define OMV_SPI_MODE_MASTER     (1)

#define OMV_SPI_LSB_FIRST       (0)
#define OMV_SPI_MSB_FIRST       (1)

#define OMV_SPI_BUS_TX          (1 << 0)
#define OMV_SPI_BUS_RX          (1 << 1)
#define OMV_SPI_BUS_TX_RX       (OMV_SPI_BUS_TX | OMV_SPI_BUS_RX)

#define OMV_SPI_CPOL_LOW        (0)
#define OMV_SPI_CPOL_HIGH       (1)

#define OMV_SPI_CPHA_1EDGE      (0)
#define OMV_SPI_CPHA_2EDGE      (1)

#define OMV_SPI_NSS_LOW         (0)
#define OMV_SPI_NSS_HIGH        (1)

#define OMV_SPI_MAX_8BIT_XFER   (32768U - 32U)
#define OMV_SPI_MAX_16BIT_XFER  (32768U - 16U)
#define OMV_SPI_MAX_TIMEOUT     (0xFFFFFFFF)
#define OMV_SPI_NO_DMA          (1)

#define OMV_SPI_PORT_BITS           \
struct {                            \
    SPI_Type *inst;                 \
    bool is_lp;                     \
    uint32_t spi_mode;              \
    uint32_t bus_mode;              \
    uint8_t datasize;               \
 };
#endif // __OMV_PORTCONFIG_H__
