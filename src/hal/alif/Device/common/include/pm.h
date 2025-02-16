/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*******************************************************************************
 * @file     pm.h
 * @author   Raj Ranjan
 * @email    raj.ranjan@alifsemi.com
 * @version  V1.0.0
 * @date     20-Feb-2023
 * @brief    Power Management Services API
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef PM_H_
#define PM_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "system_utils.h"


#ifdef  __cplusplus
extern "C"
{
#endif


/**
  @brief PM Return Status
 */
typedef enum _PM_STATUS
{
    PM_STATUS_OK,              /*!<  SUCCESS                 */
    PM_STATUS_UNSUPPORTED,     /*!<  Unsupported Sleep state */
    PM_STATUS_ERROR,           /*!<  ERROR                   */

    PM_STATUS_MAX = 0x7FFFFFFFUL
}PM_STATUS;

/**
  @brief enum of reset reasons

 * Note that if a subsystem powers itself down and restarts, then no
 * reset reason is indicated, as the reset was not triggered by the
 * central reset logic - this value is read from the central reset
 * controller.
 */
typedef enum _PM_RESET_STATUS
{
    PM_RESET_STATUS_POR_OR_SOC_OR_HOST_RESET = (1U << 0), /*!< Indicates that a
                                                         reset of the External
                                                         System was caused by
                                                         the POR (Reset because
                                                         of power on/off or SOC)
                                                         or Host reset i.e.
                                                         Secure Enclave       */
    PM_RESET_STATUS_NSRST_RESET              = (1U << 1), /*!< Indicates that a
                                                         last reset of the
                                                         External System was
                                                         caused by nSRST      */
    PM_RESET_STATUS_EXTERNAL_SYS_RESET       = (1U << 4), /*!< Indicates that a
                                                         reset of the External
                                                         System was caused by a
                                                         request to reset this
                                                         External System      */

    PM_RESET_STATUS_MAX = 0x7FFFFFFFUL
} PM_RESET_STATUS;

/**
 *******************************************************************************
 *                        Function documentation
 ******************************************************************************/

/**
  @fn       uint16_t pm_get_version(void)
  @brief    Get PM driver version.
  @return   uint16_t
*/
uint16_t pm_get_version(void);

/**
  @fn       uint16_t pm_core_enter_normal_sleep(void)
  @brief    Power management API which performs normal sleep operation
  @note     This function should be called with interrupts disabled.
  @note     This function is provided for consistency with the
            deeper sleeps, which require interrupts disabled.
            In interrupt-enabled context, in bare-metal use, direct
            use of __WFE() may be more appropriate.
  @return   This function return nothing
 */
static inline void pm_core_enter_normal_sleep(void)
{
    __WFI();
}

/**
  @fn       void pm_core_enter_deep_sleep(void)
  @brief    Power management API which performs deep sleep operation
  @note     This function should be called with interrupts disabled
            This enters the deepest possible CPU sleep state, without
            losing CPU state. All CPU clocks can be stopped, including
            SysTick. CPU and subsystem power will remain on, and the
            clock continues to run to the Internal Wakeup Interrupt
            Controller (IWIC), which manages the wakeup.
  @note     Possible IWIC wake sources are events, NMI, debug events
            and interrupts 0-63 only, subject to NVIC interrupt
            enable controls.
  @return   This function return nothing
 */
void pm_core_enter_deep_sleep(void);

/**
  @fn       void pm_core_enter_deep_sleep_request_subsys_off(void)
  @brief    Power management API which permits subsystem off operation
            This enters a deep sleep and indicates that it is okay for
            the CPU power, and hence potentially the entire subsystem's
            power, to be removed. Whether power actually is removed will
            depend on other factors - the CPU is not the only input
            to the decision.

            If a wake-up source is signaled before power is removed,
            the function returns from its deep sleep.

            If power is removed from the subsystem, the function does not
            return, and the CPU will reboot when/if the subsystem is next
            powered up, which could either be due to the local wakeup
            controller, or some other power-on request. Any wake-up sources will
            be indicated by a pending interrupt in the NVIC.

            As there are many reasons the subsystem could wake, applications
            should be written to call this again on reboot when they find there
            are no wake reasons.

            Where the system reboots from, can be controlled using the secure
            enclave APIs to set the initial vector table.

            The RTSS-HE core can arrange for some or all of its TCM to be
            retained when the power is turned off by making calls to the
            secure enclave to configure the retention power.

            The secure enclave can also arrange for various deep SoC sleep
            states to be entered if all subsystems have configured this, and they
            enter sleep. So this call can lead to overall SoC sleep.

  @note     This function should be called with interrupts disabled.
            A cache clean operation is performed if necessary.
  @note     This function will not return if the system goes off
            before a wake event. It will return if a wake event
            occurs before power off is possible.
  @note     Possible EWIC wake sources are a limited selection
            of interrupts 0-63 - see the HWRM for details.
            The CPU may also reboot if power is automatically
            applied to the subsystem for other reasons aside from
            EWIC wakeup.
            The pending information from EWIC is transferred
            into the NVIC on startup, so interrupt handlers
            can respond to the cause as soon as they're
            unmasked by drivers.
  @return   This function returns nothing, or causes reboot.
 */
void pm_core_enter_deep_sleep_request_subsys_off(void);

/**
  @fn       uint32_t pm_peek_subsystem reset_status(void)
  @brief    Peek reset status
            Returns the value of the current subsystem's reset status register,
            without clearing it. If pm_get_subsystem_reset_status() is not used
            to clear it, it may indicate previous resets.
  @return   Reset status return (ORred PM_RESET_STATUS values)
  */
uint32_t pm_peek_subsystem_reset_status(void);

/**
  @fn       uint32_t pm_get_subsystem reset_status(void)
  @brief    Get reset status
            Returns the value of the current subsystem's reset status register,
            and clears it. So if this call isn't made on every reset, it may
            indicate previous resets.
  @return   Reset status return (ORred PM_RESET_STATUS values)
*/
uint32_t pm_get_subsystem_reset_status(void);

/**
  @fn       bool pm_core_wakeup_is_spurious(void)
  @brief    Check if the wakeup reason is due to spurious wakeup
            RTSS domain can wake-up if any of the peripherals inside the
            domain are accessed from outside.

  @note     Should be called before pm_get_subsystem_reset_status()

  @note     Usage:

            if (pm_core_wakeup_is_spurious())
            {
                pm_core_enable_manual_pd_sequencing();
                pm_core_request_subsys_off_from_spurious_wakeup();
            }
            else
            {
                pm_core_enable_automatic_pd_sequencing();
            }

  @return   Returns false if the reason for wakeup is due to pending interrupt,
            else return true
*/
bool pm_core_wakeup_is_spurious(void);

/**
  @fn       void pm_core_enable_automatic_pd_sequencing(void)
  @brief    Enable automatic power sequence on entry to low-power state by EWIC
            If this is enabled, all the NVIC enabled status will be propagated
            to EWIC automatically.
  @note     At reset, this is enabled.
  @return   None
*/
void pm_core_enable_automatic_pd_sequencing(void);

/**
  @fn       void pm_core_enable_manual_pd_sequencing(void)
  @brief    Disable automatic sequence on entry to low-power state by EWIC.
            Application should take care of enabling the EWIC mask before entering
            to low-power state.
  @return   None
*/
void pm_core_enable_manual_pd_sequencing(void);

/**
  @fn       void pm_core_enable_automatic_pu_sequencing(void)
  @brief    Enable automatic sequence on power-up by EWIC
            If this is enabled, all the EWIC pending status will be propagated
            to NVIC automatically.
  @note     At reset, this is enabled.
  @return   None
*/
void pm_core_enable_automatic_pu_sequencing(void);

/**
  @fn       void pm_core_enable_manual_pu_sequencing(void)
  @brief    Disable automatic sequence on power-up by EWIC.
            Application should take care of verifying the pending EWIC status
            and act accordingly.
  @return   None
*/
void pm_core_enable_manual_pu_sequencing(void);

/**
  @fn       void pm_core_request_subsys_off_from_spurious_wakeup(void)
  @brief    Routine to go back to subsystem off from spurious wakeups

            This is essentially a subset of the function
            pm_core_enter_deep_sleep_request_subsys_off().

            This should be called in the very boot up state before enabling the
            caches.

            This should be part of the root_sections in the linker
  @return   None
*/
void pm_core_request_subsys_off_from_spurious_wakeup(void);

#ifdef  __cplusplus
}
#endif

#endif /* POWER_MANAGEMENT_H_ */
