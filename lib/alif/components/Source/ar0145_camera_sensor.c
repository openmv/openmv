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
#include "sys_ctrl_cpi.h"

/* Proceed only if AR0145 Camera Sensor is enabled. */
#if RTE_AR0145_CAMERA_SENSOR_CSI_ENABLE

/* I2C Instance */
#if(RTE_AR0145_CAMERA_SENSOR_I2C_INSTANCE == 4)
#define CAMERA_SENSOR_I2C_INSTANCE                           I3C
#else
#define CAMERA_SENSOR_I2C_INSTANCE                           RTE_AR0145_CAMERA_SENSOR_I2C_INSTANCE
#endif

/* Image Configuration */
#if (RTE_AR0145_CAMERA_SENSOR_IMAGE_CONFIG == 0)
#define AR0145_CAMERA_SENSOR_FRAME_HEIGHT  800
#define AR0145_CAMERA_SENSOR_FRAME_WIDTH   1280
#elif ((RTE_AR0145_CAMERA_SENSOR_IMAGE_CONFIG == 1) || (RTE_AR0145_CAMERA_SENSOR_IMAGE_CONFIG == 2))
#define AR0145_CAMERA_SENSOR_FRAME_HEIGHT  400
#define AR0145_CAMERA_SENSOR_FRAME_WIDTH   640
#elif (RTE_AR0145_CAMERA_SENSOR_IMAGE_CONFIG == 3)
#define AR0145_CAMERA_SENSOR_FRAME_HEIGHT  200
#define AR0145_CAMERA_SENSOR_FRAME_WIDTH   320
#else
#error Unsupported resolution
#endif

/* AR0145 Camera Sensor Slave Address. */
#define AR0145_CAMERA_SENSOR_SLAVE_ADDR                      0x36

/* AR0145 Camera Sensor CHIP-ID registers */
#define AR0145_CHIP_ID_REGISTER                              0x3000
#define AR0145_CHIP_ID_REGISTER_VALUE                        0x1750

/* AR0145 Camera Sensor registers index */
#define AR0145_RESET_REGISTER                                0x301A
#define AR0145_MIPI_CONFIG_REGISTER                          0x31BE
#define AR0145_COARSE_INTEGRATION_TIME_REGISTER              0x0202
#define AR0145_GLOBAL_GAIN_REGISTER                          0x305E
#define AR0145_X_ADDR_END_REGISTER                           0x3008
#define AR0145_Y_ADDR_END_REGISTER                           0x3006
#define AR0145_X_EVEN_INC_REGISTER                           0x30A0
#define AR0145_X_ODD_INC_REGISTER                            0x30A2
#define AR0145_Y_EVEN_INC_REGISTER                           0x30A4
#define AR0145_Y_ODD_INC_REGISTER                            0x30A6
#define AR0145_X_OUTPUT_SIZE_REGISTER                        0x034C
#define AR0145_Y_OUTPUT_SIZE_REGISTER                        0x034E
#define AR0145_READ_MODE_REGISTER                            0x3040
#define AR0145_SMIA_TEST_REGISTER                            0x3064
#define AR0145_DATAPATH_SELECT_REGISTER                      0x306E
#define AR0145_AE_MAX_EXPOSURE_REGISTER                      0x323A
#define AR0145_AE_STATS_CONTROL_REGISTER                     0x327A
#define AR0145_AE_CTRL_REGISTER                              0x3220
#define AR0145_AE_LUMA_TARGET_REGISTER                       0x3222

/* AR0145 Camera Sensor Configurations*/
#define AR0145_AE_LUMA_TARGET_VALUE                          0x6500
#define AR0145_AE_ENABLE                                     0x0213
#define AR0145_AE_DISABLE                                    0x0200
#define AR0145_EMBEDDED_STATISTICS_ENABLE                    0x0010
#define AR0145_EMBEDDED_DATA_ENABLE                          0x5940
#define AR0145_EMBEDDED_DATA_SELECT                          0x900F


/* Wrapper function for Delay
 * Delay for microsecond:
 * Provide delay using PMU(Performance Monitoring Unit).
 */
#define AR0145_DELAY_uSEC(usec)       sys_busy_loop_us(usec)

