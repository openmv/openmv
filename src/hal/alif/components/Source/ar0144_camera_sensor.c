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
#include <inttypes.h>


/* Proceed only if AR0144 Camera Sensor is enabled. */
#if RTE_AR0144_CAMERA_SENSOR_CSI_ENABLE

/* I2C Instance */
#if(RTE_AR0144_CAMERA_SENSOR_I2C_INSTANCE == 4)
#define CAMERA_SENSOR_I2C_INSTANCE                           I3C
#else
#define CAMERA_SENSOR_I2C_INSTANCE                           RTE_AR0144_CAMERA_SENSOR_I2C_INSTANCE
#endif

/* AR0144 Camera Sensor Slave Address. */
#define AR0144_CAMERA_SENSOR_SLAVE_ADDR                      0x18

/* AR0144 Camera Sensor CHIP-ID registers */
#define AR0144_CHIP_ID_REGISTER                              0x3000
#define AR0144_CHIP_ID_REGISTER_VALUE                        0x0356

/* AR0144 Camera Sensor registers index */
#define AR0144_SOFTWARE_RESET_REGISTER                       0x3021
#define AR0144_MODE_SELECT_REGISTER                          0x301C
#define AR0144_COARSE_INTEGRATION_TIME_REGISTER              0x3012
#define AR0144_GLOBAL_GAIN_REGISTER                          0x305E
#define AR0144_ANALOG_GAIN_REGISTER                          0x3060

/* Wrapper function for Delay
 * Delay for microsecond:
 * Provide delay using PMU(Performance Monitoring Unit).
 */
#define AR0144_DELAY_uSEC(usec)       sys_busy_loop_us(usec)

/* Wrapper function for i2c read
 *  read register value from AR0144 Camera Sensor registers
 *   using i2c read API \ref camera_sensor_i2c_read
 *
 *  for AR0144 Camera Sensor specific i2c configurations
 *   see \ref AR0144_camera_sensor_i2c_cnfg
 */
#define AR0144_READ_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_read(&ar0144_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/* Wrapper function for i2c write
 *  write register value to AR0144 Camera Sensor registers
 *   using i2c write API \ref camera_sensor_i2c_write.
 *
 *  for AR0144 Camera Sensor specific i2c configurations
 *   see \ref AR0144_camera_sensor_i2c_cnfg
 */
#define AR0144_WRITE_REG(reg_addr, reg_value, reg_size) \
    camera_sensor_i2c_write(&ar0144_camera_sensor_i2c_cnfg, \
            reg_addr,  \
            reg_value, \
            (CAMERA_SENSOR_I2C_REG_SIZE)reg_size)

/**
  \brief AR0144 Camera Sensor Register Array Structure
  used for Camera Resolution Configuration.
  */
typedef struct _AR0144_REG {
    uint16_t reg_addr;             /* AR0144 Camera Sensor Register Address*/
    uint16_t reg_value;            /* AR0144 Camera Sensor Register Value*/
} AR0144_REG;

static uint32_t current_integration_time;
static uint32_t max_integration_time;
/* AR0144 Camera reset GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_AR0144_CAMERA_SENSOR_RESET_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_RST = &ARM_Driver_GPIO_(RTE_AR0144_CAMERA_SENSOR_RESET_GPIO_PORT);

/* AR0144 Camera power GPIO port */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_AR0144_CAMERA_SENSOR_POWER_GPIO_PORT);
static ARM_DRIVER_GPIO *GPIO_Driver_CAM_PWR = &ARM_Driver_GPIO_(RTE_AR0144_CAMERA_SENSOR_POWER_GPIO_PORT);

/* I2C Driver Instance */
extern ARM_DRIVER_I2C ARM_Driver_I2C_(CAMERA_SENSOR_I2C_INSTANCE);

