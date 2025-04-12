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
 * @file     Driver_HWSEM.h
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     16-June-2022
 * @brief    Header file for hardware semaphore driver.
 ******************************************************************************/

#ifndef DRIVER_HWSEM_H_
#define DRIVER_HWSEM_H_

/* Driver Includes */
#include "Driver_Common.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define ARM_HWSEM_API_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* API version */

#define _ARM_Driver_HWSEM_(n)      DRIVER_HWSEM##n
#define  ARM_Driver_HWSEM_(n) _ARM_Driver_HWSEM_(n)

/* HWSEM Event */
#define HWSEM_AVAILABLE_CB_EVENT                         (1U << 0) ///< HWSEM Available

// Function Documentation
/**
  \fn           ARM_DRIVER_VERSION ARM_HWSEM_GetVersion(void)
  \brief        Returns the Driver Version
  \param[in]    void
  \return       ARM_DRIVER_VERSION : Driver version
 */

/**
  \fn           ARM_HWSEM_CAPABILITIES ARM_HWSEM_GetCapabilities(void)
  \brief        Returns the Driver Capabilities
  \param[in]    void
  \return       ARM_HWSEM_CAPABILITIES : Driver Capabilities
 */

/**
  \fn           int32_t ARM_HWSEM_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
  \brief        Initializes the driver instance with a call back function. The
                (optional) callback function will be invoked (with HWSEM_AVAILABLE_CB_EVENT)
                when the hwsem instance becomes available again after an unsuccessful
                TryLock attempt.
  \param[in]    cb_event : Optional call back function provided by the user.
  \return       ARM_DRIVER_OK : On successful initialization.
  \             ARM_DRIVER_ERROR_BUSY : Driver has already been initialized.
 */

/**
  \fn           int32_t ARM_HWSEM_Uninitialize(void)
  \brief        Uninitializes the driver instance.
  \return       ARM_DRIVER_OK : On success.
  \             ARM_DRIVER_ERROR : If the driver instance has not been initialized before.
 */

/**
  \fn           int32_t ARM_HWSEM_Lock(void)
  \brief        Acquire the hw semaphore. If the semaphore is not available, spin until
                it becomes available.
  \return       ARM_DRIVER_OK : If the hwsem is acquired successfully.
  \             ARM_DRIVER_ERROR : If the driver is not initialized.
 */

/**
  \fn           int32_t ARM_HWSEM_TryLock(void)
  \brief        Try to acquire the hw semaphore. If the semaphore is not available, return an error
                code indicating the lock is busy.
  \return       ARM_DRIVER_OK : If the semaphore is acquired successfully.
  \             ARM_DRIVER_ERROR_BUSY : If the semaphore is not available.
  \             ARM_DRIVER_ERROR : If the driver is not initialized.
 */

/**
  \fn           int32_t ARM_HWSEM_Unlock(void)
  \brief        Release the hw semaphore
  \return       ARM_DRIVER_OK : On a successful unlock.
  \             ARM_DRIVER_ERROR : If the driver is not initialized or the hwsem
                                   has not been locked before.
 */

/**
  \fn           int32_t ARM_HWSEM_GetCount(void)
  \brief        Get the semaphore lock count.
  \return       The current lock count of the semaphore instance.
  \             ARM_DRIVER_ERROR : if driver is not initialized
 */

/**
  \fn          void ARM_HWSEM_SignalEvent(int32_t event, uint8_t sem_id)
  \brief       Callback function that signals an HW semaphore event.
  \param[in]   event  event notification mask
  \return      none
*/
typedef void (*ARM_HWSEM_SignalEvent_t) (int32_t event, uint8_t sem_id);

/**
  \brief HW Semaphore Device Driver Capabilities.
 */
typedef struct _ARM_HWSEM_CAPABILITIES {
	uint32_t event_device_available      : 1;      ///< supports HWSEM Event Callback
	uint32_t reserved                    : 31;     ///< Reserved (must be zero)
} ARM_HWSEM_CAPABILITIES;

/**
  \brief Access structure of the Hardware Semaphore Driver
 */
typedef struct _ARM_DRIVER_HWSEM {
	ARM_DRIVER_VERSION       (*GetVersion)          (void);                               ///< Pointer to \ref ARM_HWSEM_GetVersion : Get driver version.
	ARM_HWSEM_CAPABILITIES   (*GetCapabilities)     (void);                               ///< Pointer to \ref ARM_HWSEM_GetCapabilities : Get driver capabilities.
	int32_t                  (*Initialize)          (ARM_HWSEM_SignalEvent_t cb_event);   ///< Pointer to \ref ARM_HWSEM_Initialize : Initialize HW semaphore Device.
	int32_t                  (*Uninitialize)        (void);                               ///< Pointer to \ref ARM_HWSEM_Uninitialize : De-initialize HW Semaphore Device.
	int32_t                  (*Lock)                (void);                               ///< Pointer to \ref ARM_HWSEM_Lock : Spin until the semaphore is successfully locked.
	int32_t                  (*TryLock)             (void);                               ///< Pointer to \ref ARM_HWSEM_TryLock : Try to lock the semaphore.
	int32_t                  (*Unlock)              (void);                               ///< Pointer to \ref ARM_HWSEM_Unlock : Unlock the semaphore.
	int32_t                  (*GetCount)            (void);                               ///< Pointer to \ref ARM_HWSEM_GetCount : Get the semaphore lock count.
}const ARM_DRIVER_HWSEM;

#ifdef  __cplusplus
}
#endif
#endif /* DRIVER_HWSEM_H_ */
