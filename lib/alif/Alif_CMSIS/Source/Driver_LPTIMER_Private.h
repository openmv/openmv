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
 * @file     Driver_LPTIMER_Private.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     27-March-2023
 * @brief    Header file for LPTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DRIVER_LPTIMER_PRIVATE_H_
#define DRIVER_LPTIMER_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_LPTIMER.h"
#include "lptimer.h"
#include "sys_ctrl_lptimer.h"

#define LPTIMER_MAX_CHANNEL_NUMBER                          0x4U
#define LPTIMER_CHANNEL_0                                   0x0U
#define LPTIMER_CHANNEL_1                                   0x1U
#define LPTIMER_CHANNEL_2                                   0x2U
#define LPTIMER_CHANNEL_3                                   0x3U

#define LPTIMER_FREE_RUN_MODE                               0x1U   /* To enable Free Run mode in lptimer */
#define LPTIMER_USER_RUN_MODE                               0x0U   /* To enable user mode in lptimer */
#define LPTIMER_CHANNEL_IRQ(chnl)                           (LPTIMER0_IRQ_IRQn + chnl) /* Get corresponding irq number */

/** \brief LPTIMER Driver states. */
typedef volatile struct _LPTIMER_DRIVER_STATE {
    uint32_t initialized : 1; /* Driver Initialized*/
    uint32_t powered     : 1; /* Driver powered */
    uint32_t started     : 1; /* Driver counter started */
    uint32_t set_count1  : 1; /* Driver set count 1 */
    uint32_t reserved    : 28;/* Reserved */
} LPTIMER_DRIVER_STATE;

/** \brief LPTIMER channel information block. */
typedef struct _LPTIMER_CHANNEL_INFO {
    uint8_t                    irq_priority;             /**< channel IRQ priority information >*/
    bool                       mode;                     /**< channel mode user or free run >*/
    LPTIMER_CLK_SRC            clk_src;                  /**< channel clock source >*/
    LPTIMER_DRIVER_STATE       state;                    /**< channel state >*/
    ARM_LPTIMER_SignalEvent_t  CB_function_ptr;          /**< channel call back function >*/
} LPTIMER_CHANNEL_INFO;

/** \brief Resources for a LPTIMER instance. */
typedef struct _LPTIMER_RESOURCES {
    LPTIMER_Type *regs;                                       /**< LPTIMER Register address >*/
    LPTIMER_CHANNEL_INFO ch_info[LPTIMER_MAX_CHANNEL_NUMBER]; /**< Pointer to Info structure of LPTIMER>*/
} LPTIMER_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_LPTIMER_PRIVATE_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
