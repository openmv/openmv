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
 * @file     Driver_HWSEM.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     16-June-2022
 * @brief    ARM CMSIS-Driver for Hardware Semaphore.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Driver specific include */
#include "hwsem.h"
#include "Driver_HWSEM_Private.h"

#define ARM_HWSEM_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* Driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_HWSEM_API_VERSION,
    ARM_HWSEM_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_HWSEM_CAPABILITIES DriverCapabilities =
{
    1,         /* Supports HWSEM callback */
    0
};


/**
  \fn           static ARM_DRIVER_VERSION Hwsem_GetVersion(void)
  \brief        Returns the driver version.
  \return       ARM_DRIVER_VERSION : Driver version
 */
static ARM_DRIVER_VERSION Hwsem_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn           static ARM_HWSEM_CAPABILITIES Hwsem_GetCapabilities(void)
  \brief        Returns the Driver Capabilities
  \return       ARM_HWSEM_CAPABILITIES : Driver Capabilities
 */
static ARM_HWSEM_CAPABILITIES Hwsem_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn           static int32_t Initialize(ARM_HWSEM_SignalEvent_t cb_event,
                HWSEM_RESOURCES *hwsem)
  \brief        Initializes the driver instance with a call back function. The
                (optional) callback function will be invoked (with HWSEM_AVAILABLE_CB_EVENT)
                when the hwsem instance becomes available again after an unsuccessful
                TryLock attempt.
  \param[in]    cb_event : call back function provided by user
  \param[in]    hwsem    : Pointer to resources of the particular driver instance.
  \return       ARM_DRIVER_OK : On successful initialization.
  \             ARM_DRIVER_ERROR_BUSY : Driver has already been initialized.
 */
