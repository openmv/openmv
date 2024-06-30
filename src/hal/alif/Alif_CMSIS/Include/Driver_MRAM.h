/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**************************************************************************//**
 * @file     Driver_MRAM.h
 * @Author   Tanay Rami
 * @email    <tanay@alifsemi.com>
 * @version  V1.0.0
 * @date     01-June-2023
 * @brief    CMSIS-Driver for MRAM (On-chip NVM (Non-Volatile Memory)).
 *           Derived from ARM CMSIS "Driver_Flash.h".
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef DRIVER_MRAM_H_
#define DRIVER_MRAM_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_MRAM_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_MRAM_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
/**
  \fn          ARM_MRAM_CAPABILITIES ARM_MRAM_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_MRAM_CAPABILITIES
*/
/**
  \fn          int32_t ARM_MRAM_Initialize (void)
  \brief       Initialize the MRAM Interface.
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_MRAM_Uninitialize (void)
  \brief       De-initialize the MRAM Interface.
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_MRAM_PowerControl (ARM_POWER_STATE state)
  \brief       Control the MRAM interface power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_MRAM_ReadData (uint32_t addr, void *data, uint32_t cnt)
  \brief       Read data from MRAM.
  \param[in]   addr  Data address.
  \param[out]  data  Pointer to a buffer storing the data read from MRAM.
  \param[in]   cnt   Number of data items to read.
  \return      number of data items read or \ref execution_status
*/
/**
  \fn          int32_t ARM_MRAM_ProgramData (uint32_t addr, const void *data, uint32_t cnt)
  \brief       Program data to MRAM.
  \param[in]   addr  Data address.
  \param[in]   data  Pointer to a buffer containing the data to be programmed to MRAM.
  \param[in]   cnt   Number of data items to program.
  \return      number of data items programmed or \ref execution_status
*/
/**
  \fn          int32_t ARM_MRAM_EraseSector (uint32_t addr)
  \brief       Erase MRAM Sector.
  \param[in]   addr  Sector address
  \return      \ref execution_status
*/
/**
  \fn          int32_t ARM_MRAM_EraseChip (void)
  \brief       Erase complete MRAM.
               Optional function for faster full chip erase.
  \return      \ref execution_status
*/

/**
\brief MRAM Driver Capabilities.
*/
typedef struct _ARM_MRAM_CAPABILITIES {
  uint32_t data_width   : 1;            ///< Data width: 0=128-bit
  uint32_t reserved     : 31;           ///< Reserved (must be zero)
} ARM_MRAM_CAPABILITIES;

/**
\brief Access structure of the MRAM Driver
*/
typedef struct _ARM_DRIVER_MRAM {
  ARM_DRIVER_VERSION     (*GetVersion)     (void);                                          ///< Pointer to \ref ARM_MRAM_GetVersion : Get driver version.
  ARM_MRAM_CAPABILITIES  (*GetCapabilities)(void);                                          ///< Pointer to \ref ARM_MRAM_GetCapabilities : Get driver capabilities.
  int32_t                (*Initialize)     (void);                                          ///< Pointer to \ref ARM_MRAM_Initialize : Initialize MRAM Interface.
  int32_t                (*Uninitialize)   (void);                                          ///< Pointer to \ref ARM_MRAM_Uninitialize : De-initialize MRAM Interface.
  int32_t                (*PowerControl)   (ARM_POWER_STATE state);                         ///< Pointer to \ref ARM_MRAM_PowerControl : Control MRAM Interface Power.
  int32_t                (*ReadData)       (uint32_t addr,       void *data, uint32_t cnt); ///< Pointer to \ref ARM_MRAM_ReadData : Read data from MRAM.
  int32_t                (*ProgramData)    (uint32_t addr, const void *data, uint32_t cnt); ///< Pointer to \ref ARM_MRAM_ProgramData : Program data to MRAM.
  int32_t                (*EraseSector)    (uint32_t addr);                                 ///< Pointer to \ref ARM_MRAM_EraseSector : Erase MRAM Sector.
  int32_t                (*EraseChip)      (void);                                          ///< Pointer to \ref ARM_MRAM_EraseChip : Erase complete MRAM.
} const ARM_DRIVER_MRAM;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_MRAM_H_ */
