/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * SPI bus abstraction layer.
 */
#ifndef __OMV_SPI_H__
#define __OMV_SPI_H__
#include <stdint.h>
#include <stdbool.h>
#include "omv_portconfig.h"

// Config options are defined in ports so they can be used
// directly to initialize peripherals without remapping them.

typedef enum {
    OMV_SPI_DMA_NORMAL    = (1 << 0),
    OMV_SPI_DMA_CIRCULAR  = (1 << 1),
    OMV_SPI_DMA_DOUBLE    = (1 << 2)
} omv_spi_dma_flags_t;

typedef enum {
    OMV_SPI_XFER_DMA      = (1 << 0),
    OMV_SPI_XFER_BLOCKING = (1 << 1),
    OMV_SPI_XFER_NONBLOCK = (1 << 2),
    OMV_SPI_XFER_FAILED   = (1 << 3),
    OMV_SPI_XFER_COMPLETE = (1 << 4),
    OMV_SPI_XFER_HALF     = (1 << 6), // Internal for Double Buffer mode.
} omv_spi_xfer_flags_t;

typedef struct _omv_spi_config {
    uint8_t id;
    uint32_t baudrate;
    uint8_t datasize;
    uint32_t spi_mode;
    uint32_t bus_mode;
    uint32_t bit_order;
    uint32_t clk_pol;
    uint32_t clk_pha;
    uint32_t nss_pol;
    bool nss_enable;
    uint32_t dma_flags;
    bool data_retained;
} omv_spi_config_t;

typedef struct _omv_spi omv_spi_t;

typedef void (*omv_spi_callback_t) (omv_spi_t *spi, void *userdata, void *buf);

typedef struct _omv_spi_transfer {
    void *txbuf;
    void *rxbuf;
    uint32_t size;
    uint32_t timeout;
    omv_spi_xfer_flags_t flags;
    void *userdata;
    omv_spi_callback_t callback;
} omv_spi_transfer_t;

typedef struct _omv_spi {
    uint8_t id;
    bool initialized;
    uint32_t dma_flags;
    omv_gpio_t cs;    // For soft-NSS mode.
    void *userdata;
    omv_spi_callback_t callback;
    uint32_t xfer_error;
    volatile omv_spi_xfer_flags_t xfer_flags;
    #ifdef OMV_SPI_PORT_BITS
    // Additional port-specific fields like device base pointer,
    // dma handles, more I/Os etc... are included directly here,
    // so that they can be accessible from this struct.
    OMV_SPI_PORT_BITS
    #endif
} omv_spi_t;

int omv_spi_init(omv_spi_t *spi, omv_spi_config_t *config);
// Default config: MASTER | FDX | 10MHz | 8 bits | MSB FIRST | NSS HARD | NSS/CPHA/CPOL LOW.
int omv_spi_default_config(omv_spi_config_t *config, uint32_t bus_id);
int omv_spi_deinit(omv_spi_t *spi);
int omv_spi_set_baudrate(omv_spi_t *spi, uint32_t baudrate);
int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer);
int omv_spi_transfer_abort(omv_spi_t *spi);

#endif // __OMV_SPI_H__
