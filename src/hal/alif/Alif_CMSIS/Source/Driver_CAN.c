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
 * @file     Driver_CAN.c
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     05-July-2023
 * @brief    CMSIS Driver for CANFD.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <string.h>
#include "CANFD_Private.h"

#if !(RTE_CANFD)
    #error "CANFD is not enabled in RTE_Device.h"
#endif

#if !defined (RTE_Drivers_CANFD)
#error "CANFD is not enabled in RTE_Components.h"
#endif

#define ARM_CAN_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

#if (RTE_CANFD_CLK_SPEED > CANFD_MAX_CLK_SPEED)
    #error "CANFD clock speed is exceeded"
#elif (RTE_CANFD_CLK_SPEED < 1U)
    #error "Insufficient CANFD clock speed"
#endif

/* The CANFD clock divisor */
#if RTE_CANFD_CLK_SOURCE
    #define CANFD_CLK_DIVISOR (CANFD_CLK_SRC_160MHZ_CLK / RTE_CANFD_CLK_SPEED)
    #if ((CANFD_CLK_DIVISOR < 2U ) || (CANFD_CLK_DIVISOR > 255U))
        #error "Incorrect CANFD Clock speed"
    #endif
#else
    #define CANFD_CLK_DIVISOR (CANFD_CLK_SRC_38P4MHZ_CLK / RTE_CANFD_CLK_SPEED)
    #if ((CANFD_CLK_DIVISOR < 2U ) || (CANFD_CLK_DIVISOR > 255U))
        #error "Incorrect CANFD Clock speed"
    #endif
#endif

/* CAN Error Warning limit as per CMSIS */
#define CANFD_ERROR_WARNING_LIMIT           96U

/* A map between Data length code to the payload size */
static const uint8_t canfd_dlc_to_payload_map[0x10U] =
                     {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
                      12U, 16U, 20U, 24U, 32U, 48U, 64U};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
        ARM_CAN_API_VERSION,
        ARM_CAN_DRV_VERSION
};

/* Driver Object Capabilities */
static const ARM_CAN_OBJ_CAPABILITIES DriverObjectCapabilities[2U] = {
/* Transmission Object capabilities */
{
        1,    /* Supports transmission */
        0,    /* Supports reception */
        0,    /* Does not support RTR reception and automatic Data Frame transmission */
        0,    /* Does not support RTR transmission and automatic Data Frame reception */
        0,    /* Allows assignment of multiple filters */
        0,    /* Supports exact identifier filtering */
        0,    /* Does not support range identifier filtering */
        0,    /* Supports mask identifier filtering */
        16,   /* 16 messages buffers (FIFO) supported */
        0     /* Reserved */
 },
 /* Receiver Object capabilities */
 {
        0,    /* Supports transmission */
        1,    /* Supports reception */
        0,    /* Does not support RTR reception and automatic Data Frame transmission */
        0,    /* Does not support RTR transmission and automatic Data Frame reception */
        1,    /* Allows assignment of multiple filters */
        1,    /* Supports exact identifier filtering */
        0,    /* Does not support range identifier filtering */
        1,    /* Supports mask identifier filtering */
        16,   /* 16 messages buffers (FIFO) supported */
        0     /* Reserved */
 },
};

/* Driver Object Capabilities */
static const ARM_CAN_CAPABILITIES DriverCapabilities = {
        CANFD_MAX_OBJ_SUPPORTED,    /* 2 can_objects are available */
        0,                          /* Does not support Reentrant functions for
                                       ARM_CAN_ObjectConfigure, Msg Read/Send/Abort */
        1,                          /* Support for CAN with flexible data-rate mode*/
        0,                          /* Does not support restricted operation mode */
        1,                          /* Supports bus monitoring mode */
        1,                          /* Supports Internal loopback mode */
        1,                          /* Supports External loopback mode */
        0                           /* Reserved */
};

/**
  * @fn      ARM_DRIVER_VERSION ARM_CAN_GetVersion(void)
  * @brief   Gets CAN driver version.
  * @note    none
  * @param   none
  * @return  CAN driver Version
 */
__STATIC_INLINE ARM_DRIVER_VERSION ARM_CAN_GetVersion(void)
{
    return DriverVersion;
}

/**
  * @fn      ARM_CAN_OBJ_CAPABILITIES ARM_CAN_ObjectGetCapabilities(uint32_t obj_idx)
  * @brief   Gets CAN driver Object capabilities.
  * @note    none
  * @param   obj_idx - Object Index
  * @return  \ref CAN driver Object capabilities
 */
__STATIC_INLINE ARM_CAN_OBJ_CAPABILITIES ARM_CAN_ObjectGetCapabilities(uint32_t obj_idx)
{
    ARM_CAN_OBJ_CAPABILITIES cap = {0X0U};

    /* Supported objects are -> obj0 and obj1*/
    if(obj_idx < 0x2U)
    {
        /* Returns the capabilities of an object requested by index */
        return DriverObjectCapabilities[obj_idx];
    }
    else
    {
        return cap;
    }
}

/**
  * @fn      ARM_CAN_CAPABILITIES ARM_CAN_GetCapabilities(void)
  * @brief   Gets CAN driver capabilities.
  * @note    none
  * @param   none
  * @return  \ref CAN driver capabilities
 */
__STATIC_INLINE ARM_CAN_CAPABILITIES ARM_CAN_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  * @fn      uint32_t ARM_CAN_GetClock(void)
  * @brief   Retrieves CAN base clock speed.
  * @note    none
  * @param   none
  * @return  \ref CAN driver base clock frequency
 */
__STATIC_INLINE uint32_t ARM_CAN_GetClock(void)
{
    /* Returns the current CANFD clock speed */
    return RTE_CANFD_CLK_SPEED;
}

/**
 * @fn      ARM_CAN_STATUS ARM_CAN_GetStatus(CANFD_RESOURCES *CANFD)
 * @brief   Fetches CANFD status.
 * @note    none.
 * @param   CANFD : Pointer to CANFD resources structure.
 * @return  \ref canfd driver status.
 */
__STATIC_INLINE ARM_CAN_STATUS ARM_CAN_GetStatus(CANFD_RESOURCES *CANFD)
{
    if(canfd_get_bus_status(CANFD->regs) == CANFD_BUS_STATUS_OFF)
    {
        CANFD->status.unit_state = ARM_CAN_UNIT_STATE_BUS_OFF;
    }
    CANFD->status.tx_error_count  = canfd_get_tx_error_count(CANFD->regs);
    CANFD->status.rx_error_count  = canfd_get_rx_error_count(CANFD->regs);

    /* Checks for the last encountered error */
    switch(canfd_get_last_error_code(CANFD->regs))
    {
        case CANFD_MSG_ERROR_BIT:
            CANFD->status.last_error_code = ARM_CAN_LEC_BIT_ERROR;
            break;
        case CANFD_MSG_ERROR_FORM:
            CANFD->status.last_error_code = ARM_CAN_LEC_FORM_ERROR;
            break;
        case CANFD_MSG_ERROR_STUFF:
            CANFD->status.last_error_code = ARM_CAN_LEC_STUFF_ERROR;
            break;
        case CANFD_MSG_ERROR_ACK:
            CANFD->status.last_error_code = ARM_CAN_LEC_ACK_ERROR;
            break;
        case CANFD_MSG_ERROR_CRC:
            CANFD->status.last_error_code = ARM_CAN_LEC_CRC_ERROR;
            break;
        case CANFD_MSG_ERROR_NONE:
        default:
            CANFD->status.last_error_code = ARM_CAN_LEC_NO_ERROR;
            break;
    }
    return CANFD->status;
}

