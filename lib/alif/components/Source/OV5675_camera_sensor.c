/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "Camera_Sensor.h"
#include "Camera_Sensor_i2c.h"
#include "Driver_GPIO.h"
#include "Driver_CPI.h"

#if (RTE_OV5675_CAMERA_SENSOR_ENABLE)

/* Wrapper function for i2c read
 *  read register value from OV5675 Camera Sensor registers
 *   using i2c read API \ref camera_sensor_i2c_read
 *
 *  for OV5675 Camera Sensor specific i2c configurations
 *   see \ref OV5675_camera_sensor_i2c_cnfg
 */
#define OV5675_READ_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_read(&OV5675_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/* Wrapper function for i2c write
 *  write register value to OV5675 Camera Sensor registers
 *   using i2c write API \ref camera_sensor_i2c_write.
 *
 *  for OV5675 Camera Sensor specific i2c configurations
 *   see \ref OV5675_camera_sensor_i2c_cnfg
 */
#define OV5675_WRITE_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_write(&OV5675_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)


#define OV5675_CAMERA_SENSOR_SLAVE_ADDR 0x36

#define OV5675_CHIP_ID_REGISTER_VALUE   0x5675

#define OV5675_REG_CHIPID_H         0x300b
#define OV5675_REG_CHIPID_L         0x300c

#define OV5675_REG_MODE_SELECT      0x0100
#define OV5675_MODE_STANDBY         0x00
#define OV5675_MODE_STREAMING       0x01

/*Helper macro*/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
\brief OV5675 Camera Sensor Register Array Structure
       used for Camera Configuration.
*/
typedef struct _OV5675_REG {
    uint16_t reg_addr;             /* OV5675 Camera Sensor Register Address*/
    uint16_t reg_value;            /* OV5675 Camera Sensor Register Value*/
} OV5675_REG;

/* Wrapper function for Delay
 * Delay for microsecond:
 * Provide busy loop delay
 */
#define OV5675_DELAY_uSEC(usec)       sys_busy_loop_us(usec)

