/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     WM8904_codec_i2c.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     19-Nov-2024
 * @brief    i2c driver for WM8904 Codec Slave Device Communication.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

/* Project Includes */
#include "WM8904_codec_i2c.h"

#if (defined(RTE_WM8904_CODEC) && defined(RTE_Driver_WM8904))

/* Wrapper function for Delay
 * Delay for millisecond:
 *  Provide busy loop delay
 */
#define DELAY_mSEC(msec)       sys_busy_loop_us(msec * 1000)

/* i2c callback status macros */
#define CB_XferDone   1
#define CB_XferErr    2

/* I2C trasmission status macros */
#define RESTART           (0x01)
#define STOP              (0x00)

/* i2c callback transfer completion flag :
 * used to verify status of previous
 * transferred i3c transmit/receive API calls.
 *
 * value can be CB_XferDone or CB_XferErr.
 */
static volatile uint8_t CB_XferCompletionFlag = 0;

/**
  \fn           void codec_i2c_callback(uint32_t event)
  \brief        i2c callback for WM8904 codec slave device.
  \param[in]    event: I2C Event
  \return       \ref execution_status
*/
static void codec_i2c_callback(uint32_t event)
{
    if (event & (ARM_I2C_EVENT_TRANSFER_INCOMPLETE | ARM_I2C_EVENT_ADDRESS_NACK | ARM_I2C_EVENT_BUS_ERROR)) {
        /* Transfer Error. */
        CB_XferCompletionFlag = CB_XferErr;
    }

    if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
        /* Transfer Done. */
        CB_XferCompletionFlag = CB_XferDone;
    }
}

/**
  \fn           int32_t WM8904_codec_i2c_init(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c)
  \brief        initialize i2c driver connected to WM8904 Codec device.
  \param[in]    i2c  : Pointer to i2c configurations structure
                        \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_init(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c)
{
    ARM_DRIVER_I2C *drv_i2c = i2c->drv_i2c;
    int32_t ret = 0;

    /* Initialize I2C driver */
    ret = drv_i2c->Initialize(codec_i2c_callback);
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Power up I2C peripheral */
    ret = drv_i2c->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK) {
        goto error_uninitialize;
    }

    /* Set I2C speed mode */
    ret = drv_i2c->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    if(ret != ARM_DRIVER_OK) {
        goto error_poweroff;
    }

    return ARM_DRIVER_OK;

error_poweroff:
    /* Power off I2C peripheral */
    ret = drv_i2c->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

error_uninitialize:
    /* Un-initialize I2C driver */
    ret = drv_i2c->Uninitialize();
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    return ARM_DRIVER_ERROR;
}

/**
  \fn           int32_t WM8904_codec_i2c_uninit(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c)
  \brief        un-initialize i2c driver connected to WM8904 Codec device.
  \param[in]    i2c  : Pointer to i2c configurations structure
                        \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_uninit(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c)
{
    ARM_DRIVER_I2C *drv_i2c = i2c->drv_i2c;
    int32_t ret = 0;

    /* Power off I2C peripheral */
    ret = drv_i2c->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Un-initialize I2C driver */
    ret = drv_i2c->Uninitialize();
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WM8904_sensor_i2c_write(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c,
                                                uint8_t                        reg_addr,
                                                uint16_t                       reg_value)
  \brief        write value to WM8904 codec slave device register using i2c.
  \param[in]    i2c        : Pointer to i2c configurations structure
                              \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \param[in]    reg_addr   : register address of codec Sensor slave device
  \param[in]    reg_value  : register value
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_write(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c,
                               uint8_t                        reg_addr,
                               uint16_t                       reg_value)
{
    ARM_DRIVER_I2C *drv_i2c = i2c->drv_i2c;

    /* Max size of tx data = Max slave reg_addr_type(8-bit/1 byte) +
     *                       Max reg_size(16-bit/2 byte)
     */
    uint8_t tx_data[3] = {0};
    uint8_t data_len, i;

    uint32_t temp;
    uint32_t timeout;
    int32_t  ret;
    uint8_t reg_type = sizeof(reg_addr);
    uint8_t reg_size = sizeof(reg_value);

    tx_data[0] = reg_addr;

    /* total data length is register address type + register size */
    data_len = (reg_size + reg_type);

    /* For Transmit, Convert input data from Little Endian to Big Endian. */
    i    = reg_size;
    temp = reg_value;

    while(i--)
    {
        tx_data[reg_type + i] = (temp & 0xff);
        temp >>= 8;
    }

    /* clear i2c callback completion flag. */
    CB_XferCompletionFlag = 0;

    /* Transmit data to i2C TX FIFO. */
    ret = drv_i2c->MasterTransmit(i2c->wm8904_codec_slave_addr, tx_data, data_len, STOP);
    if(ret != ARM_DRIVER_OK) {
        goto error_poweroff;
    }

    /* timeout in millisecond. */
    timeout = 100;

    /* wait for i2c callback within timeout. */
    while(timeout--)
    {
        /* received callback? */
        if(CB_XferCompletionFlag) {
          break;
        }

        /* sleep or wait for millisecond depending on RTOS availability. */
        DELAY_mSEC(1);
    }

    /* i3c module failed to respond? power off and de-init i2c driver and return error. */
    if(!CB_XferCompletionFlag) {
        goto error_poweroff;
    }

    /* return error, if received transfer error. */
    if(CB_XferCompletionFlag == CB_XferErr) {
        return ARM_DRIVER_ERROR;
    }

    /* received transfer success. */
    return ARM_DRIVER_OK;

error_poweroff:
    /* Power off I2C driver */
    ret = drv_i2c->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Un-initialize I2C driver */
    ret = drv_i2c->Uninitialize();
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    return ARM_DRIVER_ERROR;
}

