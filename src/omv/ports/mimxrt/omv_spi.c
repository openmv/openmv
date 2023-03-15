/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OMV SPI bus port for mimxrt.
 */
#include "omv_boardconfig.h"
#include <stdint.h>
#include "py/mphal.h"

#include "fsl_gpio.h"
#include "fsl_lpspi.h"
#include "fsl_iomuxc.h"
#include "fsl_lpspi_edma.h"
#include "fsl_dmamux.h"
#include CLOCK_CONFIG_H

#include "mimxrt_hal.h"
#include "common.h"
#include "omv_spi.h"

typedef struct dma_descr {
    DMA_Type *dma_inst;
    DMAMUX_Type *dma_mux;
    uint32_t channel;
    uint32_t request;
} dma_descr_t;

typedef struct omv_spi_descr {
    LPSPI_Type *inst;
    omv_gpio_t cs;
    dma_descr_t dma_descr_tx;
    dma_descr_t dma_descr_rx;
} omv_spi_descr_t;

static const omv_spi_descr_t omv_spi_descr_all[] = {
    #if defined(LPSPI1_ID)
    { LPSPI1, LPSPI1_SSEL_PIN,
        { DMA0, DMAMUX, LPSPI1_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI1Tx },
        { DMA0, DMAMUX, LPSPI1_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI1Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif
    #if defined(LPSPI2_ID)
    { LPSPI2, LPSPI2_SSEL_PIN,
        { DMA0, DMAMUX, LPSPI2_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI2Tx },
        { DMA0, DMAMUX, LPSPI2_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI2Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif

    #if defined(LPSPI3_ID)
    { LPSPI3, LPSPI3_SSEL_PIN,
        { DMA0, DMAMUX, LPSPI3_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI3Tx },
        { DMA0, DMAMUX, LPSPI3_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI3Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif

    #if defined(LPSPI4_ID)
    { LPSPI4, LPSPI4_SSEL_PIN,
        { DMA0, DMAMUX, LPSPI4_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI4Tx },
        { DMA0, DMAMUX, LPSPI4_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI4Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif
};

static void spi_master_callback(LPSPI_Type *base, void *handle, status_t status, void *user)
{
    omv_spi_t *spi = (omv_spi_t *) user;

    if (status == kStatus_Success) {
        spi->xfer_flags |= OMV_SPI_XFER_COMPLETE;
    } else {
        spi->xfer_flags |= OMV_SPI_XFER_FAILED;
        spi->xfer_error = status;
        if (spi->xfer_flags & OMV_SPI_XFER_DMA) {
            LPSPI_MasterTransferAbortEDMA(spi->inst, &spi->descr_master_edma);
        } else {
            LPSPI_MasterTransferAbort(spi->inst, &spi->descr_master);
        }
    }

    if (spi->callback) {
        spi->callback(spi, spi->userdata);
    }

    uint32_t flags = (OMV_SPI_XFER_DMA | OMV_SPI_XFER_CIRCULAR | OMV_SPI_XFER_COMPLETE);
    if ((spi->xfer_flags & flags) == flags) {
        // Restart transfer for circular transfers.
        spi->xfer_flags &= ~(OMV_SPI_XFER_COMPLETE);
        LPSPI_MasterTransferEDMA(spi->inst, &spi->descr_master_edma, &spi->xfer_descr);
    }
}

int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer)
{
    spi->callback   = xfer->callback;
    spi->userdata   = xfer->userdata;
    spi->xfer_flags = xfer->flags;
    // Duplicate & stash transfer in spi struct, to avoid recreating
    // it for circular transfers. NOTE: If circular transfers ever
    // works this can be removed.
    spi->xfer_descr.txData      = xfer->txbuf;
    spi->xfer_descr.rxData      = xfer->rxbuf;
    spi->xfer_descr.dataSize    = xfer->size;
    spi->xfer_descr.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;
    spi->xfer_flags &= ~(OMV_SPI_XFER_FAILED | OMV_SPI_XFER_COMPLETE);

    if (spi->xfer_flags & OMV_SPI_XFER_DMA) {
        // DMA transfer (circular or one-shot)
        if (LPSPI_MasterTransferEDMA(spi->inst, &spi->descr_master_edma, &spi->xfer_descr) != kStatus_Success) {
            return -1;
        }
    } else if (spi->xfer_flags & (OMV_SPI_XFER_BLOCKING | OMV_SPI_XFER_NONBLOCK)) {
        // Use non-blocking mode for both non-blocking and blocking
        // transfers, this way we can control the timeout better.
        if (LPSPI_MasterTransferNonBlocking(spi->inst, &spi->descr_master, &spi->xfer_descr) != kStatus_Success) {
            return -1;
        }

        // Non-blocking transfetr, return immediately and the user
        // callback will be called when the transfer is done.
        if (spi->xfer_flags & OMV_SPI_XFER_NONBLOCK) {
            return 0;
        }

        // Blocking transfetr, wait for transfer complete or timeout.
        mp_uint_t start = mp_hal_ticks_ms();
        while (!(spi->xfer_flags & OMV_SPI_XFER_COMPLETE)) {
            if ((spi->xfer_flags & OMV_SPI_XFER_FAILED) ||
                ((mp_hal_ticks_ms() - start) > xfer->timeout)) {
                LPSPI_MasterTransferAbort(spi->inst, &spi->descr_master);
                return -1;
            }
            MICROPY_EVENT_POLL_HOOK
        }
    } else {
        return -1;
    }
    return 0;
}

int omv_spi_transfer_abort(omv_spi_t *spi)
{
    LPSPI_MasterTransferAbortEDMA(spi->inst, &spi->descr_master_edma);
    return 0;
}

static int omv_spi_dma_init(edma_handle_t *dma_handle, const dma_descr_t *dma_descr)
{
    DMAMUX_SetSource(dma_descr->dma_mux, dma_descr->channel, dma_descr->request);
    DMAMUX_EnableChannel(dma_descr->dma_mux, dma_descr->channel);
    EDMA_CreateHandle(dma_handle, dma_descr->dma_inst, dma_descr->channel);
    return 0;
}

int omv_spi_init(omv_spi_t *spi, omv_spi_config_t *config)
{
    // Configure clocks, pins and DMA MUX.
    mimxrt_hal_spi_init(config->id, config->nss_enable, config->nss_pol);

    const omv_spi_descr_t *spi_descr = &omv_spi_descr_all[config->id - 1];
    if (spi_descr->inst == NULL) {
        return -1;
    }

    memset(spi, 0, sizeof(omv_spi_t));
    spi->id = config->id;
    spi->inst = spi_descr->inst;
    spi->cs = spi_descr->cs;
    spi->dma_enabled = config->dma_enable;

    lpspi_master_config_t spi_config;
    LPSPI_MasterGetDefaultConfig(&spi_config);

    spi_config.whichPcs           = kLPSPI_Pcs0;
    spi_config.baudRate           = config->baudrate;
    spi_config.bitsPerFrame       = config->datasize;
    spi_config.direction          = config->bit_order;
    spi_config.pcsActiveHighOrLow = config->nss_pol;
    spi_config.cpol               = config->clk_pol;
    spi_config.cpha               = config->clk_pha;
    spi_config.dataOutConfig      = config->data_retained ? kLpspiDataOutRetained : kLpspiDataOutTristate;
    LPSPI_MasterInit(spi->inst, &spi_config, BOARD_BOOTCLOCKRUN_LPSPI_CLK_ROOT);

    if (config->dma_enable) {
        // Configure DMA.
        // Note the FSL driver doesn't support half-duplex, so the both
        // TX/RX channels, descriptors etc... must be initialized.
        omv_spi_dma_init(&spi->dma_descr_tx, &spi_descr->dma_descr_tx);
        omv_spi_dma_init(&spi->dma_descr_rx, &spi_descr->dma_descr_rx);

        // Link TX/RX DMA descriptors to SPI descriptor.
        LPSPI_MasterTransferCreateHandleEDMA(
                spi->inst,
                &spi->descr_master_edma,
                (lpspi_master_edma_transfer_callback_t) spi_master_callback,
                spi,
                &spi->dma_descr_rx,
                &spi->dma_descr_tx);
    } else {
        LPSPI_MasterTransferCreateHandle(
                spi->inst,
                &spi->descr_master,
                (lpspi_master_transfer_callback_t) spi_master_callback,
                spi);
    }
    spi->initialized = true;
    return 0;
}

int omv_spi_deinit(omv_spi_t *spi)
{
    return 0;
}

int omv_spi_default_config(omv_spi_config_t *config, uint32_t bus_id)
{
    config->id          = bus_id;
    config->baudrate    = 10000000;
    config->datasize    = 8;
    config->spi_mode    = OMV_SPI_MODE_MASTER;
    config->bus_mode    = OMV_SPI_BUS_TX_RX;
    config->bit_order   = OMV_SPI_MSB_FIRST;
    config->clk_pol     = OMV_SPI_CPOL_HIGH;
    config->clk_pha     = OMV_SPI_CPHA_2EDGE;
    config->nss_pol     = OMV_SPI_NSS_LOW;
    config->nss_enable  = true;
    config->dma_enable  = false;
    config->data_retained = true;
    return 0;
}
