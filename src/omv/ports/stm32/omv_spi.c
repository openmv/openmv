/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OMV SPI port for stm32.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include STM32_HAL_H
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "irq.h"
#include "common.h"
#include "dma_utils.h"
#include "omv_spi.h"

// If an SPI handle is already defined in MicroPython, reuse that handle to allow
// MicroPython to process the SPI IRQ, otherwise define the IRQ handler and its handle.
#define DEFINE_SPI_INSTANCE(n)             \
    static SPI_HandleTypeDef SPIHandle##n; \
    void SPI##n##_IRQHandler(void) { HAL_SPI_IRQHandler(&SPIHandle##n); }

#if defined(SPI1_ID) && defined(MICROPY_HW_SPI1_SCK)
extern SPI_HandleTypeDef SPIHandle1;
#elif defined(SPI1_ID)
DEFINE_SPI_INSTANCE(1)
#endif

#if defined(SPI2_ID) && defined(MICROPY_HW_SPI2_SCK)
extern SPI_HandleTypeDef SPIHandle2;
#elif defined(SPI2_ID)
DEFINE_SPI_INSTANCE(2)
#endif

#if defined(SPI3_ID) && defined(MICROPY_HW_SPI3_SCK)
extern SPI_HandleTypeDef SPIHandle3;
#elif defined(SPI3_ID)
DEFINE_SPI_INSTANCE(3)
#endif

#if defined(SPI4_ID) && defined(MICROPY_HW_SPI4_SCK)
extern SPI_HandleTypeDef SPIHandle4;
#elif defined(SPI4_ID)
DEFINE_SPI_INSTANCE(4)
#endif

#if defined(SPI5_ID) && defined(MICROPY_HW_SPI5_SCK)
extern SPI_HandleTypeDef SPIHandle5;
#elif defined(SPI5_ID)
DEFINE_SPI_INSTANCE(5)
#endif

#if defined(SPI6_ID) && defined(MICROPY_HW_SPI6_SCK)
extern SPI_HandleTypeDef SPIHandle6;
#elif defined(SPI6_ID)
DEFINE_SPI_INSTANCE(6)
#endif

#define INITIALIZE_SPI_DESCR(spi, spi_number)                                   \
    do {                                                                        \
        (spi)->id = spi_number;                                                 \
        (spi)->irqn = SPI##spi_number##_IRQn;                                   \
        (spi)->cs = SPI##spi_number##_SSEL_PIN;                                 \
        (spi)->descr = &SPIHandle##spi_number;                                  \
        (spi)->descr->Instance = SPI##spi_number;                               \
        (spi)->dma_descr_tx = (DMA_HandleTypeDef)                               \
        {SPI##spi_number##_DMA_TX_CHANNEL, {DMA_REQUEST_SPI##spi_number##_TX}}; \
        (spi)->dma_descr_rx = (DMA_HandleTypeDef)                               \
        {SPI##spi_number##_DMA_RX_CHANNEL, {DMA_REQUEST_SPI##spi_number##_RX}}; \
    } while (0)

static omv_spi_t *omv_spi_descr_all[6] = { NULL };

static uint32_t omv_spi_clocksource(SPI_TypeDef *spi) {
    #if defined(MCU_SERIES_H7)
    if (spi == SPI1 || spi == SPI2 || spi == SPI3) {
        return HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
    } else if (spi == SPI4 || spi == SPI5) {
        return HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI45);
    } else {
        return HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI6);
    }
    #else
    if (spi == SPI2) {
        // SPI2 is on APB1
        return HAL_RCC_GetPCLK1Freq();
    } else if (spi == SPI3) {
        // SPI3 is on APB1
        return HAL_RCC_GetPCLK1Freq();
    } else {
        // SPI1, SPI4, SPI5 and SPI6 are on APB2
        return HAL_RCC_GetPCLK2Freq();
    }
    #endif
}

static int omv_spi_prescaler(SPI_TypeDef *spi, uint32_t baudrate) {
    const uint32_t prescalers[] = {
        SPI_BAUDRATEPRESCALER_2,
        SPI_BAUDRATEPRESCALER_4,
        SPI_BAUDRATEPRESCALER_8,
        SPI_BAUDRATEPRESCALER_16,
        SPI_BAUDRATEPRESCALER_32,
        SPI_BAUDRATEPRESCALER_64,
        SPI_BAUDRATEPRESCALER_128,
        SPI_BAUDRATEPRESCALER_256
    };

    uint32_t clksource = omv_spi_clocksource(spi);

    for (size_t i = 0; i < OMV_ARRAY_SIZE(prescalers); i++) {
        uint32_t target = clksource / ((2 << i));
        if (target <= baudrate) {
            return prescalers[i];
        }
    }

    return SPI_BAUDRATEPRESCALER_256;
}