/**
  \fn           int32_t WM8904_codec_i2c_read(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c,
                                              uint8_t                       reg_addr,
                                              uint16_t                      *reg_value)
  \brief        read value from WM8904 Codec slave device register using i2c.
  \param[in]    i2c        : Pointer to WM8904 Codec device i2c configurations structure
                              \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \param[in]    reg_addr   : register address of WM8904 codec slave device
  \param[in]    reg_value  : pointer to register value
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_read(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c,
                              uint8_t                        reg_addr,
                              uint16_t                      *reg_value)
{
    ARM_DRIVER_I2C *drv_i2c = i2c->drv_i2c;

    /* Max size of tx data = Max reg_addr type(1 byte) */
    uint8_t tx_data = 0;

    /* Max size of rx data = Max reg_size(2 byte) */
    uint8_t rx_data[2] = {0};
    uint8_t data_len, i;

    uint16_t temp;
    uint32_t timeout;
    int32_t  ret;
    uint8_t reg_type = sizeof(reg_addr);
    uint8_t reg_size = sizeof(uint16_t);

    tx_data  = reg_addr;

    /* transmit register location, data length is only register address type. */
    data_len = reg_type;

    /* clear i2c callback completion flag. */
    CB_XferCompletionFlag = 0;

    /* Transmit data to I2C TX FIFO. */
    ret = drv_i2c->MasterTransmit(i2c->wm8904_codec_slave_addr, &tx_data, data_len, STOP);
    if(ret != ARM_DRIVER_OK) {
        goto error_poweroff;
    }

    /* timeout in millisecond. */
    timeout = 100;

    /* wait for i2c callback within timeout. */
    while(timeout--)
    {
        /* received callback? */
        if(CB_XferCompletionFlag) {
            break;
        }

        /* sleep or wait for millisecond depending on RTOS availability. */
        DELAY_mSEC(1);
    }

    /* i3c module failed to respond? power off and de-init i2c driver and return error. */
    if(!CB_XferCompletionFlag) {
        goto error_poweroff;
    }

    /* return error, if received transfer error. */
    if(CB_XferCompletionFlag == CB_XferErr) {
        return ARM_DRIVER_ERROR;
    }

    /* received transfer success.
     * now, read from register address location.
     * data length will be register address size. */
    data_len = reg_size;

    /* clear i2c callback completion flag. */
    CB_XferCompletionFlag = 0;

    /* Receive data from I2C RX FIFO. */
    ret = drv_i2c->MasterReceive(i2c->wm8904_codec_slave_addr, rx_data, data_len, 0x00);
    if(ret != ARM_DRIVER_OK) {
        goto error_poweroff;
    }

    /* timeout in millisecond. */
    timeout = 100;

    /* wait for i2c callback within timeout. */
    while(timeout--)
    {
        /* received callback? */
        if(CB_XferCompletionFlag) {
            break;
        }

        /* sleep or wait for millisecond depending on RTOS availability. */
        DELAY_mSEC(1);
    }

    /* i3c module failed to respond? power off and de-init i2c driver and return error. */
    if(!CB_XferCompletionFlag) {
        goto error_poweroff;
    }

    /* return error, if received transfer error. */
    if(CB_XferCompletionFlag == CB_XferErr) {
        return ARM_DRIVER_ERROR;
    }

    /* received transfer success.
     * now for Receive, Convert received data back from Big Endian to Little Endian. */
    i = 0;
    temp = 0;
    for(i = 0; i < reg_size; i++)
    {
        temp <<= 8;
        temp |= rx_data[i];
    }

    /* update register value */
    *reg_value = temp;

    return ARM_DRIVER_OK;

error_poweroff:
    /* Power off I2C peripheral */
    ret = drv_i2c->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* Un-initialize I2C driver */
    ret = drv_i2c->Uninitialize();
    if(ret != ARM_DRIVER_OK) {
        return ret;
    }

    return ARM_DRIVER_ERROR;
}

#endif /* #if (defined(RTE_WM8904_CODEC) && defined(RTE_Driver_WM8904)) */
/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
