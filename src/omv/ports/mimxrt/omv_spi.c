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
 * OMV SPI bus port for mimxrt.
 */
#include "omv_boardconfig.h"
#include "py/mphal.h"

#include "fsl_gpio.h"
#include "fsl_lpspi.h"
#include "fsl_iomuxc.h"
#include "fsl_lpspi_edma.h"
#include "fsl_dmamux.h"
#include CLOCK_CONFIG_H

#include "mimxrt_hal.h"
#include "omv_common.h"
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
    #if defined(OMV_SPI1_ID)
    { LPSPI1, OMV_SPI1_SSEL_PIN,
      { OMV_SPI1_DMA, OMV_SPI1_DMA_MUX, OMV_SPI1_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI1Tx },
      { OMV_SPI1_DMA, OMV_SPI1_DMA_MUX, OMV_SPI1_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI1Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif

    #if defined(OMV_SPI2_ID)
    { LPSPI2, OMV_SPI2_SSEL_PIN,
      { OMV_SPI2_DMA, OMV_SPI2_DMA_MUX, OMV_SPI2_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI2Tx },
      { OMV_SPI2_DMA, OMV_SPI2_DMA_MUX, OMV_SPI2_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI2Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif

    #if defined(OMV_SPI3_ID)
    { LPSPI3, OMV_SPI3_SSEL_PIN,
      { OMV_SPI3_DMA, OMV_SPI3_DMA_MUX, OMV_SPI3_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI3Tx },
      { OMV_SPI3_DMA, OMV_SPI3_DMA_MUX, OMV_SPI3_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI3Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif

    #if defined(OMV_SPI4_ID)
    { LPSPI4, OMV_SPI4_SSEL_PIN,
      { OMV_SPI4_DMA, OMV_SPI4_DMA_MUX, OMV_SPI4_DMA_TX_CHANNEL, kDmaRequestMuxLPSPI4Tx },
      { OMV_SPI4_DMA, OMV_SPI4_DMA_MUX, OMV_SPI4_DMA_RX_CHANNEL, kDmaRequestMuxLPSPI4Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 } },
    #endif
};

// Enable the SPI frame transfer complete at the end of the DMA transfer, when are absolutely sure that the whole
// DMA transfer is complete. This way we get one interrupt at the end of the transfer, vs after every SPI frame.
// descr_master.rxData must be set to null to ensure LPSPI_MasterTransferHandleIRQ clears the
// kLPSPI_TransferCompleteFlag and calls LPSPI_MasterTransferComplete which calls spi_master_callback.
static void EDMA_LpspiMasterTxCallback(edma_handle_t *edmaHandle, void *user, bool transferDone, uint32_t tcds) {
    omv_spi_t *spi = (omv_spi_t *) user;
    spi->descr_master_edma.state = (uint8_t) kLPSPI_Idle;
    spi->descr_master.rxData = NULL;
    LPSPI_EnableInterrupts(spi->inst, (uint32_t) kLPSPI_TransferCompleteFlag);
}

static void spi_master_callback(LPSPI_Type *base, void *handle, status_t status, void *user) {
    omv_spi_t *spi = (omv_spi_t *) user;

    if (status == kStatus_Success) {
        spi->xfer_flags |= OMV_SPI_XFER_COMPLETE;
    } else {
        spi->xfer_flags |= OMV_SPI_XFER_FAILED;
        spi->xfer_error = status;
        omv_spi_transfer_abort(spi);
    }

    void *buf = spi->xfer_descr.rxData;

    if (buf == NULL) {
        buf = spi->xfer_descr.txData;
    }

    // The IMXRT doesn't support half complete transfer interrupts (in the lpspi driver) like the STM32
    // does. So, mimick support for them by toggling the half flag.
    uint32_t flags = (OMV_SPI_XFER_DMA | OMV_SPI_XFER_COMPLETE);
    if ((spi->dma_flags & OMV_SPI_DMA_DOUBLE) && ((spi->xfer_flags & flags) == flags)) {
        int32_t offset = (spi->xfer_flags & OMV_SPI_XFER_HALF) ? -spi->xfer_descr.dataSize : spi->xfer_descr.dataSize;
        if (spi->xfer_descr.rxData) {
            spi->xfer_descr.rxData += offset;
        }
        spi->xfer_flags ^= OMV_SPI_XFER_HALF;
    }

    // Start the next DMA transfer before calling the callback. This minimizes the time
    // when DMA is not running. Also, if the callback aborts this will stop the DMA transfer.
    if ((spi->dma_flags & OMV_SPI_DMA_CIRCULAR) && ((spi->xfer_flags & flags) == flags)) {
        // Restart transfer for circular transfers. Note that we can't be interrupted again
        // by this until this callback finishes so it is okay to clear xfer complete later.
        LPSPI_MasterTransferEDMA(spi->inst, &spi->descr_master_edma, &spi->xfer_descr);
    }

    if (spi->callback) {
        spi->callback(spi, spi->userdata, buf);
    }

    // Clear after the callback so the callback gets the xfer complete flag set.
    // This needs to be cleared to prevent circular DMA from re-triggering on a failure.
    if (spi->dma_flags & OMV_SPI_DMA_CIRCULAR) {
        spi->xfer_flags &= ~(OMV_SPI_XFER_COMPLETE);
    }
}

int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer) {
    // No TX transfers in circular or double buffer mode.
    if ((spi->dma_flags & (OMV_SPI_DMA_CIRCULAR | OMV_SPI_DMA_DOUBLE)) && xfer->txbuf) {
        return -1;
    }

    spi->callback = xfer->callback;
    spi->userdata = xfer->userdata;
    spi->xfer_error = 0;
    spi->xfer_flags = xfer->flags;

    spi->xfer_descr.txData = xfer->txbuf;
    spi->xfer_descr.rxData = xfer->rxbuf;
    spi->xfer_descr.dataSize = xfer->size * (spi->config_backup.bitsPerFrame / 8);
    spi->xfer_descr.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;
    spi->xfer_flags &= ~(OMV_SPI_XFER_FAILED | OMV_SPI_XFER_COMPLETE | OMV_SPI_XFER_HALF);

    if (spi->dma_flags & OMV_SPI_DMA_DOUBLE) {
        spi->xfer_descr.dataSize /= 2;
    }

    if (spi->xfer_flags & OMV_SPI_XFER_DMA) {
        // DMA transfer (circular or one-shot)
        if (LPSPI_MasterTransferEDMA(spi->inst, &spi->descr_master_edma, &spi->xfer_descr) != kStatus_Success) {
            return -1;
        }
        if (xfer->txbuf) {
            // There isn't a race condition here to worry about since the major interrupt status flag
            // will remain asserted even if the DMA transfer was to complete before we enable the interrupt.
            EDMA_SetCallback(&spi->dma_descr_tx, EDMA_LpspiMasterTxCallback, spi);
            EDMA_EnableChannelInterrupts(spi->dma_descr_tx.base, spi->dma_descr_tx.channel,
                                         (uint32_t) kEDMA_MajorInterruptEnable);
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
        while (!(spi->xfer_flags & (OMV_SPI_XFER_COMPLETE | OMV_SPI_XFER_FAILED))) {
            if ((spi->xfer_flags & OMV_SPI_XFER_FAILED) ||
                ((mp_hal_ticks_ms() - start) > xfer->timeout)) {
                // The SPI bus was aborted by spi_master_callback.
                return -1;
            }
            MICROPY_EVENT_POLL_HOOK
        }
    } else {
        return -1;
    }
    return 0;
}

int omv_spi_transfer_abort(omv_spi_t *spi) {
    if (spi->dma_flags & (OMV_SPI_DMA_NORMAL | OMV_SPI_DMA_CIRCULAR)) {
        LPSPI_MasterTransferAbortEDMA(spi->inst, &spi->descr_master_edma);
    }
    // The SPI bus must be aborted too on LPSPI_MasterTransferAbortEDMA.
    LPSPI_MasterTransferAbort(spi->inst, &spi->descr_master);
    LPSPI_MasterInit(spi->inst, &spi->config_backup, BOARD_BOOTCLOCKRUN_LPSPI_CLK_ROOT);
    return 0;
}

static int omv_spi_dma_init(edma_handle_t *dma_handle, const dma_descr_t *dma_descr) {
    DMAMUX_SetSource(dma_descr->dma_mux, dma_descr->channel, dma_descr->request);
    DMAMUX_EnableChannel(dma_descr->dma_mux, dma_descr->channel);
    EDMA_CreateHandle(dma_handle, dma_descr->dma_inst, dma_descr->channel);
    return 0;
}

int omv_spi_init(omv_spi_t *spi, omv_spi_config_t *config) {
    memset(spi, 0, sizeof(omv_spi_t));

    const omv_spi_descr_t *spi_descr = &omv_spi_descr_all[config->id - 1];
    if (spi_descr->inst == NULL) {
        return -1;
    }

    spi->id = config->id;
    spi->inst = spi_descr->inst;
    spi->cs = spi_descr->cs;
    spi->dma_flags = config->dma_flags;

    lpspi_master_config_t spi_config;
    LPSPI_MasterGetDefaultConfig(&spi_config);

    spi_config.whichPcs = kLPSPI_Pcs0;
    spi_config.baudRate = config->baudrate;
    spi_config.bitsPerFrame = config->datasize;
    spi_config.direction = config->bit_order;
    spi_config.pcsActiveHighOrLow = config->nss_pol;
    spi_config.cpol = config->clk_pol;
    spi_config.cpha = config->clk_pha;
    spi_config.pcsToSckDelayInNanoSec = 0;
    spi_config.lastSckToPcsDelayInNanoSec = 0;
    spi_config.betweenTransferDelayInNanoSec = 0;
    spi_config.pinCfg = kLPSPI_SdiInSdoOut;
    spi_config.dataOutConfig = config->data_retained ? kLpspiDataOutRetained : kLpspiDataOutTristate;
    spi_config.enableInputDelay = false;
    LPSPI_MasterInit(spi->inst, &spi_config, BOARD_BOOTCLOCKRUN_LPSPI_CLK_ROOT);
    spi->config_backup = spi_config;

    // Configure pins.
    mimxrt_hal_spi_init(config->id, config->nss_enable, config->nss_pol);

    LPSPI_MasterTransferCreateHandle(
        spi->inst,
        &spi->descr_master,
        (lpspi_master_transfer_callback_t) spi_master_callback,
        spi);

    if (config->dma_flags & (OMV_SPI_DMA_NORMAL | OMV_SPI_DMA_CIRCULAR)) {
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
    }

    spi->initialized = true;
    return 0;
}

int omv_spi_deinit(omv_spi_t *spi) {
    if (spi && spi->initialized) {
        spi->initialized = false;
        omv_spi_transfer_abort(spi);
        if (spi->dma_flags & (OMV_SPI_DMA_NORMAL | OMV_SPI_DMA_CIRCULAR)) {
            const omv_spi_descr_t *spi_descr = &omv_spi_descr_all[spi->id - 1];
            DMAMUX_DisableChannel(spi_descr->dma_descr_tx.dma_mux, spi->dma_descr_tx.channel);
            DMAMUX_DisableChannel(spi_descr->dma_descr_rx.dma_mux, spi->dma_descr_rx.channel);
        }
        LPSPI_Deinit(spi->inst);
        mimxrt_hal_spi_deinit(spi->id);
    }
    return 0;
}

int omv_spi_set_baudrate(omv_spi_t *spi, uint32_t baudrate) {
    // LPSPI_MasterSetBaudRate doesn't work. Change the baudrate via a reinit here.
    spi->config_backup.baudRate = baudrate;
    omv_spi_transfer_abort(spi);
    return 0;
}

int omv_spi_default_config(omv_spi_config_t *config, uint32_t bus_id) {
    config->id = bus_id;
    config->baudrate = 10000000;
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
