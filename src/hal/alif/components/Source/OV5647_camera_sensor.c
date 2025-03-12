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

#if (RTE_OV5647_CAMERA_SENSOR_ENABLE)

/* Wrapper function for i2c read
 *  read register value from OV5647 Camera Sensor registers
 *   using i2c read API \ref camera_sensor_i2c_read
 *
 *  for OV5647 Camera Sensor specific i2c configurations
 *   see \ref OV5647_camera_sensor_i2c_cnfg
 */
#define OV5647_READ_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_read(&OV5647_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/* Wrapper function for i2c write
 *  write register value to OV5647 Camera Sensor registers
 *   using i2c write API \ref camera_sensor_i2c_write.
 *
 *  for OV5647 Camera Sensor specific i2c configurations
 *   see \ref OV5647_camera_sensor_i2c_cnfg
 */
#define OV5647_WRITE_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_write(&OV5647_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)


#define OV5647_CAMERA_SENSOR_SLAVE_ADDR 0x36

#define OV5647_CHIP_ID_REGISTER_VALUE   0x5647

#define OV5647_SW_STANDBY            0x0100
#define OV5647_SW_RESET              0x0103
#define OV5647_REG_CHIPID_H          0x300a
#define OV5647_REG_CHIPID_L          0x300b
#define OV5640_REG_PAD_OUT           0x300d
#define OV5647_REG_FRAME_OFF_NUMBER  0x4202
#define OV5647_REG_MIPI_CTRL00       0x4800
#define OV5647_REG_MIPI_CTRL14       0x4814

#define MIPI_CTRL00_CLOCK_LANE_GATE      BIT(5)
#define MIPI_CTRL00_LINE_SYNC_ENABLE     BIT(4)
#define MIPI_CTRL00_BUS_IDLE             BIT(2)
#define MIPI_CTRL00_CLOCK_LANE_DISABLE   BIT(0)

/*Helper macro*/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
\brief OV5647 Camera Sensor Register Array Structure
       used for Camera Configuration.
*/
typedef struct _OV5647_REG {
    uint16_t reg_addr;             /* OV5647 Camera Sensor Register Address*/
    uint16_t reg_value;            /* OV5647 Camera Sensor Register Value*/
} OV5647_REG;

/* Wrapper function for Delay
 * Delay for microsecond:
 * Provide busy loop delay
 */
#define OV5647_DELAY_uSEC(usec)       sys_busy_loop_us(usec)

/* OV5647 Camera reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_OV5647_CAMERA_SENSOR_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_RST = &ARM_Driver_GPIO_(RTE_OV5647_CAMERA_SENSOR_RESET_GPIO_PORT);

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(RTE_OV5647_CAMERA_SENSOR_I2C_INSTANCE);

/**
\brief OV5647 Camera Sensor slave i2c Configuration
\ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
*/
static CAMERA_SENSOR_SLAVE_I2C_CONFIG OV5647_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(RTE_OV5647_CAMERA_SENSOR_I2C_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = OV5647_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

static const OV5647_REG sensor_oe_disable_regs[] = {
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
};

static const OV5647_REG sensor_oe_enable_regs[] = {
    {0x3000, 0x0f},
    {0x3001, 0xff},
    {0x3002, 0xe4},
};

/**
\brief OV5647 Camera Sensor Resolution 640x480
*/
static const OV5647_REG ov5647_640x480_10bpp[] = {
    {0x0100, 0x00},
    {0x0103, 0x01},
    {0x3034, 0x08},
    {0x3035, 0x11},
    {0x3036, 0x46},
    {0x303c, 0x11},
    {0x3821, 0x07},
    {0x3820, 0x41},
    {0x370c, 0x03},
    {0x3612, 0x59},
    {0x3618, 0x00},
    {0x5000, 0x06},
    {0x5001, 0x01},
    {0x5002, 0x41},
    {0x5003, 0x08},
    {0x5a00, 0x08},
    {0x3000, 0xff},
    {0x3001, 0xff},
    {0x3002, 0xff},
    {0x301d, 0xf0},
    {0x3a18, 0x00},
    {0x3a19, 0xf8},
    {0x3c01, 0x80},
    {0x3b07, 0x0c},
    {0x380c, 0x07},
    {0x380d, 0x3c},
    {0x3814, 0x35},
    {0x3815, 0x35},
    {0x3708, 0x64},
    {0x3709, 0x52},
    {0x3808, 0x02},
    {0x3809, 0x80},
    {0x380a, 0x01},
    {0x380b, 0xe0},
    {0x3800, 0x00},
    {0x3801, 0x10},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x2f},
    {0x3806, 0x07},
    {0x3807, 0x9f},
    {0x3630, 0x2e},
    {0x3632, 0xe2},
    {0x3633, 0x23},
    {0x3634, 0x44},
    {0x3620, 0x64},
    {0x3621, 0xe0},
    {0x3600, 0x37},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x3731, 0x02},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3f05, 0x02},
    {0x3f06, 0x10},
    {0x3f01, 0x0a},
    {0x3a08, 0x01},
    {0x3a09, 0x2e},
    {0x3a0a, 0x00},
    {0x3a0b, 0xfb},
    {0x3a0d, 0x02},
    {0x3a0e, 0x01},
    {0x3a0f, 0x58},
    {0x3a10, 0x50},
    {0x3a1b, 0x58},
    {0x3a1e, 0x50},
    {0x3a11, 0x60},
    {0x3a1f, 0x28},
    {0x4001, 0x02},
    {0x4004, 0x02},
    {0x4000, 0x09},
    {0x4050, 0x6e},
    {0x4051, 0x8f},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3017, 0xe0},
    {0x301c, 0xfc},
    {0x3636, 0x06},
    {0x3016, 0x08},
    {0x3827, 0xec},
    {0x3018, 0x44},
    {0x3035, 0x21},
    {0x3106, 0xf5},
    {0x3034, 0x1a},
    {0x301c, 0xf8},
    {0x350a, 0x01},
    {0x350b, 0x8f},
    {0x3500, 0x00},
    {0x3501, 0x0F},
    {0x3502, 0xFF},
    {0x0100, 0x01},
};

