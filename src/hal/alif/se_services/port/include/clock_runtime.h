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
 * @file     clock_runtime.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     29-Nov-2023
 * @brief    Update System Clock values at runtime
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef CLOCK_RUNTIME_H_
#define CLOCK_RUNTIME_H_

#include "stdint.h"

/**
  \fn          int32_t system_update_clock_values(void)
  \brief       Update system clock values retrieved from SE services
  \return      0 for SUCCESS
*/
int32_t system_update_clock_values(void);

#endif /* CLOCK_RUNTIME_H_ */
