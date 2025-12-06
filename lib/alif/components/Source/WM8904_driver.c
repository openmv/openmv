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
 * @file     WM8904_driver.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     19-Nov-2024
 * @brief    driver for WM8904 Codec Device.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include "WM8904_driver.h"
#include "WM8904_Private.h"
#include "WM8904_codec_i2c.h"
#include "Driver_I2C.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#if (defined(RTE_WM8904_CODEC) && defined(RTE_Driver_WM8904))

static uint8_t volatile drv_state = 0;

extern ARM_DRIVER_I2C ARM_Driver_I2C_(RTE_WM8904_CODEC_I2C_INSTANCE);

WM8904_CODEC_SLAVE_I2C_CONFIG i2c_cnfg =
{
    .drv_i2c                        = &ARM_Driver_I2C_(RTE_WM8904_CODEC_I2C_INSTANCE),
    .bus_speed                      = ARM_I2C_BUS_SPEED_STANDARD,
    .wm8904_codec_slave_addr        = WM8904_SLAVE
};

/**
  \fn        int32_t WM8904_UpdateVolume(uint8_t volume)
  \brief     update volume for codec
  \param[in] volume level from of 0 to 100
  \return    execution_status
*/
static int32_t WM8904_UpdateVolume(uint8_t volume)
{
    uint16_t reg_val;

    reg_val = (uint16_t) VOLUME_OUT_CONVERT(volume);

    /* set volume for left headphone */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_ANALOG_OUT1_LEFT, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    /* volume change synchronization for both right and left */
    reg_val |= 0x80U;

    /* set volume for right headphone */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_ANALOG_OUT1_RIGHT, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn        int32_t WM8904_Config(void)
  \brief     configure codec
  \param[in] none
  \return    execution_status
*/
static int32_t WM8904_Config(void)
{
    uint16_t reg_val;

    /* soft reset */
    reg_val = 0x0U;
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_SW_RESET_ID, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    reg_val = 0x000F; /* MCLK_INV=0, SYSCLK_SRC=0, TOCLK_RATE=0, OPCLK_ENA=1,
                       * CLK_SYS_ENA=1, CLK_DSP_ENA=1, TOCLK_ENA=1 */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_CLOCK_RATES2, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    reg_val = 0x0100U; /* WSEQ_ENA=1, WSEQ_WRITE_INDEX=0_0000 */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_WRITE_SEQUENCER0, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    reg_val = 0x0100U; /* WSEQ_START=1, WSEQ_WRITE_INDEX=0_0000 */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_WRITE_SEQUENCER3, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    /* wait till sequencer busy flag is cleared */
    do {
        if (WM8904_codec_i2c_read(&i2c_cnfg, WM8904_WRITE_SEQUENCER4, &reg_val) != ARM_DRIVER_OK) {
            return ARM_DRIVER_ERROR;
        }
    } while (((reg_val & 1U) != 0U));

    reg_val = 0x0050U;  /* AIFADCR_SRC = AIFDACR_SRC = 1 */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_AUDIO_INTERFACE0, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    reg_val = 0x0040U;  /* DAC_OSR128 = 1 */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_DAC_DIGITAL1, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    reg_val = 0x0001U; /* CP_DYN_PWR = 1 */
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_CLASS_W0, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    reg_val = 0x0002U; /* Set protocol to I2S*/
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_AUDIO_INTERFACE1, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    /* update voulme to default value */
    return WM8904_UpdateVolume((uint8_t)DEFAULT_VOLUME_VALUE);
}

/**
  \fn      int32_t ARM_WM8904_Initialize(void)
  \brief   Initialize the WM8904 codec
  \return  execution_status
*/
static int32_t ARM_WM8904_Initialize(void)
{
    int32_t ret;
    uint16_t reg_id;

    ret = WM8904_codec_i2c_init(&i2c_cnfg);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    /* check for device ID */
    if (WM8904_codec_i2c_read(&i2c_cnfg, WM8904_SW_RESET_ID, &reg_id) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    if (reg_id != WM8904_DEV_ID) {
        return ARM_DRIVER_ERROR;
    }

    drv_state |= WM8904_DRIVER_INITIALIZED;

    return ARM_DRIVER_OK;
}

/**
  \fn      int32_t ARM_WM8904_Uninitialize(void)
  \brief   Un-initialize the WM8904 codec
  \return  execution_status
*/
static int32_t ARM_WM8904_Uninitialize(void)
{
    int32_t ret;

    ret = WM8904_codec_i2c_uninit(&i2c_cnfg);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    drv_state &= ~WM8904_DRIVER_INITIALIZED;

    return ARM_DRIVER_OK;
}

/**
  \fn      int32_t ARM_WM8904_PowerControl(ARM_POWER_STATE state).
  \brief   Handles the WM8904 codec power.
  \param   state  : power state.
  \return  execution_status
*/
static int32_t ARM_WM8904_PowerControl(ARM_POWER_STATE state)
{
    uint16_t reg_val;

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            if (drv_state & WM8904_DRIVER_POWERED) {
                return ARM_DRIVER_OK;
            }

            /* Reset registers */
            reg_val = 0x0U;
            if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_SW_RESET_ID, reg_val) != ARM_DRIVER_OK) {
                return ARM_DRIVER_ERROR;
            }

            drv_state &= ~WM8904_DRIVER_POWERED;
            break;
        }
        case ARM_POWER_FULL:
        {
            if (!(drv_state & WM8904_DRIVER_INITIALIZED)) {
                return ARM_DRIVER_ERROR;
            }

            if(WM8904_Config() != ARM_DRIVER_OK) {
                return ARM_DRIVER_ERROR;
            }

            drv_state |= WM8904_DRIVER_POWERED;
            break;
        }
        case ARM_POWER_LOW:
        default:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }
    return ARM_DRIVER_OK;
}