/**
  \fn           int32_t OV5647_Bulk_Write_Reg(const OV5647_REG OV5647_reg[],
                                              uint32_t total_num, uint32_t reg_size))
  \brief        write array of registers value to OV5647 Camera Sensor registers.
  \param[in]    OV5647_reg : OV5647 Camera Sensor Register Array Structure
  \ref OV5647_REG
  \param[in]    total_num   : total number of registers(size of array)
  \param[in]    reg_size    : register size in bytes.
  \return       \ref execution_status
  */
static int32_t OV5647_Bulk_Write_Reg(const OV5647_REG OV5647_reg[],
                                     uint32_t total_num, uint32_t reg_size)
{
    uint32_t i  = 0;
    int32_t ret = 0;

    for(i = 0; i < total_num; i++)
    {
        ret = OV5647_WRITE_REG(OV5647_reg[i].reg_addr, OV5647_reg[i].reg_value, \
                reg_size);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t OV5647_Camera_Hard_Reseten(void)
  \brief        Hard Reset OV5647 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5647_Camera_Hard_Reseten(void)
{
    int32_t ret = 0;

    ret = GPIO_Driver_CAM_RST->Initialize(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetDirection(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    OV5647_DELAY_uSEC(20000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t OV5647_Stream_On(void)
  \brief        Start streaming OV5647 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5647_Stream_On(void)
{
    int32_t  ret = 0;

    ret = OV5647_WRITE_REG(OV5647_REG_MIPI_CTRL00,
                           MIPI_CTRL00_CLOCK_LANE_GATE |
                           MIPI_CTRL00_BUS_IDLE |
                           MIPI_CTRL00_LINE_SYNC_ENABLE, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = OV5647_WRITE_REG(OV5647_REG_FRAME_OFF_NUMBER, 0x00, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return OV5647_WRITE_REG(OV5640_REG_PAD_OUT, 0x00, 1);
}

/**
  \fn           int32_t OV5647_Stream_Off(void)
  \brief        Stop streaming OV5647 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5647_Stream_Off(void)
{
    int32_t  ret = 0;

    ret = OV5647_WRITE_REG(OV5647_REG_MIPI_CTRL00,
                           MIPI_CTRL00_CLOCK_LANE_GATE |
                           MIPI_CTRL00_BUS_IDLE |
                           MIPI_CTRL00_CLOCK_LANE_DISABLE, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = OV5647_WRITE_REG(OV5647_REG_FRAME_OFF_NUMBER, 0x0F, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return OV5647_WRITE_REG(OV5640_REG_PAD_OUT, 0x01, 1);
}

/**
  \fn           int32_t OV5647_Init(void)
  \brief        Initialize OV5647 Camera Sensor
  this function will
  - initialize i2c instance
  - software reset OV5647 Camera Sensor
  - read OV5647 chip-id, proceed only it is correct.
  \return       \ref execution_status
  */
static int32_t OV5647_Init(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /*camera sensor resten*/
    ret = OV5647_Camera_Hard_Reseten();
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Initialize i2c using i3c driver instance depending on
     *  OV5647 Camera Sensor specific i2c configurations
     *   \ref OV5647_camera_sensor_i2c_cnfg
     */
    ret = camera_sensor_i2c_init(&OV5647_camera_sensor_i2c_cnfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Read OV5647 Camera Sensor CHIP ID */
    ret = OV5647_READ_REG(OV5647_REG_CHIPID_H, &rcv_data, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    uint16_t chipid = rcv_data << 8;

    ret = OV5647_READ_REG(OV5647_REG_CHIPID_L, &rcv_data, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    chipid |= rcv_data;

    /* Proceed only if CHIP ID is correct. */
    if(chipid != OV5647_CHIP_ID_REGISTER_VALUE)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    ret = OV5647_WRITE_REG(OV5647_SW_RESET, 0x00, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = OV5647_Bulk_Write_Reg(sensor_oe_enable_regs,
                                ARRAY_SIZE(sensor_oe_enable_regs),
                                1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return OV5647_Stream_Off();
}

/**
  \fn           int32_t OV5647_Start(void)
  \brief        Start OV5647 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5647_Start(void)
{
    /* Start streaming */
    return OV5647_Stream_On();

}

/**
  \fn           int32_t OV5647_Stop(void)
  \brief        Stop OV5647 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5647_Stop(void)
{
    /* Suspend any stream */
    return OV5647_Stream_Off();
}

/**
  \fn           int32_t OV5647_Control(uint32_t control, uint32_t arg)
  \brief        Control OV5647 Camera Sensor.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
static int32_t OV5647_Control(uint32_t control, uint32_t arg)
{
    switch (control)
    {
        case CPI_CAMERA_SENSOR_CONFIGURE:
            return OV5647_Bulk_Write_Reg(ov5647_640x480_10bpp, ARRAY_SIZE(ov5647_640x480_10bpp), 1);
            break;
        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
}

/**
  \fn           int32_t OV5647_Uninit(void)
  \brief        Un-initialize OV5647 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t OV5647_Uninit(void)
{
    int32_t ret;

    ret = OV5647_Bulk_Write_Reg(sensor_oe_disable_regs,
                                ARRAY_SIZE(sensor_oe_disable_regs),
                                1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return GPIO_Driver_CAM_RST->Uninitialize(RTE_OV5647_CAMERA_SENSOR_RESET_PIN_NO);
}

/**
\brief OV5647 Camera Sensor CSI informations
\ref CSI_INFO
*/
static CSI_INFO OV5647_csi_info =
{
    .frequency                = RTE_OV5647_CAMERA_SENSOR_CSI_FREQ,
    .dt                       = RTE_OV5647_CAMERA_SENSOR_CSI_DATA_TYPE,
    .n_lanes                  = RTE_OV5647_CAMERA_SENSOR_CSI_N_LANES,
    .vc_id                    = RTE_OV5647_CAMERA_SENSOR_CSI_VC_ID,
    .cpi_cfg.override         = RTE_OV5647_CAMERA_SENSOR_OVERRIDE_CPI_COLOR_MODE,
    .cpi_cfg.cpi_color_mode   = RTE_OV5647_CAMERA_SENSOR_CPI_COLOR_MODE
};

/**
\brief OV5647 Camera Sensor Operations
\ref CAMERA_SENSOR_OPERATIONS
*/
static CAMERA_SENSOR_OPERATIONS OV5647_ops =
{
    .Init    = OV5647_Init,
    .Uninit  = OV5647_Uninit,
    .Start   = OV5647_Start,
    .Stop    = OV5647_Stop,
    .Control = OV5647_Control,
};

/**
\brief OV5647 Camera Sensor Device Structure
\ref CAMERA_SENSOR_DEVICE
*/
static CAMERA_SENSOR_DEVICE OV5647_camera_sensor =
{
    .interface  = CAMERA_SENSOR_INTERFACE_MIPI,
    .width      = RTE_OV5647_CAMERA_SENSOR_FRAME_WIDTH,
    .height     = RTE_OV5647_CAMERA_SENSOR_FRAME_HEIGHT,
    .csi_info   = &OV5647_csi_info,
    .ops        = &OV5647_ops,
};

/* Registering CPI sensor */
CAMERA_SENSOR(OV5647_camera_sensor)

#endif /* RTE_OV5647_CAMERA_SENSOR_ENABLE */
