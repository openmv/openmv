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
 * OMV SPI port for stm32.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include STM32_HAL_H
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "irq.h"
#include "omv_common.h"
#include "dma_utils.h"
#include "omv_gpio.h"
#include "omv_spi.h"

// If an SPI handle is already defined in MicroPython, reuse that handle to allow
// MicroPython to process the SPI IRQ, otherwise define the IRQ handler and its handle.
#define DEFINE_SPI_INSTANCE(n)             \
    static SPI_HandleTypeDef SPIHandle##n; \
    void SPI##n##_IRQHandler(void) { HAL_SPI_IRQHandler(&SPIHandle##n); }

#if defined(OMV_SPI1_ID) && defined(MICROPY_HW_SPI1_SCK)
extern SPI_HandleTypeDef SPIHandle1;
#elif defined(OMV_SPI1_ID)
DEFINE_SPI_INSTANCE(1)
#endif

#if defined(OMV_SPI2_ID) && defined(MICROPY_HW_SPI2_SCK)
extern SPI_HandleTypeDef SPIHandle2;
#elif defined(OMV_SPI2_ID)
DEFINE_SPI_INSTANCE(2)
#endif

#if defined(OMV_SPI3_ID) && defined(MICROPY_HW_SPI3_SCK)
extern SPI_HandleTypeDef SPIHandle3;
#elif defined(OMV_SPI3_ID)
DEFINE_SPI_INSTANCE(3)
#endif

#if defined(OMV_SPI4_ID) && defined(MICROPY_HW_SPI4_SCK)
extern SPI_HandleTypeDef SPIHandle4;
#elif defined(OMV_SPI4_ID)
DEFINE_SPI_INSTANCE(4)
#endif

#if defined(OMV_SPI5_ID) && defined(MICROPY_HW_SPI5_SCK)
extern SPI_HandleTypeDef SPIHandle5;
#elif defined(OMV_SPI5_ID)
DEFINE_SPI_INSTANCE(5)
#endif

#if defined(OMV_SPI6_ID) && defined(MICROPY_HW_SPI6_SCK)
extern SPI_HandleTypeDef SPIHandle6;
#elif defined(OMV_SPI6_ID)
DEFINE_SPI_INSTANCE(6)
#endif

