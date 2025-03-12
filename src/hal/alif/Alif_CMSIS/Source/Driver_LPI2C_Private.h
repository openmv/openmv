/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*---Project include ----*/
#include "Driver_I2C.h"
#include "lpi2c.h"

/*---System include ----*/
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#ifndef DRIVER_LPI2C_PRIVATE_H_
#define DRIVER_LPI2C_PRIVATE_H_

typedef volatile struct _LPI2C_DRIVER_STATE {
    uint32_t initialized : 1;                    /**< Driver Initialized */
    uint32_t powered     : 1;                    /**< Driver powered     */
    uint32_t reserved    : 30;                   /**< Reserved           */
} LPI2C_DRIVER_STATE;

/* @brief Structure to save contexts for a lpi2c channel */
typedef struct _LPI2C_RESOURCES{
  ARM_I2C_SignalEvent_t   cb_event;
  ARM_I2C_STATUS          status;
  LPI2C_TYPE              *regs;
  LPI2C_DRIVER_STATE      state;
  LPI2C_XFER_INFO_T       transfer;
  uint32_t                clk;
  IRQn_Type               irq_num;
  uint32_t                irq_priority;
}LPI2C_RESOURCES;

#endif /* DRIVER_LPI2C_PRIVATE_H_ */
