/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023 OpenMV, LLC.
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
 * OpenMV MIMXRT port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__

#include "fsl_gpio.h"
#include "fsl_lpi2c.h"
#include "fsl_lpspi.h"
#include "fsl_iomuxc.h"
#include "fsl_edma.h"
#include "fsl_lpspi_edma.h"

#define IOMUXC_PAD_CTL(opt, val)    (IOMUXC_SW_PAD_CTL_PAD_##opt(val))
#define IOMUXC_MUX_CTL(opt, val)    (IOMUXC_SW_MUX_CTL_PAD_##opt(val))

// *INDENT-OFF*
// omv_gpio_t definitions

// GPIO speeds.
#define OMV_GPIO_SPEED_LOW      ((1 << 17) | IOMUXC_PAD_CTL(SPEED, 0U) | IOMUXC_PAD_CTL(SRE, 0U))   // 50MHz
#define OMV_GPIO_SPEED_MED      ((2 << 17) | IOMUXC_PAD_CTL(SPEED, 1U) | IOMUXC_PAD_CTL(SRE, 0U))   // 100MHz
#define OMV_GPIO_SPEED_HIGH     ((3 << 17) | IOMUXC_PAD_CTL(SPEED, 2U) | IOMUXC_PAD_CTL(SRE, 1U))   // 150MHz
#define OMV_GPIO_SPEED_MAX      ((4 << 17) | IOMUXC_PAD_CTL(SPEED, 3U) | IOMUXC_PAD_CTL(SRE, 1U))   // 200MHz

// GPIO pull.
#define OMV_GPIO_PULL_NONE      ((1 << 17) | IOMUXC_PAD_CTL(PKE, 0U) | IOMUXC_PAD_CTL(PUE, 0) | IOMUXC_PAD_CTL(PUS, 0))
#define OMV_GPIO_PULL_UP        ((2 << 17) | IOMUXC_PAD_CTL(PKE, 1U) | IOMUXC_PAD_CTL(PUE, 1) | IOMUXC_PAD_CTL(PUS, 2))
#define OMV_GPIO_PULL_DOWN      ((3 << 17) | IOMUXC_PAD_CTL(PKE, 1U) | IOMUXC_PAD_CTL(PUE, 1) | IOMUXC_PAD_CTL(PUS, 0))

// GPIO modes.
#define OMV_GPIO_MODE_INPUT     ((1 << 17) | IOMUXC_PAD_CTL(DSE, 0U) | IOMUXC_PAD_CTL(HYS, 1U) | IOMUXC_PAD_CTL(ODE, 0U))
#define OMV_GPIO_MODE_OUTPUT    ((2 << 17) | IOMUXC_PAD_CTL(DSE, 6U) | IOMUXC_PAD_CTL(HYS, 0U) | IOMUXC_PAD_CTL(ODE, 0U))
#define OMV_GPIO_MODE_OUTPUT_OD ((3 << 17) | IOMUXC_PAD_CTL(DSE, 6U) | IOMUXC_PAD_CTL(HYS, 0U) | IOMUXC_PAD_CTL(ODE, 1U))
#define OMV_GPIO_MODE_ALT       ((4 << 17) | IOMUXC_PAD_CTL(DSE, 6U) | IOMUXC_PAD_CTL(HYS, 0U) | IOMUXC_PAD_CTL(ODE, 0U))
#define OMV_GPIO_MODE_ALT_OD    ((5 << 17) | IOMUXC_PAD_CTL(DSE, 6U) | IOMUXC_PAD_CTL(HYS, 0U) | IOMUXC_PAD_CTL(ODE, 1U))

// GPIO IT modes.
#define OMV_GPIO_MODE_IT_FALL   ((6 << 17) | IOMUXC_PAD_CTL(DSE, 0U) | IOMUXC_PAD_CTL(HYS, 1U))
#define OMV_GPIO_MODE_IT_RISE   ((7 << 17) | IOMUXC_PAD_CTL(DSE, 0U) | IOMUXC_PAD_CTL(HYS, 1U))
#define OMV_GPIO_MODE_IT_BOTH   ((8 << 17) | IOMUXC_PAD_CTL(DSE, 0U) | IOMUXC_PAD_CTL(HYS, 1U))
// *INDENT-OFF*

