/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     Driver_DAC.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     22-Feb-2022
 * @brief    DAC(Digital to Analog Converter) driver definitions.
 ******************************************************************************/

#ifndef DRIVER_DAC_H
#define DRIVER_DAC_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_DAC_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/* DAC Control Code */
#define ARM_DAC_RESET                         (0x01UL)  /* To reset the DAC               */
#define ARM_DAC_INPUT_BYPASS_MODE             (0x02UL)  /* Pass input through bypass mode */
#define ARM_DAC_CAPACITANCE_HP_MODE           (0x03UL)  /* Set DAC capacitance            */
#define ARM_DAC_SELECT_IBIAS_OUTPUT           (0x04UL)  /* Set DAC output current         */

/* Select the DAC output current */
#define ARM_DAC_0UA_OUT_CUR         (0x0UL)  /* Set DAC output current to 0 micro amps    */
#define ARM_DAC_100UA_OUT_CUR       (0x1UL)  /* Set DAC output current to 100 micro amps  */
#define ARM_DAC_200UA_OUT_CUR       (0x2UL)  /* Set DAC output current to 200 micro amps  */
#define ARM_DAC_300UA_OUT_CUR       (0x3UL)  /* Set DAC output current to 300 micro amps  */
#define ARM_DAC_400UA_OUT_CUR       (0x4UL)  /* Set DAC output current to 400 micro amps  */
#define ARM_DAC_500UA_OUT_CUR       (0x5UL)  /* Set DAC output current to 500 micro amps  */
#define ARM_DAC_600UA_OUT_CUR       (0x6UL)  /* Set DAC output current to 600 micro amps  */
#define ARM_DAC_700UA_OUT_CUR       (0x7UL)  /* Set DAC output current to 700 micro amps  */
#define ARM_DAC_800UA_OUT_CUR       (0x8UL)  /* Set DAC output current to 800 micro amps  */
#define ARM_DAC_900UA_OUT_CUR       (0x9UL)  /* Set DAC output current to 900 micro amps  */
#define ARM_DAC_1000UA_OUT_CUR      (0xAUL)  /* Set DAC output current to 1000 micro amps */
#define ARM_DAC_1100UA_OUT_CUR      (0xBUL)  /* Set DAC output current to 1100 micro amps */
#define ARM_DAC_1200UA_OUT_CUR      (0xCUL)  /* Set DAC output current to 1200 micro amps */
#define ARM_DAC_1300UA_OUT_CUR      (0xDUL)  /* Set DAC output current to 1300 micro amps */
#define ARM_DAC_1400UA_OUT_CUR      (0xEUL)  /* Set DAC output current to 1400 micro amps */
#define ARM_DAC_1500UA_OUT_CUR      (0xFUL)  /* Set DAC output current to 1500 micro amps */

/* Select the DAC capacitance */
#define ARM_DAC_2PF_CAPACITANCE     (0x0UL)  /* Set DAC capacitance to 2PF */
#define ARM_DAC_4PF_CAPACITANCE     (0x1UL)  /* Set DAC capacitance to 4PF */
#define ARM_DAC_6PF_CAPACITANCE     (0x2UL)  /* Set DAC capacitance to 6PF */
#define ARM_DAC_8PF_CAPACITANCE     (0x3UL)  /* Set DAC capacitance to 8PF */

/* Function documentation */
/**
  @fn          ARM_DRIVER_VERSION GetVersion (void)
  @brief       Get DAC driver version.
  @return      @ref DAC_DRIVER_VERSION

  @fn          ARM_DAC_CAPABILITIES GetCapabilities (void)
  @brief       Get DAC driver capabilities
  @return      @ref DAC_CAPABILITIES

  @fn          int32_t DAC_Initialize (DAC_DRV_INFO      *dac)
  @brief       Initialize the DAC interface
  @param[in]   dac : Pointer to dac resources
  @return      \ref execution_status

  @fn          int32_t DAC_Uninitialize (DAC_DRV_INFO *dac)
  @brief       De-Initialize the DAC interface
  @param[in]   dac : Pointer to dac resources
  @return      \ref execution_status

  @fn          int32_t DAC_PowerControl (ARM_POWER_STATE   state)
  @brief       CMSIS-DRIVER DAC power control
  @param[in]   state : Power state
  @param[in]   DAC   : Pointer to DAC resources
  @return      \ref execution_status

  @fn          int32_t DAC_Control (uint32_t control,uint32_t arg,DAC_resources_t *dac)
  @brief       CMSIS-Driver dac control
  @param[in]   control : Operation \ref Driver_DAC.h : DAC control codes
  @param[in]   arg     : Argument of operation (optional)
  @param[in]   dac     : Pointer to dac resources
  @return      common \ref execution_status

  @fn          DAC_Start (DAC_resources_t *dac)
  @brief       CMSIS-Driver DAC Start
  @param[in]   dac  : Pointer to dac resources
  @return      \ref execution_status

  @fn          DAC_Stop (DAC_resources_t *dac)
  @brief       CMSIS-Driver DAC Stop
  @param[in]   dac : Pointer to dac resources
  @return      \ref execution_status

  @fn          DAC_SetInput(uint32_t num, DAC_resources_t *dac)
  @brief       CMSIS-Driver to set the DAC input and output will be in GPIO pins
  @param[in]   Input : Operation
  @param[in]   value : DAC input
  @param[in]   dac   : Pointer to dac device resources
  @return      \ref execution_status
 */

/**
@brief DAC Device Driver Capabilities.
*/
typedef struct ARM_DAC_CAPABILITIES {
    uint32_t resolution : 1;   /* DAC Resolution */
    uint32_t reserved   : 31;  /* Reserved (must be Zero) */
}ARM_DAC_CAPABILITIES;

/**
@brief Access structure of the DAC Driver.
*/
typedef struct ARM_Driver_DAC
{
    ARM_DRIVER_VERSION    (*GetVersion)      (void);                           /* pointer is pointing to DAC_GetVersion : used to get the driver version        */
    ARM_DAC_CAPABILITIES  (*GetCapabilities) (void);                           /* pointer is pointing to DAC_Capabilities : used to get the driver capabilities */
    int32_t               (*Initialize)      (void);                           /* Initialize the DAC Interface                                                  */
    int32_t               (*Uninitialize)    (void);                           /* Pointer to DAC_Uninitialize : De-initialize DAC Interface                     */
    int32_t               (*PowerControl)    (ARM_POWER_STATE state);          /* Pointer to DAC_PowerControl : Control DAC Interface Power                     */
    int32_t               (*Control)         (uint32_t control, uint32_t arg); /* Pointer to DAC_Control : Control DAC Interface                                */
    int32_t               (*Start)           (void);                           /* Pointer to start DAC                                                          */
    int32_t               (*Stop)            (void);                           /* Pointer to stop the DAC                                                       */
    int32_t               (*SetInput)        (uint32_t value);                 /* Pointer to Set_input                                                          */
}const ARM_DRIVER_DAC;

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_DAC_H_ */