/* Wrapper function for i2c read
 *  read register value from AR0145 Camera Sensor registers
 *   using i2c read API \ref camera_sensor_i2c_read
 *
 *  for AR0145 Camera Sensor specific i2c configurations
 *   see \ref AR0145_camera_sensor_i2c_cnfg
 */
#define AR0145_READ_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_read(&AR0145_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/* Wrapper function for i2c write
 *  write register value to AR0145 Camera Sensor registers
 *   using i2c write API \ref camera_sensor_i2c_write.
 *
 *  for AR0145 Camera Sensor specific i2c configurations
 *   see \ref AR0145_camera_sensor_i2c_cnfg
 */
#define AR0145_WRITE_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_write(&AR0145_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

static uint32_t current_integration_time;
static uint32_t max_integration_time;

/* AR0145 Camera reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_AR0145_CAMERA_SENSOR_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_RST = &ARM_Driver_GPIO_(RTE_AR0145_CAMERA_SENSOR_RESET_GPIO_PORT);

/* AR0145 Camera power GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_AR0145_CAMERA_SENSOR_POWER_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_PWR = &ARM_Driver_GPIO_(RTE_AR0145_CAMERA_SENSOR_POWER_GPIO_PORT);

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(CAMERA_SENSOR_I2C_INSTANCE);

/**
  \brief AR0145 Camera Sensor Register Array Structure
  used for Camera Resolution Configuration.
  */
typedef struct _AR0145_REG {
    uint16_t reg_addr;             /* AR0145 Camera Sensor Register Address*/
    uint16_t reg_value;            /* AR0145 Camera Sensor Register Value*/
} AR0145_REG;

/**
  \brief AR0145 Camera Sensor slave i2c Configuration
  \ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
  */