static void omv_spi_callback(SPI_HandleTypeDef *hspi) {
    omv_spi_t *spi = NULL;
    if (0) {
    #if defined(SPI1_ID)
    } else if (hspi->Instance == SPI1) {
        spi = omv_spi_descr_all[0];
    #endif
    #if defined(SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        spi = omv_spi_descr_all[1];
    #endif
    #if defined(SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        spi = omv_spi_descr_all[2];
    #endif
    #if defined(SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        spi = omv_spi_descr_all[3];
    #endif
    #if defined(SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        spi = omv_spi_descr_all[4];
    #endif
    #if defined(SPI6_ID)
    } else if (hspi->Instance == SPI6) {
        spi = omv_spi_descr_all[5];
    #endif
    }
    if (spi == NULL) {
        return;
    }

    if (hspi->ErrorCode != HAL_SPI_ERROR_NONE) {
        spi->xfer_flags |= OMV_SPI_XFER_FAILED;
        spi->xfer_error = hspi->ErrorCode;
        omv_spi_transfer_abort(spi);
    }

    if (spi->callback) {
        spi->callback(spi, spi->userdata);
    }
}

int omv_spi_transfer_start(omv_spi_t *spi, omv_spi_transfer_t *xfer) {
    spi->callback = xfer->callback;
    spi->userdata = xfer->userdata;
    spi->xfer_error = 0;
    spi->xfer_flags = xfer->flags;

    if (spi->xfer_flags & OMV_SPI_XFER_BLOCKING) {
        if (xfer->txbuf && xfer->rxbuf) {
            if (HAL_SPI_TransmitReceive(spi->descr, xfer->txbuf,
                                        xfer->rxbuf, xfer->size, xfer->timeout) != HAL_OK) {
                return -1;
            }
        } else if (xfer->txbuf) {
            if (HAL_SPI_Transmit(spi->descr, xfer->txbuf, xfer->size, xfer->timeout) != HAL_OK) {
                return -1;
            }
        } else if (xfer->rxbuf) {
            if (HAL_SPI_Receive(spi->descr, xfer->rxbuf, xfer->size, xfer->timeout) != HAL_OK) {
                return -1;
            }
        }
    } else if (spi->xfer_flags & OMV_SPI_XFER_DMA) {
        if (xfer->txbuf && xfer->rxbuf) {
            if (HAL_SPI_TransmitReceive_DMA(spi->descr, xfer->txbuf,
                                            xfer->rxbuf, xfer->size) != HAL_OK) {
                return -1;
            }
        } else if (xfer->txbuf) {
            if (HAL_SPI_Transmit_DMA(spi->descr, xfer->txbuf, xfer->size) != HAL_OK) {
                return -1;
            }
        } else if (xfer->rxbuf) {
            if (HAL_SPI_Receive_DMA(spi->descr, xfer->rxbuf, xfer->size) != HAL_OK) {
                return -1;
            }
        }
    } else {
        return -1;
    }
    return 0;
}

int omv_spi_transfer_abort(omv_spi_t *spi) {
    HAL_NVIC_DisableIRQ(spi->irqn);
    HAL_SPI_Abort(spi->descr);
    return 0;
}

static int omv_spi_dma_init(omv_spi_t *spi, uint32_t direction) {
    DMA_HandleTypeDef *dma_descr;

    if (direction == DMA_MEMORY_TO_PERIPH) {
        dma_descr = &spi->dma_descr_tx;
    } else {
        dma_descr = &spi->dma_descr_rx;
    }

    // Configure the SPI DMA steam.
    dma_descr->Init.Mode = DMA_CIRCULAR;                // TODO FIX
    dma_descr->Init.Priority = DMA_PRIORITY_HIGH;
    dma_descr->Init.Direction = direction;
    // When the DMA is configured in direct mode (the FIFO is disabled), the source and
    // destination transfer widths are equal, and both defined by PSIZE (MSIZE is ignored).
    // Additionally, burst transfers are not possible (MBURST and PBURST are both ignored).
    dma_descr->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_descr->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    // Note MBURST and PBURST are ignored.
    dma_descr->Init.MemBurst = DMA_MBURST_INC4;
    dma_descr->Init.PeriphBurst = DMA_PBURST_INC4;
    dma_descr->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_descr->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_descr->Init.MemInc = DMA_MINC_ENABLE;
    dma_descr->Init.PeriphInc = DMA_PINC_DISABLE;

    // Initialize the DMA stream
    HAL_DMA_DeInit(dma_descr);
    if (HAL_DMA_Init(dma_descr) != HAL_OK) {
        // Initialization Error
        return -1;
    }

    // Link the DMA handle to SPI handle.
    if (direction == DMA_MEMORY_TO_PERIPH) {
        __HAL_LINKDMA(spi->descr, hdmatx, spi->dma_descr_tx);
    } else {
        __HAL_LINKDMA(spi->descr, hdmarx, spi->dma_descr_rx);
    }
    // Set the SPI handle used by the DMA channel's IRQ handler.
    dma_utils_set_irq_descr(dma_descr->Instance, dma_descr);

    // Get DMA channel's IRQ number.
    uint8_t dma_irqn = dma_utils_channel_to_irqn(dma_descr->Instance);

    // Configure and enable DMA IRQ channel.
    NVIC_SetPriority(dma_irqn, IRQ_PRI_DMA21);
    HAL_NVIC_EnableIRQ(dma_irqn);

    return 0;
}

