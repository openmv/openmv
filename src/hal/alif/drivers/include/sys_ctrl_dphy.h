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
 * @file     sys_ctrl_dphy.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     19-April-2023
 * @brief    DPHY system control Specific Header file.
 ******************************************************************************/
#ifndef SYS_CTRL_DPHY_H_
#define SYS_CTRL_DPHY_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum DPHY_PLL_CLKSEL
 * DPHY pll clock selection
 */
typedef enum _DPHY_PLL_CLKSEL
{
    DPHY_PLL_CLKSEL_CLOCK_STOP,            /**< DPHY Clocks stopped                */
    DPHY_PLL_CLKSEL_CLOCK_GENERAT,         /**< DPHY CLOCK generation              */
    DPHY_PLL_CLKSEL_CLOCK_BYPASS,          /**< Buffered CLKEXT (PLL bypass clock) */
}DPHY_PLL_CLKSEL;

/**
 * enum DPHY_TESTPORT_SELECT
 * DPHY dphy testport selection
 */
typedef enum _DPHY_TESTPORT_SELECT
{
    DPHY_TESTPORT_SELECT_TX,               /**< DPHY Select TX_TESTPORT */
    DPHY_TESTPORT_SELECT_RX,               /**< DPHY Select RX_TESTPORT */
}DPHY_TESTPORT_SELECT;

/**
 * enum DPHY_MODE
 * DPHY mode Master/Slave.
*/
typedef enum _DPHY_MODE
{
    DPHY_MODE_MASTER,                     /**< DPHY mode master */
    DPHY_MODE_SLAVE                       /**< DPHY mode slave  */
}DPHY_MODE;

/**
 * enum DPHY_BIST_OK
 * DPHY BISt OK SET/NOT_SET.
*/
typedef enum _DPHY_BIST_OK_STATUS
{
    DPHY_BIST_OK_STATUS_NOT_SET  = (0U << 2),   /**< DPHY BIST OK set */
    DPHY_BIST_OK_STATUS_SET      = (1U << 2)    /**< DPHY BIST OK not set */
}DPHY_BIST_OK_STATUS;

/**
 * enum DPHY_DATA_LANE
 * DPHY data lanes.
*/
typedef enum _DPHY_DATA_LANE
{
    DPHY_DATA_LANE_0  = (1U << 0),         /**< DPHY data lane 0 */
    DPHY_DATA_LANE_1  = (1U << 1)          /**< DPHY data lane 1 */
}DPHY_DATA_LANE;

/**
 * @struct  _pll_config_t
 * @brief   pll configuration parameters description.
 */
typedef struct _pll_config_t
{
    uint8_t  pll_gmp_ctrl;      /* Controls the effective loop-filter resistance   */
    uint16_t pll_m;             /* Control of the input frequency division ratio   */
    uint8_t  pll_n;             /* Control of the feedback multiplication ratio    */
    uint8_t  pll_cpbias_ctrl;   /* Charge pump bias control                        */
    uint8_t  pll_int_ctrl;      /* Integral charge pump control                    */
    uint8_t  pll_prop_ctrl;     /* Proportional charge pump control                */
    uint8_t  pll_vco_ctrl;      /* VCO operating range                             */
} pll_config_t;

/**
  \fn          static inline void enable_dphy_pll_force_lock (void)
  \brief       Enable Force lock to device.
  \return      none.
*/
static inline void enable_dphy_pll_force_lock (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 |= DPHY_PLL_CTRL0_FORCE_LOCK;
}

/**
  \fn          static inline void disable_dphy_pll_force_lock (void)
  \brief       Disable Force lock to device.
  \return      none.
*/
static inline void disable_dphy_pll_force_lock (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 &= ~DPHY_PLL_CTRL0_FORCE_LOCK;
}

/**
  \fn          static inline void enable_dphy_pll_shadow_control (void)
  \brief       Enable dphy pll shadow control.
  \return      none.
*/
static inline void enable_dphy_pll_shadow_control (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 |= DPHY_PLL_CTRL0_SHADOW_CONTROL;
}

