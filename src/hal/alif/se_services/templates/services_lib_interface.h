
/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**
 * @file services_lib_interface.h
 * @brief Services library interface header file
 * @par
 */
#ifndef __SERVICES_LIB_INTERFACE_H__
#define __SERVICES_LIB_INTERFACE_H__

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#ifndef A32_LINUX
#include "services_lib_bare_metal.h"
#endif
/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

#define CPU_M55_HE         0x1001
#define CPU_M55_HP         0x1002
#define REV_A              0x1
#define REV_B0             0x2
#define REV_A1             0x3

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/*******************************************************************************
 *  G L O B A L   D E F I N E S
 ******************************************************************************/

#define PRINT_BUFFER_SIZE           256

/*******************************************************************************
 *  F U N C T I O N   P R O T O T Y P E S
 ******************************************************************************/

#ifndef A32_LINUX
void SERVICES_Setup(MHU_send_message_t send_message, uint32_t timeout);
#endif
int SERVICES_print(const char * fmt, ...);
int32_t SERVICES_wait_ms(uint32_t wait_time_ms);

void TEST_print(uint32_t services_handle, char *fmt, ...);
void TEST_init(uint32_t services_handle);

#endif /* __SERVICES_LIB_INTERFACE_H__ */
