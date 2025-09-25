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
 * Alif SPI driver.
 */
#include <stdint.h>
#include <string.h>

#include "omv_boardconfig.h"

#include "py/mphal.h"
#include "py/runtime.h"

#include "spi.h"
#include "alif_hal.h"
#include "omv_common.h"
#include "omv_spi.h"
#include "omv_gpio.h"

typedef struct omv_spi_descr {
    SPI_Type *inst;
    omv_gpio_t cs;
    bool is_lp;
} omv_spi_descr_t;

static const omv_spi_descr_t omv_spi_descr_all[] = {
    #if defined(OMV_SPI0_ID)
    { (SPI_Type *) SPI0_BASE, OMV_SPI0_SSEL_PIN, false },
    #else
    { NULL, NULL, false },
    #endif
    #if defined(OMV_SPI1_ID)
    { (SPI_Type *) SPI1_BASE, OMV_SPI1_SSEL_PIN, false },
    #else
    { NULL, NULL, false },
    #endif
    #if defined(OMV_SPI2_ID)
    { (SPI_Type *) SPI2_BASE, OMV_SPI2_SSEL_PIN, false },
    #else
    { NULL, NULL, false },
    #endif
    #if defined(OMV_SPI3_ID)
    { (SPI_Type *) SPI3_BASE, OMV_SPI3_SSEL_PIN, false },
    #else
    { NULL, NULL, false },
    #endif
    #if defined(OMV_SPI4_ID)
    { (SPI_Type *) LPSPI0_BASE, OMV_SPI4_SSEL_PIN, true },
    #else
    { NULL, NULL, false },
    #endif
};

static int omv_spi_poll_flag(SPI_Type *spi, uint32_t flag, uint32_t timeout) {
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while (!(spi->SPI_SR & flag)) {
        if (mp_hal_ticks_ms() - tick_start >= timeout) {
            return -1;
        }
        mp_event_handle_nowait();
    }
    return 0;
}

int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer) {
    if (xfer->flags & OMV_SPI_XFER_BLOCKING) {
        volatile uint32_t *dr = spi->inst->SPI_DR;
        spi_set_tmod(spi->inst, SPI_TMOD_TX_AND_RX);

        size_t tx_sent = 0;
        size_t rx_received = 0;

        // FIFO depth is 16 words
        const size_t rx_threshold = (SPI_RX_FIFO_DEPTH - 1);

        while (rx_received < xfer->size) {
            // Fill TX FIFO as much as possible without causing RX overflow
            while (tx_sent < xfer->size &&
                   (tx_sent - rx_received) < rx_threshold &&
                   (spi->inst->SPI_SR & SPI_SR_TFNF)) {

                // Send data
                if (xfer->txbuf == NULL) {
                    *dr = 0xFFFFFFFFU;
                } else if (spi->datasize > 16) {
                    *dr = ((uint32_t *) xfer->txbuf)[tx_sent];
                } else if (spi->datasize > 8) {
                    *dr = ((uint16_t *) xfer->txbuf)[tx_sent];
                } else {
                    *dr = ((uint8_t *) xfer->txbuf)[tx_sent];
                }
                tx_sent++;
            }

            // Receive available data
            while (rx_received < tx_sent &&
                   (spi->inst->SPI_SR & SPI_SR_RFNE)) {

                if (xfer->rxbuf == NULL) {
                    (void) *dr;
                } else if (spi->datasize > 16) {
                    ((uint32_t *) xfer->rxbuf)[rx_received] = *dr;
                } else if (spi->datasize > 8) {
                    ((uint16_t *) xfer->rxbuf)[rx_received] = *dr;
                } else {
                    ((uint8_t *) xfer->rxbuf)[rx_received] = *dr;
                }
                rx_received++;
            }

            // If we're not making progress, use blocking wait
            if (tx_sent < xfer->size && (tx_sent - rx_received) >= rx_threshold) {
                // Wait for RX data to free up space
                if (omv_spi_poll_flag(spi->inst, SPI_SR_RFNE, xfer->timeout) == -1) {
                    return -1;
                }
            } else if (tx_sent == xfer->size && rx_received < tx_sent) {
                // All data sent, wait for remaining RX data
                if (omv_spi_poll_flag(spi->inst, SPI_SR_RFNE, xfer->timeout) == -1) {
                    return -1;
                }
            } else if (tx_sent < xfer->size) {
                // Wait for space in the TX FIFO
                if (omv_spi_poll_flag(spi->inst, SPI_SR_TFNF, xfer->timeout) == -1) {
                    return -1;
                }
            }

            // Handle pending events
            mp_event_handle_nowait();
        }

        return 0;
    }
    return -1;
}

int omv_spi_transfer_abort(omv_spi_t *spi) {
    return 0;
}

