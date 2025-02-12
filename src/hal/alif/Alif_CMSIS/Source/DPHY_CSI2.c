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
 * @file     DPHY_CSI.c
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     14-May-2024
 * @brief    Driver for MIPI DPHY CSI2.
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
#include "DPHY_CSI2.h"
#include "csi.h"
#include "sys_ctrl_csi.h"

/*DPHY initialize status global variables*/
static volatile uint32_t csi2_init_status = 0;

/*hsfreqrange and osc_freq_target range*/
extern const DPHY_FREQ_RANGE frequency_range[];

/**
  \fn          static void MIPI_CSI2_DPHY_Shutdown (uint8_t state)
  \brief       PHY shutdown line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
*/
static void MIPI_CSI2_DPHY_Shutdown (uint8_t state)
{
    if(state == ENABLE)
    {
        csi_enable_dphy_shutdown_line((CSI_Type *)CSI_BASE);
    }
    else
    {
        csi_disable_dphy_shutdown_line((CSI_Type *)CSI_BASE);
    }
}

/**
  \fn          static void MIPI_CSI2_DPHY_Testclr (uint8_t state)
  \brief       PHY testclr line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
*/
static void MIPI_CSI2_DPHY_Testclr (uint8_t state)
{
    if(state == ENABLE)
    {
        csi_enable_dphy_testclr_line((CSI_Type *)CSI_BASE);
    }
    else
    {
        csi_disable_dphy_testclr_line((CSI_Type *)CSI_BASE);
    }
}

/**
  \fn          static void MIPI_CSI2_DPHY_Rst (uint8_t state)
  \brief       PHY reset line control callback function.
  \param[in]   state ENABLE/DISABLE the line.
*/
static void MIPI_CSI2_DPHY_Rst (uint8_t state)
{
    if(state == ENABLE)
    {
        csi_enable_dphy_reset_line((CSI_Type *)CSI_BASE);
    }
    else
    {
        csi_disable_dphy_reset_line((CSI_Type *)CSI_BASE);
    }
}

/**
  \fn          static uint8_t MIPI_CSI2_DPHY_Stopstate (void)
  \brief       status of stopstate from PHY
  \return      ret status of stopstate.
*/
static DPHY_STOPSTATE MIPI_CSI2_DPHY_Stopstate (void)
{
    uint8_t ret = 0;

    if(csi_get_lane_stopstate_status((CSI_Type *)CSI_BASE, CSI_LANE_CLOCK) == CSI_LANE_STOPSTATE_ON)
    {
        ret |= DPHY_STOPSTATE_CLOCK;
    }

    if(csi_get_lane_stopstate_status((CSI_Type *)CSI_BASE, CSI_LANE_0) == CSI_LANE_STOPSTATE_ON)
    {
        ret |= DPHY_STOPSTATE_LANE0;
    }

    if(csi_get_lane_stopstate_status((CSI_Type *)CSI_BASE, CSI_LANE_1) == CSI_LANE_STOPSTATE_ON)
    {
        ret |= DPHY_STOPSTATE_LANE1;
    }

    return ret;
}

/**
  \fn          uint8_t DPHY_CSI2_Read_Mask (uint16_t address,
                                            uint8_t  pos,
                                            uint8_t  width)
  \brief       Read Mask CSI2 DPHY registers.
  \param[in]   address is register index.
  \param[in]   pos  is start bit position.
  \param[in]   width is number bits to read.
  \return      return received data from DPHY register.
*/
uint8_t DPHY_CSI2_Read_Mask (uint16_t address,
                             uint8_t  pos,
                             uint8_t  width)
{
    return (MIPI_DPHY_Read(address, DPHY_MODE_CFG_CSI2) >> pos) & ((1 << width) - 1);
}

/**
  \fn          void DPHY_CSI2_Write_Mask (uint16_t address,
                                          uint8_t  data,
                                          uint8_t  pos,
                                          uint8_t  width)
  \brief       write Mask CSI2 DPHY registers.
  \param[in]   address is register index
  \param[in]   data is value to be write to the DPHY register.
  \param[in]   pos  is start bit position.
  \param[in]   width is number bits to write.
*/
void DPHY_CSI2_Write_Mask (uint16_t address,
                           uint8_t  data,
                           uint8_t  pos,
                           uint8_t  width)
{
    uint8_t reg_data = 0;
    uint8_t mask = (1U << width) - 1;

    reg_data = MIPI_DPHY_Read(address, DPHY_MODE_CFG_CSI2);
    reg_data &= ~(mask << pos);
    reg_data |= (data & mask) << pos;
    MIPI_DPHY_Write(address, reg_data, DPHY_MODE_CFG_CSI2);
}

/**
  \fn          void DPHY_PowerEnable (void)
  \brief       Enable DPHY Interface Power.
*/
static void DPHY_PowerEnable (void)
{
    enable_csi_periph_clk();

    enable_rxdphy_configure_clock();

}

/**
  \fn          void DPHY_PowerDisable (void)
  \brief       Disable DPHY Interface Power.
*/
static void DPHY_PowerDisable (void)
{

    disable_csi_periph_clk();

    disable_rxdphy_configure_clock();

}

