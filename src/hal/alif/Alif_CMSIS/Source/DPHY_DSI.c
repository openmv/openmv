/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     DPHY_DSI.c
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     14-May-2024
 * @brief    Driver for MIPI DPHY DSI.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
#include <stdint.h>
#include "Driver_Common.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "RTE_Device.h"
#include "DPHY_Test_and_Control_Interface.h"
#include "DPHY_Private.h"
#include "dphy.h"
#include "sys_ctrl_dphy.h"
#include "dsi.h"
#include "sys_ctrl_dsi.h"
#include "DPHY_DSI.h"

/*DPHY initialize status global variables*/
static volatile uint32_t dsi_init_status = 0;

/*hsfreqrange and osc_freq_target range*/
extern const DPHY_FREQ_RANGE frequency_range[];

/*vco_cntrl range*/
static const DPHY_PLL_VCO_CTRL vco_ctrl_range[] =
{
    { 1170, 0x03 }, { 975, 0x07 }, { 853.125, 0x08 }, { 706.875, 0x08 },
    { 585, 0x0B }, { 487.5, 0x0F }, { 426.56, 0x10 }, { 353.4, 0x10 },
    { 292.5, 0x13 }, { 243.75, 0x17 }, { 213.3, 0x18 }, { 176.72, 0x18 },
    { 146.25, 0x1B }, { 121.88, 0x1F }, { 106.64, 0x20 }, { 88.36, 0x20 },
    { 73.13, 0x23}, { 60.93, 0x27 }, { 53.32, 0x28 }, { 44.18, 0x28 },
    { 40, 0x2B}
};

/*Output division factor range*/
static const DPHY_PLL_OUTPUT_DIVISION_FACTOR pll_p_factor[] =
{
    { 1000, 2 }, { 500, 4 }, { 250, 8 }, { 125, 16 }, { 62.5, 32 }, { 40, 64 }
};

/**
  \fn          static void MIPI_DSI_DPHY_Shutdown (uint8_t state)
  \brief       PHY shutdown line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
  */
static void MIPI_DSI_DPHY_Shutdown (uint8_t state)
{
    if(state == ENABLE)
    {
        dsi_phy_shutdown_enable((DSI_Type *)DSI_BASE);
    }
    else
    {
        dsi_phy_shutdown_disable((DSI_Type *)DSI_BASE);
    }
}

/**
  \fn          static void MIPI_DSI_DPHY_Rst (uint8_t state)
  \brief       PHY reset line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
  */
static void MIPI_DSI_DPHY_Rst (uint8_t state)
{
    if(state == ENABLE)
    {
        dsi_phy_reset_enable((DSI_Type *)DSI_BASE);
    }
    else
    {
        dsi_phy_reset_disable((DSI_Type *)DSI_BASE);
    }
}

/**
  \fn          static void MIPI_DSI_DPHY_Enableclk (uint8_t state)
  \brief       PHY enable clock line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
  */
static void MIPI_DSI_DPHY_Enableclk (uint8_t state)
{
    if(state == ENABLE)
    {
        dsi_phy_enable_clock((DSI_Type *)DSI_BASE);
    }
    else
    {
        dsi_phy_disable_clock((DSI_Type *)DSI_BASE);
    }
}

/**
  \fn          static void MIPI_DSI_DPHY_Testclr (uint8_t state)
  \brief       PHY testclr line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
  */
static void MIPI_DSI_DPHY_Testclr (uint8_t state)
{
    if(state == ENABLE)
    {
        dsi_phy_testclr_enable((DSI_Type *)DSI_BASE);
    }
    else
    {
        dsi_phy_testclr_disable((DSI_Type *)DSI_BASE);
    }

}

/**
  \fn          static DSI_PLL_STATUS MIPI_DSI_DPHY_PLL_Lock (void)
  \brief       PHY testclr line control callback function.
  \return      return status of the PLL lock.
  */
static DSI_PLL_STATUS MIPI_DSI_DPHY_PLL_Lock (void)
{
    return dsi_get_phy_lock_status((DSI_Type *)DSI_BASE);
}