static int32_t Initialize(ARM_HWSEM_SignalEvent_t cb_event, HWSEM_RESOURCES *hwsem)
{
    /* Confirm that the driver is not already in use */
    if (hwsem->state.initialized)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    if (cb_event != NULL)
    {
        hwsem->cb_event = cb_event;
    }

    hwsem->state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
  \fn           static int32_t Uninitialize(uint8_t HWSEM_RESOURCES *hwsem)
  \brief        Uninitializes the driver instance
  \param[in]    hwsem    : Pointer to resources of the particular driver instance
  \return       ARM_DRIVER_OK : On success.
  \             ARM_DRIVER_ERROR : If the driver instance has not been initialized before.
 */
static int32_t Uninitialize(HWSEM_RESOURCES *hwsem)
{
    /* Confirm that the driver is initialized */
    if (!(hwsem->state.initialized))
    {
        return ARM_DRIVER_ERROR;
    }

    hwsem->cb_event = NULL;

    hwsem->state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn           static int32_t Lock(HWSEM_RESOURCES *hwsem)
  \brief        Acquire the hw semaphore. If the semaphore is not available, spin until
                it becomes available.
  \param[in]    hwsem    : Pointer to resources of the particular driver instance
  \return       ARM_DRIVER_OK : If the hwsem is acquired successfully.
  \             ARM_DRIVER_ERROR : If the driver is not initialized.
 */
static int32_t Lock(HWSEM_RESOURCES *hwsem)
{
    /* Confirm that the driver is initialized */
    if (!(hwsem->state.initialized))
    {
        return ARM_DRIVER_ERROR;
    }

    /* Spin until we get the lock */
    while (hwsem_request(hwsem->regs, HWSEM_MASTERID) != HWSEM_MASTERID);

    return ARM_DRIVER_OK;
}

/**
  \fn           static int32_t TryLock(HWSEM_RESOURCES *hwsem)
  \brief        Try to acquire the hwsem. If the hwsem is not available, return an error
                code indicating the lock is busy.
  \param[in]    hwsem    : Pointer to Resources of the particular driver instance
  \return       ARM_DRIVER_OK : If the hwsem is acquired successfully.
  \             ARM_DRIVER_ERROR_BUSY : if semaphore is not available.
  \             ARM_DRIVER_ERROR : If the driver is not initialized.
 */
static int32_t TryLock(HWSEM_RESOURCES *hwsem)
{
    /* Confirm that the driver is initialized */
    if (!(hwsem->state.initialized))
    {
        return ARM_DRIVER_ERROR;
    }

    if (hwsem_request(hwsem->regs, HWSEM_MASTERID) == HWSEM_MASTERID)
    {
        return ARM_DRIVER_OK;
    }

    /*
     * If the semaphore is not available and if the user has provided a callback
     * function, enable the IRQ.
     */
    if (hwsem->cb_event != NULL)
    {
        NVIC_ClearPendingIRQ(hwsem->irq);
        NVIC_SetPriority(hwsem->irq, hwsem->irq_priority);
        NVIC_EnableIRQ(hwsem->irq);
    }
    return ARM_DRIVER_ERROR_BUSY;
}


/**
  \fn           static int32_t Unlock(HWSEM_RESOURCES *hwsem)
  \brief        Release the semaphore
  \param[in]    hwsem    : Pointer to Resources of the particular driver instance
  \return       ARM_DRIVER_OK : On a successful unlock.
  \             ARM_DRIVER_ERROR : If the driver is not initialized or the hwsem
                                   has not been locked before.
 */
static int32_t Unlock(HWSEM_RESOURCES *hwsem)
{
    /* Confirm that the driver is initialized */
    if (!(hwsem->state.initialized))
    {
        return ARM_DRIVER_ERROR;
    }

    /* Check if the semaphore is locked */
    if (hwsem_getcount(hwsem->regs) > 0)
    {
        /* Release the semaphore */
        hwsem_release(hwsem->regs, HWSEM_MASTERID);

        return ARM_DRIVER_OK;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}

/**
  \fn           static int32_t GetCount(HWSEM_RESOURCES *hwsem)
  \brief        Get the semaphore count
  \param[in]    hwsem      : Pointer to Resources of the particular driver instance
  \return       The current lock count of the semaphore instance.
  \             ARM_DRIVER_ERROR : if driver is not initialized
 */
static int32_t GetCount(HWSEM_RESOURCES *hwsem)
{
    /* Confirm that driver is initialized */
    if (!(hwsem->state.initialized))
    {
        return ARM_DRIVER_ERROR;
    }

    return hwsem_getcount(hwsem->regs);
}

/**
  \fn          void HWSEM_IRQHandler(HWSEM_RESOURCES *hwsem)
  \brief       HWSEM instance specific part of Hw Semaphore Interrupt handler.
  \param[in]   hwsem    :   Pointer to the HWSEM device instance
 */
static void ARM_HWSEM_IRQHandler(HWSEM_RESOURCES *hwsem)
{
    /* Disable Hw Sem IRQ*/
    NVIC_DisableIRQ(hwsem->irq);
    NVIC_ClearPendingIRQ(hwsem->irq);

    /* Call the user provided call back function */
    hwsem->cb_event(HWSEM_AVAILABLE_CB_EVENT, hwsem->sem_id);
}

/* HWSEM0 Driver Instance */
#if (RTE_HWSEM0)

/* HWSEM0 Resources */
static HWSEM_RESOURCES HWSEM0 = {
    .regs = (HWSEM_Type *) HWSEM0_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ0_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM0_IRQPRIORITY,
    .sem_id = HWSEMID0,
};

/* HWSEM0 Interrupt Handler */
void HWSEM_IRQ0Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM0);
}

static int32_t Hwsem0_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM0);
}

static int32_t Hwsem0_Uninitialize(void)
{
    return Uninitialize(&HWSEM0);
}

static int32_t Hwsem0_Lock(void)
{
    return Lock(&HWSEM0);
}

