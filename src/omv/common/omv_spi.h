/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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
    OMV_SPI_XFER_16_BIT   = (1 << 5),
    OMV_SPI_XFER_HALF     = (1 << 6), // Internal for Double Buffer mode.
} omv_spi_xfer_flags_t;

typedef struct _omv_spi_config {
    uint8_t id;
    uint32_t baudrate;
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

typedef void (*omv_spi_callback_t) (omv_spi_t *spi, void *userdata, void *rxbuf);

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
// Default config: MASTER | FDX | 10MHz | MSB FIRST | NSS HARD | NSS/CPHA/CPOL LOW.
int omv_spi_default_config(omv_spi_config_t *config, uint32_t bus_id);
int omv_spi_deinit(omv_spi_t *spi);
int omv_spi_set_baudrate(omv_spi_t *spi, uint32_t baudrate);
int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer);
int omv_spi_transfer_abort(omv_spi_t *spi);

#endif // __OMV_SPI_H__