static const AR0144_REG cfg_1280x800_12bit_60fps_2Lane_Pxlclk75MHz_Extclk20MHz[] = {
    {0xFFFF, 1000},   //DELAY= 200
    {0x301A, 0x00D9}, //RESET_REGISTER
    {0x301A, 0x3058}, //RESET_REGISTER
    {0xFFFF, 500},    //DELAY= 100
    {0x3F4C, 0x003F}, //RESERVED_MFR_3F4C
    {0x3F4E, 0x0018}, //RESERVED_MFR_3F4E
    {0x3F50, 0x17DF}, //RESERVED_MFR_3F50
    {0x30B0, 0x0028}, //DIGITAL_TEST
    {0x3060, 0x000D}, //ANALOG_GAIN
    {0x301E, 0x0000}, //DATA_PEDESTAL              // ADDED
    {0x30FE, 0x00A8}, //NOISE_PEDESTAL
    {0x306E, 0x4810}, //DATAPATH_SELECT
    {0x3064, 0x1802}, //SMIA_TEST

    /* PLL_settings */
    {0x302A, 0x0006},   //VT_PIX_CLK_DIV = 6
    {0x302C, 0x0001},   //VT_SYS_CLK_DIV = 1
    {0x302E, 0x0004},   //PRE_PLL_CLK_DIV = 4
    {0x3030, 0x005A},   //PLL_MULTIPLIER = 90
    {0x3036, 0x000C},   //OP_PIX_CLK_DIV = 12
    {0x3038, 0x0001},   //OP_SYS_CLK_DIV = 1
    {0x31B0, 0x004D},   //FRAME_PREAMBLE = 77
    {0x31B2, 0x0036},   //LINE_PREAMBLE = 54
    {0x31B4, 0x3634},   //MIPI_TIMING_0 = 13876
    {0x31B6, 0x210E},   //MIPI_TIMING_1 = 8462
    {0x31B8, 0x20C7},   //MIPI_TIMING_2 = 8391
    {0x31BA, 0x0185},   //MIPI_TIMING_3 = 389
    {0x31BC, 0x0004},   //MIPI_TIMING_4 = 4
    {0x3354, 0x002C},   //MIPI_CNTRL = 44

    /* Timing_settings */
    {0x31AE, 0x0202},   //SERIAL_FORMAT = 514
    {0x3002, 0x0000},   //Y_ADDR_START = 0
    {0x3004, 0x0004},   //X_ADDR_START = 4
    {0x3006, 0x031F},   //Y_ADDR_END = 799
    {0x3008, 0x0503},   //X_ADDR_END = 1283
    {0x300A, 0x0349},   //FRAME_LENGTH_LINES = 841
    {0x300C, 0x05D0},   //LINE_LENGTH_PCK = 1488
    {0x3012, 0x00C0},   //COARSE_INTEGRATION_TIME = 51  // MODIFIED FROM 0x0033
    {0x31AC, 0x0C0C},   //DATA_FORMAT_BITS = 3084
    {0x306E, 0x9010},   //DATAPATH_SELECT = 36880
    {0x30A2, 0x0001},   //X_ODD_INC = 1
    {0x30A6, 0x0001},   //Y_ODD_INC = 1
    {0x3082, 0x0003},   //OPERATION_MODE_CTRL = 3
    {0x3084, 0x0003},   //OPERATION_MODE_CTRL_CB = 3
    {0x308C, 0x0000},   //Y_ADDR_START_CB = 0
    {0x308A, 0x0004},   //X_ADDR_START_CB = 4
    {0x3090, 0x031F},   //Y_ADDR_END_CB = 799
    {0x308E, 0x0503},   //X_ADDR_END_CB = 1283
    {0x30AA, 0x0349},   //FRAME_LENGTH_LINES_CB = 841
    {0x303E, 0x05D0},   //LINE_LENGTH_PCK_CB = 1488
    {0x3016, 0x00C0},   //COARSE_INTEGRATION_TIME_CB = 51   // MODIFIED FROM 0x0033
    {0x30AE, 0x0001},   //X_ODD_INC_CB = 1
    {0x30A8, 0x0001},   //Y_ODD_INC_CB = 1
    {0x3040, 0x0000},   //READ_MODE = 0
    {0x31D0, 0x0000},   //COMPANDING = 0
    {0x301A, 0x005C},   //RESET_REGISTER = 92
};

/**
  \brief AR0144 Camera Sensor slave i2c Configuration
  \ref CAMERA_SENSOR_SLAVE_I2C_CONFIG
  */
static CAMERA_SENSOR_SLAVE_I2C_CONFIG ar0144_camera_sensor_i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(CAMERA_SENSOR_I2C_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .cam_sensor_slave_addr          = AR0144_CAMERA_SENSOR_SLAVE_ADDR,
    .cam_sensor_slave_reg_addr_type = CAMERA_SENSOR_I2C_REG_ADDR_TYPE_16BIT,
};

