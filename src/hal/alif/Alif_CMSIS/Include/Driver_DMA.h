/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef DRIVER_DMA_H_
#define DRIVER_DMA_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_DMA_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(4,0)  /* API version */


#define _ARM_Driver_DMA_(n)      Driver_DMA##n
#define  ARM_Driver_DMA_(n) _ARM_Driver_DMA_(n)

/****** DMA Control Codes *****/

#define ARM_DMA_CONTROL_Msk             (0xFFUL)
#define ARM_DMA_USER_PROVIDED_MCODE     (0x01UL)    ///< Use User provided microcode; arg = microcode address in memory
#define ARM_DMA_I2S_MONO_MODE           (0x02UL)    ///< Support for I2S mono mode;
#define ARM_DMA_CRC_MODE                (0x03UL)    ///< Support for CRC which doesn't require handshaking
#define ARM_DMA_ENDIAN_SWAP_SIZE        (0x04UL)    ///< Set the Endian Swap Size

/**
\brief DMA Data Direction
*/
typedef enum _ARM_DMA_DATA_DIR {
    ARM_DMA_MEM_TO_MEM,
    ARM_DMA_MEM_TO_DEV,
    ARM_DMA_DEV_TO_MEM,
    ARM_DMA_DIR_NOT_SET,
} ARM_DMA_DATA_DIR;

/**
\brief DMA Burst Size
*/
typedef enum {
    BS_BYTE_1,
    BS_BYTE_2,
    BS_BYTE_4,
    BS_BYTE_8,
    BS_BYTE_16,
} ARM_DMA_BS_Type;

/**
\brief DMA Endian Swap Size
*/
typedef enum {
    ESS_SWAP_NONE,
    ESS_SWAP_16BIT,
    ESS_SWAP_32BIT,
    ESS_SWAP_64BIT,
    ESS_SWAP_128BIT,
} ARM_DMA_ESS_Type;

/****** DMA specific error codes *****/
#define ARM_DMA_ERROR_HANDLE        (ARM_DRIVER_ERROR_SPECIFIC - 1)     ///< Handle error
#define ARM_DMA_ERROR_EVENT         (ARM_DRIVER_ERROR_SPECIFIC - 2)     ///< Event not available
#define ARM_DMA_ERROR_BUFFER        (ARM_DRIVER_ERROR_SPECIFIC - 3)     ///< Buffer overrun
#define ARM_DMA_ERROR_MAX_TRANSFER  (ARM_DRIVER_ERROR_SPECIFIC - 4)     ///< Max transfer length per req
#define ARM_DMA_ERROR_BUSY          (ARM_DRIVER_ERROR_SPECIFIC - 5)     ///< Busy
#define ARM_DMA_ERROR_FAULT         (ARM_DRIVER_ERROR_SPECIFIC - 6)     ///< Fault Occurred
#define ARM_DMA_ERROR_UNALIGNED     (ARM_DRIVER_ERROR_SPECIFIC - 7)     ///< Unaligned address or bytes

/**
\brief DMA Status
*/
typedef struct _ARM_DMA_STATUS {
  uint32_t busy             : 1;        ///< DMA busy flag
  uint32_t reserved         : 31;
} ARM_DMA_STATUS;

typedef void (*ARM_DMA_SignalEvent_t) (uint32_t event, int8_t peri_num);  ///< Pointer to \ref ARM_DMA_SignalEvent : Signal DMA Event.
typedef int32_t DMA_Handle_Type;  ///< Pointer to \ref DMA_Handle_Type : DMA handle number.

/**
\brief DMA Configuration
*/
typedef struct _ARM_DMA_PARAMS {
  int8_t                    peri_reqno;
  uint8_t                   burst_len;
  ARM_DMA_BS_Type           burst_size;
  ARM_DMA_DATA_DIR          dir;
  volatile const void       *src_addr;
  volatile void             *dst_addr;
  uint32_t                  num_bytes;
  uint32_t                  irq_priority;
  ARM_DMA_SignalEvent_t     cb_event;
} ARM_DMA_PARAMS;

/****** DMA Event *****/
#define ARM_DMA_EVENT_COMPLETE          (1UL << 0)  ///< Transfer completed
#define ARM_DMA_EVENT_ABORT             (1UL << 1)  ///< Operation Aborted



// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_DMA_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_DMA_CAPABILITIES ARM_DMA_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_DMA_CAPABILITIES

  \fn          int32_t ARM_DMA_Initialize (ARM_DMA_SignalEvent_t cb_event)
  \brief       Initialize DMA Interface.
  \param[in]   cb_event  Pointer to \ref ARM_DMA_SignalEvent
  \return      \ref execution_status

  \fn          int32_t ARM_DMA_Uninitialize (void)
  \brief       De-initialize DMA Interface.
  \return      \ref execution_status

  \fn          int32_t ARM_DMA_Allocate (DMA_Handle_Type *handle)
  \brief       Allocate a DMA handle for the transfer operation
  \param[in]   handle  Pointer to store the handle provided by the DMA driver
  \return      \ref execution_status

  \fn          int32_t ARM_DMA_Start (DMA_Handle_Type *handle, ARM_DMA_PARAMS *params)
  \brief       Start the DMA transfer operation using the params
  \param[in]   handle  DMA handle for which the transfer operation is requested
  \param[in]   params  DMA parameters required for this transfer operation
  \return      \ref execution_status

  \fn          int32_t ARM_DMA_Stop (DMA_Handle_Type *handle)
  \brief       Stop the DMA transfer operation
  \param[in]   handle  DMA handle for which the transfer operation is requested
  \return      \ref execution_status

  \fn          int32_t ARM_DMA_Control (DMA_Handle_Type *handle, uint32_t control, uint32_t arg)
  \brief       Control DMA Interface.
  \param[in]   handle  DMA handle for which the transfer operation is requested
  \param[in]   control Operation
  \param[in]   arg     Argument of operation (optional)
  \return      common \ref execution_status and driver specific \ref dma_execution_status

  \fn          ARM_DMA_STATUS ARM_DMA_GetStatus (DMA_Handle_Type *handle, uint32_t *count)
  \brief       Get DMA status.
  \param[in]   handle     DMA handle
  \param[in]   count      Return the number of items transmitted
  \return      DMA status \ref ARM_DMA_STATUS

  \fn          int32_t ARM_DMA_DeAllocate (DMA_Handle_Type *handle)
  \brief       DeAllocate a DMA handle
  \param[in]   handle  Pointer to the handle provided by the DMA driver
  \return      \ref execution_status

  \fn          void ARM_DMA_SignalEvent (uint32_t event)
  \brief       Signal DMA Events.
  \param[in]   event \ref DMA_events notification mask
  \return      none
*/

/**
\brief DMA Driver Capabilities.
*/
typedef struct _ARM_DMA_CAPABILITIES {
  uint32_t mem_to_mem            : 1;   ///< supports memory to memory operation
  uint32_t mem_to_peri           : 1;   ///< supports memory to peripheral operation
  uint32_t peri_to_mem           : 1;   ///< supports peripheral to memory operation
  uint32_t sg                    : 1;   ///< supports Scatter Gather
  uint32_t secure_mode           : 1;   ///< supports Secure/Non-Secure mode operation
  uint32_t reserved              : 26;  ///< Reserved (must be zero)
} ARM_DMA_CAPABILITIES;

/**
\brief Access structure of the DMA Driver.
*/
typedef struct _ARM_DRIVER_DMA {
  ARM_DRIVER_VERSION   (*GetVersion)      (void);                                            ///< Pointer to \ref ARM_DMA_GetVersion : Get driver version.
  ARM_DMA_CAPABILITIES (*GetCapabilities) (void);                                            ///< Pointer to \ref ARM_DMA_GetCapabilities : Get driver capabilities.
  int32_t              (*Initialize)      (void);                                            ///< Pointer to \ref ARM_DMA_Initialize : Initialize DMA Interface.
  int32_t              (*Uninitialize)    (void);                                            ///< Pointer to \ref ARM_DMA_Uninitialize : De-initialize DMA Interface.
  int32_t              (*PowerControl)    (ARM_POWER_STATE state);                           ///< Pointer to \ref ARM_DMA_PowerControl : Control DMA Interface Power.
  int32_t              (*Allocate)        (DMA_Handle_Type *handle);                         ///< Pointer to \ref ARM_DMA_Allocate : Allocate a DMA handle
  int32_t              (*Control)         (DMA_Handle_Type *handle, uint32_t control, uint32_t arg);  ///< Pointer to \ref ARM_DMA_Control : Control DMA operation
  int32_t              (*Start)           (DMA_Handle_Type *handle, ARM_DMA_PARAMS *params); ///< Pointer to \ref ARM_DMA_Start : Start DMA operation
  int32_t              (*Stop)            (DMA_Handle_Type *handle);                         ///< Pointer to \ref ARM_DMA_Stop : Stop DMA operation
  int32_t              (*GetStatus)       (DMA_Handle_Type *handle, uint32_t *count);        ///< Pointer to \ref ARM_DMA_GetStatus : Get DMA status.
  int32_t              (*DeAllocate)      (DMA_Handle_Type *handle);                         ///< Pointer to \ref ARM_DMA_DeAllocate : De-Allocate a DMA handle
} const ARM_DRIVER_DMA;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_DMA_H_ */