static int32_t Hwsem0_TryLock(void)
{
    return TryLock(&HWSEM0);
}

static int32_t Hwsem0_Unlock(void)
{
    return Unlock(&HWSEM0);
}

static int32_t Hwsem0_GetCount(void)
{
    return GetCount(&HWSEM0);
}

/* HWSEM0 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM0 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem0_Initialize ,
    Hwsem0_Uninitialize,
    Hwsem0_Lock,
    Hwsem0_TryLock,
    Hwsem0_Unlock,
    Hwsem0_GetCount
};

#endif

/* HSWEM1 Driver Instance */
#if (RTE_HWSEM1)

/* HWSEM1 Resources */
static HWSEM_RESOURCES HWSEM1 = {
    .regs = (HWSEM_Type *) HWSEM1_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ1_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM1_IRQPRIORITY,
    .sem_id = HWSEMID1,
};

/* HWSEM1 Interrupt Handler */
void HWSEM_IRQ1Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM1);
}

static int32_t Hwsem1_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM1);
}

static int32_t Hwsem1_Uninitialize(void)
{
    return Uninitialize(&HWSEM1);
}

static int32_t Hwsem1_Lock(void)
{
    return Lock(&HWSEM1);
}

static int32_t Hwsem1_TryLock(void)
{
    return TryLock(&HWSEM1);
}

static int32_t Hwsem1_Unlock(void)
{
    return Unlock(&HWSEM1);
}

static int32_t Hwsem1_GetCount(void)
{
    return GetCount(&HWSEM1);
}

/* HWSEM1 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM1 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem1_Initialize ,
    Hwsem1_Uninitialize,
    Hwsem1_Lock,
    Hwsem1_TryLock,
    Hwsem1_Unlock,
    Hwsem1_GetCount
};

#endif

/* HWSEM2 Driver Instance */
#if (RTE_HWSEM2)

/* HWSEM2 Resources */
static HWSEM_RESOURCES HWSEM2 = {
    .regs = (HWSEM_Type *) HWSEM2_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ2_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM2_IRQPRIORITY,
    .sem_id = HWSEMID2,
};

/* HWSEM2 Interrupt Handler */
void HWSEM_IRQ2Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM2);
}

static int32_t Hwsem2_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM2);
}

static int32_t Hwsem2_Uninitialize(void)
{
    return Uninitialize(&HWSEM2);
}

static int32_t Hwsem2_Lock(void)
{
    return Lock(&HWSEM2);
}

static int32_t Hwsem2_TryLock(void)
{
    return TryLock(&HWSEM2);
}

static int32_t Hwsem2_Unlock(void)
{
    return Unlock(&HWSEM2);
}

static int32_t Hwsem2_GetCount(void)
{
    return GetCount(&HWSEM2);
}

/* HWSEM2 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM2 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem2_Initialize ,
    Hwsem2_Uninitialize,
    Hwsem2_Lock,
    Hwsem2_TryLock,
    Hwsem2_Unlock,
    Hwsem2_GetCount
};

#endif

/* HWSEM3 Driver Instance */
#if (RTE_HWSEM3)

/* HWSEM3 Resources */
static HWSEM_RESOURCES HWSEM3 = {
    .regs = (HWSEM_Type *) HWSEM3_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ3_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM3_IRQPRIORITY,
    .sem_id = HWSEMID3,
};

/* HWSEM3 Interrupt Handler */
void HWSEM_IRQ3Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM3);
}

static int32_t Hwsem3_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM3);
}

static int32_t Hwsem3_Uninitialize(void)
{
    return Uninitialize(&HWSEM3);
}

static int32_t Hwsem3_Lock(void)
{
    return Lock(&HWSEM3);
}

static int32_t Hwsem3_TryLock(void)
{
    return TryLock(&HWSEM3);
}

static int32_t Hwsem3_Unlock(void)
{
    return Unlock(&HWSEM3);
}

