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
 * @file     Camera_Sensor_i2c.h
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     11-July-2023
 * @brief    i2c definitions for Camera Sensor Slave Device
 *           Communication.
 ******************************************************************************/

#ifndef CAMERA_SENSOR_I2C_H_
#define CAMERA_SENSOR_I2C_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Driver_I2C.h"

/* To ensure proper operation, please allocate sufficient stack
 * space for the camera_sensor_i2c_write_burst API in your application
 * configuration. This includes the 50-byte buffer along with the
 * driver's own stack requirements.*/
#define CAMERA_SENSOR_MAX_BURST_SIZE 50

/**
\brief Camera Sensor i2c Register Address Type
*/
typedef enum _CAMERA_SENSOR_I2C_REG_ADDR_TYPE {
  CAMERA_SENSOR_I2C_REG_ADDR_TYPE_8BIT  = 1,                             /* Camera Sensor i2c Register Address type :  8-bit */
  CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT = 2,                             /* Camera Sensor i2c Register Address type : 16-bit */
} CAMERA_SENSOR_I2C_REG_ADDR_TYPE;

/**
\brief Camera Sensor i2c Register Size
*/
typedef enum _CAMERA_SENSOR_I2C_REG_SIZE {
  CAMERA_SENSOR_I2C_REG_SIZE_8BIT  = 1,                                  /* Camera Sensor i2c Register Size :  8-bit */
  CAMERA_SENSOR_I2C_REG_SIZE_16BIT = 2,                                  /* Camera Sensor i2c Register Size : 16-bit */
  CAMERA_SENSOR_I2C_REG_SIZE_32BIT = 4,                                  /* Camera Sensor i2c Register Size : 32-bit */
} CAMERA_SENSOR_I2C_REG_SIZE;

/**
\brief Camera Sensor Slave i2c Configuration
*/
typedef struct CAMERA_SENSOR_SLAVE_I2C_CONFIG {
  ARM_DRIVER_I2C                        *drv_i2c;                        /* Camera Sensor i2C driver instance             */
  uint16_t                              bus_speed;                       /* Camera Sensor slave i2c Bus Speed             */
  uint8_t                               cam_sensor_slave_addr;           /* Camera Sensor slave i2c Address               */
  CAMERA_SENSOR_I2C_REG_ADDR_TYPE       cam_sensor_slave_reg_addr_type;  /* Camera Sensor slave i2c Register Address type */
} CAMERA_SENSOR_SLAVE_I2C_CONFIG;


/* initialize i2c driver. */
int32_t camera_sensor_i2c_init(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c);

/* write value to Camera Sensor slave register using i2c. */
int32_t camera_sensor_i2c_write(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c,
                                uint32_t                        reg_addr,
                                uint32_t                        reg_value,
                                CAMERA_SENSOR_I2C_REG_SIZE      reg_size);

/* write burst of values to Camera Sensor slave register using i2c. */
int32_t camera_sensor_i2c_write_burst(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c,
                                      uint32_t                        reg_addr,
                                      void                           *reg_values,
                                      CAMERA_SENSOR_I2C_REG_SIZE      reg_size,
                                      uint32_t                        burst_len);

/* read value from Camera Sensor slave register using i2c. */
int32_t camera_sensor_i2c_read(CAMERA_SENSOR_SLAVE_I2C_CONFIG *i2c,
                               uint32_t                        reg_addr,
                               uint32_t                       *reg_value,
                               CAMERA_SENSOR_I2C_REG_SIZE      reg_size);

#ifdef  __cplusplus
}
#endif

#endif /* CAMERA_SENSOR_I2C_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
