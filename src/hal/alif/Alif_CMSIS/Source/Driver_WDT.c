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
 * @file     Driver_WDT.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     27-April-2021
 * @brief    CMSIS-Driver for Watchdog
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Includes --------------------------------------------------------------------------- */
/* Project Includes */
#include "Driver_WDT.h"
#include "Driver_WDT_Private.h"

#if !(RTE_WDT0)
#error "Watchdog Timer WDT is not enabled in the RTE_Device.h"
#endif

#if (defined(RTE_Drivers_WDT0) && !RTE_WDT0)
#error "Watchdog Timer WDT0 not configured in RTE_Device.h!"
#endif

#define ARM_WDT_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* driver version */

/* Watchdog peripherals Clock (same as SystemCoreClock) */
extern uint32_t SystemCoreClock;
#define WDT_PERIPHERAL_CLOCK             (SystemCoreClock)

/* Generic macro calculations for msecs ticks for the watchdog timeout. */
#define FREQ                             ( (uint32_t) WDT_PERIPHERAL_CLOCK )
#define MILLI_SEC                        (1000UL)
#define WDT_TICKS_FOR_MILLI_SECS(x)      ( ( (uint64_t) x * FREQ ) / MILLI_SEC )
#define WDT_MILLI_SECS_FOR_TICKS(x)      ( ( (uint64_t) x * MILLI_SEC) / FREQ )
#define WDT_MAX_TIMEOUT_MILLI_SEC        ( ( (uint64_t) WATCHDOG_MAX_VALUE * MILLI_SEC ) / (FREQ) )

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_WDT_API_VERSION,
    ARM_WDT_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_WDT_CAPABILITIES DriverCapabilities = {
    1,  /* supports watchdog Lock  */
    0   /* Reserved (must be zero) */
};

/**
  \fn           ARM_DRIVER_VERSION WDT_GetVersion(void)
  \brief        get watchdog version
  \return       driver version
*/
static ARM_DRIVER_VERSION WDT_GetVersion(void)
{
  return DriverVersion;
}