static int32_t Hwsem3_GetCount(void)
{
    return GetCount(&HWSEM3);
}

/* HWSEM3 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM3 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem3_Initialize ,
    Hwsem3_Uninitialize,
    Hwsem3_Lock,
    Hwsem3_TryLock,
    Hwsem3_Unlock,
    Hwsem3_GetCount
};

#endif

/* HWSEM4 Driver Instance */
#if (RTE_HWSEM4)

/* HWSEM4 Resources */
static HWSEM_RESOURCES HWSEM4 = {
    .regs = (HWSEM_Type *) HWSEM4_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ4_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM4_IRQPRIORITY,
    .sem_id = HWSEMID4,
};

/* HWSEM4 Interrupt Handler */
void HWSEM_IRQ4Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM4);
}

static int32_t Hwsem4_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM4);
}

static int32_t Hwsem4_Uninitialize(void)
{
    return Uninitialize(&HWSEM4);
}

static int32_t Hwsem4_Lock(void)
{
    return Lock(&HWSEM4);
}

static int32_t Hwsem4_TryLock(void)
{
    return TryLock(&HWSEM4);
}

static int32_t Hwsem4_Unlock(void)
{
    return Unlock(&HWSEM4);
}

static int32_t Hwsem4_GetCount(void)
{
    return GetCount(&HWSEM4);
}

/* HWSEM4 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM4 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem4_Initialize ,
    Hwsem4_Uninitialize,
    Hwsem4_Lock,
    Hwsem4_TryLock,
    Hwsem4_Unlock,
    Hwsem4_GetCount
};

#endif

/* HWSEM5 Driver Instance */
#if (RTE_HWSEM5)

/* HWSEM5 Resources */
static HWSEM_RESOURCES HWSEM5 = {
    .regs = (HWSEM_Type *) HWSEM5_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ5_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM5_IRQPRIORITY,
    .sem_id = HWSEMID5,
};

/* HWSEM5 Interrupt Handler */
void HWSEM_IRQ5Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM5);
}

static int32_t Hwsem5_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM5);
}

static int32_t Hwsem5_Uninitialize(void)
{
    return Uninitialize(&HWSEM5);
}

static int32_t Hwsem5_Lock(void)
{
    return Lock(&HWSEM5);
}

static int32_t Hwsem5_TryLock(void)
{
    return TryLock(&HWSEM5);
}

static int32_t Hwsem5_Unlock(void)
{
    return Unlock(&HWSEM5);
}

static int32_t Hwsem5_GetCount(void)
{
    return GetCount(&HWSEM5);
}

/* HWSEM5 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM5 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem5_Initialize ,
    Hwsem5_Uninitialize,
    Hwsem5_Lock,
    Hwsem5_TryLock,
    Hwsem5_Unlock,
    Hwsem5_GetCount
};

#endif

/* HWSEM6 Driver Instance */
#if (RTE_HWSEM6)

/* HWSEM6 Resources */
static HWSEM_RESOURCES HWSEM6 = {
    .regs = (HWSEM_Type *) HWSEM6_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ6_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM6_IRQPRIORITY,
    .sem_id = HWSEMID6,
};

/* HWSEM6 Interrupt Handler */
void HWSEM_IRQ6Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM6);
}

static int32_t Hwsem6_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM6);
}

static int32_t Hwsem6_Uninitialize(void)
{
    return Uninitialize(&HWSEM6);
}

static int32_t Hwsem6_Lock(void)
{
    return Lock(&HWSEM6);
}

static int32_t Hwsem6_TryLock(void)
{
    return TryLock(&HWSEM6);
}

static int32_t Hwsem6_Unlock(void)
{
    return Unlock(&HWSEM6);
}

static int32_t Hwsem6_GetCount(void)
{
    return GetCount(&HWSEM6);
}