/**
  \fn          static inline void disable_dphy_pll_shadow_control (void)
  \brief       Disable dphy pll shadow control.
  \return      none.
*/
static inline void disable_dphy_pll_shadow_control (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 &= ~DPHY_PLL_CTRL0_SHADOW_CONTROL;
}

/**
  \fn          static inline void enable_dphy_updatepll (void)
  \brief       Enable dphy updatepll.
  \return      none.
*/
static inline void enable_dphy_updatepll (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 |= DPHY_PLL_CTRL0_UPDATEPLL;
}

/**
  \fn          static inline void disable_dphy_updatepll (void)
  \brief       Disable dphy updatepll.
  \return      none.
*/
static inline void disable_dphy_updatepll (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 &= ~DPHY_PLL_CTRL0_UPDATEPLL;
}

/**
  \fn          static inline void enable_dphy_pll_shadow_clear (void)
  \brief       Enable dphy pll shadow clear.
  \return      none.
*/
static inline void enable_dphy_pll_shadow_clear (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 |= DPHY_PLL_CTRL0_SHADOW_CLEAR;
}

/**
  \fn          static inline void disable_dphy_pll_shadow_clear (void)
  \brief       Disable dphy pll shadow clear.
  \return      none.
*/
static inline void disable_dphy_pll_shadow_clear (void)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 &= ~DPHY_PLL_CTRL0_SHADOW_CLEAR;
}

/**
  \fn          static inline void set_dphy_pll_configuration (pll_config_t *info)
  \brief       Set dphy pll configuration.
  \param[in]    info   Pointer to pll configuration structure.
  \return      none.
*/
static inline void set_dphy_pll_configuration (pll_config_t *info)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 &= ~DPHY_PLL_CTRL0_GMP_CTRL_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 |= (info->pll_gmp_ctrl << DPHY_PLL_CTRL0_GMP_CTRL_Pos);

    CLKCTL_PER_MST->DPHY_PLL_CTRL1 &= ~DPHY_PLL_CTRL1_FEEDBACK_MULT_RATIO_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL1 |= (info->pll_m << DPHY_PLL_CTRL1_FEEDBACK_MULT_RATIO_Pos);

    CLKCTL_PER_MST->DPHY_PLL_CTRL1 &= ~DPHY_PLL_CTRL1_INPUT_DIV_FACTOR_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL1 |= (info->pll_n << DPHY_PLL_CTRL1_INPUT_DIV_FACTOR_Pos);

    CLKCTL_PER_MST->DPHY_PLL_CTRL2 &= ~DPHY_PLL_CTRL2_CPBIAS_CTRL_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL2 |= (info->pll_cpbias_ctrl << DPHY_PLL_CTRL2_CPBIAS_CTRL_Pos);

    CLKCTL_PER_MST->DPHY_PLL_CTRL2 &= ~DPHY_PLL_CTRL2_INT_CTRL_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL2 |= (info->pll_int_ctrl << DPHY_PLL_CTRL2_INT_CTRL_Pos);

    CLKCTL_PER_MST->DPHY_PLL_CTRL2 &= ~DPHY_PLL_CTRL2_PROP_CTRL_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL2 |= (info->pll_prop_ctrl << DPHY_PLL_CTRL2_PROP_CTRL_Pos);

    CLKCTL_PER_MST->DPHY_PLL_CTRL2 &= ~DPHY_PLL_CTRL2_VCO_CTRL_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL2 |= (info->pll_vco_ctrl << DPHY_PLL_CTRL2_VCO_CTRL_Pos);
}

/**
  \fn          static inline void set_dphy_pll_clksel (void)
  \brief       Set dphy CLKEXT divider selection.
  \param[in]   clksel dphy CLKEXT divider selection to set.
  \return      none.
*/
static inline void set_dphy_pll_clksel (DPHY_PLL_CLKSEL clksel)
{
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 &= ~DPHY_PLL_CTRL0_CLKSEL_Msk;
    CLKCTL_PER_MST->DPHY_PLL_CTRL0 |= (clksel << DPHY_PLL_CTRL0_CLKSEL_Pos);
}