/**
 * @fn      uint8_t CANFD_CalculatePrescaler(uint32_t bitrate,
 *                                           uint8_t seg1,
 *                                           uint8_t seg2)
 * @brief   Calculates the Prescaler for bitrate
 * @note    none.
 * @param   bitrate : Bitrate value
 * @param   seg1    : Bit segment 1
 * @param   seg2    : Bit segment 2
 * @return  \ref canfd driver Bitrate prescaler.
 */
static uint8_t CANFD_CalculatePrescaler(uint32_t bitrate,
                                        uint8_t seg1,
                                        uint8_t seg2)
{
    /* Calculates and returns the prescaler */
    return ((uint8_t)(RTE_CANFD_CLK_SPEED/(bitrate * (seg1 + seg2))));
}

/**
 * @fn      int32_t ARM_CAN_SetBitrate(CANFD_RESOURCES *CANFD,
 *                                     ARM_CAN_BITRATE_SELECT select,
 *                                     uint32_t bitrate,
 *                                     uint32_t bit_segments)
 * @brief   Sets CANFD Bitrate.
 * @note    none.
 * @param   CANFD       : Pointer to CANFD resources structure.
 * @param   select      : Bitrate option
 * @param   bitrate     : Bitrate value
 * @param   bit_segments: Segments present in a bit time
 *                        (propagation, sampling segment 1 and 2)
 * @return  \ref canfd driver Bitrate set status.
 */
