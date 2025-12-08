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
 * @file     WM8904_driver.h
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     19-Nov-2024
 * @brief    The driver initializes the WM8904 codec for stereo audio mode with
 *           a 48KHz sample rate on the headphone outputs (HPOUTL and HPOUTR).
 *           It also provides options for mute, unmute, and volume control.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef WM8904_DRIVER_H_
#define WM8904_DRIVER_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* project includes*/
#include "Driver_Common.h"

/* Default volume level for Headphone output
 * Range: 0-100 */
#define DEFAULT_VOLUME_VALUE   60U


/* Function description */
/**
  \fn      int32_t ARM_WM8904_Initialize(void)
  \brief   Initialize the WM8904 codec
  \return  execution_status

  \fn      int32_t ARM_WM8904_Uninitialize(void)
  \brief   Un-initialize the WM8904 codec
  \return  execution_status

  \fn      int32_t ARM_WM8904_PowerControl(ARM_POWER_STATE state)
  \brief   Handles the WM8904 codec power
  \param   state  : power state
  \return  execution_status

  \fn      int32_t ARM_WM8904_Mute(void)
  \brief   enable codec mute feature
  \return  execution_status

  \fn      int32_t ARM_WM8904_UnMute(void)
  \brief   enable codec un-mute feature
  \return  execution_status

  \fn        int32_t ARM_WM8904_SetVolume(uint8_t volume)
  \brief     set volume for codec
  \param[in] volume value in range of 0 to 100
  \return    execution_status
*/

typedef struct _ARM_DRIVER_WM8904 {
    int32_t         (*Initialize)      (void);                   ///< Pointer to \ref ARM_WM8904_Initialize
    int32_t         (*Uninitialize)    (void);                   ///< Pointer to \ref ARM_WM8904_Uninitialize
    int32_t         (*PowerControl)    (ARM_POWER_STATE state);  ///< Pointer to \ref ARM_WM8904_PowerControl
    int32_t         (*Mute)            (void);                   ///< Pointer to \ref ARM_WM8904_Mute
    int32_t         (*UnMute)          (void);                   ///< Pointer to \ref ARM_WM8904_Unmute
    int32_t         (*SetVolume)       (uint8_t volume);         ///< Pointer to \ref ARM_WM8904_SetVolume
} const ARM_DRIVER_WM8904;

#ifdef  __cplusplus
}
#endif

#endif /* WM8904_CODEC_I2C_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