/**
  \fn          static inline void set_tx_dphy_txrx (DPHY_MODE mode)
  \brief       Selects master or slave configuration for the dphy.
  \param[in]   mode select Mater/Slave.
  \return      none.
*/
static inline void set_tx_dphy_txrx (DPHY_MODE mode)
{
    if(mode == DPHY_MODE_MASTER)
    {
        CLKCTL_PER_MST->TX_DPHY_CTRL0 |= DPHY_CTRL0_TXRXZ;
    }
    else
    {
        CLKCTL_PER_MST->TX_DPHY_CTRL0 &= ~DPHY_CTRL0_TXRXZ;
    }
}

/**
  \fn          static inline void set_rx_dphy_txrx (DPHY_MODE mode)
  \brief       Selects master or slave configuration for the dphy.
  \param[in]   mode select Mater/Slave.
  \return      none.
*/
static inline void set_rx_dphy_txrx (DPHY_MODE mode)
{
    if(mode == DPHY_MODE_MASTER)
    {
        CLKCTL_PER_MST->RX_DPHY_CTRL0 |= DPHY_CTRL0_TXRXZ;
    }
    else
    {
        CLKCTL_PER_MST->RX_DPHY_CTRL0 &= ~DPHY_CTRL0_TXRXZ;
    }
}

/**
  \fn          static inline void set_tx_dphy_testport_select (DPHY_TESTPORT_SELECT testport)
  \brief       Test port select for dphy.
  \param[in]   testport dphy CLKEXT divider selection to set.
  \return      none.
*/
static inline void set_tx_dphy_testport_select (DPHY_TESTPORT_SELECT testport)
{
    if(testport == DPHY_TESTPORT_SELECT_RX)
    {
        CLKCTL_PER_MST->TX_DPHY_CTRL0 |= DPHY_CTRL0_TESTPORT_SEL;
    }
    else
    {
        CLKCTL_PER_MST->TX_DPHY_CTRL0 &= ~DPHY_CTRL0_TESTPORT_SEL;
    }
}

/**
  \fn          static inline void set_rx_dphy_testport_select (DPHY_TESTPORT_SELECT testport)
  \brief       Test port select for dphy.
  \param[in]   testport dphy CLKEXT divider selection to set.
  \return      none.
*/
static inline void set_rx_dphy_testport_select (DPHY_TESTPORT_SELECT testport)
{
    if(testport == DPHY_TESTPORT_SELECT_RX)
    {
        CLKCTL_PER_MST->RX_DPHY_CTRL0 |= DPHY_CTRL0_TESTPORT_SEL;
    }
    else
    {
        CLKCTL_PER_MST->RX_DPHY_CTRL0 &= ~DPHY_CTRL0_TESTPORT_SEL;
    }
}

/**
  \fn          static inline void set_tx_dphy_hsfreqrange (uint32_t range)
  \brief       Set dphy Module operating frequency.
  \param[in]   range dphy hsfreqrange to set.
  \return      none.
*/
static inline void set_tx_dphy_hsfreqrange (uint32_t range)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL0 &= ~DPHY_CTRL0_HSFREQRANGE_Msk;
    CLKCTL_PER_MST->TX_DPHY_CTRL0 |= (range << DPHY_CTRL0_HSFREQRANGE_Pos);
}

/**
  \fn          static inline void set_rx_dphy_hsfreqrange (uint32_t range)
  \brief       Set dphy Module operating frequency.
  \param[in]   range dphy hsfreqrange to set.
  \return      none.
*/
static inline void set_rx_dphy_hsfreqrange (uint32_t range)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL0 &= ~DPHY_CTRL0_HSFREQRANGE_Msk;
    CLKCTL_PER_MST->RX_DPHY_CTRL0 |= (range << DPHY_CTRL0_HSFREQRANGE_Pos);
}

/**
  \fn          static inline void set_tx_dphy_cfgclkfreqrange (uint32_t range)
  \brief       Set dphy Input configure clock frequency
  \param[in]   range dphy cfgclkfreqrange to set.
  \return      none.
*/
static inline void set_tx_dphy_cfgclkfreqrange (uint32_t range)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL0 &= ~DPHY_CTRL0_CFGCLKFREQRANGE_Msk;
    CLKCTL_PER_MST->TX_DPHY_CTRL0 |= (range << DPHY_CTRL0_CFGCLKFREQRANGE_Pos);
}

