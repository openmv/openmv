/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*******************************************************************************
 * @file     se_services_port.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     01-Aug-2023
 * @brief    SE Service Porting Header file
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef SE_SERVICES_PORT_H_
#define SE_SERVICES_PORT_H_
#include "services_lib_bare_metal.h"

/* Secure SE Service handle which will be used across the application */
extern uint32_t        se_services_s_handle;

/**
  @fn           void se_services_port_init(void)
  @brief        Initialize the porting layer for SE Servive library
  @return       none
 */
void se_services_port_init(void);

#endif /* SE_SERVICES_PORT_H_ */
