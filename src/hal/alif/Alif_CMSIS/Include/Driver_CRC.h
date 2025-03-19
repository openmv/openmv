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
 * @file     Driver_CRC.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     18-April-2022
 * @brief    CRC(Cyclic Redundancy Check) driver definitions.
 ******************************************************************************/

#ifndef DRIVER_CRC_H
#define DRIVER_CRC_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_CRC_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

#define ARM_CRC_COMPUTE_EVENT_DONE     (1 << 0)   /* ARM CRC COMPUTE EVENT DONE */

#define ARM_CRC_CONTROL_POS             0
#define ARM_CRC_CONTROL_MASK           (0x1F << ARM_CRC_CONTROL_POS)    /* To select the Reflect, Invert, Bit, Byte, Custom polynomial bit of CRC */

#define ARM_CRC_ENABLE_BYTE_SWAP       (0x1UL  << 0)   /* Byte swap CRC Enale = 1 and Disable =0      */
#define ARM_CRC_ENABLE_BIT_SWAP        (0x1UL  << 1)   /* Bit swap CRC Enable = 1 and Disable = 0     */
#define ARM_CRC_ENABLE_CUSTOM_POLY     (0x1UL  << 2)   /* Custom poly CRC Enable = 1 and Disable = 0  */
#define ARM_CRC_ENABLE_INVERT_OUTPUT   (0x1UL  << 3)   /* Invert CRC Enable = 1 and Disable = 0       */
#define ARM_CRC_ENABLE_REFLECT_OUTPUT  (0x1UL  << 4)   /* Reflect CRC Enable = 1 and Disable = 0      */

/* CRC algorithm select */
#define ARM_CRC_ALGORITHM_SEL          (0x01UL << 5)   /* To select the below particular algorithm */

#define ARM_CRC_ALGORITHM_SEL_8_BIT_CCITT                (0x01UL)   /* To select CRC_8_CCITT algorithm give value as 0 */
#define ARM_CRC_ALGORITHM_SEL_16_BIT                     (0x02UL)   /* To select CRC_16 algorithm give value as 2 */
#define ARM_CRC_ALGORITHM_SEL_16_BIT_CCITT               (0x03UL)   /* To select CRC_16_CCITT algorithm give value as 3 */
#define ARM_CRC_ALGORITHM_SEL_32_BIT                     (0x04UL)   /* To select CRC_32 algorithm give value as 4 */
#define ARM_CRC_ALGORITHM_SEL_32_BIT_CUSTOM_POLY         (0x05UL)   /* To select CRC_32C algorithm give value as 5 */

/* Function documentation */
/**
  @fn          ARM_DRIVER_VERSION GetVersion (void)
  @brief       Get CRC driver version.
  @return      @ref CRC_DRIVER_VERSION

  @fn          ARM_CRC_CAPABILITIES GetCapabilities (void)
  @brief       Get CRC driver capabilities
  @return      @ref CRC_CAPABILITIES

  @fn          int32_t CRC_Initialize (ARM_CRC_SignalEvent_t cb_event)
  @brief       Initialize the CRC interface
  @param[in]   crc : Pointer to CRC resources
  @return      \ref execution_status

  @fn          int32_t CRC_Uninitialize (CRC_resources_t *crc)
  @brief       UnInitialize the CRC interface
  @param[in]   crc : Pointer to CRC resources
  @return      \ref execution_status

  @fn          int32_t CRC_PowerControl (ARM_POWER_STATE state)
  @brief       CMSIS-DRIVER CRC power control
  @param[in]   state : Power state
  @param[in]   crc   : Pointer to CRC resources
  @return      \ref execution_status

  @fn          int32_t CRC_Control (uint32_t control,uint32_t arg,CRC_resources_t *crc)
  @brief       CMSIS-Driver CRC control
  @param[in]   control : Operation \ref Driver_CRC.h :CRC control codes
  @param[in]   arg     : Argument of operation (optional)
  @param[in]   crc     : Pointer to CRC resources
  @return      common \ref execution_status

  @fn         int32_t CRC_Seed (uint32_t value ,CRC_resources_t *crc)
  @brief      CMSIS-DRIVER CRC Seed value
  @param[in]  seed_value : Seed value depending on whether the data is 8 bit or 16 or 32 bit
  @param[in]  crc        : pointer to CRC resources
  @return     \ref execution_status

  @fn         int32_t CRC_PolyCustom (uint32_t polynomial,CRC_resources_t *crc)
  @brief      CMSIS-DRIVER CRC custom polynomial
  @param[in]  polynomial : Polynomial data for 8 bit or 16 or 32 bit
  @param[in]  crc        : pointer to CRC resources
  @return     \ref execution_status

  @fn         int32_t CRC_Compute (void *data_in,uint32_t len,uint32_t *data_out,CRC_resources_t *crc)
  @brief      CMSIS-DRIVER CRC Input data and getting the output
  @param[in]  data_in   : 8 bit or 16 bit or 32 bit input data
  @param[in]  len    : length of the data
  @param[in]  data_out  : to get the output data of 8 bit or 16 bit or 32 bit output data
  @param[in]  crc    : pointer to CRC resources
  @return     \ref execution_status
*/

/**
 @brief CRC Device Driver Capabilities.
*/
typedef struct ARM_CRC_CAPABILITIES {
    uint32_t CRC_8_CCITT_ALGORITHM    : 1;   /* Supports CRC_8_CCITT */
    uint32_t CRC_16_ALGORITHM         : 1;   /* Supports CRC_16 */
    uint32_t CRC_16_CCITT_ALGORITHM   : 1;   /* Supports CRC_16_CCITT */
    uint32_t CRC_32_ALGORITHM         : 1;   /* Supports CRC_32 */
    uint32_t CRC_32C_ALGORITHM        : 1;   /* Supports CRC_32C */
    uint32_t reserved                 : 27;  /* Reserved (must be Zero) */
}ARM_CRC_CAPABILITIES;

typedef void (*ARM_CRC_SignalEvent_t) (uint32_t event);    /*Pointer to \ref CRC_SignalEvent : Signal CRC Event*/

/**
 @brief Access structure of the CRC Driver.
*/
typedef struct ARM_Driver_CRC
{
    ARM_DRIVER_VERSION    (*GetVersion)      (void);                                                  /* pointer is pointing to CRC_GetVersion : used to get the driver version */
    ARM_CRC_CAPABILITIES  (*GetCapabilities) (void);                                                  /* pointer is pointing to CRC_Capabilities : used to get the driver capabilities */
    int32_t               (*Initialize)      (ARM_CRC_SignalEvent_t cb_event);                        /* Initialize the CRC Interface */
    int32_t               (*Uninitialize)    (void);                                                  /* Pointer to CRC_Uninitialize : De-initialize CRC Interface */
    int32_t               (*PowerControl)    (ARM_POWER_STATE state);                                 /* Pointer to CRC_PowerControl : Control CRC Interface Power */
    int32_t               (*Control)         (uint32_t control, uint32_t arg);                        /* Pointer to CRC_Control : Control CRC Interface */
    int32_t               (*Seed)            (uint32_t seed_value);                                   /* Pointer to CRC_Seed : used to give the seed value*/
    int32_t               (*PolyCustom)      (uint32_t polynomial);                                   /* Pointer to CRC_PolyCustom : used to give the poly custom value*/
    int32_t               (*Compute)         (const void *data_in, uint32_t len, uint32_t *data_out); /* Pointer to CRC_Compute : used to give the input data and output data */
}const ARM_DRIVER_CRC;

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_CRC_H_ */