/* HWSEM6 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM6 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem6_Initialize ,
    Hwsem6_Uninitialize,
    Hwsem6_Lock,
    Hwsem6_TryLock,
    Hwsem6_Unlock,
    Hwsem6_GetCount
};

#endif

/* HWSEM0 Driver Instance */
#if (RTE_HWSEM7)

/* HWSEM7 Resources */
static HWSEM_RESOURCES HWSEM7 = {
    .regs = (HWSEM_Type *) HWSEM7_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ7_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM7_IRQPRIORITY,
    .sem_id = HWSEMID7,
};

/* HWSEM7 Interrupt Handler */
void HWSEM_IRQ7Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM7);
}

static int32_t Hwsem7_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM7);
}

static int32_t Hwsem7_Uninitialize(void)
{
    return Uninitialize(&HWSEM7);
}

static int32_t Hwsem7_Lock(void)
{
    return Lock(&HWSEM7);
}

static int32_t Hwsem7_TryLock(void)
{
    return TryLock(&HWSEM7);
}

static int32_t Hwsem7_Unlock(void)
{
    return Unlock(&HWSEM7);
}

static int32_t Hwsem7_GetCount(void)
{
    return GetCount(&HWSEM7);
}

/* HWSEM7 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM7 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem7_Initialize ,
    Hwsem7_Uninitialize,
    Hwsem7_Lock,
    Hwsem7_TryLock,
    Hwsem7_Unlock,
    Hwsem7_GetCount
};

#endif

/* HWSEM8 Driver Instance */
#if (RTE_HWSEM8)

/* HWSEM8 Resources */
static HWSEM_RESOURCES HWSEM8 = {
    .regs = (HWSEM_Type *) HWSEM8_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ8_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM8_IRQPRIORITY,
    .sem_id = HWSEMID8,
};

/* HWSEM8 Interrupt Handler */
void HWSEM_IRQ8Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM8);
}

static int32_t Hwsem8_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM8);
}

static int32_t Hwsem8_Uninitialize(void)
{
    return Uninitialize(&HWSEM8);
}

static int32_t Hwsem8_Lock(void)
{
    return Lock(&HWSEM8);
}

static int32_t Hwsem8_TryLock(void)
{
    return TryLock(&HWSEM8);
}

static int32_t Hwsem8_Unlock(void)
{
    return Unlock(&HWSEM8);
}

static int32_t Hwsem8_GetCount(void)
{
    return GetCount(&HWSEM8);
}

/* HWSEM8 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM8 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem8_Initialize ,
    Hwsem8_Uninitialize,
    Hwsem8_Lock,
    Hwsem8_TryLock,
    Hwsem8_Unlock,
    Hwsem8_GetCount
};

#endif

/* HWSEM9 Driver Instance */
#if (RTE_HWSEM9)

/* HWSEM9 Resources */
static HWSEM_RESOURCES HWSEM9 = {
    .regs = (HWSEM_Type *) HWSEM9_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ9_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM9_IRQPRIORITY,
    .sem_id = HWSEMID9,
};

/* HWSEM9 Interrupt Handler */
void HWSEM_IRQ9Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM9);
}

static int32_t Hwsem9_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM9);
}

static int32_t Hwsem9_Uninitialize(void)
{
    return Uninitialize(&HWSEM9);
}

static int32_t Hwsem9_Lock(void)
{
    return Lock(&HWSEM9);
}

static int32_t Hwsem9_TryLock(void)
{
    return TryLock(&HWSEM9);
}

static int32_t Hwsem9_Unlock(void)
{
    return Unlock(&HWSEM9);
}

static int32_t Hwsem9_GetCount(void)
{
    return GetCount(&HWSEM9);
}

/* HWSEM9 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM9 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem9_Initialize ,
    Hwsem9_Uninitialize,
    Hwsem9_Lock,
    Hwsem9_TryLock,
    Hwsem9_Unlock,
    Hwsem9_GetCount
};

#endif

/* HWSEM10 Driver Instance */
#if (RTE_HWSEM10)