#define INITIALIZE_SPI_DESCR(spi, spi_number)                                               \
    do {                                                                                    \
        (spi)->id = spi_number;                                                             \
        (spi)->irqn = SPI##spi_number##_IRQn;                                               \
        (spi)->cs = OMV_SPI##spi_number##_SSEL_PIN;                                         \
        (spi)->descr = &SPIHandle##spi_number;                                              \
        (spi)->descr->Instance = SPI##spi_number;                                           \
        (spi)->dma_descr_tx = (DMA_HandleTypeDef)                                           \
        { OMV_SPI##spi_number##_DMA_TX_CHANNEL, { OMV_SPI##spi_number##_DMA_TX_REQUEST } }; \
        (spi)->dma_descr_rx = (DMA_HandleTypeDef)                                           \
        { OMV_SPI##spi_number##_DMA_RX_CHANNEL, { OMV_SPI##spi_number##_DMA_RX_REQUEST } }; \
    } while (0)

static omv_spi_t *omv_spi_descr_all[6] = { NULL };

static uint32_t omv_spi_clocksource(SPI_TypeDef *spi) {
    #if defined(STM32H7)
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
    #if defined(OMV_SPI1_ID)
    } else if (hspi->Instance == SPI1) {
        spi = omv_spi_descr_all[0];
    #endif
    #if defined(OMV_SPI2_ID)
    } else if (hspi->Instance == SPI2) {
        spi = omv_spi_descr_all[1];
    #endif
    #if defined(OMV_SPI3_ID)
    } else if (hspi->Instance == SPI3) {
        spi = omv_spi_descr_all[2];
    #endif
    #if defined(OMV_SPI4_ID)
    } else if (hspi->Instance == SPI4) {
        spi = omv_spi_descr_all[3];
    #endif
    #if defined(OMV_SPI5_ID)
    } else if (hspi->Instance == SPI5) {
        spi = omv_spi_descr_all[4];
    #endif
    #if defined(OMV_SPI6_ID)
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
    } else {
        spi->xfer_flags |= OMV_SPI_XFER_COMPLETE;
    }

    if (spi->callback) {
        uint8_t *buf = spi->descr->pRxBuffPtr ? spi->descr->pRxBuffPtr : spi->descr->pTxBuffPtr;
        if (spi->dma_flags & OMV_SPI_DMA_DOUBLE) {
            if (spi->xfer_flags & OMV_SPI_XFER_HALF) {
                uint32_t size = spi->descr->RxXferSize ? spi->descr->RxXferSize : spi->descr->TxXferSize;
                buf += (size * ((spi->descr->Init.DataSize == SPI_DATASIZE_8BIT) ? 1 : 2)) / 2;
            }
            spi->xfer_flags ^= OMV_SPI_XFER_HALF;
        }
        spi->callback(spi, spi->userdata, buf);
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

    spi->xfer_flags &= ~(OMV_SPI_XFER_FAILED | OMV_SPI_XFER_COMPLETE | OMV_SPI_XFER_HALF);

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
    } else if (spi->xfer_flags & OMV_SPI_XFER_NONBLOCK) {
        if (xfer->txbuf && xfer->rxbuf) {
            if (HAL_SPI_TransmitReceive_IT(spi->descr, xfer->txbuf,
                                           xfer->rxbuf, xfer->size) != HAL_OK) {
                return -1;
            }
        } else if (xfer->txbuf) {
            if (HAL_SPI_Transmit_IT(spi->descr, xfer->txbuf, xfer->size) != HAL_OK) {
                return -1;
            }
        } else if (xfer->rxbuf) {
            if (HAL_SPI_Receive_IT(spi->descr, xfer->rxbuf, xfer->size) != HAL_OK) {
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
    if (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) {
        HAL_SPI_Abort_IT(spi->descr);
    } else {
        HAL_SPI_Abort(spi->descr);
    }
    return 0;
}

static int omv_spi_dma_init(omv_spi_t *spi, uint32_t direction, omv_spi_config_t *config) {
    DMA_HandleTypeDef *dma_descr;

    if (direction == DMA_MEMORY_TO_PERIPH) {
        dma_descr = &spi->dma_descr_tx;
    } else {
        dma_descr = &spi->dma_descr_rx;
    }

    // Configure the SPI DMA steam.
    dma_descr->Init.Mode = (config->dma_flags & OMV_SPI_DMA_CIRCULAR) ? DMA_CIRCULAR : DMA_NORMAL;
    dma_descr->Init.Priority = DMA_PRIORITY_HIGH;
    dma_descr->Init.Direction = direction;
    // When the DMA is configured in direct mode (the FIFO is disabled), the source and
    // destination transfer widths are equal, and both defined by PSIZE (MSIZE is ignored).
    // Additionally, burst transfers are not possible (MBURST and PBURST are both ignored).
    dma_descr->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_descr->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    // Note MBURST and PBURST are ignored.
    dma_descr->Init.MemBurst = DMA_MBURST_SINGLE;
    dma_descr->Init.PeriphBurst = DMA_PBURST_SINGLE;
    dma_descr->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    #if defined(STM32H7)
    dma_descr->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    #else
    dma_descr->Init.PeriphDataAlignment = (config->datasize == 8) ? DMA_PDATAALIGN_BYTE : DMA_PDATAALIGN_HALFWORD;
    #endif
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
    NVIC_SetPriority(dma_irqn, IRQ_PRI_DMA);
    HAL_NVIC_EnableIRQ(dma_irqn);

    return 0;
}

static int omv_spi_bus_init(omv_spi_t *spi, omv_spi_config_t *config) {
    SPI_HandleTypeDef *spi_descr = spi->descr;

    spi_descr->Init.Mode = config->spi_mode;
    spi_descr->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_descr->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_descr->Init.NSS = (config->nss_enable == false) ? SPI_NSS_SOFT : SPI_NSS_HARD_OUTPUT;
    spi_descr->Init.DataSize = (config->datasize == 8) ? SPI_DATASIZE_8BIT : SPI_DATASIZE_16BIT;
    spi_descr->Init.FirstBit = config->bit_order;
    spi_descr->Init.CLKPhase = config->clk_pha;
    spi_descr->Init.CLKPolarity = config->clk_pol;
    spi_descr->Init.BaudRatePrescaler = omv_spi_prescaler(spi_descr->Instance, config->baudrate);
    #if defined(STM32F7) || defined(STM32H7)
    spi_descr->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    #if defined(STM32H7)
    spi_descr->Init.NSSPolarity = (config->nss_pol == 0) ? SPI_NSS_POLARITY_LOW : SPI_NSS_POLARITY_HIGH;
    spi_descr->Init.FifoThreshold = SPI_FIFO_THRESHOLD_04DATA;
    spi_descr->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    spi_descr->Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    spi_descr->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    spi_descr->Init.MasterKeepIOState = (config->data_retained == true) ?
                                        SPI_MASTER_KEEP_IO_STATE_ENABLE : SPI_MASTER_KEEP_IO_STATE_DISABLE;
    spi_descr->Init.IOSwap = SPI_IO_SWAP_DISABLE;
    #endif
    #endif

    // Configure bus direction.
    if (config->bus_mode == OMV_SPI_BUS_TX_RX) {
        spi_descr->Init.Direction = SPI_DIRECTION_2LINES;
    } else if (config->bus_mode == OMV_SPI_BUS_RX) {
        spi_descr->Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
    } else {
        #if defined(STM32H7)
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
    #if defined(OMV_SPI1_ID)
    } else if (config->id == 1) {
        INITIALIZE_SPI_DESCR(spi, 1);
    #endif
    #if defined(OMV_SPI2_ID)
    } else if (config->id == 2) {
        INITIALIZE_SPI_DESCR(spi, 2);
    #endif
    #if defined(OMV_SPI3_ID)
    } else if (config->id == 3) {
        INITIALIZE_SPI_DESCR(spi, 3);
    #endif
    #if defined(OMV_SPI4_ID)
    } else if (config->id == 4) {
        INITIALIZE_SPI_DESCR(spi, 4);
    #endif
    #if defined(OMV_SPI5_ID)
    } else if (config->id == 5) {
        INITIALIZE_SPI_DESCR(spi, 5);
    #endif
    #if defined(OMV_SPI6_ID)
    } else if (config->id == 6) {
        INITIALIZE_SPI_DESCR(spi, 6);
    #endif
    } else {
        return -1;
    }

    if (omv_spi_bus_init(spi, config) != 0) {
        return -1;
    }

    if (config->dma_flags & (OMV_SPI_DMA_NORMAL | OMV_SPI_DMA_CIRCULAR)) {
        if (config->bus_mode & OMV_SPI_BUS_TX) {
            omv_spi_dma_init(spi, DMA_MEMORY_TO_PERIPH, config);
        }
        if (config->bus_mode & OMV_SPI_BUS_RX) {
            omv_spi_dma_init(spi, DMA_PERIPH_TO_MEMORY, config);
        }
    }
    // Configure and enable SPI IRQ channel.
    NVIC_SetPriority(spi->irqn, IRQ_PRI_SPI);
    HAL_NVIC_EnableIRQ(spi->irqn);

    // Install TX/RX callbacks even if DMA mode is not enabled for non-blocking transfers.
    HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_TX_RX_COMPLETE_CB_ID, omv_spi_callback);
    HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_TX_COMPLETE_CB_ID, omv_spi_callback);
    HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_RX_COMPLETE_CB_ID, omv_spi_callback);
    if (config->dma_flags & OMV_SPI_DMA_DOUBLE) {
        HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_TX_RX_HALF_COMPLETE_CB_ID, omv_spi_callback);
        HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_TX_HALF_COMPLETE_CB_ID, omv_spi_callback);
        HAL_SPI_RegisterCallback(spi->descr, HAL_SPI_RX_HALF_COMPLETE_CB_ID, omv_spi_callback);
    }

    spi->initialized = true;
    spi->dma_flags = config->dma_flags;
    omv_spi_descr_all[config->id - 1] = spi;
    return 0;
}

int omv_spi_deinit(omv_spi_t *spi) {
    if (spi && spi->initialized) {
        spi->initialized = false;
        omv_spi_descr_all[spi->id - 1] = NULL;
        omv_spi_transfer_abort(spi);
        if (spi->dma_flags & (OMV_SPI_DMA_NORMAL | OMV_SPI_DMA_CIRCULAR)) {
            if (spi->descr->hdmatx != NULL) {
                HAL_DMA_Abort(spi->descr->hdmatx);
            }
            if (spi->descr->hdmarx != NULL) {
                HAL_DMA_Abort(spi->descr->hdmarx);
            }
        }
        HAL_SPI_DeInit(spi->descr);
        HAL_NVIC_DisableIRQ(spi->irqn);
        // Deinit the CS pin here versus in HAL_SPI_MspDeInit which is shared code.
        omv_gpio_deinit(spi->cs);
    }
    return 0;
}

// This function is only needed for the py_tv driver on the RT1060 to slow down the SPI bus on reads.
// The STM32 is capable of reading data on the SPI bus at high speeds without issues...
int omv_spi_set_baudrate(omv_spi_t *spi, uint32_t baudrate) {
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