static CAMERA_SENSOR_SLAVE_I2C_CONFIG AR0145_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(CAMERA_SENSOR_I2C_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = AR0145_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

static const AR0145_REG cfg_8bit_2Lane_Pxlclk80MHz_Extclk25MHz[] = {
        { 0x301A, 0x0019 }, // RESET_REGISTER
        { 0xFFFF, 0x2    }, // DELAY= 200
        { 0x0300, 0x0005 }, // VT_PIX_CLK_DIV
        { 0x0302, 0x0001 }, // VT_SYS_CLK_DIV
        { 0x0304, 0x0505 }, // PRE_PLL_CLK_DIV
        { 0x0306, 0x8050 }, // PLL_MULTIPLIER
        { 0x0308, 0x0008 }, // OP_PIX_CLK_DIV
        { 0x030A, 0x0001 }, // OP_SYS_CLK_DIV
        { 0x0344, 0x0008 }, // X_ADDR_START
        { 0x0346, 0x0008 }, // Y_ADDR_START
        { 0x3040, 0x0000 }, // READ_MODE
        { 0x0400, 0x0000 }, // SCALING_MODE
        { 0x0404, 0x0010 }, // SCALE_M
        { 0x0342, 0x0640 }, // LINE_LENGTH_PCK
        { 0x0340, 0x0340 }, // FRAME_LENGTH_LINES
        { 0x0112, 0x0808 }, // CCP_DATA_FORMAT
        { 0x31B0, 0x0041 }, // FRAME_PREAMBLE
        { 0x31B2, 0x0024 }, // LINE_PREAMBLE
        { 0x31B4, 0x320A }, // MIPI_TIMING_0
        { 0x31B6, 0x32E6 }, // MIPI_TIMING_1
        { 0x31B8, 0x1412 }, // MIPI_TIMING_2
        { 0x31BA, 0x1442 }, // MIPI_TIMING_3
        { 0x31BC, 0x8407 }, // MIPI_TIMING_4

};

/**
  \fn           int32_t AR0145_Bulk_Write_Reg(const AR0145_REG AR0145_reg[],
                                              uint32_t total_num, uint32_t reg_size))
  \brief        write array of registers value to AR0145 Camera Sensor registers.
  \param[in]    AR0145_reg : AR0145 Camera Sensor Register Array Structure
  \ref AR0145_REG
  \param[in]    total_num   : total number of registers(size of array)
  \return       \ref execution_status
  */
static int32_t AR0145_Bulk_Write_Reg(const AR0145_REG AR0145_reg[],
                                     uint32_t total_num, uint32_t reg_size)
{
    uint32_t i  = 0;
    int32_t ret = 0;

    for(i = 0; i < total_num; i++)
    {
        if (0xFFFF == AR0145_reg[i].reg_addr) {
            for(uint32_t j = 0; j < AR0145_reg[i].reg_value  ; j++)
            {
                AR0145_DELAY_uSEC(100 * 1000);
            }
            continue;
        }
        ret = AR0145_WRITE_REG(AR0145_reg[i].reg_addr, AR0145_reg[i].reg_value, \
                reg_size);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           void AR0145_Sensor_Enable_Clk_Src(void)
  \brief        Enable AR0145 Camera Sensor external clock source configuration.
  \param[in]    none
  \return       none
  */
static void AR0145_Sensor_Enable_Clk_Src(void)
{
    set_cpi_pixel_clk(CPI_PIX_CLKSEL_400MZ, RTE_AR0145_CAMERA_SENSOR_CSI_CLK_SCR_DIV);
}

/**
  \fn           void AR0145_Sensor_Disable_Clk_Src(void)
  \brief        Disable AR0145 Camera Sensor external clock source configuration.
  \param[in]    none
  \return       none
  */
static void AR0145_Sensor_Disable_Clk_Src(void)
{
    clear_cpi_pixel_clk();
}

/**
  \fn           int32_t AR0145_Camera_Hard_Reseten(void)
  \brief        Hard Reset AR0145 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t AR0145_Camera_Hard_Reseten(void)
{
    int32_t ret = 0;

    ret = GPIO_Driver_CAM_RST->Initialize(RTE_AR0145_CAMERA_SENSOR_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_AR0145_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetDirection(RTE_AR0145_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->Initialize(RTE_AR0145_CAMERA_SENSOR_POWER_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_AR0145_CAMERA_SENSOR_POWER_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetDirection(RTE_AR0145_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_AR0145_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0145_DELAY_uSEC(2000);

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_AR0145_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0145_DELAY_uSEC(1000);

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_AR0145_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0145_DELAY_uSEC(100000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0145_Camera_Soft_Reseten(void)
  \brief        Software Reset AR0145 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t AR0145_Camera_Soft_Reseten(void)
{
    int32_t ret = 0;

    ret = AR0145_WRITE_REG(AR0145_RESET_REGISTER, 0x01, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* @Observation: more delay is required for Camera Sensor
     *               to setup after Soft Reset.
     */
    AR0145_DELAY_uSEC(10000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0145_Sensor_Configuration(void)
  \brief        AR0145 Camera Sensor configuration
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0145_Sensor_Configuration(void)
{
    int32_t ret;
    uint32_t total_num, skip = 1280 / AR0145_CAMERA_SENSOR_FRAME_WIDTH;

    total_num = (sizeof(cfg_8bit_2Lane_Pxlclk80MHz_Extclk25MHz) / sizeof(AR0145_REG));
    ret = AR0145_Bulk_Write_Reg(cfg_8bit_2Lane_Pxlclk80MHz_Extclk25MHz, total_num, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_X_ADDR_END_REGISTER, 1287 - (skip - 1), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_Y_ADDR_END_REGISTER, 807 - (skip - 1), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_X_EVEN_INC_REGISTER, skip, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;
    ret = AR0145_WRITE_REG(AR0145_X_ODD_INC_REGISTER, skip, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;
    ret = AR0145_WRITE_REG(AR0145_Y_EVEN_INC_REGISTER, skip, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;
    ret = AR0145_WRITE_REG(AR0145_Y_ODD_INC_REGISTER, skip, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_X_OUTPUT_SIZE_REGISTER, AR0145_CAMERA_SENSOR_FRAME_WIDTH, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;
    ret = AR0145_WRITE_REG(AR0145_Y_OUTPUT_SIZE_REGISTER, AR0145_CAMERA_SENSOR_FRAME_HEIGHT, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_COARSE_INTEGRATION_TIME_REGISTER, AR0145_CAMERA_SENSOR_FRAME_HEIGHT + 31, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

#if(RTE_AR0145_CAMERA_SENSOR_IMAGE_CONFIG == 1)
    uint32_t  rcv_data;
    ret = AR0145_READ_REG(AR0145_READ_MODE_REGISTER, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_READ_MODE_REGISTER, rcv_data | (1U << 11) | (1U << 13), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;
#endif

    max_integration_time = current_integration_time = 0;
    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0145_Camera_Gain(uint32_t gain)
  \brief        Set camera gain
  this function will
  - configure Camera Sensor gain and integration time as per input parameter.

  Gain 1 is the default integration time with camera gain set to 1.

  For requested gain > 1, we use the default integration time and adjust the analogue+digital gain.
  For requested gain < 1, we set gain to 1 and use a reduced integration time.

  \note This has been adapted to AR0145 from ARX3A0
  \param[in]    gain    : gain value * 65536 (so 1.0 gain = 65536); 0 to read
  \return       \ref actual gain
  */
static int32_t AR0145_Camera_Gain(const uint32_t gain)
{
    uint32_t digital_gain;
    uint32_t fine_gain = gain;
    uint32_t coarse_gain;
    uint32_t t,s;

    if (max_integration_time == 0)
    {
        /* Record the integration time set by the configuration tables. We won't adjust it upwards, as it may
         * interfere with frame timing, but we can freely adjust it downwards.
         */
        int32_t ret = AR0145_READ_REG(AR0145_COARSE_INTEGRATION_TIME_REGISTER, &current_integration_time, 2);
        if (ret != 0)
            return ret;

        max_integration_time = current_integration_time;
    }

    if (gain != 0)
    {
        /* Request to set gain */

        /* From the Design Guide:
         *
         * First clamp analogue gain, using digital gain to get more if
         * necessary. Otherwise digital gain is used to fine adjust.
         */
        uint32_t new_integration_time = max_integration_time;
        if (gain < 0x10000)
        {
            /* Minimum gain is 1.0 */
            fine_gain = 0x10000;

            /* But we can lower integration time */
            new_integration_time = (uint32_t) (((float) max_integration_time * gain) * 0x1p-16f + 0.5f);
        }
        else if (gain > 0x80000)
        {
            /* Maximum analogue gain is 8.0 */
            fine_gain = 0x80000;
        }

        /* Set integration time */
        if (new_integration_time != current_integration_time)
        {
            int32_t ret = AR0145_WRITE_REG(AR0145_COARSE_INTEGRATION_TIME_REGISTER, new_integration_time, 2);
            if (ret != 0)
            {
                return ret;
            }
            current_integration_time = new_integration_time;
        }

        /*
         * First get coarse analogue power of two, leaving fine gain in range [0x10000,0x1FFFF]
         */
        s = 0;
        while (fine_gain >= 0x20000)
        {
            s++;
            fine_gain /= 2;
        }

        /* Fine gain is produced as 1 + (t / 16), roughly. Rearranged, t = 16(fine_gain - 1)*/
        /* Ensure we round down, by rounding up the division */
        t = 16 * ((fine_gain / 0x10000) - 1);

        /* Use digital gain to extend gain beyond the analogue limits of
         * x1 to x8, or to fine-tune within that range.
         *
         * We don't let digital gain go below 1.0 - it just loses information,
         * and clamping it lets an auto-gain controller see that we are
         * unable to improve exposure by such lowering. Another camera might
         * be able to usefully set gain to <1.0, so a controller could try it.
         *
         * (When we're fine tuning, digital gain is always >=1.0, because we
         * round down analogue gain, so it can only go below 1.0 by the user
         * requesting total gain < 1.0).
         */

         /* coarse_gain = 2^s, Total analog gain = coarse_gain * (1 + t / 16) */
        coarse_gain = (1 << s);
        float resulting_analogue_gain = coarse_gain * (1 + (t)/16.f);

        digital_gain = (uint32_t) ((0x1p5f * 0x1p-16f * gain) / resulting_analogue_gain + 0.5f);
        if (digital_gain > 0x1FF)
        {
            /* Maximum digital gain is just under 16.0 (limited by register size) */
            digital_gain = 0x1FF;
        }
        else if (digital_gain < 32)
        {
            /* Digital gain >= 1.0, as per discussion above */
            digital_gain = 32;
        }
        int32_t ret = AR0145_WRITE_REG(AR0145_GLOBAL_GAIN_REGISTER, (digital_gain << 7) |(s << 4) | t, 2);
        if (ret != 0)
            return ret;
    }
    else
    {
        uint32_t reg_value;

        int32_t ret = AR0145_READ_REG(AR0145_GLOBAL_GAIN_REGISTER, &reg_value, 2);
        if (ret != 0)
            return ret;
        digital_gain = (reg_value >> 7) & 0x1FF;
        s = (reg_value >> 4) & 7;
        t = reg_value & 0xF;
        coarse_gain = (1 << s);
    }

    float resulting_gain = coarse_gain * (1 + (t)/16.f) * (digital_gain * 0x1p-5f);

    // And adjust for any reduction of integration time (in which case gain should be 0x10000 initially...)
    if (current_integration_time != max_integration_time)
    {
        resulting_gain = ((resulting_gain * current_integration_time) / max_integration_time + 0.5f);
    }
    return (uint32_t) (resulting_gain * 0x1p16f + 0.5f);
}

/**
  \fn           int32_t AR0145_Camera_AE(const uint32_t enable)
  \brief        Set camera Auto Exposure
  \param[in]    enable: 0=disable, 1=enable
  \return       \ref execution_statusn
  */
static int32_t AR0145_Camera_AE(const uint32_t enable)
{
    int32_t  ret = 0;
    if(enable)
    {
        uint32_t rcv_data = 0;
        ret = AR0145_READ_REG(AR0145_COARSE_INTEGRATION_TIME_REGISTER, &rcv_data, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;

        ret = AR0145_WRITE_REG(AR0145_AE_MAX_EXPOSURE_REGISTER, rcv_data, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;

        ret = AR0145_WRITE_REG(AR0145_AE_STATS_CONTROL_REGISTER, AR0145_EMBEDDED_STATISTICS_ENABLE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;

        ret = AR0145_WRITE_REG(AR0145_SMIA_TEST_REGISTER, AR0145_EMBEDDED_DATA_ENABLE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;

        ret = AR0145_WRITE_REG(AR0145_DATAPATH_SELECT_REGISTER, AR0145_EMBEDDED_DATA_SELECT, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;

        ret = AR0145_WRITE_REG(AR0145_AE_CTRL_REGISTER, AR0145_AE_ENABLE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;

        ret = AR0145_WRITE_REG(AR0145_AE_LUMA_TARGET_REGISTER, AR0145_AE_LUMA_TARGET_VALUE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }
    else
    {
        ret = AR0145_WRITE_REG(AR0145_AE_CTRL_REGISTER, AR0145_AE_DISABLE, 2);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0145_Init(void)
  \brief        Initialize AR0145 Camera Sensor
  this function will
  - initialize i2c using i3c instance
  - software reset AR0145 Camera Sensor
  - read AR0145 chip-id, proceed only it is correct.
  \return       \ref execution_status
  */
int32_t AR0145_Init(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /*Enable camera sensor clock source config*/
    AR0145_Sensor_Enable_Clk_Src();

    /*camera sensor resten*/
    ret = AR0145_Camera_Hard_Reseten();
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Initialize i2c using i3c driver instance depending on
     *  AR0145 Camera Sensor specific i2c configurations
     *   \ref AR0145_camera_sensor_i2c_cnfg
     */
    ret = camera_sensor_i2c_init(&AR0145_camera_sensor_i2c_cnfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Soft Reset AR0145 Camera Sensor */
    ret = AR0145_Camera_Soft_Reseten();
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Read AR0145 Camera Sensor CHIP ID */
    ret = AR0145_READ_REG(AR0145_CHIP_ID_REGISTER, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Proceed only if CHIP ID is correct. */
    if(rcv_data != AR0145_CHIP_ID_REGISTER_VALUE)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    /* Enable LP11 on standby */
    ret = AR0145_READ_REG(AR0145_MIPI_CONFIG_REGISTER, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_MIPI_CONFIG_REGISTER, rcv_data | (1U << 7), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /*start streaming*/
    ret = AR0145_READ_REG(AR0145_RESET_REGISTER, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_RESET_REGISTER, rcv_data | (1U << 2), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0145_DELAY_uSEC(50000);

    /*stop streaming*/
    ret = AR0145_WRITE_REG(AR0145_RESET_REGISTER, rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /*Adding delay to finish streaming*/
    AR0145_DELAY_uSEC(500000);

    /* Force re-reading of the registers */
    max_integration_time = current_integration_time = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0145_Start(void)
  \brief        Start AR0145 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0145_Start(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /* Start streaming */
    ret = AR0145_READ_REG(AR0145_RESET_REGISTER, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0145_WRITE_REG(AR0145_RESET_REGISTER, rcv_data | (1U << 2), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return ret;
}

/**
  \fn           int32_t AR0145_Stop(void)
  \brief        Stop AR0145 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0145_Stop(void)
{
    /* Suspend any stream */
    return AR0145_WRITE_REG(AR0145_RESET_REGISTER, 0x00, 1);
}

/**
  \fn           int32_t AR0145_Control(uint32_t control, uint32_t arg)
  \brief        Control AR0145 Camera Sensor.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
int32_t AR0145_Control(uint32_t control, uint32_t arg)
{
    switch (control)
    {
        case CPI_CAMERA_SENSOR_CONFIGURE:
            return AR0145_Sensor_Configuration();
        case CPI_CAMERA_SENSOR_GAIN:
            return AR0145_Camera_Gain(arg);
        case CPI_CAMERA_SENSOR_AE:
            return AR0145_Camera_AE(arg);
        case CPI_CAMERA_SENSOR_AE_TARGET_LUMA:
            /* Legal values: [0, 65535]. To set to 1/2 saturation, value should be set to 0x8000. */
            return AR0145_WRITE_REG(AR0145_AE_LUMA_TARGET_REGISTER, arg, 2);
        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}


/**
  \fn           int32_t AR0145_Uninit(void)
  \brief        Un-initialize AR0145 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0145_Uninit(void)
{

    /*Disable camera sensor clock source config*/
    AR0145_Sensor_Disable_Clk_Src();

    return ARM_DRIVER_OK;
}

/**
\brief AR0145 Camera Sensor CSi informations
\ref CSI_INFO
*/
static CSI_INFO AR0145_csi_info =
{
    .frequency                = RTE_AR0145_CAMERA_SENSOR_CSI_FREQ,
    .dt                       = RTE_AR0145_CAMERA_SENSOR_CSI_DATA_TYPE,
    .n_lanes                  = RTE_AR0145_CAMERA_SENSOR_CSI_N_LANES,
    .vc_id                    = RTE_AR0145_CAMERA_SENSOR_CSI_VC_ID,
    .cpi_cfg.override         = RTE_AR0145_CAMERA_SENSOR_OVERRIDE_CPI_COLOR_MODE,
    .cpi_cfg.cpi_color_mode   = RTE_AR0145_CAMERA_SENSOR_CPI_COLOR_MODE
};

/**
\brief AR0145 Camera Sensor Operations
\ref CAMERA_SENSOR_OPERATIONS
*/
static CAMERA_SENSOR_OPERATIONS AR0145_ops =
{
    .Init    = AR0145_Init,
    .Uninit  = AR0145_Uninit,
    .Start   = AR0145_Start,
    .Stop    = AR0145_Stop,
    .Control = AR0145_Control,
};

/**
\brief AR0145 Camera Sensor Device Structure
\ref CAMERA_SENSOR_DEVICE
*/
static CAMERA_SENSOR_DEVICE AR0145_camera_sensor =
{
    .interface  = CAMERA_SENSOR_INTERFACE_MIPI,
    .width      = AR0145_CAMERA_SENSOR_FRAME_WIDTH,
    .height     = AR0145_CAMERA_SENSOR_FRAME_HEIGHT,
    .csi_info   = &AR0145_csi_info,
    .ops        = &AR0145_ops,
};

/* Registering CPI sensor */
CAMERA_SENSOR(AR0145_camera_sensor)

#endif /* RTE_AR0145_CAMERA_SENSOR_ENABLE */
