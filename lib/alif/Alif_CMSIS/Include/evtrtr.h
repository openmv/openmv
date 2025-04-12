/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     evtrtr.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     5-May-2023
 * @brief    System Control Device information for Event Router
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef EVTRTR_H

#define EVTRTR_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "peripheral_types.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#ifdef  __cplusplus
extern "C"
{
#endif

/**
 * enum DMA_ACK_COMPLETION
 * Defines the completion of REQ-ACK with DMA
 */
typedef enum _DMA_ACK_COMPLETION
{
    DMA_ACK_COMPLETION_PERIPHERAL,  /**< Peripheral completes with DMA    */
    DMA_ACK_COMPLETION_EVTRTR,      /**< Event Router completes with DMA  */
} DMA_ACK_COMPLETION;

#define EVTRTR_DMA_CHANNEL_MAX_Msk   0x1F     /* Channel range 0-31 */

/**
\brief Event Router Configuration
*/
typedef struct _EVTRTR_CONFIG {

    /*!< Instance number  */
    const uint8_t instance;

    /*!< Group number */
    const uint8_t group;

    /*!< Channel number  */
    const uint8_t channel;

    /*!< Enable handshake */
    const bool enable_handshake;
} EVTRTR_CONFIG;


static inline void evtrtr0_enable_dma_channel(uint8_t channel,
                                              uint8_t group,
                                              DMA_ACK_COMPLETION ack_type)
{
    volatile uint32_t *dma_ctrl_addr = &EVTRTR0->DMA_CTRL0 +
                                       (channel & EVTRTR_DMA_CHANNEL_MAX_Msk);

    *dma_ctrl_addr = DMA_CTRL_ENA | (DMA_CTRL_SEL_Msk & group)
                     | (ack_type << DMA_CTRL_ACK_TYPE_Pos);
}

static inline void evtrtr0_disable_dma_channel(uint8_t channel)
{
    volatile uint32_t *dma_ctrl_addr = &EVTRTR0->DMA_CTRL0 +
                                       (channel & EVTRTR_DMA_CHANNEL_MAX_Msk);

    *dma_ctrl_addr = 0;
}

static inline void evtrtr0_enable_dma_req(void)
{
    EVTRTR0->DMA_REQ_CTRL |= (DMA_REQ_CTRL_CB | DMA_REQ_CTRL_CLB
                              | DMA_REQ_CTRL_CS | DMA_REQ_CTRL_CLS);
}

static inline void evtrtr0_disable_dma_req(void)
{
    EVTRTR0->DMA_REQ_CTRL = 0;
}

static inline void evtrtr0_enable_dma_handshake(uint8_t channel, uint8_t group)
{
    volatile uint32_t *dma_ack_type_addr = &EVTRTR0->DMA_ACK_TYPE0 +
                                           (DMA_CTRL_SEL_Msk & group);

    __disable_irq();
    *dma_ack_type_addr |= (1 << (channel & EVTRTR_DMA_CHANNEL_MAX_Msk));
    __enable_irq();
}

static inline void evtrtr0_disable_dma_handshake(uint8_t channel, uint8_t group)
{
    volatile uint32_t *dma_ack_type_addr = &EVTRTR0->DMA_ACK_TYPE0 +
                                           (DMA_CTRL_SEL_Msk & group);

    __disable_irq();
    *dma_ack_type_addr &= ~(1 << (channel & EVTRTR_DMA_CHANNEL_MAX_Msk));
    __enable_irq();
}

static inline void evtrtrlocal_enable_dma_channel(uint8_t channel,
                                                  DMA_ACK_COMPLETION ack_type)
{
    volatile uint32_t *dma_ctrl_addr = &EVTRTRLOCAL->DMA_CTRL0 +
                                       (channel & EVTRTR_DMA_CHANNEL_MAX_Msk);

    *dma_ctrl_addr = DMA_CTRL_ENA | (ack_type << DMA_CTRL_ACK_TYPE_Pos);
}

static inline void evtrtrlocal_disable_dma_channel(uint8_t channel)
{
    volatile uint32_t *dma_ctrl_addr = &EVTRTRLOCAL->DMA_CTRL0 +
                                       (channel & EVTRTR_DMA_CHANNEL_MAX_Msk);

    *dma_ctrl_addr = 0;
}

static inline void evtrtrlocal_enable_dma_req(void)
{
    EVTRTRLOCAL->DMA_REQ_CTRL |= (DMA_REQ_CTRL_CB | DMA_REQ_CTRL_CLB
                                  | DMA_REQ_CTRL_CS | DMA_REQ_CTRL_CLS);
}

static inline void evtrtrlocal_disable_dma_req(void)
{
    EVTRTRLOCAL->DMA_REQ_CTRL = 0;
}

#ifdef  __cplusplus
}
#endif
#endif /* EVTRTR_H */
