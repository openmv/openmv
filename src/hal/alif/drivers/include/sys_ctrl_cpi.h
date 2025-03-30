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
 * @file     sys_ctrl_cpi.h
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     19-April-2023
 * @brief    CPI system control Specific Header file.
 ******************************************************************************/
#ifndef SYS_CTRL_CPI_H_
#define SYS_CTRL_CPI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum CPI_PIX_CLKSEL
 * CPI pixel clock source selection
 */
typedef enum _CPI_PIX_CLKSEL
{
    CPI_PIX_CLKSEL_400MZ,                   /**< Select 400 MHz clock source (PLL_CLK1/2) */
    CPI_PIX_CLKSEL_480MZ                    /**< Select 480 MHz clock source (PLL_CLK3) */
}CPI_PIX_CLKSEL;

/**
  \fn          void enable_cpi_periph_clk(void)
  \brief       Enable clock supply for CPI controller.
  \return      none.
*/
static inline void enable_cpi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_CPI_CKEN;
}

/**
  \fn          void disable_cpi_periph_clk(void)
  \brief       Disable clock supply for CPI controller.
  \return      none.
*/
static inline void disable_cpi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_CPI_CKEN;
}

/**
  \fn          void enable_lpcpi_periph_clk(void)
  \brief       Enable clock supply for LPCPI controller.
  \return      none.
*/
static inline void enable_lpcpi_periph_clk(void)
{
    M55HE_CFG->HE_CLK_ENA |= HE_CLK_ENA_LPCPI_CKEN;
}

/**
  \fn          void disable_lpcpi_periph_clk(void)
  \brief       Disable clock supply for LPCPI controller.
  \return      none.
*/
static inline void disable_lpcpi_periph_clk(void)
{
    M55HE_CFG->HE_CLK_ENA &= ~HE_CLK_ENA_LPCPI_CKEN;
}

/**
  \fn          void set_cpi_pixel_clk(CPI_PIX_CLKSEL clksel, uint32_t div)
  \brief       Set cpi pixel clock.
  param[in]    clksel pixel clock source to select.
  param[in]    div    pixel clock divisor.
  \return      none.
*/
static inline void set_cpi_pixel_clk(CPI_PIX_CLKSEL clksel, uint32_t div)
{

    CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL &= ~CAMERA_PIXCLK_CTRL_DIVISOR_Msk;
    CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL |= (div << CAMERA_PIXCLK_CTRL_DIVISOR_Pos);

    switch(clksel)
    {
        case CPI_PIX_CLKSEL_400MZ:
        {
            CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL &= ~CAMERA_PIXCLK_CTRL_CLK_SEL;
            break;
        }

        case CPI_PIX_CLKSEL_480MZ:
        {
            CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL |= CAMERA_PIXCLK_CTRL_CLK_SEL;
            break;
        }

        default:
        {
            break;
        }
    }

    CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL |= CAMERA_PIXCLK_CTRL_CKEN;
}

/**
  \fn          void set_lpcpi_pixel_clk(uint32_t div)
  \brief       Set lpcpi pixel clock.
  param[in]    div    pixel clock divisor.
  \return      none.
*/
static inline void set_lpcpi_pixel_clk(uint32_t div)
{

    M55HE_CFG->HE_CAMERA_PIXCLK &= ~HE_CAMERA_PIXCLK_CTRL_DIVISOR_Msk;
    M55HE_CFG->HE_CAMERA_PIXCLK |= (div << HE_CAMERA_PIXCLK_CTRL_DIVISOR_Pos);

    M55HE_CFG->HE_CAMERA_PIXCLK |= HE_CAMERA_PIXCLK_CTRL_CKEN;
}

/**
  \fn          void clear_cpi_pixel_clk(void)
  \brief       Clear cpi pixel clock.
  \return      none.
*/
static inline void clear_cpi_pixel_clk(void)
{
    CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL = 0;
}

/**
  \fn          void clear_lpcpi_pixel_clk(void)
  \brief       Clear lpcpi pixel clock.
  \return      none.
*/
static inline void clear_lpcpi_pixel_clk(void)
{
    M55HE_CFG->HE_CAMERA_PIXCLK = 0;
}

#ifdef  __cplusplus
}
#endif

#endif /* SYS_CTRL_CPI_H_ */