static int omv_spi_bus_init(omv_spi_t *spi, omv_spi_config_t *config) {
    SPI_HandleTypeDef *spi_descr = spi->descr;

    spi_descr->Init.Mode = config->spi_mode;
    spi_descr->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_descr->Init.NSS = (config->nss_enable == false) ? SPI_NSS_SOFT : SPI_NSS_HARD_OUTPUT;
    spi_descr->Init.DataSize = (config->datasize == 8) ? SPI_DATASIZE_8BIT : SPI_DATASIZE_16BIT;
    spi_descr->Init.FirstBit = config->bit_order;
    spi_descr->Init.CLKPhase = config->clk_pha;
    spi_descr->Init.CLKPolarity = config->clk_pol;
    spi_descr->Init.BaudRatePrescaler = omv_spi_prescaler(spi_descr->Instance, config->baudrate);
    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
    spi_descr->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    #if defined(MCU_SERIES_H7)
    spi_descr->Init.NSSPolarity = (config->nss_pol == 0) ? SPI_NSS_POLARITY_LOW : SPI_NSS_POLARITY_HIGH;
    spi_descr->Init.FifoThreshold = SPI_FIFO_THRESHOLD_04DATA;
    spi_descr->Init.MasterKeepIOState = (config->data_retained == true) ?
                                        SPI_MASTER_KEEP_IO_STATE_ENABLE : SPI_MASTER_KEEP_IO_STATE_ENABLE;
    #endif
    #endif

    // Configure bus direction.
    if (config->bus_mode == OMV_SPI_BUS_TX_RX) {
        spi_descr->Init.Direction = SPI_DIRECTION_2LINES;
    } else if (config->bus_mode == OMV_SPI_BUS_RX) {
        spi_descr->Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
    } else {
        #if defined(MCU_SERIES_H7)
        spi_descr->Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
        #else
        spi_descr->Init.Direction = SPI_DIRECTION_1LINE;
        #endif
    }

    if (HAL_SPI_Init(spi_descr) != HAL_OK) {
        HAL_SPI_DeInit(spi_descr);
        return -1;
    }
    return 0;
}

int omv_spi_init(omv_spi_t *spi, omv_spi_config_t *config) {
    memset(spi, 0, sizeof(omv_spi_t));

    if (0) {
    #if defined(SPI1_ID)
    } else if (config->id == 1) {
        INITIALIZE_SPI_DESCR(spi, 1);
    #endif
    #if defined(SPI2_ID)
    } else if (config->id == 2) {
        INITIALIZE_SPI_DESCR(spi, 2);
    #endif
    #if defined(SPI3_ID)
    } else if (config->id == 3) {
        INITIALIZE_SPI_DESCR(spi, 3);
    #endif
    #if defined(SPI4_ID)
    } else if (config->id == 4) {
        INITIALIZE_SPI_DESCR(spi, 4);
    #endif
    #if defined(SPI5_ID)
    } else if (config->id == 5) {
        INITIALIZE_SPI_DESCR(spi, 5);
    #endif
    #if defined(SPI6_ID)
    } else if (config->id == 6) {
        INITIALIZE_SPI_DESCR(spi, 6);
    #endif
    } else {
        return -1;
    }

    if (omv_spi_bus_init(spi, config) != 0) {
        return -1;
    }

    if (config->dma_enable) {
        if (config->bus_mode & OMV_SPI_BUS_TX) {
            omv_spi_dma_init(spi, DMA_MEMORY_TO_PERIPH);
        }
        if (config->bus_mode & OMV_SPI_BUS_RX) {
            omv_spi_dma_init(spi, DMA_PERIPH_TO_MEMORY);
        }
        // Configure and enable SPI IRQ channel.
        NVIC_SetPriority(spi->irqn, IRQ_PRI_DCMI);// TODO use lower priority
        HAL_NVIC_EnableIRQ(spi->irqn);
    }

    // Install TX/RX callbacks even if DMA mode is not enabled for non-blocking transfers.
    if (config->bus_mode & OMV_SPI_BUS_TX) {
        HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_TX_COMPLETE_CB_ID, omv_spi_callback);
    }

    if (config->bus_mode & OMV_SPI_BUS_RX) {
        HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_RX_COMPLETE_CB_ID, omv_spi_callback);
    }

    spi->initialized = true;
    spi->dma_enabled = config->dma_enable;
    omv_spi_descr_all[config->id - 1] = spi;
    return 0;
}

int omv_spi_deinit(omv_spi_t *spi) {
    if (spi && spi->initialized) {
        spi->initialized = false;
        omv_spi_descr_all[spi->id - 1] = NULL;
        omv_spi_transfer_abort(spi);
        if (spi->dma_enabled) {
            // TODO: Deinit DMA
        }
        HAL_SPI_DeInit(spi->descr);
    }
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
    config->dma_enable = false;
    config->data_retained = true;
    return 0;
}
