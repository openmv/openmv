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
 * @file     DPHY_DSI.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     14-May-2024
 * @brief    Driver Specific Header file for DPHY DSI Driver.
 ******************************************************************************/

#ifndef DPHY_DSI_H_
#define DPHY_DSI_H_

#include <stdint.h>

/**
  \fn          int32_t DSI_DPHY_Initialize (uint32_t frequency, uint8_t n_lanes)
  \brief       Initialize MIPI DSI DPHY Interface.
  \param[in]   frequency to configure DPHY PLL.
  \param[in]   n_lanes number of lanes.
  \return      \ref execution_status
  */
int32_t DSI_DPHY_Initialize (uint32_t frequency,  uint8_t n_lanes);

/**
  \fn          int32_t DSI_DPHY_Uninitialize (void)
  \brief       Uninitialize MIPI DSI DPHY Interface.
  \return      \ref execution_status
  */
int32_t DSI_DPHY_Uninitialize (void);

#endif /* DPHY_DSI_H_ */