/**
  \fn          static uint8_t MIPI_DSI_DPHY_Stopstate (void)
  \brief       status of stopstate from PHY
  \return      return status of stopstate.
  */
static DPHY_STOPSTATE MIPI_DSI_DPHY_Stopstate (void)
{
    uint8_t ret = 0;

    if(dsi_get_lane_stopstate_status((DSI_Type *)DSI_BASE, DSI_LANE_CLOCK) == DSI_LANE_STOPSTATE_ON)
    {
        ret |= DPHY_STOPSTATE_CLOCK;
    }

    if(dsi_get_lane_stopstate_status((DSI_Type *)DSI_BASE, DSI_LANE_0) == DSI_LANE_STOPSTATE_ON)
    {
        ret |= DPHY_STOPSTATE_LANE0;
    }

    if(dsi_get_lane_stopstate_status((DSI_Type *)DSI_BASE, DSI_LANE_1) == DSI_LANE_STOPSTATE_ON)
    {
        ret |= DPHY_STOPSTATE_LANE1;
    }

    return ret;

}

/**
  \fn          uint8_t DPHY_DSI_Read_Mask (uint16_t address,
                                           uint8_t  pos,
                                           uint8_t  width)
  \brief       Read Mask DSI DPHY registers.
  \param[in]   address is register index.
  \param[in]   pos  is start bit position.
  \param[in]   width is number bits to read.
  \return      return received data from DPHY register.
*/
uint8_t DPHY_DSI_Read_Mask (uint16_t address,
                            uint8_t  pos,
                            uint8_t  width)
{
    return (MIPI_DPHY_Read(address, DPHY_MODE_CFG_DSI) >> pos) & ((1 << width) - 1);
}


/**
  \fn          void DPHY_DSI_Write_Mask (uint16_t address,
                                         uint8_t  data,
                                         uint8_t  pos,
                                         uint8_t  width)
  \brief       write Mask DSI DPHY registers.
  \param[in]   address is register index
  \param[in]   data is value to be write to the DPHY register.
  \param[in]   pos  is start bit position.
  \param[in]   width is number bits to write.
*/
void DPHY_DSI_Write_Mask (uint16_t address,
                          uint8_t  data,
                          uint8_t  pos,
                          uint8_t  width)
{
    uint8_t reg_data = 0;
    uint8_t mask = (1U << width) - 1;

    reg_data = MIPI_DPHY_Read(address, DPHY_MODE_CFG_DSI);
    reg_data &= ~(mask << pos);
    reg_data |= (data & mask) << pos;
    MIPI_DPHY_Write(address,reg_data, DPHY_MODE_CFG_DSI);
}

/**
  \fn          void DPHY_PowerEnable (void)
  \brief       Enable DPHY Interface Power.
*/
static void DPHY_PowerEnable (void)
{
    enable_dphy_pll_reference_clock();

    enable_txdphy_configure_clock();

    enable_dsi_periph_clk();
}

/**
  \fn          void DPHY_PowerDisable (void)
  \brief       Disable DPHY Interface Power.
*/
static void DPHY_PowerDisable (void)
{

    disable_dsi_periph_clk();

    disable_txdphy_configure_clock();

    disable_dphy_pll_reference_clock();
}

