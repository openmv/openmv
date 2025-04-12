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
 * @file     Driver_CPI_Private.h
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     27-March-2023
 * @brief    CMSIS Driver Private Header file.
 ******************************************************************************/

#ifndef DRIVER_CPI_PRIVATE_H_

#define DRIVER_CPI_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* Project Includes */
#include "Driver_CPI.h"

#include "cpi.h"

#include "sys_ctrl_cpi.h"

/**
 * enum CPI_INSTANCE.
 * CPI instances.
 */
typedef enum _CPI_INSTANCE
{
    CPI_INSTANCE_CPI0,                                    /**< CPI instance as CPI                                */
    CPI_INSTANCE_LPCPI                                    /**< CPI instance as LPCPI                              */
} CPI_INSTANCE;

/** \brief CPI Frame Configuration */
typedef struct _CPI_FRAME_CONFIG {
    uint16_t                             width;           /**< Frame Width(Column)                                */
    uint16_t                             height;          /**< Frame Height(Row)                                  */
} CPI_FRAME_CONFIG;

/** \brief CPI FIFO Configuration */
typedef struct _CPI_FIFO_CONFIG {
    uint8_t                              read_watermark;  /**< FIFO Read  Water mark 0,1: illegal                 */
    uint8_t                              write_watermark; /**< FIFO Write Water mark 0,1: illegal                 */
} CPI_FIFO_CONFIG;

/** \brief CPI Configurations */
typedef struct _CPI_CONFIG {
    CPI_FRAME_CONFIG                     frame;           /**< Frame Configuration                                */
    uint32_t                             framebuff_saddr; /**< Frame Buffer Start Address Configuration           */
    CPI_FIFO_CONFIG                      *fifo;           /**< FIFO Configuration                                 */
}CPI_CONFIG;

/** \brief CPI Status */
typedef struct CPI_DRIVER_STATE {
    uint32_t initialized       : 1;                       /**< Driver Initialized                                 */
    uint32_t powered           : 1;                       /**< Driver powered                                     */
    uint32_t sensor_configured : 1;                       /**< Camera sensor configured                           */
    uint32_t reserved          : 29;                      /**< Reserved                                           */
} CPI_DRIVER_STATE;

/** \brief CPI Device Resource Structure */
typedef struct _CPI_RESOURCES {
    ARM_CPI_SignalEvent_t                 cb_event;       /**< CPI Application Event Callback                     */
    CPI_Type                              *regs;          /**< CPI Register Base Address                          */
    CPI_INSTANCE                          drv_instance;   /**< CPI driver instances                               */
    CPI_DRIVER_STATE                      status;         /**< CPI Status                                         */
    uint8_t                               irq_priority;   /**< CPI Interrupt Priority                             */
    IRQn_Type                             irq_num;        /**< CPI Interrupt Vector Number                        */
    CPI_ROW_ROUNDUP                       row_roundup;    /**< CPI row roundup                                    */
    CPI_MODE_SELECT                       capture_mode;   /**< CPI capture mode                                   */
    CPI_CONFIG                            *cnfg;          /**< CPI Configurations                                 */
} CPI_RESOURCES;

#define DEFAULT_WRITE_WMARK     0x18

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_CPI_PRIVATE_H_ */