/**
  \fn           ARM_WDT_CAPABILITIES WDT_GetCapabilities(void)
  \brief        get watchdog capabilites
  \return       driver capabilites
*/
static ARM_WDT_CAPABILITIES WDT_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
  \fn           int32_t WDT_Start(WDT_RESOURCES *wdt)
  \brief        Start the watchdog timer.
  \param[in]    wdt  : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_Start(WDT_RESOURCES *wdt)
{
    if (!wdt)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(wdt->flags & WATCHDOG_POWERED) )
        return ARM_DRIVER_ERROR;

    if (wdt_locked(wdt->regs))
        return ARM_DRIVER_ERROR_BUSY;

    wdt_start(wdt->regs, wdt->timeout);

    wdt->flags |= WATCHDOG_STARTED;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WDT_Feed(WDT_RESOURCES *wdt)
  \brief        Feeds the watchdog to not cause a reset.
  \param[in]    wdt  : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_Feed(WDT_RESOURCES *wdt)
{
    if (!wdt)
        return ARM_DRIVER_ERROR_PARAMETER;

    if ( !(wdt->flags & WATCHDOG_STARTED) )
        return ARM_DRIVER_ERROR;

    if (wdt_locked(wdt->regs))
        return ARM_DRIVER_ERROR_BUSY;

    wdt_feed(wdt->regs, wdt->timeout);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WDT_GetRemainingTime(uint32_t *val, WDT_RESOURCES *wdt)
  \brief        Get watchdog remaining time before reset.
  \param[out]   val   : Pointer to the address where watchdog remaining time in milliseconds.
  \param[in]    wdt   : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_GetRemainingTime(uint32_t *val, WDT_RESOURCES *wdt)
{
    if (!wdt)
        return ARM_DRIVER_ERROR_PARAMETER;

    if ( !(wdt->flags & WATCHDOG_STARTED) )
        return ARM_DRIVER_ERROR;

    if (wdt_locked(wdt->regs))
        return ARM_DRIVER_ERROR_BUSY;

    *val = WDT_MILLI_SECS_FOR_TICKS(wdt_get_remaining_ticks(wdt->regs));

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WDT_Stop(WDT_RESOURCES *wdt)
  \brief        Stop the watchdog timer.
  \param[in]    wdt  : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_Stop(WDT_RESOURCES *wdt)
{
    if (!wdt)
        return ARM_DRIVER_ERROR_PARAMETER;

    if ( !(wdt->flags & WATCHDOG_STARTED) )
        return ARM_DRIVER_ERROR;

    if (wdt_locked(wdt->regs))
        return ARM_DRIVER_ERROR_BUSY;

    wdt_stop(wdt->regs);

    wdt->flags &= ~WATCHDOG_STARTED;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WDT_PowerControl(ARM_POWER_STATE   state,
                                         WDT_RESOURCES     *wdt)
  \brief        watchdog power control
  \param[in]    state : Power state
  \param[in]    wdt   : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_PowerControl(ARM_POWER_STATE   state,
                                WDT_RESOURCES     *wdt)
{
    int ret = ARM_DRIVER_OK;

    switch (state)
    {
        case ARM_POWER_OFF:

            if ( (wdt->flags & WATCHDOG_STARTED) )
            {
                /* watchdog is still running,
                 * Stop the watchdog before Power-off.
                 */
                return ARM_DRIVER_ERROR_BUSY;
            }

            /* Reset watchdog power flag. */
            wdt->flags &= (~WATCHDOG_POWERED);
            break;

        case ARM_POWER_FULL:

            if ( !(wdt->flags & WATCHDOG_INITIALIZED) )
                return ARM_DRIVER_ERROR;

            /* Set the power flag enabled */
            wdt->flags |= WATCHDOG_POWERED;
            break;

        case ARM_POWER_LOW:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ret;
}

/**
  \fn           int32_t WDT_Initialize(uint32_t      timeout,
                                       WDT_RESOURCES *wdt)
  \brief        Initialize the watchdog device.
                The init function leaves the watchdog in a clean state:
                  - initialized;
                  - stop;
                  - locked.
  \param[in]    timeout  : Timeout in msec, must be > 0.
  \param[in]    wdt      : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_Initialize(uint32_t      timeout_msec,
                              WDT_RESOURCES *wdt)
{
    if (!wdt)
        return ARM_DRIVER_ERROR_PARAMETER;

    if ( (wdt->flags & WATCHDOG_STARTED) )
    {
        wdt_unlock(wdt->regs);

        /* Stop the watchdog */
        wdt_stop(wdt->regs);
        wdt->flags &= ~WATCHDOG_STARTED;
    }

    if (!timeout_msec)
        return ARM_DRIVER_ERROR_PARAMETER;

    if( timeout_msec > WDT_MAX_TIMEOUT_MILLI_SEC )
        return ARM_DRIVER_ERROR_PARAMETER;

    wdt_lock(wdt->regs);

    /* Update watchdog timeout. */
    wdt->timeout = WDT_TICKS_FOR_MILLI_SECS(timeout_msec);

    /* Set flag to initialized. */
    wdt->flags = WATCHDOG_INITIALIZED;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WDT_Uninitialize(WDT_RESOURCES *wdt)
  \brief        Uninitialize the watchdog device.
  \param[in]    wdt  : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_Uninitialize(WDT_RESOURCES *wdt)
{
    if ( (wdt->flags & WATCHDOG_STARTED) )
    {
        /* watchdog is still running,
         * Stop the watchdog before Uninitialize.
         */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* reset watchdog flags and timeout. */
    wdt->flags = 0;
    wdt->timeout = 0;

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t WDT_Control(uint32_t      control,
                                    uint32_t      arg,
                                    WDT_RESOURCES *wdt)
  \brief        CMSIS-Driver watchdog control.
                Control watchdog Interface.
  \param[in]    control : Operation
  \param[in]    arg     : Argument of operation (optional)
  \param[in]    wdt     : Pointer to watchdog device resources
  \return       \ref execution_status
*/
static int32_t WDT_Control(uint32_t      control,
                           uint32_t      arg,
                           WDT_RESOURCES *wdt)
{
    ARG_UNUSED(arg);
    int ret = ARM_DRIVER_OK;

    if( !(wdt->flags & WATCHDOG_POWERED ) )
        return ARM_DRIVER_ERROR;

    switch (control)
    {
        case ARM_WATCHDOG_LOCK:

            /* Lock watchdog. */
            wdt_lock(wdt->regs);
            break;

        case ARM_WATCHDOG_UNLOCK:

            /* Unlock watchdog. */
            wdt_unlock(wdt->regs);
            break;

        default:
            /* Unsupported command */
            ret =  ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
    }
    return ret;
}
// End of watchdog Interface


/* Watchdog-0  Driver Instance */
#if (RTE_WDT0)

/* Watchdog-0 device configuration */
static WDT_RESOURCES WDT0 = {
    .regs                     = (WDT_CTRL_Type *) LOCAL_WDT_CTRL_BASE,
    .flags                     = 0,
    .timeout                   = 0
};


/* Function Name: WDT0_Initialize */
static int32_t WDT0_Initialize(uint32_t timeout)
{
    return (WDT_Initialize(timeout, &WDT0));
}

/* Function Name: WDT0_Uninitialize */
static int32_t WDT0_Uninitialize(void)
{
    return (WDT_Uninitialize(&WDT0));
}

/* Function Name: WDT0_PowerControl */
static int32_t WDT0_PowerControl(ARM_POWER_STATE state)
{
    return (WDT_PowerControl(state, &WDT0));
}

/* Function Name: WDT0_Start */
static int32_t WDT0_Start(void)
{
    return (WDT_Start(&WDT0));
}

/* Function Name: WDT0_Feed */
static int32_t WDT0_Feed(void)
{
    return (WDT_Feed(&WDT0));
}

/* Function Name: WDT0_Stop */
static int32_t WDT0_Stop(void)
{
    return (WDT_Stop(&WDT0));
}

/* Function Name: WDT0_Control */
static int32_t WDT0_Control(uint32_t control, uint32_t arg)
{
    return (WDT_Control(control, arg, &WDT0));
}

/* Function Name: WDT0_GetRemainingTime */
static int32_t WDT0_GetRemainingTime(uint32_t *val)
{
    return (WDT_GetRemainingTime(val, &WDT0));
}


extern ARM_DRIVER_WDT Driver_WDT0;
ARM_DRIVER_WDT Driver_WDT0 =
{
    WDT_GetVersion,
    WDT_GetCapabilities,
    WDT0_Initialize,
    WDT0_Uninitialize,
    WDT0_PowerControl,
    WDT0_Start,
    WDT0_Feed,
    WDT0_Stop,
    WDT0_Control,
    WDT0_GetRemainingTime
};
#endif /* End of RTE_WDT0 */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
