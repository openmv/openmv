/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2021 Lake Fu at <lake_fu@pixart.com>
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Pixart SPI driver.
 */
#include "omv_boardconfig.h"
#ifdef ISC_SPI_ID

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "omv_gpio.h"
#include "omv_spi.h"

#include "pixspi.h"

#define SPI_BUS_TIMEOUT         (5000)  // in ms
#ifdef PIXART_SPI_DEBUG
#define debug_printf(...)   printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif

static omv_spi_t spi_bus;

static int spi_send(uint8_t *txbuf, uint16_t len)
{
    int ret = 0;

    omv_spi_transfer_t spi_xfer = {
        .txbuf = txbuf,
        .rxbuf = NULL,
        .size = len,
        .timeout = SPI_BUS_TIMEOUT,
        .flags = OMV_SPI_XFER_BLOCKING,
        .callback = NULL,
        .userdata = NULL,
    };

    omv_gpio_write(spi_bus.cs, 0);
    ret = omv_spi_transfer_start(&spi_bus, &spi_xfer);
    omv_gpio_write(spi_bus.cs, 1);

    return ret;
}

static int spi_send_recv(uint8_t *txbuf, uint8_t *rxbuf, uint16_t len)
{
    int ret = 0;

    omv_spi_transfer_t spi_xfer = {
        .timeout = SPI_BUS_TIMEOUT,
        .flags = OMV_SPI_XFER_BLOCKING,
        .callback = NULL,
        .userdata = NULL,
    };

    omv_gpio_write(spi_bus.cs, 0);

    spi_xfer.size = 1;
    spi_xfer.txbuf = txbuf;
    spi_xfer.rxbuf = NULL;
    ret |= omv_spi_transfer_start(&spi_bus, &spi_xfer);

    spi_xfer.size = len;
    spi_xfer.txbuf = NULL;
    spi_xfer.rxbuf = rxbuf;
    ret |= omv_spi_transfer_start(&spi_bus, &spi_xfer);

    omv_gpio_write(spi_bus.cs, 1);
    return ret;
}

bool pixspi_init()
{
    // Init SPI
    omv_spi_config_t spi_config;
    omv_spi_default_config(&spi_config, ISC_SPI_ID);

    spi_config.baudrate    = 5000000;
    spi_config.nss_enable  = false; // Soft NSS
    spi_config.clk_pol     = OMV_SPI_CPOL_HIGH;
    spi_config.clk_pha     = OMV_SPI_CPHA_2EDGE;

    if (omv_spi_init(&spi_bus, &spi_config) != 0) {
        return false;
    }
    return true;
}

void pixspi_release()
{
    omv_gpio_write(spi_bus.cs, 1);
    omv_spi_deinit(&spi_bus);
}

int pixspi_regs_read(uint8_t addr, uint8_t * data, uint16_t length)
{
    if (addr & 0x80) {
        debug_printf("pixspi_regs_read() address (0x%x) overflow.\n", addr);
        return -1;
    }
    addr |= 0x80;
    if (spi_send_recv(&addr, data, length) == -1) {
        debug_printf("spi_send_recv() failed.\n");
        return -1;
    }
    return 0;
}

int pixspi_regs_write(uint8_t addr, const uint8_t * data, uint16_t length)
{
    uint8_t buff[64] = {};
    if (addr & 0x80) {
        debug_printf("pixspi_regs_read() address (0x%x) overflow.\n", addr);
        return -1;
    }

    int32_t remaining = length;
    const static uint16_t MAX_LENGTH = 255;

    do {
        uint16_t len = remaining > MAX_LENGTH ? MAX_LENGTH : remaining;
        buff[0] = addr;
        memcpy(buff+1, data, len);
        if (spi_send(buff, len + 1) == -1) {
            debug_printf("spi_send() failed.\n");
            return -1;
        }
        remaining = remaining - MAX_LENGTH;
    } while (remaining > 0);

    return 0;
}
#endif
