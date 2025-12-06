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
 * @file     WM8904_Private.h
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     19-Nov-2024
 * @brief    header file for WM8904 driver file.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef WM8904_PRIVATE_H_
#define WM8904_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* slave id and address for WM8904 */
#define WM8904_SLAVE                 0x1A
#define WM8904_DEV_ID                0x8904

/* SetVolume converter */
#define VOLUME_OUT_CONVERT(vol)      (vol >= 100) ? (63U) : (uint8_t)((vol * 63U) / 100U)

/* WM8904 driver state */
#define WM8904_DRIVER_INITIALIZED    (1U << 0)
#define WM8904_DRIVER_POWERED        (1U << 1)

/* Register Mapping */
#define WM8904_SW_RESET_ID            0x00U
#define WM8904_BIAS_CONTROL0          0x04U
#define WM8904_VMID_CONTROL0          0x05U
#define WM8904_MIC_BIAS_CONTROL0      0x06U
#define WM8904_MIC_BIAS_CONTROL1      0x07U
#define WM8904_ANALOG_ADC0            0x0AU
#define WM8904_PWR_MANAGEMENT0        0x0CU
#define WM8904_PWR_MANAGEMENT2        0x0EU
#define WM8904_PWR_MANAGEMENT3        0x0FU
#define WM8904_PWR_MANAGEMENT6        0x12U
#define WM8904_CLOCK_RATES0           0x14U
#define WM8904_CLOCK_RATES1           0x15U
#define WM8904_CLOCK_RATES2           0x16U
#define WM8904_AUDIO_INTERFACE0       0x18U
#define WM8904_AUDIO_INTERFACE1       0x19U
#define WM8904_AUDIO_INTERFACE2       0x1AU
#define WM8904_AUDIO_INTERFACE3       0x1BU
#define WM8904_DAC_DIGITAL_VOL_LEFT   0x1EU
#define WM8904_DAC_DIGITAL_VOL_RIGHT  0x1FU
#define WM8904_DAC_DIGITAL0           0x20U
#define WM8904_DAC_DIGITAL1           0x21U
#define WM8904_ADC_DIGITAL_VOL_LEFT   0x24U
#define WM8904_ADC_DIGITAL_VOL_RIGHT  0x25U
#define WM8904_ADC_DIGITAL0           0x26U
#define WM8904_DIGITAL_MICROPHONE0    0x27U
#define WM8904_DRC0                   0x28U
#define WM8904_DRC1                   0x29U
#define WM8904_DRC2                   0x2AU
#define WM8904_DRC3                   0x2BU
#define WM8904_ANALOG_LEFT_INPUT0     0x2CU
#define WM8904_ANALOG_RIGHT_INPUT0    0x2DU
#define WM8904_ANALOG_LEFT_INPUT1     0x2EU
#define WM8904_ANALOG_RIGHT_INPUT1    0x2FU
#define WM8904_ANALOG_OUT1_LEFT       0x39U
#define WM8904_ANALOG_OUT1_RIGHT      0x3AU
#define WM8904_ANALOG_OUT2_LEFT       0x3BU
#define WM8904_ANALOG_OUT2_RIGHT      0x3CU
#define WM8904_ANALOG_OUT12_ZC        0x3DU
#define WM8904_DC_SERVO0              0x43U
#define WM8904_DC_SERVO1              0x44U
#define WM8904_DC_SERVO2              0x45U
#define WM8904_DC_SERVO4              0x47U
#define WM8904_DC_SERVO5              0x48U
#define WM8904_DC_SERVO6              0x49U
#define WM8904_DC_SERVO7              0x4AU
#define WM8904_DC_SERVO8              0x4BU
#define WM8904_DC_SERVO9              0x4CU
#define WM8904_DC_SERVO_READBACK0     0x4DU
#define WM8904_ANALOG_HP0             0x5AU
#define WM8904_ANALOG_LINEOUT0        0x5EU
#define WM8904_CHARGE_PUMP0           0x62U
#define WM8904_CLASS_W0               0x68U
#define WM8904_WRITE_SEQUENCER0       0x6CU
#define WM8904_WRITE_SEQUENCER1       0x6DU
#define WM8904_WRITE_SEQUENCER2       0x6EU
#define WM8904_WRITE_SEQUENCER3       0x6FU
#define WM8904_WRITE_SEQUENCER4       0x70U
#define WM8904_FLL_CONTROL1           0x74U
#define WM8904_FLL_CONTROL2           0x75U
#define WM8904_FLL_CONTROL3           0x76U
#define WM8904_FLL_CONTROL4           0x77U
#define WM8904_FLL_CONTROL5           0x78U
#define WM8904_GPIO_CONTROL1          0x79U
#define WM8904_GPIO_CONTROL2          0x7AU
#define WM8904_GPIO_CONTROL3          0x7BU
#define WM8904_GPIO_CONTROL4          0x7CU
#define WM8904_DIGITAL_PULLS          0x7EU
#define WM8904_INTERRUPT_STATUS       0x7FU
#define WM8904_INTERRUPT_STATUS_MASK  0x80U
#define WM8904_INTERRUPT_POLARITY     0x81U
#define WM8904_INTERRUPT_DEBOUNCE     0x82U
#define WM8904_EQ1                    0x86U
#define WM8904_EQ2                    0x87U
#define WM8904_EQ3                    0x88U
#define WM8904_EQ4                    0x89U
#define WM8904_EQ5                    0x8AU
#define WM8904_EQ6                    0x8BU
#define WM8904_EQ7                    0x8CU
#define WM8904_EQ8                    0x8DU
#define WM8904_EQ9                    0x8EU
#define WM8904_EQ10                   0x8FU
#define WM8904_EQ11                   0x90U
#define WM8904_EQ12                   0x91U
#define WM8904_EQ13                   0x92U
#define WM8904_EQ14                   0x93U
#define WM8904_EQ15                   0x94U
#define WM8904_EQ16                   0x95U
#define WM8904_EQ17                   0x96U
#define WM8904_EQ18                   0x97U
#define WM8904_EQ19                   0x98U
#define WM8904_EQ20                   0x99U
#define WM8904_EQ21                   0x9AU
#define WM8904_EQ22                   0x9BU
#define WM8904_EQ23                   0x9CU
#define WM8904_EQ24                   0x9DU
#define WM8904_ADC_TEST0              0xC6U
#define WM8904_FLL_NCO_TEST0          0xF7U
#define WM8904_FLL_NCO_TEST1          0xF8U

/**
  \fn        int32_t WM8904_UpdateVolume(uint8_t volume)
  \brief     set volume for codec
  \param[in] volume value in range of 0 to 100
  \return    execution_status
*/
static int32_t WM8904_UpdateVolume(uint8_t volume);

/**
  \fn        int32_t WM8904_Config(void)
  \brief     configure codec
  \param[in] none
  \return    execution_status
*/
static int32_t WM8904_Config(void);

#ifdef  __cplusplus
}
#endif

#endif /* WM8904_PRIVATE_H_ */
