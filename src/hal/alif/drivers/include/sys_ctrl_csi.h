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
 * @file     sys_ctrl_csi.h
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     19-April-2023
 * @brief    CSI system control Specific Header file.
 ******************************************************************************/

#ifndef SYS_CTRL_CSI_H_
#define SYS_CTRL_CSI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum CSI_PIX_CLKSEL
 * CSI pixel clock source selection
 */
typedef enum _CSI_PIX_CLKSEL
{
    CSI_PIX_CLKSEL_400MZ,                   /**< Select 400 MHz clock source (PLL_CLK1/2) */
    CSI_PIX_CLKSEL_480MZ                    /**< Select 480 MHz clock source (PLL_CLK3)   */
} CSI_PIX_CLKSEL;

/**
  \fn          void enable_csi_periph_clk(void)
  \brief       Enable clock supply for CSI controller.
  \return      none.
*/
static inline void enable_csi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_CSI_CKEN;
}

/**
  \fn          void disable_csi_periph_clk(void)
  \brief       Disable clock supply for CSI controller.
  \return      none.
*/
static inline void disable_csi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_CSI_CKEN;
}

/**
  \fn          void set_csi_pixel_clk(CSI_PIX_CLKSEL clksel, uint32_t div)
  \brief       Set CSI pixel clock.
  param[in]    clksel pixel clock source to select.
  param[in]    div    pixel clock divisor.
  \return      none.
*/
static inline void set_csi_pixel_clk(CSI_PIX_CLKSEL clksel, uint32_t div)
{

    CLKCTL_PER_MST->CSI_PIXCLK_CTRL &= ~CSI_PIXCLK_CTRL_DIVISOR_Msk;
    CLKCTL_PER_MST->CSI_PIXCLK_CTRL |= (div << CSI_PIXCLK_CTRL_DIVISOR_Pos);

    switch(clksel)
    {
        case CSI_PIX_CLKSEL_400MZ:
        {
            CLKCTL_PER_MST->CSI_PIXCLK_CTRL &= ~CSI_PIXCLK_CTRL_CLK_SEL;
            break;
        }

        case CSI_PIX_CLKSEL_480MZ:
        {
            CLKCTL_PER_MST->CSI_PIXCLK_CTRL |= CSI_PIXCLK_CTRL_CLK_SEL;
            break;
        }

        default:
        {
            break;
        }
    }

    CLKCTL_PER_MST->CSI_PIXCLK_CTRL |= CSI_PIXCLK_CTRL_CKEN;
}

/**
  \fn          void clear_csi_pixel_clk(void)
  \brief       Clear CSI pixel clock.
  \return      none.
*/
static inline void clear_csi_pixel_clk(void)
{
    CLKCTL_PER_MST->CSI_PIXCLK_CTRL = 0;
}

#ifdef  __cplusplus
}
#endif

#endif /* SYS_CTRL_CSI_H_ */
