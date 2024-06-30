/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     Driver_SAI_EX.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     30-Sep-2023
 * @brief    Extended Header for SAI Driver
 * @bug      None
 * @Note     None
 ******************************************************************************/

#ifndef Driver_SAI_EX_H_
#define Driver_SAI_EX_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/****** SAI Control Codes *****/
#define ARM_SAI_USE_CUSTOM_DMA_MCODE_TX           (0xA0UL)    ///< Use User defined DMA microcode arg1 provides address
#define ARM_SAI_USE_CUSTOM_DMA_MCODE_RX           (0xA1UL)    ///< Use User defined DMA microcode arg1 provides address

#ifdef  __cplusplus
}
#endif

#endif /* Driver_SAI_EX_H_ */