/* HWSEM10 Resources */
static HWSEM_RESOURCES HWSEM10 = {
    .regs = (HWSEM_Type *) HWSEM10_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ10_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM10_IRQPRIORITY,
    .sem_id = HWSEMID10,
};

/* HWSEM10 Interrupt Handler */
void HWSEM_IRQ10Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM10);
}

static int32_t Hwsem10_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM10);
}

static int32_t Hwsem10_Uninitialize(void)
{
    return Uninitialize(&HWSEM10);
}

static int32_t Hwsem10_Lock(void)
{
    return Lock(&HWSEM10);
}

static int32_t Hwsem10_TryLock(void)
{
    return TryLock(&HWSEM10);
}

static int32_t Hwsem10_Unlock(void)
{
    return Unlock(&HWSEM10);
}

static int32_t Hwsem10_GetCount(void)
{
    return GetCount(&HWSEM10);
}

/* HWSEM10 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM10 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem10_Initialize ,
    Hwsem10_Uninitialize,
    Hwsem10_Lock,
    Hwsem10_TryLock,
    Hwsem10_Unlock,
    Hwsem10_GetCount
};

#endif

/* HWSEM11 Driver Instance */
#if (RTE_HWSEM11)

/* HWSEM11 Resources */
static HWSEM_RESOURCES HWSEM11 = {
    .regs = (HWSEM_Type *) HWSEM11_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ11_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM11_IRQPRIORITY,
    .sem_id = HWSEMID11,
};

/* HWSEM11 Interrupt Handler */
void HWSEM_IRQ11Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM11);
}

static int32_t Hwsem11_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM11);
}

static int32_t Hwsem11_Uninitialize(void)
{
    return Uninitialize(&HWSEM11);
}

static int32_t Hwsem11_Lock(void)
{
    return Lock(&HWSEM11);
}

static int32_t Hwsem11_TryLock(void)
{
    return TryLock(&HWSEM11);
}

static int32_t Hwsem11_Unlock(void)
{
    return Unlock(&HWSEM11);
}

static int32_t Hwsem11_GetCount(void)
{
    return GetCount(&HWSEM11);
}

/* HWSEM11 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM11 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem11_Initialize ,
    Hwsem11_Uninitialize,
    Hwsem11_Lock,
    Hwsem11_TryLock,
    Hwsem11_Unlock,
    Hwsem11_GetCount
};

#endif

/* HWSEM12 Driver Instance */
#if (RTE_HWSEM12)

/* HWSEM12 Resources */
static HWSEM_RESOURCES HWSEM12 = {
    .regs =(HWSEM_Type *) HWSEM12_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ12_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM12_IRQPRIORITY,
    .sem_id = HWSEMID12,
};

/* HWSEM12 Interrupt Handler */
void HWSEM_IRQ12Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM12);
}

static int32_t Hwsem12_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM12);
}

static int32_t Hwsem12_Uninitialize(void)
{
    return Uninitialize(&HWSEM12);
}

static int32_t Hwsem12_Lock(void)
{
    return Lock(&HWSEM12);
}

static int32_t Hwsem12_TryLock(void)
{
    return TryLock(&HWSEM12);
}

static int32_t Hwsem12_Unlock(void)
{
    return Unlock(&HWSEM12);
}

static int32_t Hwsem12_GetCount(void)
{
    return GetCount(&HWSEM12);
}

/* HWSEM12 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM12 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem12_Initialize ,
    Hwsem12_Uninitialize,
    Hwsem12_Lock,
    Hwsem12_TryLock,
    Hwsem12_Unlock,
    Hwsem12_GetCount
};

#endif

/* HWSEM13 Driver Instance */
#if (RTE_HWSEM13)

/* HWSEM13 Resources */
static HWSEM_RESOURCES HWSEM13 = {
    .regs = (HWSEM_Type *) HWSEM13_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ13_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM13_IRQPRIORITY,
    .sem_id = HWSEMID13,
};