/**
  \fn          int32_t DPHY_ConfigurePLL(uint32_t clock_frequency)
  \brief       configuring MIPI TX DPHY PLL.
  \param[in]   clock_frequency DPHY clock frequency.
  \return      \ref execution_status
*/
static int32_t DPHY_ConfigurePLL(uint32_t clock_frequency)
{
    float frequency_in_mhz = clock_frequency/1000000.0f;
    uint32_t pll_m = 0;
    uint8_t pll_p = 0;
    uint8_t vco_ctrl = 0;
    uint8_t range = 0;
    pll_config_t pll_config;

    uint8_t pll_n = RTE_MIPI_DSI_PLL_INPUT_DIV_FACTOR_N;

    if(((DPHY_FCLKIN_MHZ/pll_n) > 24) || ((DPHY_FCLKIN_MHZ/pll_n) < 8))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    for( range = 0; (range < ARRAY_SIZE(vco_ctrl_range) - 1) &&
        ((frequency_in_mhz) < vco_ctrl_range[range].frequency_mhz);
        ++range);

    vco_ctrl = vco_ctrl_range[range].vco_ctrl;

    for( range = 0; (range < ARRAY_SIZE(pll_p_factor) - 1) &&
    ((frequency_in_mhz) <= pll_p_factor[range].frequency_mhz);
    ++range);

    pll_p = pll_p_factor[range].p;

    pll_m = (uint32_t)((frequency_in_mhz * pll_n * pll_p * 2) / DPHY_FCLKIN_MHZ);

    set_dphy_pll_clksel(DPHY_PLL_CLKSEL_CLOCK_GENERAT);

    enable_dphy_pll_shadow_clear();

    sys_busy_loop_us(1);

    disable_dphy_pll_shadow_clear();

    pll_config.pll_gmp_ctrl = DPHY_GMP_CNTRL;
    pll_config.pll_m = pll_m;
    pll_config.pll_n = (pll_n - 1);
    pll_config.pll_cpbias_ctrl = DPHY_CPBIAS_CNTRL;
    pll_config.pll_int_ctrl = DPHY_INT_CNTRL;
    pll_config.pll_prop_ctrl = DPHY_PROP_CNTRL;
    pll_config.pll_vco_ctrl = vco_ctrl;

    set_dphy_pll_configuration(&pll_config);

    sys_busy_loop_us(1);

    enable_dphy_updatepll();

    sys_busy_loop_us(1);

    disable_dphy_updatepll();

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_PLL_17, 0x1, 7, 1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_PLL_17, 0x1, 6, 1);

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DPHY_MasterSetup (uint32_t clock_frequency,  uint8_t n_lanes)
  \brief       MIPI DPHY Tx startup sequence.
  \param[in]   clock_frequency DPHY clock frequency.
  \param[in]   n_lanes number of lanes.
  \return      \ref execution_status