/**
  \fn          static inline void set_rx_dphy_cfgclkfreqrange (uint32_t range)
  \brief       Set dphy Input configure clock frequency
  \param[in]   range dphy cfgclkfreqrange to set.
  \return      none.
*/
static inline void set_rx_dphy_cfgclkfreqrange (uint32_t range)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL0 &= ~DPHY_CTRL0_CFGCLKFREQRANGE_Msk;
    CLKCTL_PER_MST->RX_DPHY_CTRL0 |= (range << DPHY_CTRL0_CFGCLKFREQRANGE_Pos);
}

/**
  \fn          static inline void set_tx_dphy_basedir (DPHY_DATA_LANE lane)
  \brief       Configures the base direction for dphy data lane
  \param[in]   lane dphy base direction for data lane.
  \return      none.
*/
static inline void set_tx_dphy_basedir (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL0 |= (lane << DPHY_CTRL0_BASEDIR_Pos);
}

/**
  \fn          static inline void unset_tx_dphy_basedir (DPHY_DATA_LANE lane)
  \brief       Configures the base direction for dphy data lane
  \param[in]   lane dphy base direction for data lane.
  \return      none.
*/
static inline void unset_tx_dphy_basedir (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL0 &= ~(lane << DPHY_CTRL0_BASEDIR_Pos);
}

/**
  \fn          static inline void set_rx_dphy_basedir (DPHY_DATA_LANE lane)
  \brief       Configures the base direction for dphy data lane
  \param[in]   lane dphy base direction for data lane.
  \return      none.
*/
static inline void set_rx_dphy_basedir (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL0 |= (lane << DPHY_CTRL0_BASEDIR_Pos);
}

/**
  \fn          static inline void unset_rx_dphy_basedir (DPHY_DATA_LANE lane)
  \brief       Configures the base direction for dphy data lane
  \param[in]   lane dphy base direction for data lane.
  \return      none.
*/
static inline void unset_rx_dphy_basedir (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL0 &= ~(lane << DPHY_CTRL0_BASEDIR_Pos);
}

/**
  \fn          static inline void set_tx_dphy_forcerxmode (DPHY_DATA_LANE lane)
  \brief       Controls forcerxmode pin of dphy.
  \param[in]   lane controls forcerxmode pin for data lane.
  \return      none.
*/
static inline void set_tx_dphy_forcerxmode (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL1 |= (lane << DPHY_CTRL1_FORCERXMODE_Pos);
}

/**
  \fn          static inline void unset_tx_dphy_forcerxmode (DPHY_DATA_LANE lane)
  \brief       Controls forcerxmode pin of dphy.
  \param[in]   lane controls forcerxmode pin for data lane.
  \return      none.
*/
static inline void unset_tx_dphy_forcerxmode (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL1 &= ~(lane << DPHY_CTRL1_FORCERXMODE_Pos);
}

/**
  \fn          static inline void set_rx_dphy_forcerxmode (DPHY_DATA_LANE lane)
  \brief       Controls forcerxmode pin of dphy.
  \param[in]   lane controls forcerxmode pin for data lane.
  \return      none.
*/
static inline void set_rx_dphy_forcerxmode (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL1 |= (lane << DPHY_CTRL1_FORCERXMODE_Pos);
}

/**
  \fn          static inline void unset_rx_dphy_forcerxmode (DPHY_DATA_LANE lane)
  \brief       Controls forcerxmode pin of dphy.
  \param[in]   lane controls forcerxmode pin for data lane.
  \return      none.
*/
static inline void unset_rx_dphy_forcerxmode (DPHY_DATA_LANE lane)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL1 &= ~(lane << DPHY_CTRL1_FORCERXMODE_Pos);
}

/**
  \fn          static inline void enable_tx_dphy_bist_on (void)
  \brief       Enable configure clock for TX D-PHY
  \return      none.
*/
static inline void enable_tx_dphy_bist_on (void)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL0  |= DPHY_CTRL0_BIST_ON;
}