/* OV5675 Camera power GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_OV5675_CAMERA_SENSOR_POWER_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_PWR = &ARM_Driver_GPIO_(RTE_OV5675_CAMERA_SENSOR_POWER_GPIO_PORT);

/* OV5675 Camera reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_OV5675_CAMERA_SENSOR_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_RST = &ARM_Driver_GPIO_(RTE_OV5675_CAMERA_SENSOR_RESET_GPIO_PORT);

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(RTE_OV5675_CAMERA_SENSOR_I2C_INSTANCE);

/**
\brief OV5675 Camera Sensor slave i2c Configuration
\ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
*/
static CAMERA_SENSOR_SLAVE_I2C_CONFIG OV5675_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(RTE_OV5675_CAMERA_SENSOR_I2C_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = OV5675_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/**
\brief OV5675 Camera Sensor Resolution 1296x972
*/
static const OV5675_REG OV5675_1296x972_10bpp[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x0300, 0x04},
    {0x0302, 0x8d},
    {0x0303, 0x00},
    {0x030d, 0x26},
    {0x3002, 0x21},
    {0x3107, 0x23},
    {0x3501, 0x20},
    {0x3503, 0x0c},
    {0x3508, 0x03},
    {0x3509, 0x00},
    {0x3600, 0x66},
    {0x3602, 0x30},
    {0x3610, 0xa5},
    {0x3612, 0x93},
    {0x3620, 0x80},
    {0x3642, 0x0e},
    {0x3661, 0x00},
    {0x3662, 0x08},
    {0x3664, 0xf3},
    {0x3665, 0x9e},
    {0x3667, 0xa5},
    {0x366e, 0x55},
    {0x366f, 0x55},
    {0x3670, 0x11},
    {0x3671, 0x11},
    {0x3672, 0x11},
    {0x3673, 0x11},
    {0x3714, 0x28},
    {0x371a, 0x3e},
    {0x3733, 0x10},
    {0x3734, 0x00},
    {0x373d, 0x24},
    {0x3764, 0x20},
    {0x3765, 0x20},
    {0x3766, 0x12},
    {0x37a1, 0x14},
    {0x37a8, 0x1c},
    {0x37ab, 0x0f},
    {0x37c2, 0x14},
    {0x37cb, 0x00},
    {0x37cc, 0x00},
    {0x37cd, 0x00},
    {0x37ce, 0x00},
    {0x37d8, 0x02},
    {0x37d9, 0x04},
    {0x37dc, 0x04},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x3f},
    {0x3806, 0x07},
    {0x3807, 0xb7},
    {0x3808, 0x05},
    {0x3809, 0x10},
    {0x380a, 0x03},
    {0x380b, 0xcc},
    {0x380c, 0x02},
    {0x380d, 0xee},
    {0x380e, 0x07},
    {0x380f, 0xd0},
    {0x3811, 0x08},
    {0x3813, 0x0d},
    {0x3814, 0x03},
    {0x3815, 0x01},
    {0x3816, 0x03},
    {0x3817, 0x01},
    {0x381e, 0x02},
    {0x3820, 0x8b},
    {0x3821, 0x01},
    {0x3832, 0x04},
    {0x3c80, 0x01},
    {0x3c82, 0x00},
    {0x3c83, 0xc8},
    {0x3c8c, 0x0f},
    {0x3c8d, 0xa0},
    {0x3c90, 0x07},
    {0x3c91, 0x00},
    {0x3c92, 0x00},
    {0x3c93, 0x00},
    {0x3c94, 0xd0},
    {0x3c95, 0x50},
    {0x3c96, 0x35},
    {0x3c97, 0x00},
    {0x4001, 0xe0},
    {0x4008, 0x00},
    {0x4009, 0x07},
    {0x400f, 0x80},
    {0x4013, 0x02},
    {0x4040, 0x00},
    {0x4041, 0x03},
    {0x404c, 0x50},
    {0x404e, 0x20},
    {0x4500, 0x06},
    {0x4503, 0x00},
    {0x450a, 0x04},
    {0x4809, 0x04},
    {0x480c, 0x12},
    {0x4819, 0x70},
    {0x4825, 0x32},
    {0x4826, 0x32},
    {0x482a, 0x06},
    {0x4833, 0x08},
    {0x4837, 0x0d},
    {0x5000, 0x77},
    {0x5b00, 0x01},
    {0x5b01, 0x10},
    {0x5b02, 0x01},
    {0x5b03, 0xdb},
    {0x5b05, 0x6c},
    {0x5e10, 0xfc},
    {0x3500, 0x00},
    {0x3501, 0x1F},
    {0x3502, 0x20},
    {0x3503, 0x08},
    {0x3508, 0x04},
    {0x3509, 0x00},
    {0x3832, 0x48},
    {0x5780, 0x3e},
    {0x5781, 0x0f},
    {0x5782, 0x44},
    {0x5783, 0x02},
    {0x5784, 0x01},
    {0x5785, 0x01},
    {0x5786, 0x00},
    {0x5787, 0x04},
    {0x5788, 0x02},
    {0x5789, 0x0f},
    {0x578a, 0xfd},
    {0x578b, 0xf5},
    {0x578c, 0xf5},
    {0x578d, 0x03},
    {0x578e, 0x08},
    {0x578f, 0x0c},
    {0x5790, 0x08},
    {0x5791, 0x06},
    {0x5792, 0x00},
    {0x5793, 0x52},
    {0x5794, 0xa3},
    {0x4003, 0x40},
    {0x3107, 0x01},
    {0x3c80, 0x08},
    {0x3c83, 0xb1},
    {0x3c8c, 0x10},
    {0x3c8d, 0x00},
    {0x3c90, 0x00},
    {0x3c94, 0x00},
    {0x3c95, 0x00},
    {0x3c96, 0x00},
    {0x37cb, 0x09},
    {0x37cc, 0x15},
    {0x37cd, 0x1f},
    {0x37ce, 0x1f},
};

/**
  \fn           int32_t OV5675_Bulk_Write_Reg(const OV5675_REG OV5675_reg[],
                                              uint32_t total_num, uint32_t reg_size))
  \brief        write array of registers value to OV5675 Camera Sensor registers.
  \param[in]    OV5675_reg : OV5675 Camera Sensor Register Array Structure
  \param[in]    total_num   : total number of registers(size of array)
  \param[in]    reg_size    : register size in bytes.
  \return       \ref execution_status
  */
