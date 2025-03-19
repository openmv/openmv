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
 * @file     Driver_CMP.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     20-June-2022
 * @brief    Analog Comparator driver definitions.
 ******************************************************************************/

#ifndef DRIVER_CMP_H_
#define DRIVER_CMP_H_

#include"Driver_Common.h"

#ifdef _cplusplus
extern "c"
{
#endif

#define ARM_CMP_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/* Control code for Analog Comparator */
#define ARM_CMP_POLARITY_CONTROL               (0X01UL)  /* Used to invert the comparator input signal before processing */
#define ARM_CMP_WINDOW_CONTROL_ENABLE          (0X02UL)  /* Window control used to define when to look at the comparator input */
#define ARM_CMP_WINDOW_CONTROL_DISABLE         (0X03UL)  /* Disable the window control */
#define ARM_CMP_FILTER_CONTROL                 (0X04UL)  /* Used to define how many times the comparator input must be sampled */
#define ARM_CMP_PRESCALER_CONTROL              (0X05UL)  /* comparator input will be sampled for every prescaler value */

/* Comparator event */
#define ARM_CMP_FILTER_EVENT_OCCURRED          (0x01UL)  /* Filter event occurred */

/* CMP window control macros */
#define ARM_CMP_WINDOW_CONTROL_SRC_0           (0x0UL)   /* ARM CMP Window control SOURCE 0 */
#define ARM_CMP_WINDOW_CONTROL_SRC_1           (0x01UL)  /* ARM CMP Window control SOURCE 1 */
#define ARM_CMP_WINDOW_CONTROL_SRC_2           (0x02UL)  /* ARM CMP Window control SOURCE 2 */
#define ARM_CMP_WINDOW_CONTROL_SRC_3           (0x03UL)  /* ARM CMP Window control SOURCE 3 */

/* Function documentation */
/**
  @fn            ARM_DRIVER_VERSION GetVersion (void)
  @brief         Get CMP driver version.
  @return        @ref CMP_DRIVER_VERSION

  @fn            ARM_CMP_CAPABILITIES GetCapabilities (void)
  @brief         Get CMP driver capabilities
  @return        @ref CMP_CAPABILITIES

  @func          int32_t CMP_Initialize (ARM_CMP_SignalEvent_t cb_event)
  @brief         Initialize CMP interface
  @parameter[1]  cb_event : Pointer Comparator Event \ref ARM_CMP_Signal_Event
  @return        \ref execution_status

  @fn            int32_t CMP_Uninitialize (void)
  @brief         UnInitialize the CMP interface
  @param[in]     None
  @return        \ref execution_status

  @fn            int32_t CMP_PowerControl (ARM_POWER_STATE state)
  @brief         CMSIS-DRIVER CMP power control
  @param[in]     state : Power state
  @param[in]     CMP   : Pointer to CMP_resources_t
  @return        \ref execution_status

  @fn            int32_t CMP_Control (uint32_t control,uint32_t arg)
  @brief         CMSIS-Driver CMP control
  @param[in]     control : Operation \ref Driver_Comparator.h :Comparator control codes
  @param[in]     arg     : Argument of operation (optional)
  @param[in]     cmp     : Pointer to CMP_resources_t
  @return        \ref execution_status

  @fn            int32_t CMP_Start (void)
  @brief         Start the Analog comparator Interface
  @param[in]     None
  @return        \ref execution_status

  @fn            int32_t CMP_Stop (void)
  @brief         Stop the Analog Comparator Interface
  @param[in]     None
  @return        \ref execution_status
*/

typedef void (*ARM_Comparator_SignalEvent_t) (uint32_t event);  /*Pointer to \ref Comparator_SignalEvent : Signal Comparator Event*/

typedef struct _ARM_COMPARATOR_CAPABILITIES{
    uint32_t Polarity_invert     :1;  /* Ability to invert the input signal */
    uint32_t Windowing           :1;  /* Used to define when to look at the comparator input */
    uint32_t Filter_Control      :1;  /* Supports Filter function */
    uint32_t Prescaler           :1;  /* Supports Prescaler function */
    uint32_t Reserved            :28; /* Reserved */
}ARM_COMPARATOR_CAPABILITIES;

/**
 @brief  Access Structure of Analog Comparator Driver
*/
typedef struct ARM_DRIVER_COMPARATOR{
    ARM_DRIVER_VERSION            (*GetVersion)        (void);                                   /* pointer is pointing to CMP_GetVersion : used to get the driver version */
    ARM_COMPARATOR_CAPABILITIES   (*GetCapabilities)   (void);                                   /* pointer is pointing to CMP_Capabilities : used to get the driver capabilities */
    int32_t                       (*Initialize)        (ARM_Comparator_SignalEvent_t cb_event);  /* Pointer pointing to \ref CMP_intialize */
    int32_t                       (*Uninitialize)      (void);                                   /* Pointer to CMP_Uninitialize : Un-initialize comparator Interface */
    int32_t                       (*PowerControl)      (ARM_POWER_STATE state);                  /* Pointer to CMP_PowerControl : Control Comparator Interface Power */
    int32_t                       (*Control)           (uint32_t control, uint32_t arg);         /* Pointer to CMP_Control : Control Comparator Interface */
    int32_t                       (*Start)             (void);                                   /* Pointer to start Comparator */
    int32_t                       (*Stop)              (void);                                   /* Pointer to Stop the Comparator */
}const ARM_DRIVER_CMP;


#endif /* DRIVER_CMP_H_ */
