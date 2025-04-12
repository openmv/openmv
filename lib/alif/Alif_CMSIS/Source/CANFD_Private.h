/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     CANFD_Private.h
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     05-July-2023
 * @brief    Private container of canfd
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef CANFD_PRIVATE_H_
#define CANFD_PRIVATE_H_

#include "canfd.h"
#include "sys_ctrl_canfd.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "Driver_CAN_EX.h"

#define CANFD_MAX_OBJ_SUPPORTED  2U

/*CANFD operational Modes */
typedef enum _CANFD_OP_MODE
{
  CANFD_OP_MODE_NONE               = 0x0,       /* No mode                */
  CANFD_OP_MODE_INIT               = 0x1,       /* Initialization mode    */
  CANFD_OP_MODE_NORMAL             = 0x2,       /* Normal operation mode  */
  CANFD_OP_MODE_MONITOR            = 0x3,       /* Bus monitoring mode    */
  CANFD_OP_MODE_LOOPBACK_INTERNAL  = 0x4,       /* Loopback internal mode */
  CANFD_OP_MODE_LOOPBACK_EXTERNAL  = 0x5        /* Loopback external mode */
}CANFD_OP_MODE;

/* canfd oject status*/
typedef struct _CANFD_OBJ_STATUS
{
    uint8_t            obj_id;                  /* Object ID */
    ARM_CAN_OBJ_CONFIG state;                   /* Object state */
}CANFD_OBJ_STATUS;

/* CANFD Driver state*/
typedef struct _CANFD_DRIVER_STATE
{
    uint32_t    initialized       :1;           /* Driver Initialized    */
    uint32_t    standby           :1;           /* Driver in Standby     */
    uint32_t    powered           :1;           /* Driver Powered up     */
    uint32_t    filter_configured :1;           /* Driver Acceptance filter Configured  */
    uint32_t    rx_busy           :1;           /* Busy in reception     */
    uint32_t    use_prim_buf      :1;           /* Primary buf is in use */
    uint32_t    prim_buf_busy     :1;           /* Primary buf is busy   */
    uint32_t    reserved          :25;          /* Reserved              */
}CANFD_DRIVER_STATE;

/* Resource structure for CANFD */
typedef struct _CANFD_RESOURCES
{
    CANFD_Type                  *regs;                         /* Base address of the current instance of CANFD */
    CANFD_CNT_Type              *cnt_regs;                     /* Base address of the CANFD Timer counter       */
    ARM_CAN_SignalUnitEvent_t   cb_unit_event;                 /* Call back function                            */
    ARM_CAN_SignalObjectEvent_t cb_obj_event;                  /* Call back function for object event           */
    canfd_transfer_t            data_transfer;                 /* Data transfer information                     */
    CANFD_OP_MODE               op_mode;                       /* canfd operational mode                        */
    CANFD_DRIVER_STATE          state;                         /* CANFD Driver State                            */
#if RTE_CANFD_BLOCKING_MODE_ENABLE
    bool                        blocking_mode;                 /* CANFD blocking mode transfer enable           */
#endif
    uint8_t                     irq_priority;                  /* Interrupt priority                            */
    IRQn_Type                   irq_num;                       /* Instance IRQ number                           */
    ARM_CAN_STATUS              status;                        /* CANFD instance status                         */
    bool                        fd_mode;                       /* CANFD Clock Control                           */
    CANFD_OBJ_STATUS            objs[CANFD_MAX_OBJ_SUPPORTED]; /* Number of objects supported */
}CANFD_RESOURCES;

#endif /* CANFD_PRIVATE_H_ */