static int32_t OV5675_Bulk_Write_Reg(const OV5675_REG OV5675_reg[],
                                     uint32_t total_num, uint32_t reg_size)
{
    uint32_t i  = 0;
    int32_t ret = 0;

    for(i = 0; i < total_num; i++)
    {
        ret = OV5675_WRITE_REG(OV5675_reg[i].reg_addr, OV5675_reg[i].reg_value, \
                reg_size);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t OV5675_Camera_Hard_Reseten(void)
  \brief        Hard Reset OV5675 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5675_Camera_Hard_Reseten(void)
{
    int32_t ret = 0;

    ret = GPIO_Driver_CAM_RST->Initialize(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetDirection(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->Initialize(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetDirection(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    OV5675_DELAY_uSEC(20000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t OV5675_Init(void)
  \brief        Initialize OV5675 Camera Sensor
  this function will
  - initialize i2c instance
  - software reset OV5675 Camera Sensor
  - read OV5675 chip-id, proceed only it is correct.
  \return       \ref execution_status
  */
static int32_t OV5675_Init(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /*camera sensor resten*/
    ret = OV5675_Camera_Hard_Reseten();
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Initialize i2c using i3c driver instance depending on
     *  OV5675 Camera Sensor specific i2c configurations
     *   \ref OV5675_camera_sensor_i2c_cnfg
     */
    ret = camera_sensor_i2c_init(&OV5675_camera_sensor_i2c_cnfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Read OV5675 Camera Sensor CHIP ID */
    ret = OV5675_READ_REG(OV5675_REG_CHIPID_H, &rcv_data, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    uint16_t chipid = rcv_data << 8;

    ret = OV5675_READ_REG(OV5675_REG_CHIPID_L, &rcv_data, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    chipid |= rcv_data;

    /* Proceed only if CHIP ID is correct. */
    if(chipid != OV5675_CHIP_ID_REGISTER_VALUE)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    return OV5675_WRITE_REG(OV5675_REG_MODE_SELECT, OV5675_MODE_STANDBY, 1);
}

/**
  \fn           int32_t OV5675_Start(void)
  \brief        Start OV5675 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5675_Start(void)
{
    /* Start streaming */
    return OV5675_WRITE_REG(OV5675_REG_MODE_SELECT, OV5675_MODE_STREAMING, 1);

}

/**
  \fn           int32_t OV5675_Stop(void)
  \brief        Stop OV5675 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5675_Stop(void)
{
    /* Suspend stream */
    return OV5675_WRITE_REG(OV5675_REG_MODE_SELECT, OV5675_MODE_STANDBY, 1);
}

/**
  \fn           int32_t OV5675_Control(uint32_t control, uint32_t arg)
  \brief        Control OV5675 Camera Sensor.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
static int32_t OV5675_Control(uint32_t control, uint32_t arg)
{
    switch (control)
    {
        case CPI_CAMERA_SENSOR_CONFIGURE:
            return OV5675_Bulk_Write_Reg(OV5675_1296x972_10bpp, ARRAY_SIZE(OV5675_1296x972_10bpp), 1);
            break;
        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
}

/**
  \fn           int32_t OV5675_Uninit(void)
  \brief        Un-initialize OV5675 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5675_Uninit(void)
{
    int32_t ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return GPIO_Driver_CAM_RST->Uninitialize(RTE_OV5675_CAMERA_SENSOR_RESET_PIN_NO);

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO, ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return GPIO_Driver_CAM_PWR->Uninitialize(RTE_OV5675_CAMERA_SENSOR_POWER_PIN_NO);
}

/**
\brief OV5675 Camera Sensor CSI informations
\ref CSI_INFO
*/
static CSI_INFO OV5675_csi_info =
{
    .frequency                = RTE_OV5675_CAMERA_SENSOR_CSI_FREQ,
    .dt                       = RTE_OV5675_CAMERA_SENSOR_CSI_DATA_TYPE,
    .n_lanes                  = RTE_OV5675_CAMERA_SENSOR_CSI_N_LANES,
    .vc_id                    = RTE_OV5675_CAMERA_SENSOR_CSI_VC_ID,
    .cpi_cfg.override         = RTE_OV5675_CAMERA_SENSOR_OVERRIDE_CPI_COLOR_MODE,
    .cpi_cfg.cpi_color_mode   = RTE_OV5675_CAMERA_SENSOR_CPI_COLOR_MODE
};

/**
\brief OV5675 Camera Sensor Operations
\ref CAMERA_SENSOR_OPERATIONS
*/
static CAMERA_SENSOR_OPERATIONS OV5675_ops =
{
    .Init    = OV5675_Init,
    .Uninit  = OV5675_Uninit,
    .Start   = OV5675_Start,
    .Stop    = OV5675_Stop,
    .Control = OV5675_Control,
};

/**
\brief OV5675 Camera Sensor Device Structure
\ref CAMERA_SENSOR_DEVICE
*/
static CAMERA_SENSOR_DEVICE OV5675_camera_sensor =
{
    .interface  = CAMERA_SENSOR_INTERFACE_MIPI,
    .width      = RTE_OV5675_CAMERA_SENSOR_FRAME_WIDTH,
    .height     = RTE_OV5675_CAMERA_SENSOR_FRAME_HEIGHT,
    .csi_info   = &OV5675_csi_info,
    .ops        = &OV5675_ops,
};

/* Registering CPI sensor */
CAMERA_SENSOR(OV5675_camera_sensor)

#endif /* RTE_OV5675_CAMERA_SENSOR_ENABLE */
