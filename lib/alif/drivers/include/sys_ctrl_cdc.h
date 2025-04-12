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
 * @file     sys_ctrl_cdc.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     10-April-2023
 * @brief    CDC system control Specific Header file.
 ******************************************************************************/

#ifndef SYS_CTRL_CDC_H_
#define SYS_CTRL_CDC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum CDC_PIX_CLKSEL
 * CDC pixel clock source selection
 */
typedef enum _CDC_PIX_CLKSEL
{
    CDC_PIX_CLKSEL_400MZ,            /**< Select 400 MHz clock source (PLL_CLK1/2) */
    CDC_PIX_CLKSEL_480MZ             /**< Select 480 MHz clock source (PLL_CLK3) */
}CDC_PIX_CLKSEL;

/**
  \fn          static inline void enable_dpi_periph_clk(void)
  \brief       Enable clock supply for DPI controller (CDC).
  \return      none.
*/
static inline void enable_dpi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_DPI_CKEN;
}

/**
  \fn          static inline void disable_dpi_periph_clk(void)
  \brief       Disable clock supply for DPI controller (CDC).
  \return      none.
*/
static inline void disable_dpi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_DPI_CKEN;
}

/**
  \fn          static inline void set_cdc_pixel_clk(CDC_PIX_CLKSEL clksel, uint32_t div)
  \brief       Set cdc pixel clock.
  param[in]    clksel pixel clock source to select.
  param[in]    div    pixel clock divisor.
  \return      none.
*/
static inline void set_cdc_pixel_clk(CDC_PIX_CLKSEL clksel, uint32_t div)
{

    CLKCTL_PER_MST->CDC200_PIXCLK_CTRL &= ~CDC200_PIXCLK_CTRL_DIVISOR_Msk;
    CLKCTL_PER_MST->CDC200_PIXCLK_CTRL |= (div << CDC200_PIXCLK_CTRL_DIVISOR_Pos);

    switch(clksel)
    {
        case CDC_PIX_CLKSEL_400MZ:
            CLKCTL_PER_MST->CDC200_PIXCLK_CTRL &= ~CDC200_PIXCLK_CTRL_CLK_SEL;
            break;
        case CDC_PIX_CLKSEL_480MZ:
            CLKCTL_PER_MST->CDC200_PIXCLK_CTRL |= CDC200_PIXCLK_CTRL_CLK_SEL;
            break;
        default:
            break;
    }

    CLKCTL_PER_MST->CDC200_PIXCLK_CTRL |= CDC200_PIXCLK_CTRL_CKEN;
}

/**
  \fn          static inline void disable_cdc_pixel_clk(void)
  \brief       Disable cdc pixel clock.
  \return      none.
*/
static inline void disable_cdc_pixel_clk(void)
{
    CLKCTL_PER_MST->CDC200_PIXCLK_CTRL = 0;
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_CDC_H_ */
