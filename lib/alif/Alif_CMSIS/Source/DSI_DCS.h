/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     DSI_DCS.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     01-July-2022
 * @brief    DCS Specific Header file for MIPI DSI Driver.
 ******************************************************************************/

#ifndef MIPI_DSI_DCS_H_
#define MIPI_DSI_DCS_H_

/**
  \fn          void DSI_DCS_Short_Write (uint8_t cmd, uint8_t data)
  \brief       Perform MIPI DSI DCS Short write.
  \param[in]   cmd is DCS command info.
  \param[in]   data to send.
*/
void DSI_DCS_Short_Write (uint8_t cmd, uint8_t data);

/**
  \fn          void DSI_DCS_CMD_Short_Write (uint8_t cmd)
  \brief       Perform MIPI DSI DCS Short write only command.
  \param[in]   cmd is DCS command info.
*/
void DSI_DCS_CMD_Short_Write (uint8_t cmd);

/**
  \fn          void DSI_DCS_Long_Write (uint8_t cmd, uint32_t data)
  \brief       Perform MIPI DSI DCS Short write.
  \param[in]   data pointer to data buffer.
  \param[in]   len data buffer length.
*/
void DSI_DCS_Long_Write (uint8_t* data, uint32_t len);

#endif /* MIPI_DSI_DCS_H_ */
