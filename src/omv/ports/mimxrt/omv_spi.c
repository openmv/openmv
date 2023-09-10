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
#include "py/mphal.h"

#include "fsl_gpio.h"
#include "fsl_lpspi.h"
#include "fsl_iomuxc.h"
#include "fsl_lpspi_edma.h"
#include "fsl_dmamux.h"
#include CLOCK_CONFIG_H
#include "dma_manager.h"

#include "mimxrt_hal.h"
#include "omv_common.h"
#include "omv_spi.h"

typedef struct dma_descr {
    DMA_Type *dma_inst;
    DMAMUX_Type *dma_mux;
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
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI1Tx },
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI1Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0 }, { NULL, NULL, 0 } },
    #endif

    #if defined(LPSPI2_ID)
    { LPSPI2, LPSPI2_SSEL_PIN,
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI2Tx },
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI2Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0 }, { NULL, NULL, 0 } },
    #endif

    #if defined(LPSPI3_ID)
    { LPSPI3, LPSPI3_SSEL_PIN,
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI3Tx },
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI3Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0 }, { NULL, NULL, 0 } },
    #endif

    #if defined(LPSPI4_ID)
    { LPSPI4, LPSPI4_SSEL_PIN,
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI4Tx },
      { DMA0, DMAMUX, kDmaRequestMuxLPSPI4Rx } },
    #else
    { NULL, NULL, { NULL, NULL, 0 }, { NULL, NULL, 0 } },
    #endif
};

// LPSPI_MasterTransferEDMA doesn't have any code at all to generate a callback on transmit complete. So, we
// add one here that simulates that. However, the DMA interrupt will occur before the SPI bus is done sending
// data. So, we cannot call spi_master_callback below as it will happen before the data is actually sent. Instead,
// we use the SPI transfer complete interrupt to call spi_master_callback at the right time. Note that we have
// to zero out the descriptor to prevent LPSPI_MasterTransferHandleIRQ from doing anything we don't want it to do.
static void EDMA_LpspiMasterTxCallback(edma_handle_t *edmaHandle, void *user, bool transferDone, uint32_t tcds) {
    omv_spi_t *spi = (omv_spi_t *) user;
    spi->descr_master_edma.state = (uint8_t) kLPSPI_Idle;
    spi->descr_master.state = (uint8_t) kLPSPI_Idle;
    spi->descr_master.txData = NULL;
    spi->descr_master.rxData = NULL;
    spi->descr_master.txRemainingByteCount = 0;
    spi->descr_master.rxRemainingByteCount = 0;
    spi->descr_master.totalByteCount = 0;
    spi->descr_master.writeTcrInIsr = false;
    spi->descr_master.bytesPerFrame = 0;
    spi->descr_master.writeRegRemainingTimes = 0;
    spi->descr_master.readRegRemainingTimes = 0;
    spi->descr_master.txBuffIfNull = 0;
    spi->descr_master.fifoSize = 0;
    spi->descr_master.isPcsContinuous = 0;
    spi->descr_master.isByteSwap = 0;
    spi->descr_master.bytesEachWrite = 0;
    spi->descr_master.bytesEachRead = 0;
    spi->descr_master.rxWatermark = 0;
    spi->descr_master.isTxMask = 0;
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

    void *rxBuf = spi->xfer_descr.rxData;

    // The IMXRT doesn't support half complete transfer interrupts (in the lpspi driver) like the STM32
    // does. So, mimick support for them by toggling the half flag.
    uint32_t flags = (OMV_SPI_XFER_DMA | OMV_SPI_XFER_COMPLETE);
    if ((spi->dma_flags & OMV_SPI_DMA_DOUBLE) && ((spi->xfer_flags & flags) == flags)) {
        if (spi->xfer_flags & OMV_SPI_XFER_HALF) {
            spi->xfer_flags &= ~OMV_SPI_XFER_HALF;
            if (spi->xfer_descr.txData) {
                spi->xfer_descr.txData -= spi->xfer_descr.dataSize;
            }
            if (spi->xfer_descr.rxData) {
                spi->xfer_descr.rxData -= spi->xfer_descr.dataSize;
            }
        } else {
            spi->xfer_flags |= OMV_SPI_XFER_HALF;
            if (spi->xfer_descr.txData) {
                spi->xfer_descr.txData += spi->xfer_descr.dataSize;
            }
            if (spi->xfer_descr.rxData) {
                spi->xfer_descr.rxData += spi->xfer_descr.dataSize;
            }
        }
    }

    // Start the next DMA transfer before calling the callback. This minimizes the time
    // when DMA is not running. Also, if the callback aborts this will stop the DMA transfer.
    if ((spi->dma_flags & OMV_SPI_DMA_CIRCULAR) && ((spi->xfer_flags & flags) == flags)) {
        // Restart transfer for circular transfers. Note that we can't be interrupted again
        // by this until this callback finishes so it is okay to clear xfer complete later.
        LPSPI_MasterTransferEDMA(spi->inst, &spi->descr_master_edma, &spi->xfer_descr);
    }

    if (spi->callback) {
        spi->callback(spi, spi->userdata, rxBuf);
    }

    // Clear after the callback so the callback gets the xfer complete flag set.
    // This needs to be cleared to prevent circular DMA from re-triggering on a failure.
    spi->xfer_flags &= ~(OMV_SPI_XFER_COMPLETE);
}

