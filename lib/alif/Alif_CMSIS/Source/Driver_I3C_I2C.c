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
 * @file     Driver_I3C_I2C.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     24-Feb-2022
 * @brief    Driver for I2C over I3C adapter.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/
#include "Driver_I2C.h"
#include "Driver_I3C.h"
#include "RTE_Device.h"

#if !(RTE_I3C)
#error "I3C is not enabled in the RTE_Device.h"
#endif

#if !(RTE_I2CI3C)
#error "I2CI3C is not enabled in the RTE_Device.h"
#endif

#define ARM_I3C_I2C_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_I2C_API_VERSION,
    ARM_I3C_I2C_DRV_VERSION
};

/* I3C driver instance */
extern ARM_DRIVER_I3C Driver_I3C;

/* Pointer to \ref ARM_I2C_SignalEvent  */
static ARM_I2C_SignalEvent_t i2c_event;

/* i2c status parameter */
static ARM_I2C_STATUS i2c_status;

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities = {
    0,  /* supports 10-bit addressing */
    0   /* reserved */
};

/**
 * @fn      void i3c_driver_callback (uint32_t event)
 * @brief   callback routine from the i3c driver mapped to i2c signal events
 * @param   none
 * @retval  none
 */
static void i3c_driver_callback (uint32_t event)
{
    if(event & ARM_I3C_EVENT_TRANSFER_DONE)
    {
        i2c_event(ARM_I2C_EVENT_TRANSFER_DONE);
    }

    if(event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        i2c_event(ARM_I2C_EVENT_TRANSFER_INCOMPLETE);
    }
}

/**
 * @fn      ARM_DRIVER_VERSION ARM_I3C_I2C_GetVersion (void)
 * @brief   get i2c over i3c driver version
 * @param   none
 * @retval  driver version
 */
static ARM_DRIVER_VERSION ARM_I3C_I2C_GetVersion (void)
{
    return DriverVersion;
}

/**
 * @fn      static ARM_I2C_CAPABILITIES ARM_I3C_I2C_GetCapabilities (void)
 * @brief   get i2c over i3c capabilities
 * @param   none
 * @retval  driver capabilities
 */
static ARM_I2C_CAPABILITIES ARM_I3C_I2C_GetCapabilities (void)
{
    return DriverCapabilities;
}

/**
 * @fn      int32_t ConvertI2CBusSpeedToI3C (uint32_t i2c_bus_speed)
 * @brief   get i2c over i3c bus speed
 * @param   i2c_bus_speed    : i2c bus speed
 *          ARM_I2C_BUS_SPEED_STANDARD
 *          ARM_I2C_BUS_SPEED_FAST
 *          ARM_I2C_BUS_SPEED_FAST_PLUS
 * @retval  none
 */
