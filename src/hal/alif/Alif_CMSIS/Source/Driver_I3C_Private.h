/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DRIVER_I3C_PRIVATE_H
#define DRIVER_I3C_PRIVATE_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_I3C.h"
#include "i3c.h"

/* Check if DMA Support is enable? */
#if RTE_I3C_DMA_ENABLE
#define I3C_DMA_ENABLE  1
#else
#define I3C_DMA_ENABLE  0
#endif

#if I3C_DMA_ENABLE
#include <DMA_Common.h>
#endif

#define I3C_TARGET_SLAVE_TYPE_I2C      (1U << 7U)   /* Represents slave type */
/**
\brief I3C Driver states.
*/
typedef volatile struct _I3C_DRIVER_STATE
{
    uint32_t initialized    : 1; /* Driver initialized    */
    uint32_t powered        : 1; /* Driver powered        */
    uint32_t is_master      : 1; /* Driver master mode    */
    uint32_t enabled        : 1; /* Driver enabled        */
    uint32_t reserved       : 28;/* Reserved              */
} I3C_DRIVER_STATE;

/**
\brief I3C Slave Device Address info
*/
typedef struct _I3C_SLAVE_DAT_TYPE
{
    uint32_t               datp;                 /* DAT (Device Address Table) offset                  */
    uint32_t               maxdevs;              /* maximum number of slaves supported                 */
    uint8_t                addrs[I3C_MAX_DEVS];  /* Assigned dynamic(i3c) or static address(i2c slave) */
    uint32_t               freepos;              /* bitmask of used addresses                          */
    uint32_t               last_asgd_addr_pos;   /* Last assigned slave address positions              */
}I3C_SLAVE_DAT_TYPE;

#if I3C_DMA_ENABLE
typedef struct _I3C_DMA_HW_CONFIG
{
    DMA_PERIPHERAL_CONFIG dma_tx; /* DMA Tx interface */
    DMA_PERIPHERAL_CONFIG dma_rx; /* DMA Rx interface */
} I3C_DMA_HW_CONFIG;
#endif

/**
\brief I3C Device Resources
*/
typedef struct _I3C_RESOURCES
{
    I3C_Type               *regs;            /* Pointer to i3c regs                                */
    ARM_I3C_SignalEvent_t  cb_event;         /* Pointer to call back function                      */

#if I3C_DMA_ENABLE
    ARM_DMA_SignalEvent_t  dma_cb;           /* Pointer to DMA  Callback                           */
#endif

    uint32_t               core_clk;         /* i3c core clock frequency                           */
    I3C_SLAVE_DAT_TYPE     slave_dat;        /* i3c slave devices address local information        */
    i3c_xfer_t             xfer;             /* i3c transfer structure                             */
    ARM_I3C_STATUS         status;           /* i3c driver status                                  */
    I3C_DRIVER_STATE       state;            /* I3C driver state                                   */
#if RTE_I3C_BLOCKING_MODE_ENABLE
    bool                   blocking_mode;    /* I3C blocking mode transfer enable                  */
#endif
    bool                   adaptive_mode;    /* I3C slave I2C/I3C adaptive mode                    */
    IRQn_Type              irq;              /* i3c interrupt number                               */
    uint32_t               irq_priority;     /* i3c interrupt priority                             */

#if I3C_DMA_ENABLE
    I3C_DMA_HW_CONFIG     *dma_cfg;          /* DMA Controller configuration                       */
    const uint32_t         dma_irq_priority; /* DMA IRQ priority number                            */
#endif

}I3C_RESOURCES;


#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_I3C_PRIVATE_H */
