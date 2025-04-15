/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DRIVER_DAC_PRIVATE_H_
#define DRIVER_DAC_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* System includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* Project includes */
#include "dac.h"
#include "Driver_DAC.h"
#include "sys_ctrl_dac.h"

/**
 @brief   : DAC flags to check the DAC initialization, DAC power done and DAC started.
 */
typedef struct _DAC_DRIVER_STATE{
    uint32_t    initialized       :1;           /* Driver Initialized */
    uint32_t    powered           :1;           /* Driver Powered up  */
    uint32_t    dac_drv_start     :1;           /* Driver is Started  */
    uint32_t    reserved          :29;          /* Reserved           */
} DAC_DRIVER_STATE;

/**
 * struct DAC_RESOURCES: structure representing a DAC device
 * @regs     : Register address of the DAC
 * @flags    : DAC driver flags
 * @config   : DAC configuration information
 */
typedef struct _DAC_resources
{
    DAC_Type            *regs;            /* DAC register address                             */
    DAC_DRIVER_STATE     flags;           /* DAC Driver Flags                                 */
    DAC_INSTANCE         instance;        /* DAC Driver instance                              */
    bool                 dac_twoscomp_in; /* Convert two's complement to unsigned binary data */
    uint8_t              input_mux_val;   /* DAC input data source                            */
}DAC_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_DAC_PRIVATE_H_ */