static int32_t ConvertI2CBusSpeedToI3C (uint32_t i2c_bus_speed)
{
    int32_t speed = 0;

    switch (i2c_bus_speed)
    {
        case ARM_I2C_BUS_SPEED_STANDARD:
            /* Standard Speed (100kHz) */
            speed = I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS;
            break;
        case ARM_I2C_BUS_SPEED_FAST:
            /* Fast Speed (400kHz) */
            speed = I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS;
            break;
        case ARM_I2C_BUS_SPEED_FAST_PLUS:
            /* Fast+ Speed (1MHz) */
            speed = I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS;
            break;
        case ARM_I2C_BUS_SPEED_HIGH:
            /* Fast+ Speed (3.4MHz) */
            return ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return speed;
}

/**
 * @fn      int32_t ARM_I3C_I2C_Initialize (ARM_I2C_SignalEvent_t cb_event)
 * @brief   CMSIS-Driver i2c over i3c initialize.
 * @param   cb_event      : Pointer to \ref ARM_I2C_SignalEvent
 * @retval  \ref execution_status
 */
static int32_t ARM_I3C_I2C_Initialize (ARM_I2C_SignalEvent_t cb_event)
{
    if (cb_event == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    i2c_event = cb_event;

    return Driver_I3C.Initialize (i3c_driver_callback);
}

/**
 * @fn      int32_t ARM_I3C_I2C_Uninitialize (void)
 * @brief   CMSIS-Driver i2c over i3c uninitialize
 * @note    none
 * @retval  \ref execution_status
 */
static int32_t ARM_I3C_I2C_Uninitialize (void)
{
    i2c_event = NULL;

    return Driver_I3C.Uninitialize ();
}

/**
 * @fn       int32_t ARM_I3C_I2C_PowerControl (ARM_POWER_STATE state)
 * @brief    Power the driver and enable the NVIC
 * @param    state : Power state
 * @return   \ref execution_status
 */
static int32_t ARM_I3C_I2C_PowerControl (ARM_POWER_STATE state)
{
    return Driver_I3C.PowerControl (state);
}

/**
 * @fn      static int32_t ARM_I3C_I2C_MasterTransmit (uint32_t        addr,
                                                       const uint8_t   *data,
                                                       uint32_t        num,
                                                       bool            xfer_pending)
 * @brief   CMSIS-Driver i2c master transmit
 *          Start sending data to i2c over i3c transmitter
 * @param   addr : Slave address (7-bit)
 * @param   data : Pointer to buffer with data to send to i2c transmitter
 * @param   num  : Number of data items to send
 * @retval  \ref execution_status
 */
static int32_t ARM_I3C_I2C_MasterTransmit (uint32_t        addr,
                                           const uint8_t   *data,
                                           uint32_t        num,
                                           bool            xfer_pending)
{
    int32_t ret, detach_ret;
    ARM_I3C_DEVICE_TYPE dev_type = ARM_I3C_DEVICE_TYPE_I2C;

    /* Currently this feature is not supported */
    (void) xfer_pending;

    /* I2C Master Mode*/
    i2c_status.mode = 1;

    ret = Driver_I3C.AttachSlvDev(dev_type, addr);
    if (ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    ret = Driver_I3C.MasterTransmit (addr, data, num);

    detach_ret = Driver_I3C.Detachdev (addr);
    if(ret == ARM_DRIVER_OK)
    {
        ret = detach_ret;
    }

    return ret;

}

/**
 * @fn      static int32_t ARM_I3C_I2C_MasterReceive (uint32_t   addr,
                                                      uint8_t    *data,
                                                      uint32_t   num,
                                                      bool       xfer_pending)
 * @brief   CMSIS-Driver i2c master receive
 *          Start receiving data from i2c over i3c receiver.
 * @param   addr : addr          Slave address (7-bit)
 * @param   data : Pointer to buffer for data to receive from i2c receiver
 * @param   num  : Number of data items to receive
 * @retval  \ref execution_status
 */
static int32_t ARM_I3C_I2C_MasterReceive (uint32_t   addr,
                                          uint8_t    *data,
                                          uint32_t   num,
                                          bool       xfer_pending)
{
    int32_t ret, detach_ret;
    ARM_I3C_DEVICE_TYPE dev_type = ARM_I3C_DEVICE_TYPE_I2C;

    /* Currently this feature is not supported */
    (void) xfer_pending;

    /* I2C Master Mode*/
    i2c_status.mode = 1;

    ret = Driver_I3C.AttachSlvDev(dev_type, addr);
    if (ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    ret = Driver_I3C.MasterReceive (addr, data, num);

    detach_ret = Driver_I3C.Detachdev (addr);
    if(ret == ARM_DRIVER_OK)
    {
        ret = detach_ret;
    }

    return ret;
}

/**
 * @fn      int32_t ARM_I3C_I2C_SlaveTransmit (const uint8_t *data,
                                               uint32_t      num)
 * @brief   CMSIS-Driver i2c slave transmit
 *          Start sending data to i2c over i3c master.
 * @param   data : Pointer to buffer with data to send to i2c master
 * @param   num  : Number of data items to send
 * @retval  \ref execution_status
 */
static int32_t ARM_I3C_I2C_SlaveTransmit (const uint8_t *data,
                                          uint32_t      num)
{
    (void) data;
    (void) num;

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @fn      int32_t ARM_I3C_I2C_SlaveReceive (uint8_t   *data,
                                              uint32_t  num)
 * @brief   CMSIS-Driver i2c slave receive
 *          Start receiving data from i2c over i3c master.
 * @param   data : Pointer to buffer for data to receive from i2c master
 * @param   num  : Number of data items to receive
 * @retval  \ref execution_status
 */
static int32_t ARM_I3C_I2C_SlaveReceive (uint8_t   *data,
                                         uint32_t  num)
{
    (void) data;
    (void) num;

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @fn      int32_t ARM_I3C_I2C_GetDataCount (void)
 * @brief   CMSIS-Driver i2c get transfer data count
 * @retval  transfer data count
 */
static int32_t ARM_I3C_I2C_GetDataCount (void)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @fn      int32_t ARM_I3C_I2C_Control (uint32_t   control,
                                         uint32_t   arg)
 * @brief   CMSIS-Driver i2c control
 *          Control i2c over i3c Interface.
 * @note    none
 * @param   control : Operation
 * @param   arg     : Argument of operation (optional)
 * @retval  common \ref execution_status and driver specific \ref i2c_execution_status
 */
static int32_t ARM_I3C_I2C_Control (uint32_t   control,
                                    uint32_t   arg)
{
    int32_t ret = ARM_DRIVER_OK;
    int32_t speed = 0;

    switch (control)
    {
        case ARM_I2C_OWN_ADDRESS:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
        case ARM_I2C_BUS_SPEED:
            speed = ConvertI2CBusSpeedToI3C (arg);

            if (speed == ARM_DRIVER_ERROR_UNSUPPORTED)
                return ARM_DRIVER_ERROR_UNSUPPORTED;

            ret = Driver_I3C.Control (I3C_MASTER_INIT, 0);
            ret = Driver_I3C.Control (I3C_MASTER_SET_BUS_MODE, speed);
            ret = Driver_I3C.Control (I3C_MASTER_SETUP_HOT_JOIN_ACCEPTANCE, 0);
            ret = Driver_I3C.Control (I3C_MASTER_SETUP_MR_ACCEPTANCE, 0);
            break;
        case ARM_I2C_BUS_CLEAR:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
        case ARM_I2C_ABORT_TRANSFER:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ret;
}

/**
 * @fn      ARM_I2C_STATUS ARM_I3C_I2C_GetStatus (void)
 * @brief   CMSIS-Driver i2c get status
 * @note    none
 * @retval  ARM_i2c_STATUS
 */
static ARM_I2C_STATUS ARM_I3C_I2C_GetStatus (void)
{
    return i2c_status;
}

/* I2CI3C Driver Instance */
#if (RTE_I2CI3C)

/* I2CI3C Driver Control Block */
extern ARM_DRIVER_I2C Driver_I2CI3C;
ARM_DRIVER_I2C Driver_I2CI3C = {
    ARM_I3C_I2C_GetVersion,
    ARM_I3C_I2C_GetCapabilities,
    ARM_I3C_I2C_Initialize,
    ARM_I3C_I2C_Uninitialize,
    ARM_I3C_I2C_PowerControl,
    ARM_I3C_I2C_MasterTransmit,
    ARM_I3C_I2C_MasterReceive,
    ARM_I3C_I2C_SlaveTransmit,
    ARM_I3C_I2C_SlaveReceive,
    ARM_I3C_I2C_GetDataCount,
    ARM_I3C_I2C_Control,
    ARM_I3C_I2C_GetStatus
};

#endif /* RTE_I2CI3C */