/**
  \fn      int32_t ARM_WM8904_Mute(void)
  \brief   enable codec mute feature
  \return  execution_status
*/
static int32_t ARM_WM8904_Mute(void)
{
    uint16_t reg_val;

    if (!(drv_state & WM8904_DRIVER_POWERED)) {
        return ARM_DRIVER_ERROR;
    }

    if (WM8904_codec_i2c_read(&i2c_cnfg, WM8904_DAC_DIGITAL1, &reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    /* mute DAC */
    reg_val |= 0x0008U;
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_DAC_DIGITAL1, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn      int32_t ARM_WM8904_UnMute(void)
  \brief   enable codec un-mute feature
  \return  execution_status
*/
static int32_t ARM_WM8904_UnMute(void)
{
    uint16_t reg_val;

    if (!(drv_state & WM8904_DRIVER_POWERED)) {
        return ARM_DRIVER_ERROR;
    }

    if (WM8904_codec_i2c_read(&i2c_cnfg, WM8904_DAC_DIGITAL1, &reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    /* un-mute DAC */
    reg_val &= ~0x0008U;
    if (WM8904_codec_i2c_write(&i2c_cnfg, WM8904_DAC_DIGITAL1, reg_val) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn        int32_t ARM_WM8904_SetVolume(uint8_t volume)
  \brief     set volume for codec
  \param[in] volume value in range of 0 to 100
  \return    execution_status
*/
static int32_t ARM_WM8904_SetVolume(uint8_t volume)
{
    if (!(drv_state & WM8904_DRIVER_POWERED)) {
        return ARM_DRIVER_ERROR;
    }

    return WM8904_UpdateVolume(volume);
}

ARM_DRIVER_WM8904 WM8904 =
{
    ARM_WM8904_Initialize,
    ARM_WM8904_Uninitialize,
    ARM_WM8904_PowerControl,
    ARM_WM8904_Mute,
    ARM_WM8904_UnMute,
    ARM_WM8904_SetVolume
};

#endif /* (defined(RTE_WM8904_CODEC) && defined(RTE_Driver_WM8904)) */
/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
