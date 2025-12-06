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
 * @file     WM8904_codec_i2c.h
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     19-Nov-2024
 * @brief    header file for i2c driver for WM8904 Codec Slave Device Communication.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef WM8904_CODEC_I2C_H_
#define WM8904_CODEC_I2C_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Driver_I2C.h"

/**
\brief WM8904 Codec Slave i2c Configuration
*/
typedef struct _WM8904_CODEC_SLAVE_I2C_CONFIG {
    ARM_DRIVER_I2C                        *drv_i2c;                   /* WM8904 Codec i2C driver instance   */
    uint16_t                              bus_speed;                  /* WM8904 Codec i2c Bus Speed         */
    uint8_t                               wm8904_codec_slave_addr;    /* WM8904 Codec slave address         */
} WM8904_CODEC_SLAVE_I2C_CONFIG;


/**
  \fn           int32_t WM8904_codec_i2c_init(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c)
  \brief        initialize i2c driver connected to WM8904 Codec device.
  \param[in]    i2c  : Pointer to i2c configurations structure
                        \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_init(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c);

/**
  \fn           int32_t WM8904_codec_i2c_uninit(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c)
  \brief        un-initialize i2c driver connected to WM8904 Codec device.
  \param[in]    i2c  : Pointer to i2c configurations structure
                        \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_uninit(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c);

/**
  \fn           int32_t WM8904_sensor_i2c_write(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c,
                                                uint8_t                        reg_addr,
                                                uint16_t                        reg_value)
  \brief        write value to WM8904 codec slave device register using i2c.
  \param[in]    i2c        : Pointer to i2c configurations structure
                              \ref WM8904_CODEC_SLAVE_I2C_CONFIG
  \param[in]    reg_addr   : register address of codec Sensor slave device
  \param[in]    reg_value  : register value
  \return       \ref execution_status
*/
int32_t WM8904_codec_i2c_write(WM8904_CODEC_SLAVE_I2C_CONFIG *i2c,
                               uint8_t                        reg_addr,
                               uint16_t                       reg_value);

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
                              uint16_t                      *reg_value);

#ifdef  __cplusplus
}
#endif

#endif /* WM8904_CODEC_I2C_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