/**
  \fn           int32_t AR0144_Bulk_Write_Reg(const AR0144_REG ar0144_reg[],
                                              uint32_t total_num, uint32_t reg_size))
  \brief        write array of registers value to AR0144 Camera Sensor registers.
  \param[in]    ar0144_reg : AR0144 Camera Sensor Register Array Structure
  \ref AR0144_REG
  \param[in]    total_num   : total number of registers(size of array)
  \return       \ref execution_status
  */
static int32_t AR0144_Bulk_Write_Reg(const AR0144_REG ar0144_reg[],
                                     uint32_t total_num, uint32_t reg_size)
{
    uint32_t i  = 0;
    int32_t ret = 0;

    for(i = 0; i < total_num; i++)
    {
        if (0xFFFF == ar0144_reg[i].reg_addr) {
            AR0144_DELAY_uSEC(ar0144_reg[i].reg_value * 200);
            continue;
        }
        ret = AR0144_WRITE_REG(ar0144_reg[i].reg_addr, ar0144_reg[i].reg_value, \
                reg_size);
        if(ret != ARM_DRIVER_OK)
            return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           void AR0144_Sensor_Enable_Clk_Src(void)
  \brief        Enable AR0144 Camera Sensor external clock source configuration.
  \param[in]    none
  \return       none
  */
static void AR0144_Sensor_Enable_Clk_Src(void)
{
    set_cpi_pixel_clk(CPI_PIX_CLKSEL_400MZ, RTE_AR0144_CAMERA_SENSOR_CSI_CLK_SCR_DIV);
}

/**
  \fn           void AR0144_Sensor_Disable_Clk_Src(void)
  \brief        Disable AR0144 Camera Sensor external clock source configuration.
  \param[in]    none
  \return       none
  */
static void AR0144_Sensor_Disable_Clk_Src(void)
{
    clear_cpi_pixel_clk();
}

/**
  \fn           int32_t AR0144_Camera_Hard_Reseten(void)
  \brief        Hard Reset AR0144 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t AR0144_Camera_Hard_Reseten(void)
{
    int32_t ret = 0;

    ret = GPIO_Driver_CAM_RST->Initialize(RTE_AR0144_CAMERA_SENSOR_RESET_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->PowerControl(RTE_AR0144_CAMERA_SENSOR_RESET_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetDirection(RTE_AR0144_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->Initialize(RTE_AR0144_CAMERA_SENSOR_POWER_PIN_NO, NULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->PowerControl(RTE_AR0144_CAMERA_SENSOR_POWER_PIN_NO, ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_PWR->SetDirection(RTE_AR0144_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_DIRECTION_OUTPUT);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_AR0144_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_LOW);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0144_DELAY_uSEC(2000);

    ret = GPIO_Driver_CAM_PWR->SetValue(RTE_AR0144_CAMERA_SENSOR_POWER_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0144_DELAY_uSEC(1000);

    ret = GPIO_Driver_CAM_RST->SetValue(RTE_AR0144_CAMERA_SENSOR_RESET_PIN_NO, GPIO_PIN_OUTPUT_STATE_HIGH);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0144_DELAY_uSEC(100000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0144_Camera_Soft_Reseten(void)
  \brief        Software Reset AR0144 Camera Sensor
  \param[in]    none
  \return       \ref execution_status
  */
static int32_t AR0144_Camera_Soft_Reseten(void)
{
    int32_t ret = 0;

    ret = AR0144_WRITE_REG(AR0144_SOFTWARE_RESET_REGISTER, 0x01, 1);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* @Observation: more delay is required for Camera Sensor
     *               to setup after Soft Reset.
     */
    AR0144_DELAY_uSEC(10000);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0144_Camera_Cfg(ARM_CAMERA_RESOLUTION cam_resolution)
  \brief        Initialize AR0144 Camera Sensor.
  \return       \ref execution_status
  */
static int32_t AR0144_Camera_Cfg(void)
{
    return ARM_DRIVER_OK;
}

/**
  \fn           float s_t_to_analogue_gain(unsigned s, unsigned t)
  \brief        Method to converts coarse and fine gain to total gain value.
  \param[in]    s : Coarse gain.
  \param[in]    t : Fine gain.
  \return       Total gain value in floating point.
  */
static float s_t_to_analogue_gain(unsigned s, unsigned t)
{
    float fine_gain;

    switch (s) {
    case 0:
    case 2:
        fine_gain = 32.f / (32 - t);
        break;
    case 1:
    case 3:
        fine_gain = 16.f / (16 - t / 2);
        break;
    default:
        fine_gain = 8.f / (8 - t / 4);
        break;
    }

    return fine_gain * (1 << s);
}

/**
  \fn           int32_t AR0144_Camera_Gain(uint32_t gain)
  \brief        Set camera gain
  this function will
  - configure Camera Sensor gain and integration time as per input parameter.

  Gain 1 is the default integration time with camera gain set to 1.

  For requested gain > 1, we use the default integration time and adjust the analogue+digital gain.
  For requested gain < 1, we set gain to 1 and use a reduced integration time.

  \note This has been adapted to AR0144 from ARX3A0
        In AR0144 the digital gain and analog gain are in separate registers instead of one as in ARX3A0
  \param[in]    gain    : gain value * 65536 (so 1.0 gain = 65536); 0 to read
  \return       \ref actual gain
  */
static int32_t AR0144_Camera_Gain(const uint32_t gain)
{
    uint32_t digital_gain;
    uint32_t fine_gain = gain;
    uint32_t coarse_gain;
    uint32_t t;

    if (max_integration_time == 0)
    {
        /* Record the integration time set by the configuration tables. We won't adjust it upwards, as it may
         * interfere with frame timing, but we can freely adjust it downwards.
         */
        int32_t ret = AR0144_READ_REG(AR0144_COARSE_INTEGRATION_TIME_REGISTER, &current_integration_time, 2);
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
        else if (gain > 0x100000)
        {
            /* Maximum analogue gain is 16.0 */
            fine_gain = 0x100000;
        }

        /* Set integration time */
        if (new_integration_time != current_integration_time)
        {
            int32_t ret = AR0144_WRITE_REG(AR0144_COARSE_INTEGRATION_TIME_REGISTER, new_integration_time, 2);
            if (ret != 0)
            {
                return ret;
            }
            current_integration_time = new_integration_time;
        }

        /*
         * First get coarse analogue power of two, leaving fine gain in range [0x10000,0x1FFFF]
         */
        coarse_gain = 0;
        while (fine_gain >= 0x20000)
        {
            coarse_gain++;
            fine_gain /= 2;
        }

        /* Fine gain is produced as 1/(1-(t/32)), roughly. Rearranged, t = 32 - 32/fine_gain */
        /* Ensure we round down, by rounding up the division */
        t = 32 - (32*0x10000 + fine_gain - 1) / fine_gain;

        /* Use digital gain to extend gain beyond the analogue limits of
         * x1 to x16, or to fine-tune within that range.
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
        float resulting_analogue_gain = s_t_to_analogue_gain(coarse_gain, t);

        digital_gain = (uint32_t) ((0x1p7f * 0x1p-16f * gain) / resulting_analogue_gain + 0.5f);
        if (digital_gain > 0x7FF)
        {
            /* Maximum digital gain is just under 16.0 (limited by register size) */
            digital_gain = 0x7FF;
        }
        else if (digital_gain < 128)
        {
            /* Digital gain >= 1.0, as per discussion above */
            digital_gain = 128;
        }
        int32_t ret = AR0144_WRITE_REG(AR0144_ANALOG_GAIN_REGISTER, (coarse_gain << 4) | t, 2);
        if (ret != 0)
            return ret;
        ret = AR0144_WRITE_REG(AR0144_GLOBAL_GAIN_REGISTER, digital_gain, 2);
        if (ret != 0)
            return ret;
    }
    else
    {
        uint32_t reg_value;

        int32_t ret = AR0144_READ_REG(AR0144_GLOBAL_GAIN_REGISTER, &reg_value, 2);
        if (ret != 0)
            return ret;
        digital_gain = reg_value;

        ret = AR0144_READ_REG(AR0144_ANALOG_GAIN_REGISTER, &reg_value, 2);
        if (ret != 0)
            return ret;
        coarse_gain = (reg_value >> 4) & 7;
        t = reg_value & 0xF;
    }

    float resulting_gain = s_t_to_analogue_gain(coarse_gain, t) * (digital_gain * 0x1p-7f);

    // And adjust for any reduction of integration time (in which case gain should be 0x10000 initially...)
    if (current_integration_time != max_integration_time)
    {
        resulting_gain = ((resulting_gain * current_integration_time) / max_integration_time + 0.5f);
    }
    return (uint32_t) (resulting_gain * 0x1p16f + 0.5f);
}

/**
  \fn           int32_t AR0144_Init(void)
  \brief        Initialize AR0144 Camera Sensor
  this function will
  - initialize i2c using i3c instance
  - software reset AR0144 Camera Sensor
  - read AR0144 chip-id, proceed only it is correct.
  \return       \ref execution_status
  */
int32_t AR0144_Init(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /*Enable camera sensor clock source config*/
    AR0144_Sensor_Enable_Clk_Src();

    /*camera sensor resten*/
    AR0144_Camera_Hard_Reseten();

    /* Initialize i2c using i3c driver instance depending on
     *  AR0144 Camera Sensor specific i2c configurations
     *   \ref ar0144_camera_sensor_i2c_cnfg
     */
    ret = camera_sensor_i2c_init(&ar0144_camera_sensor_i2c_cnfg);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Soft Reset AR0144 Camera Sensor */
    ret = AR0144_Camera_Soft_Reseten();
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Read AR0144 Camera Sensor CHIP ID */
    ret = AR0144_READ_REG(AR0144_CHIP_ID_REGISTER, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /* Proceed only if CHIP ID is correct. */
    if(rcv_data != AR0144_CHIP_ID_REGISTER_VALUE)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    uint32_t total_num;

    total_num = (sizeof(cfg_1280x800_12bit_60fps_2Lane_Pxlclk75MHz_Extclk20MHz) / sizeof(AR0144_REG));
    AR0144_Bulk_Write_Reg(cfg_1280x800_12bit_60fps_2Lane_Pxlclk75MHz_Extclk20MHz, total_num, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0144_READ_REG(0x301a, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0144_WRITE_REG(0x301a, rcv_data | (1U << 2), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    AR0144_DELAY_uSEC(50000);

    /*stop streaming*/
    ret = AR0144_WRITE_REG(0x301a, rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /*Adding delay to finish streaming*/
    AR0144_DELAY_uSEC(500000);

    /* Force re-reading of the registers */
    max_integration_time = current_integration_time = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t AR0144_Start(void)
  \brief        Start AR0144 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0144_Start(void)
{
    int32_t  ret = 0;
    uint32_t rcv_data = 0;

    /* Start streaming */
    ret = AR0144_READ_REG(0x301a, &rcv_data, 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    ret = AR0144_WRITE_REG(0x301a, rcv_data | (1U << 2), 2);
    if(ret != ARM_DRIVER_OK)
        return ret;

    return ret;
}

/**
  \fn           int32_t AR0144_Stop(void)
  \brief        Stop AR0144 Camera Sensor Streaming.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0144_Stop(void)
{
    /* Suspend any stream */
    return AR0144_WRITE_REG(AR0144_MODE_SELECT_REGISTER, 0x00, 1);
}

#if (RTE_AR0144_CAMERA_SENSOR_FRAME_HEIGHT == 800) && (RTE_AR0144_CAMERA_SENSOR_FRAME_WIDTH == 1280)
#elif (RTE_AR0144_CAMERA_SENSOR_FRAME_HEIGHT == 400) && (RTE_AR0144_CAMERA_SENSOR_FRAME_WIDTH == 640)
#elif (RTE_AR0144_CAMERA_SENSOR_FRAME_HEIGHT == 200) && (RTE_AR0144_CAMERA_SENSOR_FRAME_WIDTH == 320)
#elif (RTE_AR0144_CAMERA_SENSOR_FRAME_HEIGHT == 100) && (RTE_AR0144_CAMERA_SENSOR_FRAME_WIDTH == 160)
#else
#error Unsupported resolution
#endif
int32_t AR0144_SetResolution(uint32_t width)
{
    uint32_t skip = 1280 / width;
    if (1280 - (skip * width)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    int32_t ret = ARM_DRIVER_OK;
    uint32_t reg_data = 0;

    if (skip == 1 || skip == 2 || skip == 4 || skip == 8 || skip == 16) {
        ret = AR0144_READ_REG(0x30B0, &reg_data, 2);
        if (ret) { return ret; }
        if (skip == 1) { // native resolution
            reg_data &= ~(1 << 7);
        } else {
            reg_data |= (1 << 7); // Set this for monochrome binning
        }
        ret = AR0144_WRITE_REG(0x30B0, reg_data, 2);
        if (ret) { return ret; }

        // row binning + col binning
        ret = AR0144_READ_REG(0x3040, &reg_data, 2);
        if (ret) { return ret; }
        if (skip == 1) {
            // Clear binning in native resolution
            reg_data &= ~((1 << 13) | (1 << 12) | (1 << 5));
        } else {
            reg_data |= (1 << 13) | (1 << 12) | (1 << 5);
        }
        ret = AR0144_WRITE_REG(0x3040, reg_data, 2);
        if (ret) { return ret; }

        // skipping
        reg_data = ((skip - 1) << 1) | 0x01;
        ret = AR0144_WRITE_REG(0x30A2, reg_data, 2); // X
        if (ret) { return ret; }
        ret = AR0144_WRITE_REG(0x30A6, reg_data, 2); // Y
        if (ret) { return ret; }

        // Set lower coarse integration time when binning (due to summing)
        reg_data = (skip == 1) ? 0x00C0 : 0x0030;
        ret = AR0144_WRITE_REG(AR0144_COARSE_INTEGRATION_TIME_REGISTER, reg_data, 2);
        if (ret) { return ret; }
        max_integration_time = current_integration_time = 0;
        return ARM_DRIVER_OK;
    } else {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

/**
  \fn           int32_t AR0144_Control(uint32_t control, uint32_t arg)
  \brief        Control AR0144 Camera Sensor.
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
  */
int32_t AR0144_Control(uint32_t control, uint32_t arg)
{

    switch (control)
    {
        case CPI_CAMERA_SENSOR_CONFIGURE:
            return AR0144_SetResolution(RTE_AR0144_CAMERA_SENSOR_FRAME_WIDTH);
        case CPI_CAMERA_SENSOR_GAIN:
            return AR0144_Camera_Gain(arg);
        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
}

/**
  \fn           int32_t AR0144_Uninit(void)
  \brief        Un-initialize AR0144 Camera Sensor.
  \param[in]    none
  \return       \ref execution_status
  */
int32_t AR0144_Uninit(void)
{

    /*Disable camera sensor clock source config*/
    AR0144_Sensor_Disable_Clk_Src();

    return ARM_DRIVER_OK;
}

/**
\brief AR0144 Camera Sensor CSi informations
\ref CSI_INFO
*/
static CSI_INFO ar0144_csi_info =
{
    .frequency                = RTE_AR0144_CAMERA_SENSOR_CSI_FREQ,
    .dt                       = RTE_AR0144_CAMERA_SENSOR_CSI_DATA_TYPE,
    .n_lanes                  = RTE_AR0144_CAMERA_SENSOR_CSI_N_LANES,
    .vc_id                    = RTE_AR0144_CAMERA_SENSOR_CSI_VC_ID,
    .cpi_cfg.override         = RTE_AR0144_CAMERA_SENSOR_OVERRIDE_CPI_COLOR_MODE,
    .cpi_cfg.cpi_color_mode   = RTE_AR0144_CAMERA_SENSOR_CPI_COLOR_MODE
};

/**
\brief AR0144 Camera Sensor Operations
\ref CAMERA_SENSOR_OPERATIONS
*/
static CAMERA_SENSOR_OPERATIONS ar0144_ops =
{
    .Init    = AR0144_Init,
    .Uninit  = AR0144_Uninit,
    .Start   = AR0144_Start,
    .Stop    = AR0144_Stop,
    .Control = AR0144_Control,
};

/**
\brief AR0144 Camera Sensor Device Structure
\ref CAMERA_SENSOR_DEVICE
*/
static CAMERA_SENSOR_DEVICE ar0144_camera_sensor =
{
    .interface  = CAMERA_SENSOR_INTERFACE_MIPI,
    .width      = RTE_AR0144_CAMERA_SENSOR_FRAME_WIDTH,
    .height     = RTE_AR0144_CAMERA_SENSOR_FRAME_HEIGHT,
    .csi_info   = &ar0144_csi_info,
    .ops        = &ar0144_ops,
};

/* Registering CPI sensor */
CAMERA_SENSOR(ar0144_camera_sensor)

#endif /* RTE_AR0144_CAMERA_SENSOR_ENABLE */
