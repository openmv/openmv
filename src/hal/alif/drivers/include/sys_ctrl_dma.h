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
 * @file     sys_ctrl_dma.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     18-Apr-2023
 * @brief    System Control Device information for DMA.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef SYS_CTRL_DMA_H

#define SYS_CTRL_DMA_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "peripheral_types.h"

#ifdef  __cplusplus
extern "C"
{
#endif

/**
 * enum DMA_INSTANCE
 * DMA instances
 */
typedef enum _DMA_INSTANCE
{
    DMA_INSTANCE_0,                         /**< DMA0       */
    DMA_INSTANCE_LOCAL,                     /**< DMALOCAL   */
} DMA_INSTANCE;

static inline void dma0_enable_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_DMA_CKEN;
}

static inline void dma0_disable_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_DMA_CKEN;
}

static inline void dma0_set_boot_manager_secure(void)
{
    CLKCTL_PER_MST->DMA_CTRL &= ~DMA_CTRL_BOOT_MANAGER;
}

static inline void dma0_set_boot_manager_nonsecure(void)
{
    CLKCTL_PER_MST->DMA_CTRL |= DMA_CTRL_BOOT_MANAGER;
}

static inline void dma0_set_boot_irq_ns_mask(uint32_t boot_irq_ns_mask)
{
    CLKCTL_PER_MST->DMA_IRQ = boot_irq_ns_mask;
}

static inline void dma0_set_boot_periph_ns_mask(uint32_t boot_periph_ns_mask)
{
    CLKCTL_PER_MST->DMA_PERIPH = boot_periph_ns_mask;
}

static inline void dma0_reset(void)
{
    CLKCTL_PER_MST->DMA_CTRL |= DMA_CTRL_SW_RST;
}

static inline void dmalocal_enable_periph_clk(void)
{
    M55LOCAL_CFG->CLK_ENA |= CLK_ENA_DMA_CKEN;
}

static inline void dmalocal_disable_periph_clk(void)
{
    M55LOCAL_CFG->CLK_ENA &= ~CLK_ENA_DMA_CKEN;
}

static inline void dmalocal_set_boot_manager_secure(void)
{
    M55LOCAL_CFG->DMA_CTRL &= ~DMA_CTRL_BOOT_MANAGER;
}

static inline void dmalocal_set_boot_manager_nonsecure(void)
{
    M55LOCAL_CFG->DMA_CTRL |= DMA_CTRL_BOOT_MANAGER;
}

static inline void dmalocal_set_boot_irq_ns_mask(uint32_t boot_irq_ns_mask)
{
    M55LOCAL_CFG->DMA_IRQ = boot_irq_ns_mask;
}

static inline void dmalocal_set_boot_periph_ns_mask(uint32_t boot_periph_ns_mask)
{
    M55LOCAL_CFG->DMA_PERIPH = boot_periph_ns_mask;
}

static inline void dmalocal_reset(void)
{
    M55LOCAL_CFG->DMA_CTRL |= DMA_CTRL_SW_RST;
}

static inline void dma0_set_glitch_filter(uint32_t glitch_filter)
{
    CLKCTL_PER_MST->DMA_GLITCH_FLT = glitch_filter;
}

static inline void dmalocal_set_glitch_filter(uint8_t glitch_filter)
{
    M55LOCAL_CFG->DMA_SEL |= (glitch_filter << DMA_SEL_FLT_ENA_Pos);
}

static inline void lppdm_select_dma0(void)
{
    M55HE_CFG->HE_DMA_SEL |= HE_DMA_SEL_PDM_DMA0;
}

static inline void lpi2s_select_dma0(void)
{
    M55HE_CFG->HE_DMA_SEL |= HE_DMA_SEL_I2S_DMA0;
}

static inline void lpspi_select_dma0(uint8_t group)
{
    M55HE_CFG->HE_DMA_SEL |= ((group << HE_DMA_SEL_LPSPI_Pos) & HE_DMA_SEL_LPSPI_Msk);
}

static inline void lpuart_select_dma0(void)
{
    M55HE_CFG->HE_DMA_SEL |= HE_DMA_SEL_LPUART_DMA0;
}

#ifdef  __cplusplus
}
#endif
#endif /* SYS_CTRL_DMA_H */