int omv_spi_init(omv_spi_t *spi, omv_spi_config_t *config) {
    memset(spi, 0, sizeof(omv_spi_t));
    const omv_spi_descr_t *spi_descr = &omv_spi_descr_all[config->id];

    spi->id = config->id;
    spi->inst = spi_descr->inst;
    spi->cs = spi_descr->cs;
    spi->is_lp = spi_descr->is_lp;
    spi->spi_mode = config->spi_mode;
    spi->bus_mode = config->bus_mode;
    spi->datasize = config->datasize;

    if (spi->inst == NULL) {
        return -1;
    }

    if (config->dma_flags) {
        // TODO DMA mode not supported.
        return -1;
    }

    // Disable SPI.
    spi_disable(spi->inst);

    // Initialize GPIOs and clocks.
    alif_hal_spi_init(config->id, config->nss_enable, config->nss_pol);

    // Disable all interrupts.
    spi_mask_interrupts(spi->inst);

    // Configure baudrate clock
    omv_spi_set_baudrate(spi, config->baudrate);

    // Configure FIFOs
    spi_set_tx_threshold(spi->inst, 0);
    spi_set_rx_threshold(spi->inst, 0);
    if (!spi_descr->is_lp) {
        spi_set_rx_sample_delay(spi->inst, 0);
        spi_set_tx_fifo_start_level(spi->inst, 0);
    }

    // Configure SPI bus mode.
    uint32_t spi_mode = (config->clk_pol << 1) | config->clk_pha;
    if (!spi_descr->is_lp) {
        spi_set_mode(spi->inst, spi_mode);
    } else{
        lpspi_set_mode(spi->inst, spi_mode);
    }

    // Configure SPI bus protocol.
    uint32_t spi_proto = SPI_PROTO_SPI;
    if (!spi_descr->is_lp) {
        spi_set_protocol(spi->inst, spi_proto);
    } else{
        lpspi_set_protocol(spi->inst, spi_proto);
    }

    // Configure SPI transfer mode.
    if (!spi_descr->is_lp) {
        if (config->spi_mode == OMV_SPI_MODE_MASTER) {
            spi_mode_master(spi->inst);
            //ctrl_ss_in(spi->inst, SS_IN_IO_PIN);
        } else {
            // TODO not support.
            //spi_mode_slave(spi->inst);
        }
    }

    // Configure frame size.
    if (!spi_descr->is_lp) {
        spi_set_dfs(spi->inst, config->datasize);
    } else {
        lpspi_set_dfs(spi->inst, config->datasize);
    }

    // Configure slave select pin
    spi_control_ss(spi->inst, 0, true);
    if (!spi_descr->is_lp) {
        spi_set_sste(spi->inst, false);
    } else{
        lpspi_set_sste(spi->inst, false);
    }

    (void) spi->inst->SPI_ICR;
    spi_enable(spi->inst);
    spi->initialized = true;
    return 0;
}

int omv_spi_deinit(omv_spi_t *spi) {
    // Disable all interrupts.
    spi_mask_interrupts(spi->inst);
    // Disable SCLK clock
    omv_spi_set_baudrate(spi, 0);
    // Disable SS pin.
    spi_control_ss(spi->inst, 0, 0);
    // Disable SPI.
    spi_disable(spi->inst);
    // Deinitialize GPIOs and clocks.
    alif_hal_spi_deinit(spi->id);
    return 0;
}

int omv_spi_set_baudrate(omv_spi_t *spi, uint32_t baudrate) {
    omv_spi_transfer_abort(spi);
    uint32_t sclk_source = spi->is_lp ? GetSystemCoreClock() : GetSystemAHBClock();
    spi_set_bus_speed(spi->inst, baudrate, sclk_source);
    return 0;
}

int omv_spi_default_config(omv_spi_config_t *config, uint32_t bus_id) {
    config->id = bus_id;
    config->baudrate = 4000000;
    config->datasize = 8;
    config->spi_mode = OMV_SPI_MODE_MASTER;
    config->bus_mode = OMV_SPI_BUS_TX_RX;
    config->bit_order = OMV_SPI_MSB_FIRST;
    config->clk_pol = OMV_SPI_CPOL_LOW;
    config->clk_pha = OMV_SPI_CPHA_1EDGE;
    config->nss_pol = OMV_SPI_NSS_LOW;
    config->nss_enable = true;
    config->dma_flags = 0;
    config->data_retained = true;
    return 0;
}

int omv_spi_test() {
    static omv_spi_t spi_bus;
    omv_spi_config_t spi_config;
    omv_spi_default_config(&spi_config, OMV_IMU_SPI_ID);

    spi_config.baudrate = 500000;
    spi_config.clk_pol = OMV_SPI_CPOL_LOW;
    spi_config.clk_pha = OMV_SPI_CPHA_1EDGE;
    spi_config.nss_enable = false; // Soft NSS

    omv_spi_init(&spi_bus, &spi_config);

    while (true) {
        omv_spi_transfer_t spi_xfer = {
            .timeout = 0,
            .flags = OMV_SPI_XFER_BLOCKING,
            .callback = NULL,
            .userdata = NULL,
        };

        uint8_t data[] = {0x7f, 0x8f, 0x9f, 0xAA, 0xBB };
        spi_xfer.size = sizeof(data);
        spi_xfer.txbuf = data;
        spi_xfer.rxbuf = NULL;

        omv_gpio_write(spi_bus.cs, 0);
        omv_spi_transfer_start(&spi_bus, &spi_xfer);
        omv_gpio_write(spi_bus.cs, 1);
        mp_hal_delay_ms(500);
    }
}
