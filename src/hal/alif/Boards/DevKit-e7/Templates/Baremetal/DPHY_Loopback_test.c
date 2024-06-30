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
 * @file     DPHY_Loopback_test.c
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     14-May-2024
 * @brief    Driver for MIPI DPHY loopback test.
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
#include "DPHY_Loopback_test.h"

#include "DPHY_CSI2.h"
#include "DPHY_DSI.h"

/*DPHY DSI/CSI Read and Write functions*/
uint8_t DPHY_DSI_Read_Mask (uint16_t address,
                            uint8_t  pos,
                            uint8_t  width);

void DPHY_DSI_Write_Mask (uint16_t address,
                          uint8_t  data,
                          uint8_t  pos,
                          uint8_t  width);

uint8_t DPHY_CSI2_Read_Mask (uint16_t address,
                             uint8_t  pos,
                             uint8_t  width);

void DPHY_CSI2_Write_Mask (uint16_t address,
                           uint8_t  data,
                           uint8_t  pos,
                           uint8_t  width);

/**
  \fn          int32_t DPHY_ADC_Probing_Procedure (void)
  \brief       ADC Probing Procedure.
  \return      \ref execution_status
  */
static int32_t DPHY_ADC_Probing_Procedure (void)
{
    uint32_t lp_count = 0;

    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_CB_2, 0, 0, 2);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_LANE0_LANE_0, 1, 0 , 1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_DAC_0, 0, 0 ,1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_DAC_0, 1, 1 ,1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_DAC_0, 0, 1 ,1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_DAC_0, 1, 0 ,1);

    while(! (DPHY_DSI_Read_Mask(dphy4txtester_DIG_RD_TX_DAC_0, 0, 1)))
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

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_DAC_0, 0, 0 ,1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_DAC_0, 0, 1 ,1);

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t PHY_2_PHY_BIST_Test(void)
  \brief       PHY2PHY High Speed BIST test.
  \return      \ref execution_status
  */
static int32_t PHY_2_PHY_BIST_Test(void)
{
    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);

    DPHY_CSI2_Write_Mask(dphy4txtester_DIG_RDWR_TX_SYS_3, 0x1, 3, 1);

    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_SYS_3, 0x1, 3, 1);

    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_LANE0_LANE_0, 0x1, 7, 1);

    DPHY_DSI_Write_Mask(dphy4txtester_DIG_RDWR_TX_LANE1_LANE_0, 0x1, 7, 1);

    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);

    DPHY_DSI_Write_Mask(dphy4rxtester_DIG_RDWR_RX_BIST_3, 0XF, 0, 8);

    set_tx_dphy_testport_select(DPHY_TESTPORT_SELECT_TX);

    enable_tx_dphy_bist_on();

    if(get_tx_dphy_bist_ok() == DPHY_BIST_OK_STATUS_NOT_SET)
    {
        return ARM_DRIVER_ERROR;
    }


    set_rx_dphy_testport_select(DPHY_TESTPORT_SELECT_RX);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_LANE0_LANE_9, 0x3, 5, 2);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_LANE1_LANE_9, 0x3, 5, 2);

    sys_busy_loop_us(10);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_LANE0_LANE_9, 0x1, 7, 1);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_LANE0_LANE_12, 0x1, 7, 1);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_LANE1_LANE_9, 0x1, 7, 1);

    DPHY_CSI2_Write_Mask(dphy4rxtester_DIG_RDWR_RX_LANE1_LANE_12, 0x1, 7, 1);

    if(DPHY_CSI2_Read_Mask(dphy4rxtester_DIG_RD_RX_LANE0_LANE_7, 0, 8) != 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if(DPHY_CSI2_Read_Mask(dphy4rxtester_DIG_RD_RX_LANE0_LANE_8, 0, 8) != 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if(DPHY_CSI2_Read_Mask(dphy4rxtester_DIG_RD_RX_LANE1_LANE_7, 0, 8) != 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if(DPHY_CSI2_Read_Mask(dphy4rxtester_DIG_RD_RX_LANE1_LANE_8, 0, 8) != 0)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DPHY_External_Loopback_Test (uint32_t frequency, uint32_t loopback_test_run_time_us)
  \brief       External Loopback test.
  \param[in]   frequency to configure DPHY PLL.
  \param[in]   time in microseconds for which loopback test should run.
  \return      return test status TPASS or TFAIL.
  */
int32_t DPHY_External_Loopback_Test (uint32_t frequency, uint32_t loopback_test_run_time_us)
{
    int32_t  ret = ARM_DRIVER_OK;
    uint32_t lp_count = 0;

    /* Configuring Master */
    ret = DSI_DPHY_Initialize (frequency, 2);
    if(ret != ARM_DRIVER_OK)
    {
       return ARM_DRIVER_ERROR;
    }

    /* Configuring ADC */
    ret = DPHY_ADC_Probing_Procedure();
    if(ret != ARM_DRIVER_OK)
    {
        goto error_dsi_uinit;
    }

    /* Configuring Slave*/
    ret = CSI2_DPHY_Initialize(frequency, 2);
    if(ret != ARM_DRIVER_OK)
    {
        goto error_dsi_uinit;
    }

    /*Run PHY2PHY High speed BIST test*/
    ret = PHY_2_PHY_BIST_Test();
    if(ret != ARM_DRIVER_OK)
    {
        goto error_csi2_uinit;
    }

    while(lp_count++ < loopback_test_run_time_us)
    {
        sys_busy_loop_us(1);
    }

    /*Test Failed*/
    if(ret != ARM_DRIVER_OK)
    {
        return TFAIL;
    }

error_csi2_uinit:

    ret = CSI2_DPHY_Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

error_dsi_uinit:

    ret = DSI_DPHY_Uninitialize ();
    if(ret != ARM_DRIVER_OK)
    {
       return ARM_DRIVER_ERROR;
    }

    /*Test Passed*/
    return TPASS;
}