typedef struct {
    uint8_t idx;
    uint16_t input_register;
    uint8_t  input_daisy;
} imxrt_pad_af_t;

typedef struct {
    GPIO_Type *port;
    uint8_t pin;
    uint16_t iomux_base;
    uint16_t mux_register;
    uint16_t cfg_register;
    uint8_t af_count;
    const imxrt_pad_af_t *af_list;
} imxrt_pad_t;
#include "mimxrt_pads.h"

typedef struct _omv_gpio {
    const imxrt_pad_t *pad; // IMXRT pad.
    uint8_t af;             // Default AF.
    uint8_t sion;           // Software input on.
} imxrt_gpio_t;

typedef const imxrt_gpio_t *omv_gpio_t;

// For board config files.
#if OMV_GPIO_DEFINE_PINS
#define OMV_GPIO_DEFINE(pin, pad, af, sion) \
    const imxrt_gpio_t omv_pin_##pin = {&imxrt_pad_##pad, pad##_AF_##af, sion};
#else
#define OMV_GPIO_DEFINE(pin, pad, af, sion) \
    extern const imxrt_gpio_t omv_pin_##pin;
#endif
#include "omv_pins.h"

// omv_i2c_t definitions
typedef LPI2C_Type *omv_i2c_dev_t;
#define OMV_I2C_MAX_8BIT_XFER   (1024U)
#define OMV_I2C_MAX_16BIT_XFER  (512U)

// omv_spi_t definitions
#define OMV_SPI_MODE_SLAVE      (0)
#define OMV_SPI_MODE_MASTER     (1)

#define OMV_SPI_LSB_FIRST       (kLPSPI_LsbFirst)
#define OMV_SPI_MSB_FIRST       (kLPSPI_MsbFirst)

#define OMV_SPI_BUS_TX          (1 << 0)
#define OMV_SPI_BUS_RX          (1 << 1)
#define OMV_SPI_BUS_TX_RX       (OMV_SPI_BUS_TX | OMV_SPI_BUS_RX)

#define OMV_SPI_CPOL_LOW        (kLPSPI_ClockPolarityActiveHigh)
#define OMV_SPI_CPOL_HIGH       (kLPSPI_ClockPolarityActiveLow)

#define OMV_SPI_CPHA_1EDGE      (kLPSPI_ClockPhaseFirstEdge)
#define OMV_SPI_CPHA_2EDGE      (kLPSPI_ClockPhaseSecondEdge)

#define OMV_SPI_NSS_LOW         (kLPSPI_PcsActiveLow)
#define OMV_SPI_NSS_HIGH        (kLPSPI_PcsActiveHigh)

#define OMV_SPI_MAX_8BIT_XFER   (32768U - 32U)
#define OMV_SPI_MAX_16BIT_XFER  (32768U - 16U)
#define OMV_SPI_MAX_TIMEOUT     (0xFFFFFFFF)

#define OMV_SPI_PORT_BITS                               \
struct {                                                \
    LPSPI_Type *inst;                                   \
    lpspi_master_config_t config_backup;                \
    edma_handle_t dma_descr_tx;                         \
    edma_handle_t dma_descr_rx;                         \
    union {                                             \
        lpspi_slave_handle_t  descr_slave;              \
        lpspi_master_handle_t descr_master;             \
    };                                                  \
    union {                                             \
        lpspi_slave_edma_handle_t  descr_slave_edma;    \
        lpspi_master_edma_handle_t descr_master_edma;   \
    };                                                  \
    lpspi_transfer_t xfer_descr;                        \
    uint32_t bus_mode;                                  \
};

// omv_csi_t port-specific fields.
#define OMV_CSI_PORT_BITS       \
    struct {                    \
        int src_inc;            \
        int src_size;           \
        int dest_inc;           \
        bool one_shot;          \
        edma_handle_t dma_channels[OMV_CSI_DMA_CHANNEL_COUNT];  \
    };
#endif // __OMV_PORTCONFIG_H__