/**
  \fn          static inline void disable_tx_dphy_bist_on (void)
  \brief       Disable configure clock for TX D-PHY
  \return      none.
*/
static inline void disable_tx_dphy_bist_on (void)
{
    CLKCTL_PER_MST->TX_DPHY_CTRL0  &= ~DPHY_CTRL0_BIST_ON;
}

/**
  \fn          static inline void enable_rx_dphy_bist_on (void)
  \brief       Enable configure clock for RX D-PHY
  \return      none.
*/
static inline void enable_rx_dphy_bist_on (void)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL0  |= DPHY_CTRL0_BIST_ON;
}

/**
  \fn          static inline void disable_rx_dphy_bist_on (void)
  \brief       Disable configure clock for TX D-PHY
  \return      none.
*/
static inline void disable_rx_dphy_bist_on (void)
{
    CLKCTL_PER_MST->RX_DPHY_CTRL0  &= ~DPHY_CTRL0_BIST_ON;
}

/**
  \fn          static inline void disable_tx_dphy_bist_on (void)
  \brief       Disable configure clock for TX D-PHY
  \return      none.
*/
static inline DPHY_BIST_OK_STATUS get_tx_dphy_bist_ok (void)
{
    return (CLKCTL_PER_MST->TX_DPHY_CTRL0  & DPHY_CTRL0_BIST_OK);
}

/**
  \fn          static inline void disable_rx_dphy_bist_on (void)
  \brief       Disable configure clock for TX D-PHY
  \return      none.
*/
static inline DPHY_BIST_OK_STATUS get_rx_dphy_bist_ok (void)
{
    return (CLKCTL_PER_MST->RX_DPHY_CTRL0  & DPHY_CTRL0_BIST_OK);
}

/**
  \fn          static inline void enable_txdphy_configure_clock (void)
  \brief       Enable configure clock for TX D-PHY
  \return      none.
*/
static inline void enable_txdphy_configure_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN |= MIPI_CLKEN_TXDPHY_CKEN;
}

/**
  \fn          static inline void disable_txdphy_configure_clock (void)
  \brief       Disable configure clock for TX D-PHY.
  \return      none.
*/
static inline void disable_txdphy_configure_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN &= ~MIPI_CLKEN_TXDPHY_CKEN;
}

/**
  \fn          static inline void enable_rxdphy_configure_clock (void)
  \brief       Enable configure clock for RX D-PHY.
  \return      none.
*/
static inline void enable_rxdphy_configure_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN |= MIPI_CLKEN_RXDPHY_CKEN;
}

/**
  \fn          static inline void disable_rxdphy_configure_clock (void)
  \brief       Disable configure clock for RX D-PHY.
  \return      none.
*/
static inline void disable_rxdphy_configure_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN &= ~MIPI_CLKEN_RXDPHY_CKEN;
}

/**
  \fn          static inline void enable_dphy_pll_reference_clock (void)
  \brief       Enable reference clock for MIPI D-PHY PLL.
  \return      none.
*/
static inline void enable_dphy_pll_reference_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN |= MIPI_CLKEN_PLLREF_CKEN;
}

/**
  \fn          static inline void disable_dphy_pll_reference_clock (void)
  \brief       Disable reference clock for MIPI D-PHY PLL.
  \return      none.
*/
static inline void disable_dphy_pll_reference_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN &= ~MIPI_CLKEN_PLLREF_CKEN;
}

/**
  \fn          static inline void enable_dphy_pll_bypass_clock (void)
  \brief       Enable bypass clock for MIPI D-PHY PLL.
  \return      none.
*/
static inline void enable_dphy_pll_bypass_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN |= MIPI_CLKEN_BYPASS_CKEN;
}

/**
  \fn          static inline void disable_dphy_pll_bypass_clock (void)
  \brief       Disable bypass clock for MIPI D-PHY PLL.
  \return      none.
*/
static inline void disable_dphy_pll_bypass_clock (void)
{
    CLKCTL_PER_MST->MIPI_CKEN &= ~MIPI_CLKEN_BYPASS_CKEN;
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_DPHY_H_ */