static int32_t ARM_CAN_SetBitrate(CANFD_RESOURCES* CANFD,
                                  ARM_CAN_BITRATE_SELECT select,
                                  uint32_t bitrate,
                                  uint32_t bit_segments)
{
    uint8_t seg1      = 0x0U;
    uint8_t seg2      = 0x0U;
    uint8_t sjw       = 0x0U;
    uint8_t prescaler = 0x0U;

    if(CANFD->state.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    /* If operation mode is other than INIT then its an error*/
    if(CANFD->op_mode != CANFD_OP_MODE_INIT)
    {
        return ARM_DRIVER_ERROR;
    }

    /* If bitrate is less than 1b or greater than 10Mb returns an error */
    if((bitrate < 0x1U) || (bitrate > CANFD_MAX_BITRATE))
    {
        return ARM_CAN_INVALID_BITRATE_SELECT;
    }
    /* Stores segmets of a bit time */
    sjw  = ((bit_segments & ARM_CAN_BIT_SJW_Msk) >> ARM_CAN_BIT_SJW_Pos);
    seg1 = ((bit_segments & ARM_CAN_BIT_PHASE_SEG1_Msk) >> ARM_CAN_BIT_PHASE_SEG1_Pos) +
           ((bit_segments & ARM_CAN_BIT_PROP_SEG_Msk) >> ARM_CAN_BIT_PROP_SEG_Pos);
    seg2 = ((bit_segments & ARM_CAN_BIT_PHASE_SEG2_Msk) >> ARM_CAN_BIT_PHASE_SEG2_Pos);

    /* Checks for the validity of the Signal Jump Width*/
    if((sjw < 0x1U) || (sjw > 0x10U) || (sjw > seg2))
    {
        return  ARM_CAN_INVALID_BIT_SJW;
    }

    switch(select)
    {
        case ARM_CAN_BITRATE_NOMINAL:
            /* Checks for the validity of the Segment 1*/
            if((seg1 < 0x2U) || (seg1 > 0x41U))
            {
                return ARM_CAN_INVALID_BIT_PHASE_SEG1;
            }
            /* Checks for the validity of the Segment 2*/
            if((seg2 < 0x1U) || (seg2 > 0x20U))
            {
                return ARM_CAN_INVALID_BIT_PHASE_SEG2;
            }
            prescaler = CANFD_CalculatePrescaler(bitrate, seg1, seg2);
            if((prescaler < 0x1U) || (prescaler > 0x4U))
            {
                return ARM_CAN_INVALID_BITRATE;
            }

            /* Invokes LL function to set the bitrate */
            canfd_set_nominal_bit_time(CANFD->regs, bit_segments, prescaler);

            break;
        case ARM_CAN_BITRATE_FD_DATA:
            /* Checks for the validity of the Segment 1*/
            if((seg1 < 0x2U) || (seg1 > 0x11U))
            {
                return ARM_CAN_INVALID_BIT_PHASE_SEG1;
            }
            /* Checks for the validity of the Segment 2*/
            if((seg2 < 0x1U) || (seg2 > 0x8U))
            {
                return ARM_CAN_INVALID_BIT_PHASE_SEG2;
            }
            prescaler = CANFD_CalculatePrescaler(bitrate, seg1, seg2);
            if((prescaler < 0x1U) || (prescaler > 0x4U))
            {
                return ARM_CAN_INVALID_BITRATE;
            }
            /* Invokes LL function to set the bitrate */
            canfd_set_fd_bit_time(CANFD->regs, bit_segments, prescaler);

            break;
        default:
            return ARM_CAN_INVALID_BITRATE_SELECT;
    }
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_Initialize(CANFD_RESOURCES *CANFD,
 *                                     ARM_CAN_SignalUnitEvent_t cb_unit_event,
 *                                     ARM_CAN_SignalObjectEvent_t cb_object_event)
 * @brief   Initializes CANFD instance.
 * @note    none.
 * @param   CANFD           : Pointer to CANFD resources structure.
 * @param   cb_unit_event   : unit event callback
 * @param   cb_object_event : Object event callback
 * @return  \ref canfd Initialized status.
 */
static int32_t ARM_CAN_Initialize(CANFD_RESOURCES* CANFD,
                                  ARM_CAN_SignalUnitEvent_t cb_unit_event,
                                  ARM_CAN_SignalObjectEvent_t cb_object_event)
{
    /* If CANFD node is already initialized then sends driver OK*/
    if(CANFD->state.initialized == 0x1U)
    {
        return ARM_DRIVER_OK;
    }

    bool blocking_mode = false;
#if RTE_CANFD_BLOCKING_MODE_ENABLE
    if(CANFD->blocking_mode)
    {
        blocking_mode = true;
    }
#endif

    /* If callback functions are null in non-blocking mode,
     * then sends Error parameter */
    if((!blocking_mode) &&
       ((cb_unit_event == NULL) || (cb_object_event == NULL)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Initialize data transfer members */
    CANFD->data_transfer.rx_count    = 0x0U;
    CANFD->data_transfer.tx_count    = 0x0U;
    CANFD->data_transfer.tx_ptr      = NULL;
    CANFD->data_transfer.rx_ptr      = NULL;

    CANFD->op_mode                   = CANFD_OP_MODE_NONE;

    /* Store Callback functions */
    CANFD->cb_unit_event             = cb_unit_event;
    CANFD->cb_obj_event              = cb_object_event;

    CANFD->state.initialized         = 0x1U;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_Uninitialize(CANFD_RESOURCES *CANFD)
 * @brief   Uninitializes CANFD instance.
 * @note    none.
 * @param   CANFD : Pointer to CANFD resources structure.
 * @return  \ref canfd uninitialized status.
 */
static int32_t ARM_CAN_Uninitialize(CANFD_RESOURCES* CANFD)
{
    /* If CANFD node is already uninitialized then sends driver OK*/
    if(CANFD->state.initialized == 0x0U)
    {
        return ARM_DRIVER_OK;
    }

    /* If CANFD node is still Powered ON then sends driver Error*/
    if(CANFD->state.powered == 0x1U)
    {
       return ARM_DRIVER_ERROR;
    }

    /* Un-initialize data transfer members */
    CANFD->data_transfer.rx_count    = 0x0U;
    CANFD->data_transfer.tx_count    = 0x0U;
    CANFD->data_transfer.tx_ptr      = NULL;
    CANFD->data_transfer.rx_ptr      = NULL;

    CANFD->op_mode                   = CANFD_OP_MODE_NONE;

    /* Unload Callback functions */
    CANFD->cb_unit_event             = NULL;
    CANFD->cb_obj_event              = NULL;

    CANFD->state.initialized         = 0x0U;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_PowerControl(CANFD_RESOURCES *SPI,
 *                                       ARM_POWER_STATE state).
 * @brief   Handles the power control for canfd.
 * @note    none.
 * @param   CANFD : Pointer to canfd resources structure.
 * @param   state : power state.
 * @return  \ref  execution_status
 */
static int32_t ARM_CAN_PowerControl(CANFD_RESOURCES* CANFD,
                                    ARM_POWER_STATE state)
{
    if(CANFD->state.initialized == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    bool blocking_mode = false;
#if RTE_CANFD_BLOCKING_MODE_ENABLE
    if(CANFD->blocking_mode)
    {
        blocking_mode = true;
    }
#endif

    switch(state)
    {
        case ARM_POWER_OFF:
            /* If already powered OFF returns OK*/
            if(CANFD->state.powered == 0x0U)
            {
                return ARM_DRIVER_OK;
            }

            /* Enables Standby mode if in already */
            canfd_enable_standby_mode(CANFD->regs);
            CANFD->state.standby = 0x1U;

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Clears Pending IRQs and disables it. Disables CANFD clock */
                NVIC_ClearPendingIRQ(CANFD->irq_num);
                NVIC_DisableIRQ(CANFD->irq_num);
            }

            /* Resets CANFD */
            canfd_reset(CANFD->regs);
            /* Disable CANFD Clock */
            canfd_clock_disable();

            CANFD->state.powered = 0x0U;
            break;
        case ARM_POWER_FULL:
            /* If already powered ON returns OK*/
            if(CANFD->state.powered == 0x1U)
            {
                return ARM_DRIVER_OK;
            }

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Clears Pending IRQs, sets priority and enables it.
                 * Enables CANFD clock */
                NVIC_ClearPendingIRQ(CANFD->irq_num);
                NVIC_SetPriority(CANFD->irq_num, CANFD->irq_priority);
                NVIC_EnableIRQ(CANFD->irq_num);
            }

            /* Enable CANFD Clock */
            canfd_clock_enable(RTE_CANFD_CLK_SOURCE, CANFD_CLK_DIVISOR);

            /* Disable and clear all the interrupts */
            canfd_disable_tx_interrupts(CANFD->regs);
            canfd_disable_rx_interrupts(CANFD->regs);
            canfd_disable_error_interrupts(CANFD->regs);
            canfd_clear_interrupts(CANFD->regs);

            /* Disables Standby mode */
            canfd_disable_standby_mode(CANFD->regs);

            /* Wait for the CANFD Transceiver to get switch from
             * Standy mode to normal state */
            sys_busy_loop_us(CANFD_TRANSCEIVER_STANDBY_DELAY);

            CANFD->state.standby = 0x0U;
            CANFD->state.powered = 0x1U;
            break;
        case ARM_POWER_LOW:
            /* If the system is Powered ON already,
             * then only Power it low. Else error */
            if(CANFD->state.powered == 0x1U)
            {
                /* If Tx from Primary or secondary is active, then
                 * return an error busy */
                if((canfd_stb_tx_active(CANFD->regs)) ||
                   (canfd_ptb_tx_active(CANFD->regs)))
                {
                    return ARM_DRIVER_ERROR_BUSY;
                }
                /* Enables Standby mode*/
                canfd_enable_standby_mode(CANFD->regs);

                CANFD->state.standby = 0x1U;
            }
            else
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            break;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_SetMode(CANFD_RESOURCES* CANFD, ARM_CAN_MODE mode)
 * @brief   Sets the operation mode for canfd.
 * @note    none.
 * @param   CANFD : Pointer to canfd resources structure.
 * @param   mode  : Mode of operation.
 * @return \ref execution_status
 */
static int32_t ARM_CAN_SetMode(CANFD_RESOURCES* CANFD, ARM_CAN_MODE mode)
{
    if(CANFD->state.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    bool blocking_mode = false;
#if RTE_CANFD_BLOCKING_MODE_ENABLE
    if(CANFD->blocking_mode)
    {
        blocking_mode = true;
    }
#endif

    switch(mode)
    {
        case ARM_CAN_MODE_INITIALIZATION:
            /* If already reset, returns OK */
            if(CANFD->op_mode == CANFD_OP_MODE_INIT)
            {
                return ARM_DRIVER_OK;
            }

            /* Resets CANFD */
            canfd_reset(CANFD->regs);

            CANFD->status.unit_state = ARM_CAN_UNIT_STATE_INACTIVE;
            CANFD->op_mode           = CANFD_OP_MODE_INIT;
            break;

        case ARM_CAN_MODE_NORMAL:
            /* If CANFD is already in Normal mode, returns OK */
            if(CANFD->op_mode == CANFD_OP_MODE_NORMAL)
            {
                return ARM_DRIVER_OK;
            }

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Enables CANFD Rx, Tx and error interrupts */
                canfd_enable_tx_interrupts(CANFD->regs);
                canfd_enable_rx_interrupts(CANFD->regs);
                canfd_enable_error_interrupts(CANFD->regs);
            }

            /* Enables Normal mode */
            canfd_enable_normal_mode(CANFD->regs);

            CANFD->status.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
            CANFD->op_mode           = CANFD_OP_MODE_NORMAL;
            break;

        case ARM_CAN_MODE_MONITOR:
            /* If already in Monitor mode returns OK */
            if(CANFD->op_mode == CANFD_OP_MODE_MONITOR)
            {
                return ARM_DRIVER_OK;
            }

            /* If Tx from Primary or secondary is active, then
             * return an error busy */
            if((canfd_stb_tx_active(CANFD->regs)) ||
               (canfd_ptb_tx_active(CANFD->regs)))
            {
                return ARM_DRIVER_ERROR_BUSY;
            }

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Enables CANFD Rx and error interrupts */
                canfd_enable_rx_interrupts(CANFD->regs);
                canfd_enable_error_interrupts(CANFD->regs);
            }
            /* Enables Listen Only Mode */
            canfd_enable_listen_only_mode(CANFD->regs);

            CANFD->status.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
            CANFD->op_mode           = CANFD_OP_MODE_MONITOR;
            break;

        case ARM_CAN_MODE_LOOPBACK_INTERNAL:
            /* If already in Internal loopback mode returns OK */
            if(CANFD->op_mode == CANFD_OP_MODE_LOOPBACK_INTERNAL)
            {
                return ARM_DRIVER_OK;
            }

            /* If msg transmission is happening then return an Error */
            if(canfd_comm_active(CANFD->regs) == true)
            {
                return ARM_DRIVER_ERROR;
            }

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Enables CANFD Rx, Tx and error interrupts */
                canfd_enable_tx_interrupts(CANFD->regs);
                canfd_enable_rx_interrupts(CANFD->regs);
                canfd_enable_error_interrupts(CANFD->regs);
            }
            /* Enables Internal Loopback Mode */
            canfd_enable_internal_loop_back_mode(CANFD->regs);

            CANFD->status.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
            CANFD->op_mode           = CANFD_OP_MODE_LOOPBACK_INTERNAL;
            break;

        case ARM_CAN_MODE_LOOPBACK_EXTERNAL:
            /* If already in External loopback mode returns OK */
            if(CANFD->op_mode == CANFD_OP_MODE_LOOPBACK_EXTERNAL)
            {
                return ARM_DRIVER_OK;
            }

            /* If msg transmission is happening then return an Error */
            if(canfd_comm_active(CANFD->regs) == true)
            {
                return ARM_DRIVER_ERROR;
            }

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Enables CANFD Rx, Tx and error interrupts */
                canfd_enable_tx_interrupts(CANFD->regs);
                canfd_enable_rx_interrupts(CANFD->regs);
                canfd_enable_error_interrupts(CANFD->regs);
            }
            /* Enables External Loopback Mode */
            canfd_enable_external_loop_back_mode(CANFD->regs);

            CANFD->status.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
            CANFD->op_mode           = CANFD_OP_MODE_LOOPBACK_EXTERNAL;
            break;

        case ARM_CAN_MODE_RESTRICTED:
            /* Restricted mode is unsupported */
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    if(CANFD->op_mode != CANFD_OP_MODE_INIT)
    {
        /* Sets the CAN Error and Rx buf almost full warning limits */
        canfd_set_err_warn_limit(CANFD->regs, CANFD_ERROR_WARNING_LIMIT);
        canfd_set_rbuf_almost_full_warn_limit(CANFD->regs, CANFD_RBUF_AFWL_MAX);
    }
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_ObjectSetFilter(CANFD_RESOURCES* CANFD,
 *                                          uint32_t obj_idx,
 *                                          ARM_CAN_FILTER_OPERATION operation,
 *                                          uint32_t id, uint32_t arg)
 * @brief   Sets the object filter for canfd.
 * @note    none.
 * @param   CANFD      : Pointer to canfd resources structure.
 * @param   obj_idx    : Object ID
 * @param   operation  : Type of operation.
 * @param   id         : Acceptance CODE.
 * @param   arg        : Acceptance Mask.
 * @return  \ref execution_status
 */
static int32_t ARM_CAN_ObjectSetFilter(CANFD_RESOURCES* CANFD, uint32_t obj_idx,
                                       ARM_CAN_FILTER_OPERATION operation,
                                       uint32_t id, uint32_t arg)
{
    uint8_t  filter_num            = 0x0U;
    bool     filter_avail          = false;
    canfd_acpt_fltr_t filter_cfg   = {0x0U};

    if(CANFD->state.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    /* If the object is not configured for Reception */
    if((CANFD->objs[ARM_CAN_OBJ_RX - 0x1U].obj_id != obj_idx)      ||
       (CANFD->objs[ARM_CAN_OBJ_RX - 0x1U].state != ARM_CAN_OBJ_RX))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* If none of the filters configured then resets the first filter*/
    if(CANFD->state.filter_configured == 0x0U)
    {
        canfd_reset_acpt_fltrs(CANFD->regs);
    }

    switch(operation)
    {
        case ARM_CAN_FILTER_ID_EXACT_ADD:
            /* If operation mode is other than INIT then its an error*/
            if(CANFD->op_mode != CANFD_OP_MODE_INIT)
            {
                return ARM_DRIVER_ERROR;
            }

            for(filter_num = 0x0U; filter_num < CANFD_MAX_ACCEPTANCE_FILTERS;
                filter_num++)
            {
                if(canfd_get_acpt_fltr_status(CANFD->regs, filter_num) ==
                                              CANFD_ACPT_FLTR_STATUS_FREE)
                {
                    if((id & ARM_CAN_OBJECT_FILTER_EXT_FRAMES) ==
                        ARM_CAN_OBJECT_FILTER_EXT_FRAMES)
                    {
                        filter_cfg.frame_type = CANFD_ACPT_FILTER_CFG_EXT_FRAMES;
                    }
                    else if(id & ARM_CAN_OBJECT_FILTER_STD_FRAMES)
                    {
                        filter_cfg.frame_type = CANFD_ACPT_FILTER_CFG_STD_FRAMES;
                    }
                    else
                    {
                        filter_cfg.frame_type = CANFD_ACPT_FILTER_CFG_ALL_FRAMES;
                    }

                    filter_cfg.ac_code    = ARM_CAN_OBJECT_ID(id);
                    filter_cfg.ac_mask    = 0x0U;
                    filter_cfg.op_code    = CANFD_ACPT_FLTR_OP_ADD_EXACT_ID;
                    filter_cfg.filter     = filter_num;

                    /* If the filter is available, then stores the values*/
                    canfd_enable_acpt_fltr(CANFD->regs, filter_cfg);
                    filter_avail = true;
                    break;
                }
            }
            break;

        case ARM_CAN_FILTER_ID_EXACT_REMOVE:
            for(filter_num = 0x0U; filter_num < CANFD_MAX_ACCEPTANCE_FILTERS;
                filter_num++)
            {
                filter_cfg.filter  = filter_num;
                filter_cfg.op_code = CANFD_ACPT_FLTR_OP_REMOVE_EXACT_ID;
                canfd_get_acpt_fltr_data(CANFD->regs, &filter_cfg);
                if((filter_cfg.ac_code == id) && (filter_cfg.ac_mask == 0x0U))
                {
                    /* If the filter is found with the same ID and
                     *  mask as requested,
                     *  then resets that filter and disables it*/
                    canfd_disable_acpt_fltr(CANFD->regs, filter_num);
                    filter_avail = true;
                    break;
                }
            }
            break;

        case ARM_CAN_FILTER_ID_MASKABLE_ADD:
            /* If operation mode is other than INIT then its an error*/
            if(CANFD->op_mode != CANFD_OP_MODE_INIT)
            {
                return ARM_DRIVER_ERROR;
            }

            for(filter_num = 0x0U; filter_num < CANFD_MAX_ACCEPTANCE_FILTERS;
                filter_num++)
            {
                if(canfd_get_acpt_fltr_status(CANFD->regs, filter_num) ==
                                              CANFD_ACPT_FLTR_STATUS_FREE)
                {
                    if((id & ARM_CAN_OBJECT_FILTER_EXT_FRAMES) ==
                        ARM_CAN_OBJECT_FILTER_EXT_FRAMES)
                    {
                        filter_cfg.frame_type = CANFD_ACPT_FILTER_CFG_EXT_FRAMES;
                    }
                    else if(id & ARM_CAN_OBJECT_FILTER_STD_FRAMES)
                    {
                        filter_cfg.frame_type = CANFD_ACPT_FILTER_CFG_STD_FRAMES;
                    }
                    else
                    {
                        filter_cfg.frame_type = CANFD_ACPT_FILTER_CFG_ALL_FRAMES;
                    }

                    filter_cfg.ac_code    = ARM_CAN_OBJECT_ID(id);
                    filter_cfg.ac_mask    = arg;
                    filter_cfg.op_code    = CANFD_ACPT_FLTR_OP_ADD_MASKABLE_ID;
                    filter_cfg.filter     = filter_num;
                    /* If the filter is available, then configures the values*/
                    canfd_enable_acpt_fltr(CANFD->regs, filter_cfg);
                    filter_avail = true;
                    break;
                }
            }
            break;

        case ARM_CAN_FILTER_ID_MASKABLE_REMOVE:
            for(filter_num = 0x0U; filter_num < CANFD_MAX_ACCEPTANCE_FILTERS;
                filter_num++)
            {
                filter_cfg.filter  = filter_num;
                filter_cfg.op_code = CANFD_ACPT_FLTR_OP_REMOVE_MASKABLE_ID;
                canfd_get_acpt_fltr_data(CANFD->regs, &filter_cfg);
                if((filter_cfg.ac_code == id) && (filter_cfg.ac_mask == arg))
                {
                    /* If the filter is found with the same ID and
                     *  mask as requested,
                     *  then resets that filter and disables it*/
                    canfd_disable_acpt_fltr(CANFD->regs, filter_num);
                    filter_avail = true;
                    break;
                }
            }
            break;

        case ARM_CAN_FILTER_ID_RANGE_ADD:
        case ARM_CAN_FILTER_ID_RANGE_REMOVE:
            /* These features are not supported */
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    /* If atleast one filter is configured then it sets the flag*/
    if(canfd_acpt_fltr_configured(CANFD->regs))
    {
        CANFD->state.filter_configured = 0x1U;
    }
    else
    {
        CANFD->state.filter_configured = 0x0U;
    }

    /* If either the filter is not available or
     * the requested configuration is unavailable
     * then returns Specific error */
    if(!(filter_avail))
    {
        return ARM_DRIVER_ERROR_SPECIFIC;
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_ObjectConfigure(CANFD_RESOURCES* CANFD,
 *                                          uint32_t obj_idx,
 *                                          ARM_CAN_OBJ_CONFIG obj_cfg)
 * @brief   Configures the object of canfd.
 * @note    none.
 * @param   CANFD    : Pointer to canfd resources structure.
 * @param   obj_idx  : Object ID
 * @param   obj_cfg  : Type of configuration.
 * @return  \ref execution_status
 */
static int32_t ARM_CAN_ObjectConfigure(CANFD_RESOURCES* CANFD,
                                       uint32_t obj_idx,
                                       ARM_CAN_OBJ_CONFIG obj_cfg)
{
    /* Only 2 objects supported -> obj0 and obj1*/
    if(obj_idx > 0x1U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch(obj_cfg)
    {
        case ARM_CAN_OBJ_INACTIVE:
            /* Sets the object state to inactive */
            if(CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].obj_id == obj_idx)
            {
                CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].state = ARM_CAN_OBJ_INACTIVE;
            }
            else
            {
                CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].state = ARM_CAN_OBJ_INACTIVE;
            }
            break;

        case ARM_CAN_OBJ_TX:
            /* Sets the object state to Transmit */
            CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].obj_id    = obj_idx;
            CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].state     = ARM_CAN_OBJ_TX;
            break;

        case ARM_CAN_OBJ_RX:
            /* Sets the object state to Receive */
            CANFD->objs[ARM_CAN_OBJ_RX - 0x1U].obj_id    = obj_idx;
            CANFD->objs[ARM_CAN_OBJ_RX - 0x1U].state     = ARM_CAN_OBJ_RX;
            break;

        case ARM_CAN_OBJ_RX_RTR_TX_DATA:
        case ARM_CAN_OBJ_TX_RTR_RX_DATA:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_MessageSend(CANFD_RESOURCES* CANFD,
 *                                      uint32_t obj_idx,
 *                                      ARM_CAN_MSG_INFO *msg_info,
 *                                      const uint8_t *data,
 *                                      uint8_t size)
 * @brief   Prepares and sends the message.
 * @note    none.
 * @param   CANFD      : Pointer to canfd resources structure.
 * @param   obj_idx    : Object ID
 * @param   msg_info   : Pointer to Tx message header
 * @param   data       : Pointer to Tx message payload
 * @param   size       : Length of payload
 * @return  \ref execution_status
 */
static int32_t ARM_CAN_MessageSend(CANFD_RESOURCES* CANFD, uint32_t obj_idx,
                                   ARM_CAN_MSG_INFO *msg_info,
                                   const uint8_t *data, uint8_t size)
{
    if(CANFD->state.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Come out of standby mode before starting transmission */
    if(CANFD->state.standby == 0x1U)
    {
        canfd_disable_standby_mode(CANFD->regs);
        CANFD->state.standby = 0x0U;
        /* Wait for the CANFD Transceiver to get switch from
         * Standy mode to normal state */
        sys_busy_loop_us(CANFD_TRANSCEIVER_STANDBY_DELAY);
    }

    /* If the object is not configured for Transmission */
    if((CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].obj_id != obj_idx)     ||
       (CANFD->objs[ARM_CAN_OBJ_TX - 0x1U].state != ARM_CAN_OBJ_TX))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* If the node is in other than below modes, returns an error */
    if((CANFD->op_mode != CANFD_OP_MODE_NORMAL)               &&
       (CANFD->op_mode != CANFD_OP_MODE_LOOPBACK_EXTERNAL)    &&
       (CANFD->op_mode != CANFD_OP_MODE_LOOPBACK_INTERNAL))
    {
        return ARM_DRIVER_ERROR;
    }

    memset(&CANFD->data_transfer.tx_header, 0x0, sizeof(canfd_tx_info_t));

    /* Perform below if primary Tx buf chosen */
    if(CANFD->state.use_prim_buf)
    {
        if(CANFD->state.prim_buf_busy)
        {
            return ARM_DRIVER_ERROR_BUSY;
        }
        else
        {
            CANFD->data_transfer.tx_header.buf_type = CANFD_BUF_TYPE_PRIMARY;
            CANFD->state.prim_buf_busy              = true;
        }
    }
    else
    {
        /* Perform below if secondary Tx buf chosen */
        if(canfd_stb_free(CANFD->regs))
        {
            CANFD->data_transfer.tx_header.buf_type = CANFD_BUF_TYPE_SECONDARY;
        }
        else
        {
            return ARM_DRIVER_ERROR_BUSY;
        }
    }

    if((msg_info->brs == 0x1U) && (msg_info->rtr == 0x1U))
    {
        /* CANFD message doesn't support RTR frame */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Returns error if the payload size not equal to src buffer size*/
    if(size != canfd_dlc_to_payload_map[msg_info->dlc])
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Returns error if data message length is greater than 8 bytes
     * when its a classical can data or fd mode is disabled */
    if((size > 0x8U) && ((msg_info->edl == 0x0U) ||
       (canfd_in_fd_mode() == false)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(data == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* If the error warning, then returns an error */
    if(canfd_err_warn_limit_reached(CANFD->regs))
    {
        return ARM_DRIVER_ERROR;
    }

    /* Stores the message id based on message frame ID type */
    CANFD->data_transfer.tx_header.frame_type   =
                                   (msg_info->id  >> ARM_CAN_ID_IDE_Pos);
    if(CANFD->data_transfer.tx_header.frame_type)
    {
        CANFD->data_transfer.tx_header.id = (ARM_CAN_EXTENDED_ID(msg_info->id)
                                             & (~ARM_CAN_ID_IDE_Msk));
    }
    else
    {
        CANFD->data_transfer.tx_header.id = ARM_CAN_STANDARD_ID(msg_info->id);
    }

    /* Copies the message header */
    CANFD->data_transfer.tx_header.edl    = msg_info->edl;
    CANFD->data_transfer.tx_header.brs    = msg_info->brs;
    CANFD->data_transfer.tx_header.dlc    = msg_info->dlc;
    CANFD->data_transfer.tx_header.rtr    = msg_info->rtr;

    /* Invokes the low level functions to prepare and send the message */
    canfd_select_tx_buf(CANFD->regs,
                        CANFD->data_transfer.tx_header.buf_type);

#if RTE_CANFD_BLOCKING_MODE_ENABLE
    if(CANFD->blocking_mode)
    {
        /* Invokes blocking mode send function */
        canfd_send_blocking(CANFD->regs, CANFD->data_transfer.tx_header, data, size);
        if(CANFD->state.prim_buf_busy)
        {
            CANFD->state.use_prim_buf  = 0x0U;
            CANFD->state.prim_buf_busy = 0x0U;
        }

    }
    else
#endif
    {
        /* Invokes interrupt mode send function */
        canfd_send(CANFD->regs, CANFD->data_transfer.tx_header, data, size);
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_MessageRead(CANFD_RESOURCES* CANFD,
 *                                      uint32_t obj_idx,
 *                                      ARM_CAN_MSG_INFO *msg_info,
 *                                      const uint8_t *data,
 *                                      uint8_t size)
 * @brief   Receives the message.
 * @note    none.
 * @param   CANFD      : Pointer to canfd resources structure.
 * @param   obj_idx    : Object ID
 * @param   msg_info   : Pointer to Rx message header
 * @param   data       : Pointer to Rx message payload
 * @param   size       : Length of payload
 * @return  \ref execution_status
 */
static int32_t ARM_CAN_MessageRead(CANFD_RESOURCES* CANFD, uint32_t obj_idx,
                                   ARM_CAN_MSG_INFO *msg_info, uint8_t *data,
                                   uint8_t size)
{
    if(CANFD->state.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    /* If the object is not configured for Reception */
    if((CANFD->objs[ARM_CAN_OBJ_RX - 0x1U].obj_id != obj_idx)      ||
       (CANFD->objs[ARM_CAN_OBJ_RX - 0x1U].state != ARM_CAN_OBJ_RX))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Check if Message read is busy */
    if(CANFD->state.rx_busy == true)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Returns parameter error if the Rx buffer is NULL or
     * the length to receive is more than Max supported data size*/
    if((data == NULL) || (size > CANFD_FAST_DATA_FRAME_SIZE_MAX))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Sets reception busy flag */
    CANFD->state.rx_busy            = true;

    CANFD->data_transfer.rx_count   = size;
    CANFD->data_transfer.rx_ptr     = data;

#if RTE_CANFD_BLOCKING_MODE_ENABLE
    if(CANFD->blocking_mode)
    {
        /* Invokes blocking mode receive function */
        canfd_receive_blocking(CANFD->regs, &CANFD->data_transfer);
    }
    else
#endif
    {
        /* Invokes interrupt mode send function */
        canfd_receive(CANFD->regs, &CANFD->data_transfer);
    }

    msg_info->id                    = (CANFD->data_transfer.rx_header.id |
                                      (CANFD->data_transfer.rx_header.frame_type
                                       << ARM_CAN_ID_IDE_Pos));
    msg_info->rtr                   = CANFD->data_transfer.rx_header.rtr;
    msg_info->edl                   = CANFD->data_transfer.rx_header.edl;
    msg_info->brs                   = CANFD->data_transfer.rx_header.brs;
    msg_info->dlc                   = CANFD->data_transfer.rx_header.dlc;
    msg_info->esi                   = CANFD->data_transfer.rx_header.esi;

    /* Resets reception busy flag */
    CANFD->state.rx_busy            = false;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_CAN_Control(CANFD_RESOURCES* CANFD,
 *                                  uint32_t control,
 *                                  uint32_t arg)
 * @brief   Controls CANFD nodes' operation
 * @note    none.
 * @param   CANFD   : Pointer to canfd resources structure.
 * @param   control : Control operation type
 * @param   arg     : Argument for control operation
 * @return  \ref execution_status
 */
static int32_t ARM_CAN_Control(CANFD_RESOURCES* CANFD,
                               uint32_t control,
                               uint32_t arg)
{
    if(CANFD->state.powered == 0x0U)
    {
        return ARM_DRIVER_ERROR;
    }

    switch((control & ARM_CAN_CONTROL_Msk))
    {
        case ARM_CAN_SET_FD_MODE:
            /* setup the CANFD fast data mode */
            CANFD->fd_mode = arg;
            canfd_setup_fd_mode(CANFD->fd_mode);
            break;

        case ARM_CAN_ABORT_MESSAGE_SEND:
            /* Aborts the current data Tx of Secondary buf */
            canfd_abort_tx(CANFD->regs, CANFD_BUF_TYPE_SECONDARY);
            break;

        case ARM_CAN_CONTROL_RETRANSMISSION:
            /* Configures secondary buffers msg retransmission feature */
            canfd_setup_tx_retrans(CANFD->regs,
                                   CANFD_BUF_TYPE_SECONDARY,
                                   (bool)arg);
            break;

        case ARM_CAN_SET_TRANSCEIVER_DELAY:
            /* If operation mode is other than INIT then its an error*/
            if(CANFD->op_mode != CANFD_OP_MODE_INIT)
            {
                return ARM_DRIVER_ERROR;
            }

            /* Sets the Transceived delay */
            if((arg > 0x40U) || (arg < 0x1U))
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            canfd_setup_tx_delay_comp(CANFD->regs, (uint8_t)arg, ENABLE);
            break;

        case ARM_CAN_SET_SPECIFICATION:
            /* If operation mode is other than INIT then its an error*/
            if(CANFD->op_mode != CANFD_OP_MODE_INIT)
            {
                return ARM_DRIVER_ERROR;
            }

            if(arg == ARM_CAN_SPECIFICATION_NON_ISO)
            {
                /* Sets NON-ISO mode */
                canfd_set_specification(CANFD->regs, CANFD_SPEC_NON_ISO);
            }
            else
            {
                /* Sets ISO mode */
                canfd_set_specification(CANFD->regs, CANFD_SPEC_ISO);
            }
            break;

        case ARM_CAN_SET_RBUF_OVERFLOW_MODE:
            if(arg == ARM_CAN_RBUF_OVERWRITE_OLD_MSG)
            {
                /* Configures to overwrite old msg */
                canfd_set_rbuf_overflow_mode(CANFD->regs,
                                             CANFD_RBUF_OVF_MODE_OVERWRITE_OLD_MSG);
            }
            else
            {
                /* Configures to discard new msg*/
                canfd_set_rbuf_overflow_mode(CANFD->regs,
                                             CANFD_RBUF_OVF_MODE_DISCARD_NEW_MSG);
            }
            break;

        case ARM_CAN_SET_RBUF_STORAGE_FORMAT:
            if(arg == ARM_CAN_RBUF_STORAGE_NORMAL_MSG)
            {
                /* Configures to store normal msg */
                canfd_set_rbuf_storage_format(CANFD->regs,
                                              CANFD_RBUF_STORE_NORMAL_MSG);
            }
            else
            {
                /* Configures to store normal and error msgs */
                canfd_set_rbuf_storage_format(CANFD->regs,
                                              CANFD_RBUF_STORE_ALL_MSG);
            }
            break;

        case ARM_CAN_SET_RBUF_ALMOST_FULL_WARN_LIMIT:
            if(arg > CANFD_RBUF_AFWL_MAX)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            /* Sets Rbuf almost full warning limit */
            canfd_set_rbuf_almost_full_warn_limit(CANFD->regs, arg);
            break;

        case ARM_CAN_SET_TRANSMISSION_MODE:
            /* Checks if the Secondary buffer is empty
             * If true, then only perform below operation*/
            if(canfd_stb_empty(CANFD->regs))
            {
                if(arg == ARM_CAN_SET_TRANSMISSION_MODE_FIFO)
                {
                    canfd_set_stb_mode(CANFD->regs,
                                       CANFD_SECONDARY_BUF_MODE_FIFO);
                }
                else
                {
                    canfd_set_stb_mode(CANFD->regs,
                                       CANFD_SECONDARY_BUF_MODE_PRIORITY);
                }
            }
            else
            {
                return ARM_DRIVER_ERROR_BUSY;
            }
            break;

        case ARM_CAN_SET_PRIMARY_TBUF:
            /* Sets prim in_use flag if requested */
            CANFD->state.use_prim_buf = 0x1U;
            break;

        case ARM_CAN_ABORT_PRIMARY_TBUF_MESSAGE_SEND:
            /* Aborts the current data Tx of Primary buf */
            canfd_abort_tx(CANFD->regs, CANFD_BUF_TYPE_PRIMARY);
            break;

        case ARM_CAN_CONTROL_PRIMARY_TBUF_RETRANSMISSION:
            /* Configures primary buffers msg retransmission feature */
            canfd_setup_tx_retrans(CANFD->regs,
                                   CANFD_BUF_TYPE_PRIMARY,
                                   (bool)arg);
            break;

        case ARM_CAN_SET_TIMER_COUNTER:
            /* Sets the counter*/
            canfd_counter_set(CANFD->cnt_regs, arg);
            break;

        case ARM_CAN_CONTROL_TIMER_COUNTER:
            if(arg == ARM_CAN_TIMER_COUNTER_START)
            {
                /* Starts the counter*/
                canfd_counter_start(CANFD->cnt_regs);
            }
            else if(arg == ARM_CAN_TIMER_COUNTER_STOP)
            {
                /* Stops the counter*/
                canfd_counter_stop(CANFD->cnt_regs);
            }
            else if(arg == ARM_CAN_TIMER_COUNTER_CLEAR)
            {
                /* Clears the counter*/
                canfd_counter_clear(CANFD->cnt_regs);
            }
            else
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            break;

        case ARM_CAN_ENABLE_TIMESTAMP:
            if(arg == CAN_TIMESTAMP_POSITION_EOF)
            {
                /* Enables msg timestamp at end of frame*/
                canfd_enable_timestamp(CANFD->regs, CANFD_TIMESTAMP_POSITION_EOF);
            }
            else
            {
                /* Enables msg timestamp at start of frame*/
                canfd_enable_timestamp(CANFD->regs, CANFD_TIMESTAMP_POSITION_SOF);
            }
            break;

        case ARM_CAN_GET_TX_TIMESTAMP:
            if(!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            *((uint32_t*)arg) = canfd_get_tx_timestamp(CANFD->regs);
            break;

        case ARM_CAN_GET_RX_TIMESTAMP:
            if(!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            *((uint32_t*)arg) = CANFD->data_transfer.rx_header.timestamp[0U];
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/* CANFD Driver Instance */
static CANFD_RESOURCES CANFD_RES =
{
    .regs                        = (CANFD_Type*)CANFD_BASE,
    .cnt_regs                    = (CANFD_CNT_Type*)CANFD_CNT_BASE,
    .cb_unit_event               = NULL,
    .cb_obj_event                = NULL,
    .data_transfer               = {0x0U},
    .op_mode                     = CANFD_OP_MODE_NONE,
    .state                       = {0x0U},
#if RTE_CANFD_BLOCKING_MODE_ENABLE
    .blocking_mode               = true,
#endif
    .irq_priority                = RTE_CANFD_IRQ_PRIORITY,
    .irq_num                     = CANFD_IRQ_IRQn,
    .fd_mode                     = false
};

static int32_t ARM_CANx_Initialize(ARM_CAN_SignalUnitEvent_t   cb_unit_event,
                                   ARM_CAN_SignalObjectEvent_t cb_object_event)
{
    return ARM_CAN_Initialize(&CANFD_RES, cb_unit_event, cb_object_event);
}

static int32_t ARM_CANx_Uninitialize(void)
{
    return ARM_CAN_Uninitialize(&CANFD_RES);
}

static int32_t ARM_CANx_PowerControl(ARM_POWER_STATE state)
{
    return ARM_CAN_PowerControl(&CANFD_RES, state);
}

static uint32_t ARM_CANx_GetClock(void)
{
    return ARM_CAN_GetClock();
}

static int32_t ARM_CANx_SetBitrate(ARM_CAN_BITRATE_SELECT select,
                                   uint32_t bitrate, uint32_t bit_segments)
{
    return ARM_CAN_SetBitrate(&CANFD_RES, select, bitrate, bit_segments);
}

static int32_t ARM_CANx_SetMode(ARM_CAN_MODE mode)
{
    return ARM_CAN_SetMode(&CANFD_RES, mode);
}

static int32_t ARM_CANx_ObjectSetFilter(uint32_t obj_idx,
                                        ARM_CAN_FILTER_OPERATION operation,
                                        uint32_t id, uint32_t arg)
{
    return ARM_CAN_ObjectSetFilter(&CANFD_RES, obj_idx, operation, id, arg);
}

static int32_t ARM_CANx_ObjectConfigure(uint32_t obj_idx,
                                        ARM_CAN_OBJ_CONFIG obj_cfg)
{
    return ARM_CAN_ObjectConfigure(&CANFD_RES, obj_idx, obj_cfg);
}

static int32_t ARM_CANx_MessageSend(uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info,
                                    const uint8_t *data, uint8_t size)
{
    return ARM_CAN_MessageSend(&CANFD_RES, obj_idx, msg_info, data, size);
}

static int32_t ARM_CANx_MessageRead(uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info,
                                    uint8_t *data, uint8_t size)
{
    return ARM_CAN_MessageRead(&CANFD_RES, obj_idx, msg_info, data, size);
}

static int32_t ARM_CANx_Control(uint32_t control, uint32_t arg)
{
    return ARM_CAN_Control(&CANFD_RES, control, arg);
}

static ARM_CAN_STATUS ARM_CANx_GetStatus(void)
{
    return ARM_CAN_GetStatus(&CANFD_RES);
}

/**
 * @fn      void CANFD_IRQHandler(void)
 * @brief   Handles the interrupt request
 * @note    none.
 * @param   none
 * @return  none
 */
void CANFD_IRQHandler(void)
{
    uint32_t irq_event = 0U;

    CANFD_RES.status.unit_state      = ARM_CAN_UNIT_STATE_ACTIVE;
    CANFD_RES.status.last_error_code = ARM_CAN_LEC_NO_ERROR;

    /* If the device is in Standby mode, come out of it */
    if(CANFD_RES.state.standby)
    {
        CANFD_RES.state.standby = 0x0U;
    }

    /* Invokes low level function to check the IRQ */
    irq_event = canfd_irq_handler(CANFD_RES.regs);

    if (irq_event & CANFD_RBUF_OVERRUN_EVENT)
    {
        /* If the Rbuf is overrun then performs below operation */
        CANFD_RES.cb_obj_event(CANFD_RES.objs[ARM_CAN_OBJ_RX - 0x1U].obj_id,
                               ARM_CAN_EVENT_RECEIVE_OVERRUN);
        irq_event = (CANFD_RBUF_OVERRUN_EVENT    |
                     CANFD_RBUF_FULL_EVENT       |
                     CANFD_RBUF_ALMOST_FULL_EVENT|
                     CANFD_RBUF_AVAILABLE_EVENT);
    }
    else if(irq_event & CANFD_RBUF_ALMOST_FULL_EVENT)
    {
       /* If the Rx buffer is almost full then performs below operation */
       CANFD_RES.cb_obj_event(CANFD_RES.objs[ARM_CAN_OBJ_RX - 0x1U].obj_id,
                              ARM_CAN_EVENT_RBUF_ALMOST_FULL);
       irq_event = (CANFD_RBUF_AVAILABLE_EVENT  |
                    CANFD_RBUF_FULL_EVENT       |
                    CANFD_RBUF_ALMOST_FULL_EVENT);
    }
    else if(irq_event & CANFD_RBUF_AVAILABLE_EVENT)
    {
        /* If the Rx msg available then performs below operation */
        CANFD_RES.cb_obj_event(CANFD_RES.objs[ARM_CAN_OBJ_RX - 0x1U].obj_id,
                               ARM_CAN_EVENT_RECEIVE);
        irq_event = (CANFD_RBUF_AVAILABLE_EVENT  |
                     CANFD_RBUF_FULL_EVENT       |
                     CANFD_RBUF_ALMOST_FULL_EVENT);
    }
    else if(irq_event & CANFD_SECONDARY_BUF_TX_COMPLETE_EVENT)
    {
        /* If the Secondary buf Tx interrupt is occurred
         * then performs below operation */
        CANFD_RES.cb_obj_event(CANFD_RES.objs[ARM_CAN_OBJ_TX - 0x1U].obj_id,
                               ARM_CAN_EVENT_SEND_COMPLETE);
        irq_event = CANFD_SECONDARY_BUF_TX_COMPLETE_EVENT;
    }
    else if(irq_event & CANFD_PRIMARY_BUF_TX_COMPLETE_EVENT)
    {
        /* If the Secondary buf Tx interrupt is occurred
         * then performs below operation */
        CANFD_RES.state.use_prim_buf  = 0x0U;
        CANFD_RES.state.prim_buf_busy = 0x0U;
        CANFD_RES.cb_obj_event(CANFD_RES.objs[ARM_CAN_OBJ_TX - 0x1U].obj_id,
                               ARM_CAN_EVENT_PRIMARY_TBUF_SEND_COMPLETE);
        irq_event = CANFD_PRIMARY_BUF_TX_COMPLETE_EVENT;
    }
    else if(irq_event & CANFD_ARBTR_LOST_EVENT)
    {
        /* If arbitration lost then perform below operation*/
        CANFD_RES.cb_unit_event((uint32_t)ARM_CAN_EVENT_ARBITRATION_LOST);
    }
    else if(irq_event & CANFD_ERROR_PASSIVE_EVENT)
    {
        if(canfd_error_passive_mode(CANFD_RES.regs) == true)
        {
            /* If Bus error passive then performs below operation*/
            CANFD_RES.status.unit_state = ARM_CAN_UNIT_STATE_PASSIVE;
            CANFD_RES.cb_unit_event((uint32_t)ARM_CAN_EVENT_UNIT_PASSIVE);
        }
        else
        {
            /* If Bus error active then performs below operation*/
            CANFD_RES.cb_unit_event((uint32_t)ARM_CAN_EVENT_UNIT_ACTIVE);
        }
    }
    else if(irq_event & CANFD_ERROR_EVENT)
    {
        if(canfd_get_bus_status(CANFD_RES.regs) == CANFD_BUS_STATUS_OFF)
        {
            /* sets the state to Bus off */
            CANFD_RES.status.unit_state = ARM_CAN_UNIT_STATE_BUS_OFF;
            CANFD_RES.cb_unit_event((uint32_t)ARM_CAN_EVENT_UNIT_BUS_OFF);
        }
        else if(canfd_err_warn_limit_reached(CANFD_RES.regs) == true)
        {
            /* invokes warning event if error warning limit is reached */
            CANFD_RES.cb_unit_event((uint32_t)ARM_CAN_EVENT_UNIT_WARNING);
        }
    }
    else if(irq_event & CANFD_BUS_ERROR_EVENT)
    {
        /* invokes warning event */
        CANFD_RES.cb_unit_event((uint32_t)ARM_CAN_EVENT_UNIT_WARNING);
    }

    /* Invokes the low level api to clear the interrupt */
    canfd_clear_interrupt(CANFD_RES.regs, irq_event);
}

/* CANFD Access structure */
ARM_DRIVER_CAN Driver_CANFD = {
    ARM_CAN_GetVersion,
    ARM_CAN_GetCapabilities,
    ARM_CANx_Initialize,
    ARM_CANx_Uninitialize,
    ARM_CANx_PowerControl,
    ARM_CANx_GetClock,
    ARM_CANx_SetBitrate,
    ARM_CANx_SetMode,
    ARM_CAN_ObjectGetCapabilities,
    ARM_CANx_ObjectSetFilter,
    ARM_CANx_ObjectConfigure,
    ARM_CANx_MessageSend,
    ARM_CANx_MessageRead,
    ARM_CANx_Control,
    ARM_CANx_GetStatus
};
