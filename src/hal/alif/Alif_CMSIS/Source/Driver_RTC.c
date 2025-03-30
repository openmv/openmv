/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/**************************************************************************//**
 * @file     Driver_RTC.c
 * @author   Tanay Rami, Manoj A Murudi
 * @email    tanay@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     23-March-2023
 * @brief    CMSIS-Driver for RTC
 ******************************************************************************/

#include "Driver_RTC.h"
#include "Driver_RTC_Private.h"
#include "rtc.h"
#include "sys_ctrl_rtc.h"

#define ARM_RTC_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* driver version */

#if !(RTE_RTC0)
#error "RTC0 is not enabled in the RTE_Device.h"
#endif

#if !defined(RTE_Drivers_RTC)
#error "RTC0 is not enabled in the RTE_Components.h"
#endif

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_RTC_API_VERSION,
    ARM_RTC_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_RTC_CAPABILITIES DriverCapabilities = {
    1,  /* supports RTC Alarm Callback */
    0   /* Reserved (must be zero) */
};

/**
  \fn        ARM_DRIVER_VERSION RTC_GetVersion(void)
  \brief     get rtc version
  \param     none
  \return    driver version
*/
static ARM_DRIVER_VERSION RTC_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn       ARM_RTC_CAPABILITIES RTC_GetCapabilities(void)
  \brief    get rtc capabilites
  \param    none
  \return   driver capabilites
*/
static ARM_RTC_CAPABILITIES RTC_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn        int32_t LPRTC_Initialize (LPRTC_RESOURCES *LPRTC_RES, ARM_RTC_SignalEvent_t cb_event)
  \brief     Initialize the lprtc device, this does:
              - Disable the counter, prescalar, and interrupts.
              - Program the default prescaler value.
              - Enable the counter and prescaler,
              - Keep interrupt generation disabled until an alarm is set.
  \param[in]  cb_event  : Pointer to LPRTC Event \ref LPRTC_SignalEvent
  \param[in]  LPRTC_RES : Pointer to lprtc device resources
  \return     execution_status
