/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/*******************************************************************************
 * @file     HM0360_camera_sensor.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     05-April-2024
 * @brief    HM0360 camera sensor driver.
 ******************************************************************************/

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "Camera_Sensor.h"
#include "Camera_Sensor_i2c.h"
#include "Driver_GPIO.h"
#include "Driver_CPI.h"
#include "sys_ctrl_cpi.h"

#if (RTE_HM0360_CAMERA_SENSOR_ENABLE)

/* I2C Instance */
#if(RTE_HM0360_CAMERA_SENSOR_I2C_INSTANCE == 4)
#define CAMERA_SENSOR_I2C_INSTANCE                           I3C
#else
#define CAMERA_SENSOR_I2C_INSTANCE                           RTE_HM0360_CAMERA_SENSOR_I2C_INSTANCE
#endif

/* Wrapper function for i2c read
 *  read register value from HM0360 Camera Sensor registers
 *   using i2c read API \ref camera_sensor_i2c_read
 *
 *  for HM0360 Camera Sensor specific i2c configurations
 *   see \ref hm0360_camera_sensor_i2c_cnfg
 */
#define HM0360_READ_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_read(&hm0360_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/* Wrapper function for i2c write
 *  write register value to HM0360 Camera Sensor registers
 *   using i2c write API \ref camera_sensor_i2c_write.
 *
 *  for HM0360 Camera Sensor specific i2c configurations
 *   see \ref hm0360_camera_sensor_i2c_cnfg
 */
#define HM0360_WRITE_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_write(&hm0360_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)


#define HM0360_CAMERA_SENSOR_SLAVE_ADDR 0x24

#define HM0360_CHIP_ID_REGISTER_VALUE   0x360

/**
\brief HM0360 Camera Sensor Register Array Structure
       used for Camera Configuration.
*/
typedef struct _HM0360_REG {
    uint16_t reg_addr;             /* HM0360 Camera Sensor Register Address*/
    uint16_t reg_value;            /* HM0360 Camera Sensor Register Value*/
} HM0360_REG;

/* Wrapper function for Delay
 * Delay for microsecond:
 * Provide busy loop delay
 */
#define HM0360_DELAY_uSEC(usec)       sys_busy_loop_us(usec)

