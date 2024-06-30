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
 * @file     sys_ctrl_spi.h
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     24-04-2023
 * @brief    SPI system control Specific Header file.
 ******************************************************************************/

#ifndef SYS_CTRL_SPI_H_
#define SYS_CTRL_SPI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum SS_IN_SEL.
 * SPI SS_IN Mode Select.
 */
typedef enum _SS_IN_SEL
{
    SS_IN_IO_PIN,                         /**< SS_IN from I/O pin */
    SS_IN_SS_IN_VAL                       /**< SS_IN from SS_IN_VAL */
} SS_IN_SEL;

/**
 * enum SPI_INSTANCE.
 * SPI instances.
 */
typedef enum _SPI_INSTANCE
{
    SPI_INSTANCE_0,                         /**< SPI instance - 0 */
    SPI_INSTANCE_1,                         /**< SPI instance - 1 */
    SPI_INSTANCE_2,                         /**< SPI instance - 2 */
    SPI_INSTANCE_3,                         /**< SPI instance - 3 */
    LPSPI_INSTANCE                          /**< Low Power SPI instance */
} SPI_INSTANCE;

/**
  \fn          static inline void ctrl_ss_in (SPI_INSTANCE instance, SS_IN_SEL ss_in_sel)
  \brief       control spi ss_in
  \param[in]   instance     spi instance
  \param[in]   ss_in_sel    ss_in signal selection
  \return      none
*/
static inline void ctrl_ss_in (SPI_INSTANCE instance, SS_IN_SEL ss_in_sel)
{
    switch (instance)
    {
        case SPI_INSTANCE_0:
        {
            if (ss_in_sel == SS_IN_SS_IN_VAL)
            {
                CLKCTL_PER_SLV->SSI_CTRL |= (SSI_CTRL_SS_IN_VAL_0 | SSI_CTRL_SS_IN_SEL_0);
            }
            if (ss_in_sel == SS_IN_IO_PIN)
            {
                CLKCTL_PER_SLV->SSI_CTRL &= ~(SSI_CTRL_SS_IN_VAL_0 | SSI_CTRL_SS_IN_SEL_0);
            }
            break;
        }
        case SPI_INSTANCE_1:
        {
            if (ss_in_sel == SS_IN_SS_IN_VAL)
            {
                CLKCTL_PER_SLV->SSI_CTRL |= (SSI_CTRL_SS_IN_VAL_1 | SSI_CTRL_SS_IN_SEL_1);
            }
            if (ss_in_sel == SS_IN_IO_PIN)
            {
                CLKCTL_PER_SLV->SSI_CTRL &= ~(SSI_CTRL_SS_IN_VAL_1 | SSI_CTRL_SS_IN_SEL_1);
            }
            break;
        }
        case SPI_INSTANCE_2:
        {
            if (ss_in_sel == SS_IN_SS_IN_VAL)
            {
                CLKCTL_PER_SLV->SSI_CTRL |= (SSI_CTRL_SS_IN_VAL_2 | SSI_CTRL_SS_IN_SEL_2);
            }
            if (ss_in_sel == SS_IN_IO_PIN)
            {
                CLKCTL_PER_SLV->SSI_CTRL &= ~(SSI_CTRL_SS_IN_VAL_2 | SSI_CTRL_SS_IN_SEL_2);
            }
            break;
        }
        case SPI_INSTANCE_3:
        {
            if (ss_in_sel == SS_IN_SS_IN_VAL)
            {
                CLKCTL_PER_SLV->SSI_CTRL |= (SSI_CTRL_SS_IN_VAL_3 | SSI_CTRL_SS_IN_SEL_3);
            }
            if (ss_in_sel == SS_IN_IO_PIN)
            {
                CLKCTL_PER_SLV->SSI_CTRL &= ~(SSI_CTRL_SS_IN_VAL_3 | SSI_CTRL_SS_IN_SEL_3);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
  \fn          static inline void enable_lpspi_clk (void)
  \brief       enable LPSPI clock
  \return      none
*/
static inline void enable_lpspi_clk (void)
{
    M55HE_CFG->HE_CLK_ENA |= HE_CLK_ENA_SPI_CKEN;
}

/**
  \fn          static inline void disable_lpspi_clk (void)
  \brief       disable LPSPI clock
  \return      none
*/
static inline void disable_lpspi_clk (void)
{
    M55HE_CFG->HE_CLK_ENA &= ~HE_CLK_ENA_SPI_CKEN;
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_SPI_H_ */
