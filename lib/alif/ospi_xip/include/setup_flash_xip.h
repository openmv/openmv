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
 * @file     setup_flash_xip.h
 * @version  V1.0.0
 * @brief    Header file for API to set up flash in XIP mode
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef SETUP_FLASH_XIP_H
#define SETUP_FLASH_XIP_H

#ifdef  __cplusplus
extern "C"
{
#endif

/**
  \fn        int setup_flash_xip(void)
  \brief     This function initializes the Flash and OSPI and enters the XIP mode.
  \param[in] none.
  \return    The status of operation (Success or Failed)
*/
int setup_flash_xip(void);

/**
  \fn         bool flash_xip_enabled(void)
  \brief      Return the status of xip initialization.
  \param[in]  none
  \return     true or false
 */
bool flash_xip_enabled(void);


#ifdef  __cplusplus
}
#endif

#endif/* SETUP_FLASH_XIP_H */