/* HM0360 Camera reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_HM0360_CAMERA_SENSOR_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_RST = &ARM_Driver_GPIO_(RTE_HM0360_CAMERA_SENSOR_RESET_GPIO_PORT);

/* HM0360 Camera power GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_HM0360_CAMERA_SENSOR_POWER_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_PWR = &ARM_Driver_GPIO_(RTE_HM0360_CAMERA_SENSOR_POWER_GPIO_PORT);

/* HM0360 Camera xsleep GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_HM0360_CAMERA_SENSOR_XSLEEP_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_XSLP = &ARM_Driver_GPIO_(RTE_HM0360_CAMERA_SENSOR_XSLEEP_GPIO_PORT);

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(CAMERA_SENSOR_I2C_INSTANCE);

/**
\brief HM0360 Camera Sensor slave i2c Configuration
\ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
*/
static CAMERA_SENSOR_SLAVE_I2C_CONFIG hm0360_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(CAMERA_SENSOR_I2C_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_FAST,
    .cam_sensor_slave_addr          = HM0360_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/**
\brief HM0360 Camera Sensor Resolution 640x480
*/
static const HM0360_REG hm0360_regs[] = {
    {0x0102, 0x00},
    {0x0350, 0xE0},
    {0x0370, 0x00},
    {0x0371, 0x00},
    {0x0372, 0x01},
    {0x1000, 0x43},
    {0x1001, 0x80},
    {0x1003, 0x20},
    {0x1004, 0x20},
    {0x1007, 0x01},
    {0x1008, 0x20},
    {0x1009, 0x20},
    {0x100A, 0x05},
    {0x100B, 0x20},
    {0x100C, 0x20},
    {0x1013, 0x00},
    {0x1014, 0x00},
    {0x1018, 0x00},
    {0x101D, 0xCF},
    {0x101E, 0x01},
    {0x101F, 0x00},
    {0x1020, 0x01},
    {0x1021, 0x5D},
    {0x102F, 0x08},
    {0x1030, 0x04},
    {0x1031, 0x08},
    {0x1032, 0x10},
    {0x1033, 0x18},
    {0x1034, 0x20},
    {0x1035, 0x28},
    {0x1036, 0x30},
    {0x1037, 0x38},
    {0x1038, 0x40},
    {0x1039, 0x50},
    {0x103A, 0x60},
    {0x103B, 0x70},
    {0x103C, 0x80},
    {0x103D, 0xA0},
    {0x103E, 0xC0},
    {0x103F, 0xE0},
    {0x1041, 0x00},
    {0x2000, 0x7F},
    {0x202B, 0x04},
    {0x202C, 0x03},
    {0x202D, 0x00},
    {0x2031, 0x60},
    {0x2032, 0x08},
    {0x2034, 0x70},
    {0x2036, 0x19},
    {0x2037, 0x08},
    {0x2038, 0x10},
    {0x203C, 0x01},
    {0x203D, 0x04},
    {0x203E, 0x01},
    {0x203F, 0x38},
    {0x2048, 0x00},
    {0x2049, 0x10},
    {0x204A, 0x40},
    {0x204B, 0x00},
    {0x204C, 0x08},
    {0x204D, 0x20},
    {0x204E, 0x00},
    {0x204F, 0x38},
    {0x2050, 0xE0},
    {0x2051, 0x00},
    {0x2052, 0x1C},
    {0x2053, 0x70},
    {0x2054, 0x00},
    {0x2055, 0x1A},
    {0x2056, 0xC0},
    {0x2057, 0x00},
    {0x2058, 0x06},
    {0x2059, 0xB0},
    {0x2061, 0x00},
    {0x2062, 0x00},
    {0x2063, 0xC8},
    {0x2080, 0x41},
    {0x2081, 0xE0},
    {0x2082, 0xF0},
    {0x2083, 0x01},
    {0x2084, 0x10},
    {0x2085, 0x10},
    {0x2086, 0x01},
    {0x2087, 0x06},
    {0x2088, 0x0C},
    {0x2089, 0x12},
    {0x208A, 0x1C},
    {0x208B, 0x30},
    {0x208C, 0x10},
    {0x208D, 0x02},
    {0x208E, 0x08},
    {0x208F, 0x0D},
    {0x2090, 0x14},
    {0x2091, 0x1D},
    {0x2092, 0x30},
    {0x2093, 0x08},
    {0x2094, 0x0A},
    {0x2095, 0x0F},
    {0x2096, 0x14},
    {0x2097, 0x18},
    {0x2098, 0x20},
    {0x2099, 0x10},
    {0x209A, 0x00},
    {0x209B, 0x01},
    {0x209C, 0x01},
    {0x209D, 0x11},
    {0x209E, 0x06},
    {0x209F, 0x20},
    {0x20A0, 0x10},
    {0x2590, 0x01},
    {0x2800, 0x09},
    {0x2804, 0x02},
    {0x2805, 0x03},
    {0x2806, 0x03},
    {0x2807, 0x08},
    {0x2808, 0x04},
    {0x2809, 0x0C},
    {0x280A, 0x03},
    {0x280F, 0x03},
    {0x2810, 0x03},
    {0x2811, 0x00},
    {0x2812, 0x09},
    {0x2821, 0xEE},
    {0x282A, 0x0F},
    {0x282B, 0x08},
    {0x282E, 0x2F},
    {0x3010, 0x00},
    {0x3013, 0x01},
    {0x3019, 0x00},
    {0x301A, 0x00},
    {0x301B, 0x20},
    {0x301C, 0xFF},
    {0x3020, 0x00},
    {0x3021, 0x00},
#if (RTE_HM0360_CAMERA_SENSOR_CSI_CXT_SEL == 0)
    {0x3024, 0x00},
#elif (RTE_HM0360_CAMERA_SENSOR_CSI_CXT_SEL == 1)
    {0x3024, 0x01},
#endif
    {0x3025, 0x12},
    {0x3026, 0x03},
    {0x3027, 0x81},
    {0x3028, 0x01},
    {0x3029, 0x15},
    {0x302A, 0x60},
    {0x302B, 0x2A},
    {0x302C, 0x00},
    {0x302D, 0x03},
    {0x302E, 0x00},
    {0x302F, 0x00},
    {0x3031, 0x01},
    {0x3034, 0x00},
    {0x3035, 0x01},
    {0x3051, 0x00},
    {0x305C, 0x03},
    {0x3060, 0x00},
    {0x3061, 0xFA},
    {0x3062, 0xFF},
    {0x3063, 0xFF},
    {0x3064, 0xFF},
    {0x3065, 0xFF},
    {0x3066, 0xFF},
    {0x3067, 0xFF},
    {0x3068, 0xFF},
    {0x3069, 0xFF},
    {0x306A, 0xFF},
    {0x306B, 0xFF},
    {0x306C, 0xFF},
    {0x306D, 0xFF},
    {0x306E, 0xFF},
    {0x306F, 0xFF},
    {0x3070, 0xFF},
    {0x3071, 0xFF},
    {0x3072, 0xFF},
    {0x3073, 0xFF},
    {0x3074, 0xFF},
    {0x3075, 0xFF},
    {0x3076, 0xFF},
    {0x3077, 0xFF},
    {0x3078, 0xFF},
    {0x3079, 0xFF},
    {0x307A, 0xFF},
    {0x307B, 0xFF},
    {0x307C, 0xFF},
    {0x307D, 0xFF},
    {0x307E, 0xFF},
    {0x307F, 0xFF},
    {0x3080, 0x00},
    {0x3081, 0x00},
    {0x3082, 0x00},
    {0x3083, 0x20},
    {0x3084, 0x00},
    {0x3085, 0x20},
    {0x3086, 0x00},
    {0x3087, 0x20},
    {0x3088, 0x00},
    {0x3089, 0x04},
    {0x3094, 0x02},
    {0x3095, 0x02},
    {0x3096, 0x00},
    {0x3097, 0x02},
    {0x3098, 0x00},
    {0x3099, 0x02},
    {0x309E, 0x05},
    {0x309F, 0x02},
    {0x30A0, 0x02},
    {0x30A1, 0x00},
    {0x30A2, 0x08},
    {0x30A3, 0x00},
    {0x30A4, 0x20},
    {0x30A5, 0x04},
    {0x30A6, 0x02},
    {0x30A7, 0x02},
    {0x30A8, 0x02},
    {0x30A9, 0x00},
    {0x30AA, 0x02},
    {0x30AB, 0x34},
    {0x30B0, 0x03},
    {0x30C4, 0x10},
    {0x30C5, 0x01},
    {0x30C6, 0x2F},
    {0x30C7, 0x00},
    {0x30C8, 0x00},
    {0x30CB, 0xFF},
    {0x30CC, 0xFF},
    {0x30CD, 0x7F},
    {0x30CE, 0x7F},
    {0x30D3, 0x01},
    {0x30D4, 0xFF},
    {0x30D5, 0x00},
    {0x30D6, 0x40},
    {0x30D7, 0x00},
    {0x30D8, 0xA7},
    {0x30D9, 0x00},
    {0x30DA, 0x01},
    {0x30DB, 0x40},
    {0x30DC, 0x00},
    {0x30DD, 0x27},
    {0x30DE, 0x05},
    {0x30DF, 0x07},
    {0x30E0, 0x40},
    {0x30E1, 0x00},
    {0x30E2, 0x27},
    {0x30E3, 0x05},
    {0x30E4, 0x47},
    {0x30E5, 0x30},
    {0x30E6, 0x00},
    {0x30E7, 0x27},
    {0x30E8, 0x05},
    {0x30E9, 0x87},
    {0x30EA, 0x30},
    {0x30EB, 0x00},
    {0x30EC, 0x27},
    {0x30ED, 0x05},
    {0x30EE, 0x00},
    {0x30EF, 0x40},
    {0x30F0, 0x00},
    {0x30F1, 0xA7},
    {0x30F2, 0x00},
    {0x30F3, 0x01},
    {0x30F4, 0x40},
    {0x30F5, 0x00},
    {0x30F6, 0x27},
    {0x30F7, 0x05},
    {0x30F8, 0x07},
    {0x30F9, 0x40},
    {0x30FA, 0x00},
    {0x30FB, 0x27},
    {0x30FC, 0x05},
    {0x30FD, 0x47},
    {0x30FE, 0x30},
    {0x30FF, 0x00},
    {0x3100, 0x27},
    {0x3101, 0x05},
    {0x3102, 0x87},
    {0x3103, 0x30},
    {0x3104, 0x00},
    {0x3105, 0x27},
    {0x3106, 0x05},
    {0x310B, 0x10},
    {0x3112, 0x04},
    {0x3113, 0xA0},
    {0x3114, 0x67},
    {0x3115, 0x42},
    {0x3116, 0x10},
    {0x3117, 0x0A},
    {0x3118, 0x3F},
    {0x311A, 0x30},
    {0x311C, 0x10},
    {0x311D, 0x06},
    {0x311E, 0x0F},
    {0x311F, 0x0E},
    {0x3120, 0x0D},
    {0x3121, 0x0F},
    {0x3122, 0x00},
    {0x3123, 0x1D},
    {0x3126, 0x03},
    {0x3127, 0xC4},
    {0x3128, 0x57},
    {0x312A, 0x11},
    {0x312B, 0x41},
    {0x312E, 0x00},
    {0x312F, 0x00},
    {0x3130, 0x0C},
    {0x3141, 0x2A},
    {0x3142, 0x9F},
    {0x3147, 0x18},
    {0x3149, 0x28},
    {0x314B, 0x01},
    {0x3150, 0x50},
    {0x3152, 0x00},
    {0x3156, 0x2C},
    {0x315A, 0x0A},
    {0x315B, 0x2F},
    {0x315C, 0xE0},
    {0x315F, 0x02},
    {0x3160, 0x1F},
    {0x3163, 0x1F},
    {0x3164, 0x7F},
    {0x3165, 0x7F},
    {0x317B, 0x94},
    {0x317C, 0x00},
    {0x317D, 0x02},
    {0x318C, 0x00},
    {0x3500, 0x78},
    {0x3501, 0x0A},
    {0x3502, 0x77},
    {0x3503, 0x02},
    {0x3504, 0x14},
    {0x3505, 0x03},
    {0x3506, 0x00},
    {0x3507, 0x00},
    {0x3508, 0x00},
    {0x3509, 0x00},
    {0x350A, 0xFF},
    {0x350B, 0x00},
    {0x350C, 0x00},
    {0x350D, 0x01},
    {0x350F, 0x00},
    {0x3510, 0x02},
    {0x3511, 0x00},
    {0x3512, 0x7F},
    {0x3513, 0x00},
    {0x3514, 0x00},
    {0x3515, 0x01},
    {0x3516, 0x00},
    {0x3517, 0x02},
    {0x3518, 0x00},
    {0x3519, 0x7F},
    {0x351A, 0x00},
    {0x351B, 0x5F},
    {0x351C, 0x00},
    {0x351D, 0x02},
    {0x351E, 0x10},
    {0x351F, 0x04},
    {0x3520, 0x03},
    {0x3521, 0x00},
    {0x3523, 0x60},
    {0x3524, 0x08},
    {0x3525, 0x19},
    {0x3526, 0x08},
    {0x3527, 0x10},
    {0x352A, 0x01},
    {0x352B, 0x04},
    {0x352C, 0x01},
    {0x352D, 0x39},
    {0x352E, 0x02},
#if (RTE_HM0360_CAMERA_SENSOR_CSI_CFG_FPS == 60)
    {0x352F, 0x0A},
    {0x3530, 0x02},
    {0x3531, 0x0A},
#elif (RTE_HM0360_CAMERA_SENSOR_CSI_CFG_FPS == 30)
    {0x352F, 0x14},
    {0x3530, 0x04},
    {0x3531, 0x10},
#else
#error " HM0360 FPS not supported "
#endif
    {0x3532, 0x06},
    {0x3533, 0x1A},
    {0x3535, 0x02},
    {0x3536, 0x03},
    {0x3537, 0x03},
    {0x3538, 0x08},
    {0x3539, 0x04},
    {0x353A, 0x0C},
    {0x353B, 0x03},
    {0x3540, 0x03},
    {0x3541, 0x03},
    {0x3542, 0x00},
    {0x3543, 0x09},
    {0x3549, 0x04},
    {0x354A, 0x35},
    {0x354B, 0x21},
    {0x354C, 0x01},
    {0x354D, 0xE0},
    {0x354E, 0xF0},
    {0x354F, 0x10},
    {0x3550, 0x10},
    {0x3551, 0x10},
    {0x3552, 0x20},
    {0x3553, 0x10},
    {0x3554, 0x01},
    {0x3555, 0x06},
    {0x3556, 0x0C},
    {0x3557, 0x12},
    {0x3558, 0x1C},
    {0x3559, 0x30},
    {0x355A, 0x78},
    {0x355B, 0x0A},
    {0x355C, 0x77},
    {0x355D, 0x01},
    {0x355E, 0x1C},
    {0x355F, 0x03},
    {0x3560, 0x00},
    {0x3561, 0x01},
    {0x3562, 0x01},
    {0x3563, 0x00},
    {0x3564, 0xFF},
    {0x3565, 0x00},
    {0x3566, 0x00},
    {0x3567, 0x01},
    {0x3569, 0x00},
    {0x356A, 0x02},
    {0x356B, 0x00},
    {0x356C, 0x7F},
    {0x356D, 0x00},
    {0x356E, 0x00},
    {0x356F, 0x01},
    {0x3570, 0x00},
    {0x3571, 0x02},
    {0x3572, 0x00},
    {0x3573, 0x3F},
    {0x3574, 0x00},
    {0x3575, 0x2F},
    {0x3576, 0x00},
    {0x3577, 0x02},
    {0x3578, 0x24},
    {0x3579, 0x04},
    {0x357A, 0x03},
    {0x357B, 0x00},
    {0x357D, 0x60},
    {0x357E, 0x08},
    {0x357F, 0x19},
    {0x3580, 0x08},
    {0x3581, 0x10},
    {0x3584, 0x01},
    {0x3585, 0x04},
    {0x3586, 0x01},
    {0x3587, 0x39},
    {0x3588, 0x02},
    {0x3589, 0x12},
    {0x358A, 0x04},
    {0x358B, 0x24},
    {0x358C, 0x06},
    {0x358D, 0x36},
    {0x358F, 0x02},
    {0x3590, 0x03},
    {0x3591, 0x03},
    {0x3592, 0x08},
    {0x3593, 0x04},
    {0x3594, 0x0C},
    {0x3595, 0x03},
    {0x359A, 0x03},
    {0x359B, 0x03},
    {0x359C, 0x00},
    {0x359D, 0x09},
    {0x35A3, 0x02},
    {0x35A4, 0x03},
    {0x35A5, 0x21},
    {0x35A6, 0x01},
    {0x35A7, 0xE0},
    {0x35A8, 0xF0},
    {0x35A9, 0x10},
    {0x35AA, 0x10},
    {0x35AB, 0x10},
    {0x35AC, 0x20},
    {0x35AD, 0x10},
    {0x35AE, 0x01},
    {0x35AF, 0x06},
    {0x35B0, 0x0C},
    {0x35B1, 0x12},
    {0x35B2, 0x1C},
    {0x35B3, 0x30},
    {0x35B4, 0x78},
    {0x35B5, 0x0A},
    {0x35B6, 0x77},
    {0x35B7, 0x00},
    {0x35B8, 0x94},
    {0x35B9, 0x03},
    {0x35BA, 0x00},
    {0x35BB, 0x03},
    {0x35BC, 0x03},
    {0x35BD, 0x00},
    {0x35BE, 0xFF},
    {0x35BF, 0x00},
    {0x35C0, 0x01},
    {0x35C1, 0x01},
    {0x35C3, 0x00},
    {0x35C4, 0x00},
    {0x35C5, 0x00},
    {0x35C6, 0x7F},
    {0x35C7, 0x00},
    {0x35C8, 0x00},
    {0x35C9, 0x01},
    {0x35CA, 0x00},
    {0x35CB, 0x02},
    {0x35CC, 0x00},
    {0x35CD, 0x0F},
    {0x35CE, 0x00},
    {0x35CF, 0x0B},
    {0x35D0, 0x00},
    {0x35D3, 0x04},
    {0x35D7, 0x18},
    {0x35D8, 0x01},
    {0x35D9, 0x20},
    {0x35DA, 0x08},
    {0x35DB, 0x14},
    {0x35DC, 0x70},
    {0x35DE, 0x00},
    {0x35DF, 0x01},
    {0x35E9, 0x02},
    {0x35EA, 0x03},
    {0x35EB, 0x03},
    {0x35EC, 0x08},
    {0x35ED, 0x04},
    {0x35EE, 0x0C},
    {0x35EF, 0x03},
    {0x35F4, 0x03},
    {0x35F5, 0x03},
    {0x35F6, 0x00},
    {0x35F7, 0x09},
    {0x35FD, 0x00},
    {0x35FE, 0x5E},
    {0x0104, 0x01},
    {0x0100, 0x01},
};


/**
  \fn           int32_t HM0360_Bulk_Write_Reg(const HM0360_REG HM0360_reg[],
                                              uint32_t total_num, uint32_t reg_size))
  \brief        write array of registers value to HM0360 Camera Sensor registers.
  \param[in]    HM0360_reg : HM0360 Camera Sensor Register Array Structure
  \ref HM0360_REG
  \param[in]    total_num   : total number of registers(size of array)
  \param[in]    reg_size    : register size in bytes.
  \return       \ref execution_status
  */
static int32_t HM0360_Bulk_Write_Reg(const HM0360_REG HM0360_reg[],
                                     uint32_t total_num, uint32_t reg_size)
{
    uint32_t i  = 0;
    int32_t ret = 0;

    for(i = 0; i < total_num; i++)
    {
        ret = HM0360_WRITE_REG(HM0360_reg[i].reg_addr, HM0360_reg[i].reg_value, \
                reg_size);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t HM0360_Camera_Hard_Reseten(void)
  \brief        Hard Reset HM0360 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t HM0360_Camera_Hard_Reseten(void)
{
    int32_t ret = 0;

    ret = GPIO_Driver_CAM_RST->Initialize(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetDirection(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->Initialize(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetDirection(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->Initialize(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->PowerControl(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->SetDirection(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->SetValue(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    HM0360_DELAY_uSEC(2000);

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    HM0360_DELAY_uSEC(1000);

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    HM0360_DELAY_uSEC(400);

    ret = GPIO_Driver_CAM_XSLP->SetValue(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    HM0360_DELAY_uSEC(100000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t HM0360_Camera_Cfg(void)
  \brief        Initialize HM0360 Camera Sensor.
  this function will
  - configure Camera Sensor resolution registers as per input parameter.
  (currently supports only 640x480(WxH) Camera resolution)
  \return       \ref execution_status
  */
static int32_t HM0360_Camera_Cfg(void)
{
    uint32_t total_num     = 0;

    /* Camera Sensor Resolution: 640x480(WxH) */
    total_num = (sizeof(hm0360_regs) / sizeof(HM0360_REG));
    return HM0360_Bulk_Write_Reg(hm0360_regs, total_num, 1);
}

/**
  \fn           int32_t HM0360_Init(void)
  \brief        Initialize HM0360 Camera Sensor
  this function will
  - initialize i2c instance
  - software reset HM0360 Camera Sensor
  - read HM0360 chip-id, proceed only it is correct.
  \return       \ref execution_status
  */
static int32_t HM0360_Init(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /*camera sensor resten*/
    ret = HM0360_Camera_Hard_Reseten();
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Initialize i2c using i3c driver instance depending on
     *  HM0360 Camera Sensor specific i2c configurations
     *   \ref hm0360_camera_sensor_i2c_cnfg
     */
    ret = camera_sensor_i2c_init(&hm0360_camera_sensor_i2c_cnfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Read HM0360 Camera Sensor CHIP ID */
    ret = HM0360_READ_REG(0x0000, &rcv_data, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    uint16_t chipid = rcv_data << 8;

    ret = HM0360_READ_REG(0x0001, &rcv_data, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    chipid |= rcv_data;

    /* Proceed only if CHIP ID is correct. */
    if(chipid != HM0360_CHIP_ID_REGISTER_VALUE)
        return ARM_DRIVER_ERROR_UNSUPPORTED;


    ret = HM0360_WRITE_REG(0x0103, 0x00, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    HM0360_DELAY_uSEC(10000);

    ret = HM0360_Camera_Cfg();
    if(ret != ARM_DRIVER_OK)
        return ret;

    HM0360_DELAY_uSEC(1000);

    return HM0360_WRITE_REG(0x0100, 0x00, 1);

}

/**
  \fn           int32_t HM0360_Start(void)
  \brief        Start HM0360 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t HM0360_Start(void)
{
    /* Start streaming */
    return HM0360_WRITE_REG(0x0100, 0x01, 1);
}

/**
  \fn           int32_t HM0360_Stop(void)
  \brief        Stop HM0360 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t HM0360_Stop(void)
{
    /* Suspend any stream */
    return HM0360_WRITE_REG(0x0100, 0x00, 1);
}

/**
  \fn           int32_t HM0360_Control(uint32_t control, uint32_t arg)
  \brief        Control HM0360 Camera Sensor.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
static int32_t HM0360_Control(uint32_t control, uint32_t arg)
{
    ARG_UNUSED(control);
    ARG_UNUSED(arg);
    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t HM0360_Uninit(void)
  \brief        Un-initialize HM0360 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t HM0360_Uninit(void)
{
    int32_t ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->Uninitialize(RTE_HM0360_CAMERA_SENSOR_RESET_PIN_NO);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO, ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->Uninitialize(RTE_HM0360_CAMERA_SENSOR_POWER_PIN_NO);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->SetValue(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->PowerControl(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO, ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_XSLP->Uninitialize(RTE_HM0360_CAMERA_SENSOR_XSLEEP_PIN_NO);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return ARM_DRIVER_OK;
}

/**
\brief HM0360 Camera Sensor CSI informations
\ref CSI_INFO
*/
static CSI_INFO HM0360_csi_info =
{
    .frequency                = RTE_HM0360_CAMERA_SENSOR_CSI_FREQ,
    .dt                       = RTE_HM0360_CAMERA_SENSOR_CSI_DATA_TYPE,
    .n_lanes                  = RTE_HM0360_CAMERA_SENSOR_CSI_N_LANES,
    .vc_id                    = RTE_HM0360_CAMERA_SENSOR_CSI_VC_ID,
    .cpi_cfg.override         = RTE_HM0360_CAMERA_SENSOR_OVERRIDE_CPI_COLOR_MODE,
    .cpi_cfg.cpi_color_mode   = RTE_HM0360_CAMERA_SENSOR_CPI_COLOR_MODE
};

/**
\brief HM0360 Camera Sensor Operations
\ref CAMERA_SENSOR_OPERATIONS
*/
static CAMERA_SENSOR_OPERATIONS HM0360_ops =
{
    .Init    = HM0360_Init,
    .Uninit  = HM0360_Uninit,
    .Start   = HM0360_Start,
    .Stop    = HM0360_Stop,
    .Control = HM0360_Control,
};

/**
\brief HM0360 Camera Sensor Device Structure
\ref CAMERA_SENSOR_DEVICE
*/
static CAMERA_SENSOR_DEVICE HM0360_camera_sensor =
{
    .interface  = CAMERA_SENSOR_INTERFACE_MIPI,
    .width      = RTE_HM0360_CAMERA_SENSOR_FRAME_WIDTH,
    .height     = RTE_HM0360_CAMERA_SENSOR_FRAME_HEIGHT,
    .csi_info   = &HM0360_csi_info,
    .ops        = &HM0360_ops,
};

/* Registering CPI sensor */
CAMERA_SENSOR(HM0360_camera_sensor)

#endif /* RTE_HM0360_CAMERA_SENSOR_ENABLE */
