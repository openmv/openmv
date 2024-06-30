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
 * @file     pm.c
 * @author   Kevin Bracey | Raj Ranjan
 * @email    kevin.bracey@alifsemi.com | raj.ranjan@alifsemi.com
 * @version  V1.0.0
 * @date     13-Feb-2023
 * @brief    Power Management Services
 * @bug      None.
 * @Note     None
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "pm.h"
#include "pm_internal.h"

#define VERSION(api, driver)        (((api) << 8) | (driver))
#define PM_DRV_VERSION              VERSION(1, 0) /*!< PM Driver Version */

/* WICCONTROL register : volatile static uint32_t *const WICCONTROL*/
#if   defined(M55_HP)
#define WICCONTROL                  (AON->RTSS_HP_CTRL)
#define RESET_STATUS_REG            (AON->RTSS_HP_RESET)
#elif defined(M55_HE)
#define WICCONTROL                  (AON->RTSS_HE_CTRL)
#define RESET_STATUS_REG            (AON->RTSS_HE_RESET)
#else
#error "Invalid CPU"
#endif

#if (defined (__FPU_USED) && (__FPU_USED == 1U)) || \
    (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
/* We only need to preserve APCS callee-preserved registers */
typedef struct fp_state {
    double      d[8];
    uint32_t    fpscr;
#if defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U)
    uint32_t    vpr;
#endif
} fp_state_t;

static inline void save_fp_state(fp_state_t *state)
{
    __asm (
        "VSTM    %0, {D8-D15}\n\t"
        "VSTR    FPSCR, [%0, #32]\n\t"
#if defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U)
        "VSTR    VPR, [%0, #36]\n\t"
#endif
        :: "r"(state) : "memory"
    );
}

static inline void restore_fp_state(const fp_state_t *state)
{
    __asm (
        "VLDM    %0, {D8-D15}\n\t"
        "VLDR    FPSCR, [%0, #32]\n\t"
#if defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U)
        "VLDR    VPR, [%0, #36]\n\t"
#endif
        :: "r"(state) : "memory"
    );
}
#endif

/**
  @fn           uint32_t pm_prepare_lpgpio_nvic_mask(void)
  @brief        Prepare NVIC mask for LPGPIO while going to subsystem off.
  @return       LPGPIO NVIC mask state
 */
static uint32_t pm_prepare_lpgpio_nvic_mask(void)
{
    uint32_t lpgpio_nvic_mask_state = 0;

    /*
     * LPGPIOs have to cause wakeup via a combined interrupt, as the
     * main interrupts are not sent to the EWIC/IWIC
     *
     * Enable it transparently.
     */

    /*
     * We could use NVIC_GetEnableIRQ eight times, but would mean
     * 8 register reads, instead peek directly to ISER.
     *
     *
     * Read 0-7 interrupt enables depending on alignment of the interrupt numbers
     */

    uint32_t lpgpio_enables = NVIC->ISER[LPGPIO_IRQ0_IRQn >> 5]
                                        >> (LPGPIO_IRQ0_IRQn & 0x1F);

    /* If split across two registers, combine enables from the second register */
    if ((LPGPIO_IRQ0_IRQn & 0x1F) > 24)
    {
        lpgpio_enables |= NVIC->ISER[(LPGPIO_IRQ0_IRQn >> 5) + 1]
                                     << (32 - (LPGPIO_IRQ0_IRQn & 0x1F));
    }

    lpgpio_enables &= 0xFF;

    /*
     * If any LPGIO is enabled, and the combined interrupt isn't,
     * activate the combined one, and we'll restore it later
     */
    if (lpgpio_enables && !NVIC_GetEnableIRQ(LPGPIO_COMB_IRQ_IRQn))
    {
        NVIC_ClearPendingIRQ(LPGPIO_COMB_IRQ_IRQn);
        NVIC_EnableIRQ(LPGPIO_COMB_IRQ_IRQn);
        lpgpio_nvic_mask_state |= 1;
    }

    return lpgpio_nvic_mask_state;
}

/**
  @fn           void pm_restore_lpgpio_nvic_mask(uint32_t lpgpio_nvic_mask_state)
  @brief        If the LPGIO NVIC mask is set, disable the Combined Interrupt
  @param[in]    lpgpio_nvic_mask_state LPGPIO NVIC mask state
  @return       none
 */
static void pm_restore_lpgpio_nvic_mask(uint32_t lpgpio_nvic_mask_state)
{
    if (lpgpio_nvic_mask_state & 1)
    {
        NVIC_DisableIRQ(LPGPIO_COMB_IRQ_IRQn);
    }
}

/**
  @fn       uint16_t pm_get_version(void)
  @brief    Get PM driver version.
  @return   uint16_t
*/
uint16_t pm_get_version(void)
{
  return PM_DRV_VERSION;
}

/**
  @fn           void pm_core_enter_wic_sleep(PM_WIC wic)
  @brief        Enter deep WIC-based sleep subroutine
  @param[in]    wic 1 for IWIC sleep, 0 for EWIC.
  @return       This function returns nothing, potentially causes power-down.
 */
static void pm_core_enter_wic_sleep(PM_WIC wic)
{
    uint32_t lpgpio_nvic_mask_state;
    /*
     * See if we have any LPGPIO individual interrupts are enabled.
     * If yes, enable the Combined interrupt.
     */
    lpgpio_nvic_mask_state = pm_prepare_lpgpio_nvic_mask();

    /* Set up WICCONTROL so that deep sleep is the required WIC sleep type */
    WICCONTROL = _VAL2FLD(WICCONTROL_WIC, 1) | _VAL2FLD(WICCONTROL_IWIC, wic);

    /* Setting DEEPSLEEP bit */
    SCB->SCR       |=  SCB_SCR_SLEEPDEEP_Msk;

    /*Data Synchronization Barrier completes all instructions before this */
    __DSB();

    /* Instruction Synchronization Barrier flushes the pipeline in the
     * processor, so that all instructions following the ISB are fetched from
     * cache or memory */
    __ISB();

    /* Put System into sleep mode */
    pm_core_enter_normal_sleep();

    /* Clearing DEEPSLEEP bit */
    SCB->SCR       &=  ~SCB_SCR_SLEEPDEEP_Msk;

    /* Clear WICCONTROL to disable WIC sleep */
    WICCONTROL = _VAL2FLD(WICCONTROL_WIC, 0);


    /* Data Synchronization Barrier completes all instructions before this */
    __DSB();

    /* Instruction Synchronization Barrier flushes the pipeline in the
     *  processor, so that all instructions following the ISB are fetched
     *  from cache or memory */
    __ISB();

    /*
     * If we enabled the LPGPIO combined interrupt while going to deep sleep,
     * disable it.
     */
    pm_restore_lpgpio_nvic_mask(lpgpio_nvic_mask_state);
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
void pm_core_enter_deep_sleep(void)
{
    /* Entering any WIC sleep could potentially cause state loss,
     * as it enables power saving on PDCORE. Unlike the other domains,
     * there is no separate mechanism to indicate "retention" beyond
     * setting CLPSTATE, so as we want a sleep call we need to
     * ensure CLPSTATE is RET or higher. (See M55 TRM section 7.5)
     *
     * Further, in our design, the IWIC is actually in the same power domain
     * as the CPU, so we need the IWIC+CPU to stay on to be able to wake,
     * which means our minimum is actually ON with clock off.
     *
     * But don't lower the entry value of CLPSTATE - caller may have a reason
     * to suppress low-power states.
     */
    uint32_t old_cpdlpstate = PWRMODCTL->CPDLPSTATE;

    if (_FLD2VAL(PWRMODCTL_CPDLPSTATE_CLPSTATE, old_cpdlpstate) > PM_LPSTATE_ON_CLK_OFF) {
        PWRMODCTL->CPDLPSTATE = (old_cpdlpstate &~ PWRMODCTL_CPDLPSTATE_CLPSTATE_Msk) |
                                _VAL2FLD(PWRMODCTL_CPDLPSTATE_CLPSTATE, PM_LPSTATE_ON_CLK_OFF);
    }

    /* Trigger the IWIC sleep */
    pm_core_enter_wic_sleep(PM_WIC_IS_IWIC);

    /* Restore low power state (probably to all OFF) */
    PWRMODCTL->CPDLPSTATE = old_cpdlpstate;
}

/**
  @fn       void pm_core_enter_deep_sleep_request_subsys_off(void)
  @brief    Power management API which performs subsystem off operation
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
void pm_core_enter_deep_sleep_request_subsys_off(void)
{
    uint32_t orig_ccr, orig_mscr, orig_demcr, orig_cppwr;
#if (defined (__FPU_USED) && (__FPU_USED == 1U)) || \
    (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    fp_state_t fp_state;
    bool fp_saved = false;
#endif

    /* We attempt to power off the subsystem by turning off all active
     * indications from the CPU, taking its power domains PDCORE, PDEPU,
     * PDRAMS and PDDEBUG to OFF. See Power chapter of M55 TRM for details.
     *
     * We assume all the LPSTATE indications are OFF as at boot, which will
     * permit everything to go off. We assume that if it's set higher, it's
     * because someone wants to block this. If they have modified it, and
     * don't intend to block this, they should put it back to OFF before
     * calling this.
     */
    /* PDEPU OFF requires that we set the State Unknown 10 flag indicating it's
     * okay to forget the FP/MVE state (S/D/Q registers, FPSR and VPR)
     */
    orig_cppwr = ICB->CPPWR;
#if (defined (__FPU_USED) && (__FPU_USED == 1U)) || \
    (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    if (!(orig_cppwr & ICB_CPPWR_SU10_Msk)) {
        /* As we are going to say it's okay to lose EPU state, we should save it;
         * we can't independently turn EPU off on our silicon, but the CPU
         * could choose to reset the registers in response to SU10.
         */

        /* Only need to save if we have our own floating point context active,
         * or lazy stack preservation is active, indicating registers hold
         * another context's state.
         */
        if ((__get_CONTROL() & CONTROL_FPCA_Msk) ||
                (FPU->FPCCR & FPU_FPCCR_LSPACT_Msk)) {
            save_fp_state(&fp_state);
            fp_saved = true;
        }
    }
#endif

    if (!(orig_cppwr & ICB_CPPWR_SU10_Msk)) {
        /* Indicate we're okay to lose MVE/FP state. Note that MVE/FP instructions
         * will fault after this, so we hope we're not doing anything that
         * prompts the compiler to generate MVE/FP code during this function.
         */
        ICB->CPPWR = orig_cppwr | (ICB_CPPWR_SU11_Msk | ICB_CPPWR_SU10_Msk);
    }

    /* Stop new data cache allocations */
    orig_ccr = SCB->CCR;
    SCB->CCR = orig_ccr & (~SCB_CCR_DC_Msk);
    __DSB();
    __ISB();

    /* Check cache status */
    orig_mscr = MEMSYSCTL->MSCR;
    if (orig_mscr & MEMSYSCTL_MSCR_DCACTIVE_Msk)
    {
        /* Make sure nothing gets dirty any more - this should stabilize DCCLEAN */
        MEMSYSCTL->MSCR = orig_mscr | MEMSYSCTL_MSCR_FORCEWT_Msk;
        __DSB();
        __ISB();

        if (!(MEMSYSCTL->MSCR & MEMSYSCTL_MSCR_DCCLEAN_Msk))
        {
            /* Clean if data cache is active, and not known to be clean. This
             * could be done earlier by pm_shut_down_dcache, before disabling
             * IRQs. But if we're making this call, we're resigned to bad
             * interrupt latency - we might be needing a full reboot to
             * respond.
             */
            SCB_CleanDCache();

            /* Should be good to manually mark clean now - M55 TRM tells us not to,
             * but after some discussion with Arm, I believe we've taken enough care
             * that this is valid at this point.
             * (If the cache ISN'T clean, then we've failed on the shutdown).
             */
            MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_DCCLEAN_Msk;
        }
    }

    /* Fully disable the caches, allowing PDRAMS OFF. (On B1 silicon it
     * is important that we don't let the M55 request MEM_RET state by having
     * PDRAMS at RET and PDCORE at OFF - the PPU will grant this,
     * and the M55 will wrongly think its cache has been retained, and skip
     * necessary auto-invalidation on the subsequent reset.) Restore FORCEWT now.
     */
    SCB->CCR = orig_ccr & ~(SCB_CCR_IC_Msk | SCB_CCR_DC_Msk);
    MEMSYSCTL->MSCR = (MEMSYSCTL->MSCR &~
                      (MEMSYSCTL_MSCR_ICACTIVE_Msk  |
                       MEMSYSCTL_MSCR_DCACTIVE_Msk  |
                       MEMSYSCTL_MSCR_FORCEWT_Msk)) |
                      (orig_mscr & MEMSYSCTL_MSCR_FORCEWT_Msk);
    /* Disable PMU/DWT - we know this is enabled at boot by system code using
     * PMU timers, so we could never permit PDDEBUG OFF otherwise.
     * When/if this is resolved, we should consider removing this, so
     * as not to interfere with deliberate debugging.
     */
    orig_demcr = DCB->DEMCR;
    DCB->DEMCR = orig_demcr & ~DCB_DEMCR_TRCENA_Msk;

    /* Assume automatic EWIC sequencing - NVIC masks transferred and EWIC
     * enabled by M55.
     */

    /* Trigger the EWIC sleep - may or may not return*/
    pm_core_enter_wic_sleep(PM_WIC_IS_EWIC);

    /* If we return, restore enables */
    MEMSYSCTL->MSCR |= orig_mscr &
                       (MEMSYSCTL_MSCR_ICACTIVE_Msk | MEMSYSCTL_MSCR_DCACTIVE_Msk);
    SCB->CCR = orig_ccr;
    DCB->DEMCR = orig_demcr;
    ICB->CPPWR = orig_cppwr;

    /* Make sure enables are synchronized */
    __DSB();
    __ISB();

#if (defined (__FPU_USED) && (__FPU_USED == 1U)) || \
    (defined (__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    /* Restore FP/MVE state */
    if (fp_saved) {
        restore_fp_state(&fp_state);
    }
#endif
}

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
void pm_core_request_subsys_off_from_spurious_wakeup(void)
{
    uint32_t orig_mscr, orig_cppwr;
    /* We attempt to power off the subsystem by turning off all active
     * indications from the CPU, taking its power domains PDCORE, PDEPU,
     * PDRAMS and PDDEBUG to OFF. See Power chapter of M55 TRM for details.
     *
     * We assume all the LPSTATE indications are OFF as at boot, which will
     * permit everything to go off. We assume that if it's set higher, it's
     * because someone wants to block this. If they have modified it, and
     * don't intend to block this, they should put it back to OFF before
     * calling this.
     */

    /* PDEPU OFF requires that we set the State Unknown 10 flag indicating it's
     * okay to forget the FP/MVE state (S/D/Q registers, FPSR and VPR)
     */
    orig_cppwr = ICB->CPPWR;
    if (!(orig_cppwr & ICB_CPPWR_SU10_Msk)) {
        /* Indicate we're okay to lose MVE/FP state. Note that MVE/FP instructions
         * will fault after this, so we hope we're not doing anything that
         * prompts the compiler to generate MVE/FP code during this function.
         */
        ICB->CPPWR = orig_cppwr | (ICB_CPPWR_SU11_Msk | ICB_CPPWR_SU10_Msk);
    }

    /* Check cache status */
    orig_mscr = MEMSYSCTL->MSCR;
    /*
     * Make Caches Inactive. Restore FORCEWT now.
     */
    MEMSYSCTL->MSCR = (MEMSYSCTL->MSCR &~
                      (MEMSYSCTL_MSCR_ICACTIVE_Msk  |
                       MEMSYSCTL_MSCR_DCACTIVE_Msk  |
                       MEMSYSCTL_MSCR_FORCEWT_Msk)) |
                      (orig_mscr & MEMSYSCTL_MSCR_FORCEWT_Msk);

    /* Trigger the EWIC sleep - may or may not return*/
    /* Set up WICCONTROL so that deep sleep is the required WIC sleep type */
    WICCONTROL = _VAL2FLD(WICCONTROL_WIC, 1) | _VAL2FLD(WICCONTROL_IWIC, PM_WIC_IS_EWIC);

    /* Setting DEEPSLEEP bit */
    SCB->SCR       |=  SCB_SCR_SLEEPDEEP_Msk;

    /*Data Synchronization Barrier completes all instructions before this */
    __DSB();

    /* Instruction Synchronization Barrier flushes the pipeline in the
     * processor, so that all instructions following the ISB are fetched from
     * cache or memory */
    __ISB();

    /* Put System into sleep mode */
    pm_core_enter_normal_sleep();

    /* Clearing DEEPSLEEP bit */
    SCB->SCR       &=  ~SCB_SCR_SLEEPDEEP_Msk;

    /* Clear WICCONTROL to disable WIC sleep */
    WICCONTROL = _VAL2FLD(WICCONTROL_WIC, 0);

    /* If we return, restore enables */
    MEMSYSCTL->MSCR |= orig_mscr &
                       (MEMSYSCTL_MSCR_ICACTIVE_Msk | MEMSYSCTL_MSCR_DCACTIVE_Msk);

    ICB->CPPWR = orig_cppwr;

    /* Make sure enables are synchronized */
    __DSB();
    __ISB();
}

/**
  @fn       uint32_t pm_peek_subsystem reset_status(void)
  @brief    Peek reset status
            Returns the value of the current subsystem's reset status register,
            without clearing it. If pm_get_subsystem_reset_status() is not used
            to clear it, it may indicate previous resets.
  @return   Reset status return (ORred PM_RESET_STATUS values)
  */
uint32_t pm_peek_subsystem_reset_status(void)
{
    /* Read the set bits */
    return RESET_STATUS_REG;
}

/**
  @fn       uint32_t pm_get_subsystem reset_status(void)
  @brief    Get reset status
            Returns the value of the current subsystem's reset status register,
            and clears it. So if this call isn't made on every reset, it may
            indicate previous resets.
  @return   Reset status return (ORred PM_RESET_STATUS values) */
uint32_t pm_get_subsystem_reset_status(void)
{
    /* Read the set bits */
    uint32_t reason = RESET_STATUS_REG;

    /* Write them back to acknowledge what we read */
    RESET_STATUS_REG = reason;

    return reason;
}

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
bool pm_core_wakeup_is_spurious(void)
{
    uint16_t num_events, count = 0;

    /* Read the last reason to confirm that it is not spurious */
    if (RESET_STATUS_REG != 0)
    {
        return false;
    }

    /*
     * Check if the WIC is enabled at this point.
     * If it is not enabled, then the cause of wakeup must be NVIC_SystemReset
     * or local WDT Reset. Both are not spurious ones. So return false
     */
    if (!(WICCONTROL & WICCONTROL_WIC_Msk))
    {
        return false;
    }

    /*
     * Compare the NVIC pending status and the EWIC Mask.
     * If any of the interrupt is in pending state, return false.
     */
    num_events = ((_EWIC->EWIC_NUMID - 3) + 31) / 32;
    for (count = 0; count < num_events; count++)
    {
        if (_EWIC->EWIC_MASKn[count]
                              && (NVIC->ISPR[count] & _EWIC->EWIC_MASKn[count]))
            return false;
    }

    /*
     * Now we have confirmed that no interrupt is pending and we have woken up
     * for unknown reason.
     */
    return true;

}

/**
  @fn       void pm_core_enable_automatic_pd_sequencing(void)
  @brief    Enable automatic power sequence on entry to low-power state by EWIC
            If this is enabled, all the NVIC enabled status will be propagated
            to EWIC automatically.
  @note     At reset, this is enabled.
  @return   None
*/
void pm_core_enable_automatic_pd_sequencing(void)
{
    _EWIC->EWIC_ASCR |= EWIC_EWIC_ASCR_ASPD_Msk;
}

/**
  @fn       void pm_core_enable_manual_pd_sequencing(void)
  @brief    Disable automatic sequence on entry to low-power state by EWIC.
            Application should take care of enabling the EWIC mask before entering
            to low-power state.
  @return   None
*/
void pm_core_enable_manual_pd_sequencing(void)
{
    uint16_t num_events, count = 0;

    _EWIC->EWIC_ASCR &= ~EWIC_EWIC_ASCR_ASPD_Msk;
    _EWIC->EWIC_CR |= EWIC_EWIC_CR_EN_Msk;

    /*
     * Copy NVIC pending registers to EWIC pending regs
     */
    num_events = ((_EWIC->EWIC_NUMID - 3) + 31) / 32;
    for (count = 0; count < num_events; count++)
    {
        _EWIC->EWIC_PENDn[count] = NVIC->ISPR[count];
    }
}

/**
  @fn       void pm_core_enable_automatic_pu_sequencing(void)
  @brief    Enable automatic sequence on power-up by EWIC
            If this is enabled, all the EWIC pending status will be propagated
            to NVIC automatically.
  @note     At reset, this is enabled.
  @return   None
*/
void pm_core_enable_automatic_pu_sequencing(void)
{
    _EWIC->EWIC_ASCR |= EWIC_EWIC_ASCR_ASPU_Msk;
}

/**
  @fn       void pm_core_enable_manual_pu_sequencing(void)
  @brief    Disable automatic sequence on power-up by EWIC.
            Application should take care of verifying the pending EWIC status
            and act accordingly.
  @return   None
*/
void pm_core_enable_manual_pu_sequencing(void)
{
    _EWIC->EWIC_ASCR &= ~EWIC_EWIC_ASCR_ASPU_Msk;
    _EWIC->EWIC_CR |= EWIC_EWIC_CR_EN_Msk;
}

#if PM_HANDLE_SPURIOUS_WAKEUP
/**
  @fn       void System_HandleSpuriousWakeup(void)
  @brief    Handle the spurious wakeup
            See if the core booted due to spurious wakeup.
            If yes, try requesting subsystem_off.
  @return   None
*/
void System_HandleSpuriousWakeup(void)
{
    if (pm_core_wakeup_is_spurious())
    {
        pm_core_enable_manual_pd_sequencing();
        pm_core_request_subsys_off_from_spurious_wakeup();
    }
    else
    {
        pm_core_enable_automatic_pd_sequencing();
    }

}
#endif /* PM_HANDLE_SPURIOUS_WAKEUP */
