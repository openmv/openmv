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
 * @file     DPHY_Common.c
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     14-May-2024
 * @brief    Driver for MIPI DPHY test and control interface and common data.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include "DPHY_Test_and_Control_Interface.h"
#include "DPHY_Private.h"
#include "dphy.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/*hsfreqrange and osc_freq_target range*/
const DPHY_FREQ_RANGE frequency_range[] =
{
    { 80, 0x00, 0x1E9 }, { 90, 0x10, 0x1E9 }, { 100, 0x20, 0x1E9 },
    { 110, 0x30, 0x1E9 }, { 120, 0x01, 0x1E9 }, { 130, 0x11, 0x1E9 },
    { 140, 0x21, 0x1E9 }, { 150, 0x31, 0x1E9 }, { 160, 0x02, 0x1E9 },
    { 170, 0x12, 0x1E9 }, { 180, 0x22, 0x1E9 }, { 190, 0x32, 0x1E9 },
    { 205, 0x03, 0x1E9 }, { 220, 0x13, 0x1E9 }, { 235, 0x23, 0x1E9 },
    { 250, 0x33, 0x1E9 }, { 275, 0x04, 0x1E9 }, { 300, 0x14, 0x1E9 },
    { 325, 0x25, 0x1E9 }, { 350, 0x35, 0x1E9 }, { 400, 0x05, 0x1E9 },
    { 450, 0x16, 0x1E9 }, { 500, 0x26, 0x1E9 }, { 550, 0x37, 0x1E9 },
    { 600, 0x07, 0x1E9 }, { 650, 0x18, 0x1E9 }, { 700, 0x28, 0x1E9 },
    { 750, 0x39, 0x1E9 }, { 800, 0x09, 0x1E9 }, { 850, 0x19, 0x1E9 },
    { 900, 0x29, 0x1E9 }, { 950, 0x3A, 0x1E9 }, { 1000, 0x0A, 0x1E9 },
    { 1050, 0x1A, 0x1E9 }, { 1100, 0x2A, 0x1E9 }, { 1150, 0x3B, 0x1E9 },
    { 1200, 0x0B, 0x1E9 }, { 1250, 0x1B, 0x1E9 }, { 1300, 0x2B, 0x1E9 },
    { 1350, 0x3C, 0x1E9 }, { 1400, 0x0C, 0x1E9 }, { 1450, 0x1C, 0x1E9 },
    { 1500, 0x2C, 0x1E9 }, { 1550, 0x3D, 0x12F }, { 1600, 0x0D, 0x139 },
    { 1650, 0x1D, 0x143 }, { 1700, 0x2E, 0x14D }, { 1750, 0x3E, 0x156 },
    { 1800, 0x0E, 0x160 }, { 1850, 0x1E, 0x16A }, { 1900, 0x2F, 0x174 },
    { 1950, 0x3F, 0x17D }, { 2000, 0x0F, 0x187 }, { 2050, 0x40, 0x191 },
    { 2100, 0x41, 0x19B }, { 2150, 0x42, 0x19B }, { 2200, 0x43, 0x19B },
    { 2250, 0x44, 0x19B }, { 2300, 0x45, 0x19B }, { 2350, 0x46, 0x19B },
    { 2400, 0x47, 0x19B }, { 2450, 0x48, 0x19B }, { 2500, 0x49, 0x19B }
};

