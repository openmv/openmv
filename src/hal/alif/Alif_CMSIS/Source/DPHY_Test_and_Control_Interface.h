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
 * @file     DPHY_Test_and_Control_Interface.h
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     14-May-2024
 * @brief    Driver for MIPI DPHY test and control interface.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef DPHY_TEST_AND_CONTROL_INTERFACE_H_
#define DPHY_TEST_AND_CONTROL_INTERFACE_H_

#include <stdint.h>
#include "DPHY_Private.h"

/**
  \fn          static uint8_t MIPI_DPHY_Read (uint16_t address, DPHY_Mode mode)
  \brief       Test and control interface protocol to read DPHY registers.
  \param[in]   address index on DPHY register.
  \param[in]   mode is to select the DPHY mode(CSI2/DSI).
  \return      ret register value.
*/
uint8_t MIPI_DPHY_Read (uint16_t address, DPHY_MODE_CFG mode);

/**
  \fn          static void MIPI_CSI2_DPHY_Write (uint16_t address, uint8_t data)
  \brief       Test and control interface protocol to Write DPHY registers.
  \param[in]   address index on DPHY register.
  \param[in]   data register value.
  \param[in]   mode is to  select the DPHY mode(CSI2/DSI).
*/
void MIPI_DPHY_Write (uint16_t address, uint8_t data, DPHY_MODE_CFG mode);

#endif /* DPHY_TEST_AND_CONTROL_INTERFACE_H_ */