*/
static int32_t LPRTC_Initialize (LPRTC_RESOURCES *LPRTC_RES, ARM_RTC_SignalEvent_t cb_event)
{
    if (LPRTC_RES->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    if (!cb_event)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* set the user callback event. */
    LPRTC_RES->cb_event = cb_event;

    /* Set rtc flag to initialized. */
    LPRTC_RES->state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t LPRTC_PowerControl (LPRTC_RESOURCES *LPRTC_RES, ARM_POWER_STATE state)
  \brief        LPRTC power control
  \param[in]    state : Power state
  \param[in]    LPRTC_RES : Pointer to lprtc device resources
  \return       none
*/
static int32_t LPRTC_PowerControl (LPRTC_RESOURCES *LPRTC_RES, ARM_POWER_STATE state)
{
    switch (state)
    {
        case ARM_POWER_OFF:
        {
            /* Disable LPRTC IRQ */
            NVIC_DisableIRQ (LPRTC_RES->irq_num);

            /* Clear Any Pending IRQ*/
            NVIC_ClearPendingIRQ (LPRTC_RES->irq_num);

            /* disable LPRTC Prescaler. */
            lprtc_prescaler_disable (LPRTC_RES->regs);

            /* disable LPRTC counter. */
            lprtc_counter_disable (LPRTC_RES->regs);

            /* disable LPRTC interrupt. */
            lprtc_interrupt_disable (LPRTC_RES->regs);

            /* disable LPRTC counter wrap. */
            lprtc_counter_wrap_disable (LPRTC_RES->regs);

            /* disable LPRTC clocks */
            disable_lprtc_clk();

            /* Reset lprtc power state. */
            LPRTC_RES->state.powered = 0;
            break;
        }

        case ARM_POWER_FULL:
        {
            if (LPRTC_RES->state.initialized == 0)
            {
                return ARM_DRIVER_ERROR;
            }

            if (LPRTC_RES->state.powered == 1)
            {
                return ARM_DRIVER_OK;
            }

            /* enable LPRTC clocks */
            enable_lprtc_clk();

            /* disable LPRTC counter wrap. */
            lprtc_counter_wrap_disable (LPRTC_RES->regs);

            /* enable LPRTC Prescaler. */
            lprtc_prescaler_enable (LPRTC_RES->regs);

            /* enable LPRTC counter. */
            lprtc_counter_enable (LPRTC_RES->regs);

            /* Enable Interrupt generation. */
            lprtc_interrupt_enable (LPRTC_RES->regs);

            /* Set the IRQ priority */
            NVIC_SetPriority (LPRTC_RES->irq_num, LPRTC_RES->irq_priority);

            /* Enable LPRTC IRQ*/
            NVIC_ClearPendingIRQ (LPRTC_RES->irq_num);
            NVIC_EnableIRQ (LPRTC_RES->irq_num);

            /* Unmask Interrupt. */
            lprtc_interrupt_unmask (LPRTC_RES->regs);

            /* Set the power state enabled */
            LPRTC_RES->state.powered = 1;
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
  \fn        int32_t LPRTC_Uninitialize (LPRTC_RESOURCES *LPRTC_RES)
  \brief     Uninitialize the lprtc device, this does:
             - Power-Off the LPRTC device.
             - Disable the counter, prescalar, and interrupts.
  \param[in] LPRTC_RES  : Pointer to lprtc device resources
  \return    execution status
*/
static int32_t LPRTC_Uninitialize (LPRTC_RESOURCES *LPRTC_RES)
{
     if (LPRTC_RES->state.initialized == 0)
     {
         return ARM_DRIVER_OK;
     }

    /* set the user callback event to NULL. */
    LPRTC_RES->cb_event = NULL;

    /* reset lprtc state. */
    LPRTC_RES->state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t LPRTC_Control (LPRTC_RESOURCES *LPRTC_RES, uint32_t control, uint32_t arg)
  \brief        CMSIS-Driver lprtc control.
                Control LPRTC Interface.
  \param[in]    control : Operation
  \param[in]    arg     : Argument of operation (optional)
  \param[in]    LPRTC_RES   : Pointer to lprtc device resources
  \return       execution status
*/
static int32_t LPRTC_Control (LPRTC_RESOURCES *LPRTC_RES, uint32_t control, uint32_t arg)
{
    if (LPRTC_RES->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (control)
    {
        case ARM_RTC_SET_PRESCALER:
        {
            /* disable LPRTC Prescaler. */
            lprtc_prescaler_disable (LPRTC_RES->regs);

            /* disable LPRTC counter. */
            lprtc_counter_disable (LPRTC_RES->regs);

            /* load LPRTC counter. */
            lprtc_load_prescaler (LPRTC_RES->regs, arg);

            /* enable LPRTC Prescaler. */
            lprtc_prescaler_enable (LPRTC_RES->regs);

            /* enable LPRTC counter. */
            lprtc_counter_enable (LPRTC_RES->regs);

            break;
        }
        case ARM_RTC_SET_ALARM:
        {
            /* load lprtc counter match register. */
            lprtc_load_counter_match_register (LPRTC_RES->regs, arg);

            /* Enable LPRTC IRQ*/
            NVIC_EnableIRQ (LPRTC_RES->irq_num);

            /* set lprtc alarm state. */
            LPRTC_RES->state.alarm = 1;

            break;
        }

        default:
            /* Unsupported command */
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
  \fn         int32_t LPRTC_ReadCounter (LPRTC_RESOURCES *LPRTC_RES, uint32_t *val)
  \brief      Read the current counter value
  \param[in]  val       : Pointer to the address where current lprtc counter value
                          needs to be copied to
  \param[in]  LPRTC_RES : Pointer to lprtc device resources
  \return     execution status
*/
static int32_t LPRTC_ReadCounter (LPRTC_RESOURCES *LPRTC_RES, uint32_t *val)
{
    if (LPRTC_RES->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* read lprtc current counter value. */
    *val = lprtc_get_count (LPRTC_RES->regs);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t LPRTC_LoadCounter (LPRTC_RESOURCES *LPRTC_RES, uint32_t loadvalue)
  \brief        Load new counter value
  \param[in]    loadvalue : Variable which contains new counter value
  \param[in]    LPRTC_RES : Pointer to lprtc device resources
  \return       execution status
*/
static int32_t LPRTC_LoadCounter (LPRTC_RESOURCES *LPRTC_RES, uint32_t loadvalue)
{
    if (LPRTC_RES->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* disable LPRTC counter. */
    lprtc_counter_disable (LPRTC_RES->regs);

    /* Load counter value */
    lprtc_load_count(LPRTC_RES->regs, loadvalue);

    /* enable LPRTC counter. */
    lprtc_counter_enable (LPRTC_RES->regs);

    return ARM_DRIVER_OK;
}

/**
  \fn           void RTC_IRQHandler (LPRTC_RESOURCES *LPRTC_RES)
  \brief        lprtc interrupt handler
  \param[in]    LPRTC_RES  : Pointer to lprtc device resources
  \return       none
*/
static void RTC_IRQHandler (LPRTC_RESOURCES *LPRTC_RES)
{
    uint32_t event = 0U;    /* callback event */

    /* Acknowledge the interrupt */
    lprtc_interrupt_ack (LPRTC_RES->regs);

    /* mark event as Alarm Triggered. */
    event |= ARM_RTC_EVENT_ALARM_TRIGGER;

    /* call the user callback if any event occurs */
    if (LPRTC_RES->cb_event != NULL)
    {
        /* call the user callback */
        LPRTC_RES->cb_event(event);
    }

    NVIC_DisableIRQ (LPRTC_RES->irq_num);

    /* Reset lprtc Alarm state. */
    LPRTC_RES->state.alarm = 0;
}

/* RTC0 Driver Instance */
#if (RTE_RTC0)

/* RTC0 device configuration */
static LPRTC_RESOURCES RTC0 = {
    .regs                      = (LPRTC_Type*) LPRTC_BASE,
    .cb_event                  = NULL,
    .irq_num                   = (IRQn_Type) LPRTC_IRQ_IRQn,
    .irq_priority              = RTE_RTC0_IRQ_PRI,
};

/* Function Name: RTC0_Initialize */
static int32_t RTC0_Initialize (ARM_RTC_SignalEvent_t cb_event)
{
    return (LPRTC_Initialize (&RTC0, cb_event));
}

/* Function Name: RTC0_Uninitialize */
static int32_t RTC0_Uninitialize (void)
{
    return (LPRTC_Uninitialize (&RTC0));
}

/* Function Name: RTC0_PowerControl */
static int32_t RTC0_PowerControl(ARM_POWER_STATE state)
{
    return (LPRTC_PowerControl(&RTC0, state));
}

/* Function Name: RTC0_Control */
static int32_t RTC0_Control(uint32_t control, uint32_t arg)
{
    return (LPRTC_Control(&RTC0, control, arg));
}

/* Function Name: RTC0_ReadCounter */
static int32_t RTC0_ReadCounter (uint32_t *val)
{
    return (LPRTC_ReadCounter (&RTC0, val));
}

/* Function Name: RTC0_LoadCounter */
static int32_t RTC0_LoadCounter (uint32_t loadval)
{
    return (LPRTC_LoadCounter (&RTC0, loadval));
}

/* Function Name: RTC0_IRQHandler */
void LPRTC_IRQHandler (void)
{
    RTC_IRQHandler (&RTC0);
}

extern ARM_DRIVER_RTC Driver_RTC0;
ARM_DRIVER_RTC Driver_RTC0 =
{
    RTC_GetVersion,
    RTC_GetCapabilities,
    RTC0_Initialize,
    RTC0_Uninitialize,
    RTC0_PowerControl,
    RTC0_Control,
    RTC0_ReadCounter,
    RTC0_LoadCounter
};
#endif /* End of RTE_RTC0 */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