/**
  \fn          static uint8_t MIPI_DPHY_Read (uint16_t address, DPHY_Mode mode)
  \brief       Test and control interface protocol to read DPHY registers.
  \param[in]   address index on DPHY register.
  \param[in]   mode is to select the DPHY mode(CSI2/DSI).
  \return      ret register value.
*/
uint8_t MIPI_DPHY_Read (uint16_t address, DPHY_MODE_CFG mode)
{
    uint8_t ret = 0;
    uint32_t read_reg = 0;
    volatile uint32_t *test_ctrl0 = NULL;
    volatile uint32_t *test_ctrl1 = NULL;

    if(mode == DPHY_MODE_CFG_DSI)
    {
        test_ctrl0 = (uint32_t *)PHY_DSI_TEST_CTRL0_BASE;
        test_ctrl1 = (uint32_t *)PHY_DSI_TEST_CTRL1_BASE;
    }
    else
    {
        test_ctrl0 = (uint32_t *)PHY_CSI_TEST_CTRL0_BASE;
        test_ctrl1 = (uint32_t *)PHY_CSI_TEST_CTRL1_BASE;
    }

    /*Ensure that t(r)x_testclk and t(r)x_testen is set to low*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    CLEAR_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Set t(r)x_testen to high. */
    SET_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Set t(r)x_testen to high. */
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Place 0x00 in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to low (with the falling edge on t(r)x_testclk,
    the t(r)x_testdin signal content is latched internally).*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testen to low*/
    CLEAR_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Place the 8-bit word corresponding to the testcode MSBs in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    read_reg |= _VAL2FLD(PHY_TESTDIN, (address >> 8));
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to high.*/
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testclk to low*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testen to high*/
    SET_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Set t(r)x_testclk to high.*/
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Place the 8-bit word test data in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    read_reg |= _VAL2FLD(PHY_TESTDIN, address);
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to low (with the falling edge on t(r)x_testclk,
    the t(r)x_testdin signal content is latched internally).*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= (PHY_TESTDOUT_Msk);
    ret = _FLD2VAL(PHY_TESTDOUT, read_reg);
    /*Set t(r)x_testen to low.*/
    CLEAR_BIT(*test_ctrl1, PHY_TESTEN_Msk);

    return ret;
}

/**
  \fn          static void MIPI_CSI2_DPHY_Write (uint16_t address, uint8_t data)
  \brief       Test and control interface protocol to Write DPHY registers.
  \param[in]   address index on DPHY register.
  \param[in]   data register value.
  \param[in]   mode is to  select the DPHY mode(CSI2/DSI).
*/
void MIPI_DPHY_Write (uint16_t address, uint8_t data, DPHY_MODE_CFG mode)
{
    uint32_t read_reg = 0;
    volatile uint32_t *test_ctrl0 = NULL;
    volatile uint32_t *test_ctrl1 = NULL;

    if(mode == DPHY_MODE_CFG_DSI)
    {
        test_ctrl0 = (uint32_t *)PHY_DSI_TEST_CTRL0_BASE;
        test_ctrl1 = (uint32_t *)PHY_DSI_TEST_CTRL1_BASE;
    }
    else
    {
        test_ctrl0 = (uint32_t *)PHY_CSI_TEST_CTRL0_BASE;
        test_ctrl1 = (uint32_t *)PHY_CSI_TEST_CTRL1_BASE;
    }

    /*Ensure that t(r)x_testclk and t(r)x_testen is set to low*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    CLEAR_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Set t(r)x_testen to high. */
    SET_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Set t(r)x_testen to high. */
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Place 0x00 in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to low (with the falling edge on t(r)x_testclk,
    the t(r)x_testdin signal content is latched internally).*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testen to low*/
    CLEAR_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Place the 8-bit word corresponding to the testcode MSBs in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    read_reg |= _VAL2FLD(PHY_TESTDIN, (address >> 8));
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to high.*/
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testclk to low*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testen to high*/
    SET_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Set t(r)x_testclk to high.*/
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Place the 8-bit word test data in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    read_reg |= _VAL2FLD(PHY_TESTDIN, address);
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to low (with the falling edge on t(r)x_testclk,
    the t(r)x_testdin signal content is latched internally).*/
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    /*Set t(r)x_testen to low.*/
    CLEAR_BIT(*test_ctrl1, PHY_TESTEN_Msk);
    /*Place the 8-bit word corresponding to the page offset in t(r)x_testdin.*/
    read_reg = READ_REG(*test_ctrl1);
    read_reg &= ~(PHY_TESTDIN_Msk);
    read_reg |= _VAL2FLD(PHY_TESTDIN, data);
    WRITE_REG(*test_ctrl1, read_reg);
    /*Set t(r)x_testclk to high (test data is programmed internally).*/
    SET_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
    CLEAR_BIT(*test_ctrl0, PHY_TESTCLK_Msk);
}