/**
  \fn          int32_t DPHY_SlaveSetup (uint32_t clock_frequency, uint8_t n_lanes)
  \brief       MIPI DPHY Rx startup sequence.
  \param[in]   clock_frequency DPHY clock frequency.
  \param[in]   n_lanes number of lanes.
  \return      \ref execution_status
*/
static int32_t DPHY_SlaveSetup (uint32_t clock_frequency, uint8_t n_lanes)
{
    uint32_t bitrate_mbps = (clock_frequency * 2)/1000000;
    uint8_t hsfreqrange = 0;
    uint8_t cfgclkfreqrange = 0;
    uint32_t osc_freq_target = 0;
    uint8_t range = 0;
    uint8_t stopstate_check =0;
    uint32_t lp_count = 0;

    csi_set_n_active_lanes((CSI_Type *)CSI_BASE, (n_lanes - 1));

    if(bitrate_mbps < 80 || bitrate_mbps > 2500)
    {
        return ARM_DRIVER_ERROR;
    }

    for(range = 0; (bitrate_mbps > frequency_range[range].bitrate_in_mbps);
        ++range);

    hsfreqrange = frequency_range[range].hsfreqrange;
    osc_freq_target = frequency_range[range].osc_freq_target;

    MIPI_CSI2_DPHY_Rst(DISABLE);

    MIPI_CSI2_DPHY_Shutdown(DISABLE);

    set_rx_dphy_txrx(DPHY_MODE_SLAVE);

    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);
    MIPI_CSI2_DPHY_Testclr(ENABLE);
    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);
    MIPI_CSI2_DPHY_Testclr(ENABLE);

    sys_busy_loop_us(1);

    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);
    MIPI_CSI2_DPHY_Testclr(DISABLE);
    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);
    MIPI_CSI2_DPHY_Testclr(DISABLE);

    set_rx_dphy_hsfreqrange(hsfreqrange);

    DPHY_CSI2_Write_Mask(dphy4txtester_DIG_RDWR_TX_PLL_13, 0x3, 0, 2);

    DPHY_CSI2_Write_Mask(dphy4txtester_DIG_RDWR_TX_CB_1, 0x2, 0, 2);

    DPHY_CSI2_Write_Mask(dphy4txtester_DIG_RDWR_TX_CB_0, 0x2, 5, 2);

    DPHY_CSI2_Write_Mask(dphy4txtester_DIG_RDWR_TX_PLL_9, 0x1, 3, 1);

    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_CLKLANE_LANE_6, 0x1, 7, 1);

    if((bitrate_mbps) == 80)
    {
        DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RD_RX_SYS_1, 0x85, 0, 8);
    }

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_RX_STARTUP_OVR_2, (uint8_t)osc_freq_target, 0, 8);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_RX_STARTUP_OVR_3, (uint8_t)(osc_freq_target >> 8), 0, 4);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_RX_STARTUP_OVR_4, 0x1, 0, 1);

    cfgclkfreqrange = (DPHY_FCFG_CLOCK_MHZ - 17) * 4;

    set_rx_dphy_cfgclkfreqrange(cfgclkfreqrange);

    set_rx_dphy_basedir((1U << n_lanes) - 1);

    set_rx_dphy_forcerxmode((1U << n_lanes) - 1);

    sys_busy_loop_us(1);

    MIPI_CSI2_DPHY_Shutdown(ENABLE);

    sys_busy_loop_us(1);

    MIPI_CSI2_DPHY_Rst(ENABLE);

    stopstate_check |= DPHY_STOPSTATE_CLOCK | (n_lanes == 1 ? (DPHY_STOPSTATE_LANE0) :
                                              (DPHY_STOPSTATE_LANE0) | (DPHY_STOPSTATE_LANE1) );

    while(MIPI_CSI2_DPHY_Stopstate() != stopstate_check)
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

    unset_rx_dphy_forcerxmode((1U << n_lanes) - 1);

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t CSI2_DPHY_Initialize (uint32_t frequency, uint8_t n_lanes)
  \brief       Initialize MIPI CSI2 DPHY Interface.
  \param[in]   frequency to configure DPHY PLL.
  \param[in]   n_lanes number of lanes.
  \return      \ref execution_status
  */
int32_t CSI2_DPHY_Initialize (uint32_t frequency, uint8_t n_lanes)
{
    int32_t ret = ARM_DRIVER_OK;

    if(csi2_init_status == DPHY_INIT_STATUS_INITIALIZED)
    {
        return ARM_DRIVER_OK;

    }

    DPHY_PowerEnable();

    ret = DPHY_SlaveSetup(frequency, n_lanes);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    csi2_init_status = DPHY_INIT_STATUS_INITIALIZED;

    return ret;
}

/**
  \fn          int32_t CSI2_DPHY_Uninitialize (void)
  \brief       Uninitialize MIPI CSI2 DPHY Interface.
  \return      \ref execution_status
  */
int32_t CSI2_DPHY_Uninitialize (void)
{
    if(csi2_init_status == DPHY_INIT_STATUS_UNINITIALIZED)
    {
        return ARM_DRIVER_OK;
    }

    MIPI_CSI2_DPHY_Rst(DISABLE);
    MIPI_CSI2_DPHY_Shutdown(DISABLE);
    DPHY_PowerDisable();

    csi2_init_status = DPHY_INIT_STATUS_UNINITIALIZED;

    return ARM_DRIVER_OK;
}