/* HWSEM13 Interrupt Handler */
void HWSEM_IRQ13Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM13);
}

static int32_t Hwsem13_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM13);
}

static int32_t Hwsem13_Uninitialize(void)
{
    return Uninitialize(&HWSEM13);
}

static int32_t Hwsem13_Lock(void)
{
    return Lock(&HWSEM13);
}

static int32_t Hwsem13_TryLock(void)
{
    return TryLock(&HWSEM13);
}

static int32_t Hwsem13_Unlock(void)
{
    return Unlock(&HWSEM13);
}

static int32_t Hwsem13_GetCount(void)
{
    return GetCount(&HWSEM13);
}

/* HWSEM13 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM13 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem13_Initialize ,
    Hwsem13_Uninitialize,
    Hwsem13_Lock,
    Hwsem13_TryLock,
    Hwsem13_Unlock,
    Hwsem13_GetCount
};

#endif

/* HWSEM14 Driver Instance */
#if (RTE_HWSEM14)

/* HWSEM14 Resources */
static HWSEM_RESOURCES HWSEM14 = {
    .regs = (HWSEM_Type *) HWSEM14_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ14_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM14_IRQPRIORITY,
    .sem_id = HWSEMID14,
};

/* HWSEM14 Interrupt Handler */
void HWSEM_IRQ14Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM14);
}

static int32_t Hwsem14_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM14);
}

static int32_t Hwsem14_Uninitialize(void)
{
    return Uninitialize(&HWSEM14);
}

static int32_t Hwsem14_Lock(void)
{
    return Lock(&HWSEM14);
}

static int32_t Hwsem14_TryLock(void)
{
    return TryLock(&HWSEM14);
}

static int32_t Hwsem14_Unlock(void)
{
    return Unlock(&HWSEM14);
}

static int32_t Hwsem14_GetCount(void)
{
    return GetCount(&HWSEM14);
}

/* HWSEM14 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM14 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem14_Initialize ,
    Hwsem14_Uninitialize,
    Hwsem14_Lock,
    Hwsem14_TryLock,
    Hwsem14_Unlock,
    Hwsem14_GetCount
};

#endif

/* HWSEM15 Driver Instance */
#if (RTE_HWSEM15)

/* HWSEM15 Resources */
static HWSEM_RESOURCES HWSEM15 = {
    .regs = (HWSEM_Type *) HWSEM15_BASE,
    .cb_event = NULL,
    .irq = (IRQn_Type) HWSEM_IRQ15_IRQn,
    .irq_priority = (uint8_t) RTE_HWSEM15_IRQPRIORITY,
    .sem_id = HWSEMID15,
};

/* HWSEM15 Interrupt Handler */
void HWSEM_IRQ15Handler(void)
{
    ARM_HWSEM_IRQHandler(&HWSEM15);
}

static int32_t Hwsem15_Initialize(ARM_HWSEM_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &HWSEM15);
}

static int32_t Hwsem15_Uninitialize(void)
{
    return Uninitialize(&HWSEM15);
}

static int32_t Hwsem15_Lock(void)
{
    return Lock(&HWSEM15);
}

static int32_t Hwsem15_TryLock(void)
{
    return TryLock(&HWSEM15);
}

static int32_t Hwsem15_Unlock(void)
{
    return Unlock(&HWSEM15);
}

static int32_t Hwsem15_GetCount(void)
{
    return GetCount(&HWSEM15);
}

/* HWSEM15 Access Struct */
ARM_DRIVER_HWSEM DRIVER_HWSEM15 =
{
    Hwsem_GetVersion,
    Hwsem_GetCapabilities,
    Hwsem15_Initialize ,
    Hwsem15_Uninitialize,
    Hwsem15_Lock,
    Hwsem15_TryLock,
    Hwsem15_Unlock,
    Hwsem15_GetCount
};

#endif