// The LPSPI_MasterTransferNonBlocking function call does not clear the LPSPI_TCR_FRAMESZ_MASK bit
// even if kLPSPI_MasterByteSwap is not set in the xfer. So, we have to clear this bit at the same
// time as changing the framesize. Splitting this will result in inconsitent settings. Finally,
// the TCR register doesn't always update itself correctly so you have to try in a loop...
void omv_spi_set_frame_size(omv_spi_t *spi, uint16_t size) {
    for (;;) {
        uint32_t tcr = spi->inst->TCR;
        uint16_t s = (uint16_t) ((tcr & LPSPI_TCR_FRAMESZ_MASK) >> LPSPI_TCR_FRAMESZ_SHIFT) + 1U;
        if (s != size) {
            spi->inst->TCR = (tcr & ~(LPSPI_TCR_BYSW_MASK | LPSPI_TCR_FRAMESZ_MASK)) | LPSPI_TCR_FRAMESZ(size - 1U);
        } else {
            break;
        }
    }
}

int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer) {
    spi->callback = xfer->callback;
    spi->userdata = xfer->userdata;
    spi->xfer_error = 0;
    spi->xfer_flags = xfer->flags;

    uint32_t datasize = 8;
    if (xfer->flags & OMV_SPI_XFER_16_BIT) {
        datasize = 16;
    }

    omv_spi_set_frame_size(spi, datasize);

    spi->xfer_descr.txData = xfer->txbuf;
    spi->xfer_descr.rxData = xfer->rxbuf;
    spi->xfer_descr.dataSize = xfer->size;
    spi->xfer_descr.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;
    spi->xfer_flags &= ~(OMV_SPI_XFER_FAILED | OMV_SPI_XFER_COMPLETE);

    if (xfer->flags & OMV_SPI_XFER_16_BIT) {
        spi->xfer_descr.dataSize *= 2;
    }

    if (spi->dma_flags & OMV_SPI_DMA_DOUBLE) {
        spi->xfer_descr.dataSize /= 2;
    }

    // Check if we can optimize the transfer by doing things in 32-bit chunks.
    if ((!(spi->xfer_descr.dataSize % 4)) && (!(xfer->flags & OMV_SPI_XFER_16_BIT))) {
        omv_spi_set_frame_size(spi, 32);
        spi->xfer_descr.configFlags |= kLPSPI_MasterByteSwap;
    }

    if (spi->xfer_flags & OMV_SPI_XFER_BLOCKING) {
        if (LPSPI_MasterTransferBlocking(spi->inst, &spi->xfer_descr) != kStatus_Success) {
            return -1;
        }
    } else if (spi->xfer_flags & OMV_SPI_XFER_NONBLOCK) {
        if (LPSPI_MasterTransferNonBlocking(spi->inst, &spi->descr_master, &spi->xfer_descr) != kStatus_Success) {
            return -1;
        }
    } else if (spi->xfer_flags & OMV_SPI_XFER_DMA) {
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
    } else {
        return -1;
    }
    return 0;
}

int omv_spi_transfer_abort(omv_spi_t *spi) {
    if (spi->dma_flags & (OMV_SPI_DMA_NORMAL | OMV_SPI_DMA_CIRCULAR)) {
        LPSPI_MasterTransferAbortEDMA(spi->inst, &spi->descr_master_edma);
    }
    LPSPI_MasterTransferAbort(spi->inst, &spi->descr_master);
    LPSPI_MasterInit(spi->inst, &spi->config_backup, BOARD_BOOTCLOCKRUN_LPSPI_CLK_ROOT);
    return 0;
}

static int omv_spi_dma_init(edma_handle_t *dma_handle, const dma_descr_t *dma_descr) {
    int channel = allocate_dma_channel();
    DMAMUX_SetSource(dma_descr->dma_mux, channel, dma_descr->request);
    DMAMUX_EnableChannel(dma_descr->dma_mux, channel);
    EDMA_CreateHandle(dma_handle, dma_descr->dma_inst, channel);
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
    spi_config.bitsPerFrame = 8;
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
            free_dma_channel(spi->dma_descr_tx.channel);
            DMAMUX_DisableChannel(spi_descr->dma_descr_rx.dma_mux, spi->dma_descr_rx.channel);
            free_dma_channel(spi->dma_descr_rx.channel);
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
    config->spi_mode = OMV_SPI_MODE_MASTER;
    config->bus_mode = OMV_SPI_BUS_TX_RX;
    config->bit_order = OMV_SPI_MSB_FIRST;
    config->clk_pol = OMV_SPI_CPOL_HIGH;
    config->clk_pha = OMV_SPI_CPHA_1EDGE;
    config->nss_pol = OMV_SPI_NSS_LOW;
    config->nss_enable = true;
    config->dma_flags = 0;
    config->data_retained = true;
    return 0;
}
