/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OpenMV STM32 port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__
#include STM32_HAL_H

// GPIO speeds.
#define OMV_GPIO_SPEED_LOW         GPIO_SPEED_FREQ_LOW
#define OMV_GPIO_SPEED_MED         GPIO_SPEED_FREQ_MEDIUM
#define OMV_GPIO_SPEED_HIGH        GPIO_SPEED_FREQ_HIGH
#define OMV_GPIO_SPEED_MAX         GPIO_SPEED_FREQ_VERY_HIGH

// GPIO pull.
#define OMV_GPIO_PULL_NONE         GPIO_NOPULL
#define OMV_GPIO_PULL_UP           GPIO_PULLUP
#define OMV_GPIO_PULL_DOWN         GPIO_PULLDOWN

// GPIO modes.
#define OMV_GPIO_MODE_INPUT        GPIO_MODE_INPUT
#define OMV_GPIO_MODE_OUTPUT       GPIO_MODE_OUTPUT_PP
#define OMV_GPIO_MODE_OUTPUT_OD    GPIO_MODE_OUTPUT_OD
#define OMV_GPIO_MODE_ALT          GPIO_MODE_AF_PP
#define OMV_GPIO_MODE_ALT_OD       GPIO_MODE_AF_OD
#define OMV_GPIO_MODE_ANALOG       GPIO_MODE_ANALOG

// GPIO IT modes.
#define OMV_GPIO_MODE_IT_FALL      GPIO_MODE_IT_FALLING
#define OMV_GPIO_MODE_IT_RISE      GPIO_MODE_IT_RISING
#define OMV_GPIO_MODE_IT_BOTH      GPIO_MODE_IT_RISING_FALLING

// omv_gpio_t definition
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint16_t af;
} stm32_gpio_t;

typedef const stm32_gpio_t *omv_gpio_t;

#define OMV_GPIO_IRQ_DESCR_PORT_BITS \
    struct {                         \
        uint32_t gpio;               \
    };

// Dummy AF for pins defined as I/O
#define GPIO_NONE_GPIO    (0)

// For board config files.
#if OMV_GPIO_DEFINE_PINS
#define OMV_GPIO_DEFINE(port, pin, af, inst) \
    const stm32_gpio_t omv_pin_##port##pin##_##inst = {GPIO##port, GPIO_PIN_##pin, GPIO_##af##_##inst};
#else
#define OMV_GPIO_DEFINE(port, pin, af, inst) \
    extern stm32_gpio_t omv_pin_##port##pin##_##inst;
#endif
#include "omv_pins.h"

// This pointer will be set to its respective I2C handle which is defined in MicroPython along with
// IRQ handlers, if this I2C is enabled in Micropython, or defined and handled in stm32fxxx_hal_msp.c.
typedef I2C_HandleTypeDef *omv_i2c_dev_t;

#define OMV_SPI_MODE_SLAVE     (SPI_MODE_SLAVE)
#define OMV_SPI_MODE_MASTER    (SPI_MODE_MASTER)

#define OMV_SPI_LSB_FIRST      (SPI_FIRSTBIT_LSB)
#define OMV_SPI_MSB_FIRST      (SPI_FIRSTBIT_MSB)

#define OMV_SPI_BUS_TX         (1 << 0)
#define OMV_SPI_BUS_RX         (1 << 1)
#define OMV_SPI_BUS_TX_RX      (OMV_SPI_BUS_TX | OMV_SPI_BUS_RX)

#define OMV_SPI_CPOL_LOW       (SPI_POLARITY_LOW)
#define OMV_SPI_CPOL_HIGH      (SPI_POLARITY_HIGH)

#define OMV_SPI_CPHA_1EDGE     (SPI_PHASE_1EDGE)
#define OMV_SPI_CPHA_2EDGE     (SPI_PHASE_2EDGE)

#define OMV_SPI_NSS_LOW        (0)
#define OMV_SPI_NSS_HIGH       (1)

#define OMV_SPI_MAX_8BIT_XFER  (65536U - 16U)
#define OMV_SPI_MAX_16BIT_XFER (65536U - 8U)
#define OMV_SPI_MAX_TIMEOUT    (HAL_MAX_DELAY)

#define OMV_SPI_PORT_BITS               \
    struct {                            \
        IRQn_Type irqn;                 \
        SPI_HandleTypeDef *descr;       \
        DMA_HandleTypeDef dma_descr_tx; \
        DMA_HandleTypeDef dma_descr_rx; \
    };

#endif // __OMV_PORTCONFIG_H__