*/
static int32_t DPHY_MasterSetup (uint32_t clock_frequency, uint8_t n_lanes)
{
    uint32_t bitrate_mbps = (clock_frequency * 2)/1000000;
    uint8_t hsfreqrange = 0;
    uint8_t cfgclkfreqrange = 0;
    uint8_t range = 0;
    uint8_t stopstate_check = 0;
    uint32_t lp_count = 0;

    if(bitrate_mbps < 80 || bitrate_mbps > 2500)
    {
        return ARM_DRIVER_ERROR;
    }

    for(range = 0; (bitrate_mbps > frequency_range[range].bitrate_in_mbps);
        ++range);

    hsfreqrange = frequency_range[range].hsfreqrange;

    dsi_set_active_lanes((DSI_Type *)DSI_BASE, n_lanes - 1);

    MIPI_DSI_DPHY_Rst(DISABLE);

    MIPI_DSI_DPHY_Shutdown(DISABLE);

    set_tx_dphy_txrx(DPHY_MODE_MASTER);

    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);
    MIPI_DSI_DPHY_Testclr(ENABLE);
    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);
    MIPI_DSI_DPHY_Testclr(ENABLE);

    sys_busy_loop_us(1);

    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);
    MIPI_DSI_DPHY_Testclr(DISABLE);
    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);
    MIPI_DSI_DPHY_Testclr(DISABLE);

    set_tx_dphy_hsfreqrange(hsfreqrange);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_PLL_13, 0x3, 0, 2);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_CB_1, 0x2, 0, 2);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_CB_0, 0x2, 5, 2);

    if(bitrate_mbps < 450)
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_CB_2, 0x1, 4, 1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_CLK_TERMLOWCAP, 0x2, 0, 2);

    if(bitrate_mbps <= 1000)
    {
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SLEW_5,
                            (uint8_t)DPHY_LESS_THEN_1GBPS_SR_OSC_FREQ_TARGET,
                            0, 8);
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SLEW_6,
                            (uint8_t)(DPHY_LESS_THEN_1GBPS_SR_OSC_FREQ_TARGET >> 8),
                            0, 4);
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SLEW_7, 0x1, 4, 1);
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SLEW_7, 0x1, 0, 1);
    }
    else if ((bitrate_mbps > 1000) && (bitrate_mbps <= 1500))
    {
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SLEW_5,
                            (uint8_t)DPHY_MORE_THEN_1GBPS_SR_OSC_FREQ_TARGET,
                            0, 8);
        DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SLEW_6,
                            (uint8_t)(DPHY_MORE_THEN_1GBPS_SR_OSC_FREQ_TARGET >> 8),
                            0, 4);
    }

    cfgclkfreqrange = (DPHY_FCFG_CLOCK_MHZ - 17) * 4;

    set_tx_dphy_cfgclkfreqrange(cfgclkfreqrange);

    if(DPHY_ConfigurePLL(clock_frequency) != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    unset_tx_dphy_basedir((1U << n_lanes) - 1);

    unset_tx_dphy_forcerxmode((1U << n_lanes) - 1);

    sys_busy_loop_us(1);

    MIPI_DSI_DPHY_Enableclk(ENABLE);

    sys_busy_loop_us(1);

    MIPI_DSI_DPHY_Shutdown(ENABLE);

    sys_busy_loop_us(1);

    MIPI_DSI_DPHY_Rst(ENABLE);

    while(MIPI_DSI_DPHY_PLL_Lock() != (DSI_PLL_STATUS) DPHY_PLL_STATUS_PLL_LOCK)
    {
        if(lp_count++ < 1000000)
        {
            sys_busy_loop_us(1);
        }
        else
        {
            return ARM_DRIVER_ERROR;
        }
    }

    stopstate_check = DPHY_STOPSTATE_CLOCK | (n_lanes == 1 ? (DPHY_STOPSTATE_LANE0) :
                                             (DPHY_STOPSTATE_LANE0) | (DPHY_STOPSTATE_LANE1) );

    lp_count = 0;
    while(MIPI_DSI_DPHY_Stopstate() != stopstate_check)
    {
        if(lp_count++ < 1000000)
        {
            sys_busy_loop_us(1);
        }
        else
        {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DSI_DPHY_Initialize (uint32_t frequency,  uint8_t n_lanes)
  \brief       Initialize MIPI DSI DPHY Interface.
  \param[in]   frequency to configure DPHY PLL.
  \param[in]   n_lanes number of lanes.
  \return      \ref execution_status
  */
int32_t DSI_DPHY_Initialize (uint32_t frequency,  uint8_t n_lanes)
{
    int32_t ret = ARM_DRIVER_OK;

    if(dsi_init_status == DPHY_INIT_STATUS_INITIALIZED)
    {
        return ARM_DRIVER_OK;
    }

    DPHY_PowerEnable();

    ret = DPHY_MasterSetup(frequency, n_lanes);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    dsi_init_status = DPHY_INIT_STATUS_INITIALIZED;

    return ret;
}

/**
  \fn          int32_t DSI_DPHY_Uninitialize (void)
  \brief       Uninitialize MIPI DSI DPHY Interface.
  \return      \ref execution_status
  */
int32_t DSI_DPHY_Uninitialize (void)
{
    if(dsi_init_status == DPHY_INIT_STATUS_UNINITIALIZED)
    {
        return ARM_DRIVER_OK;
    }

    MIPI_DSI_DPHY_Rst(DISABLE);
    MIPI_DSI_DPHY_Shutdown(DISABLE);
    MIPI_DSI_DPHY_Enableclk(DISABLE);
    DPHY_PowerDisable();

    dsi_init_status = DPHY_INIT_STATUS_UNINITIALIZED;

    return ARM_DRIVER_OK;
}
