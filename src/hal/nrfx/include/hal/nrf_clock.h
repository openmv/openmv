/*
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NRF_CLOCK_H__
#define NRF_CLOCK_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_clock_hal Clock HAL
 * @{
 * @ingroup nrf_clock
 * @brief   Hardware access layer for managing the CLOCK peripheral.
 *
 * This code can be used to managing low-frequency clock (LFCLK) and the high-frequency clock
 * (HFCLK) settings.
 */

#if defined(CLOCK_LFCLKSRC_BYPASS_Msk) && defined(CLOCK_LFCLKSRC_EXTERNAL_Msk)
// Enable support for external LFCLK sources. Read more in the Product Specification.
#define NRF_CLOCK_USE_EXTERNAL_LFCLK_SOURCES
#endif

#if defined(CLOCK_CTIV_CTIV_Msk) || defined(__NRFX_DOXYGEN__)
/**
 * @brief Presence of the Low Frequency Clock calibration.
 *
 * In some MCUs there is possibility to use LFCLK calibration.
 */
#define NRF_CLOCK_HAS_CALIBRATION 1
#else
#define NRF_CLOCK_HAS_CALIBRATION 0
#endif // defined(CLOCK_CTIV_CTIV_Msk) || defined(__NRFX_DOXYGEN__)

/**
 * @brief Low-frequency clock sources.
 * @details Used by LFCLKSRC, LFCLKSTAT, and LFCLKSRCCOPY registers.
 */
typedef enum
{
#if defined(CLOCK_LFCLKSRC_SRC_RC) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_LFCLK_RC    = CLOCK_LFCLKSRC_SRC_RC,    /**< Internal 32 kHz RC oscillator. */
#else
    NRF_CLOCK_LFCLK_RC    = CLOCK_LFCLKSRC_SRC_LFRC,  /**< Internal 32 kHz RC oscillator. */
#endif

#if defined(CLOCK_LFCLKSRC_SRC_Xtal) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_LFCLK_Xtal  = CLOCK_LFCLKSRC_SRC_Xtal,  /**< External 32 kHz crystal. */
#else
    NRF_CLOCK_LFCLK_Xtal  = CLOCK_LFCLKSRC_SRC_LFXO,  /**< External 32 kHz crystal. */
#endif

#if defined(CLOCK_LFCLKSRC_SRC_Synth) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_LFCLK_Synth = CLOCK_LFCLKSRC_SRC_Synth, /**< Internal 32 kHz synthesizer from HFCLK system clock. */
#endif
#if defined(NRF_CLOCK_USE_EXTERNAL_LFCLK_SOURCES) || defined(__NRFX_DOXYGEN__)
    /**
     * External 32 kHz low swing signal. Used only with the LFCLKSRC register.
     * For the others @ref NRF_CLOCK_LFCLK_Xtal is returned for this setting.
     */
    NRF_CLOCK_LFCLK_Xtal_Low_Swing = (CLOCK_LFCLKSRC_SRC_Xtal |
        (CLOCK_LFCLKSRC_EXTERNAL_Enabled << CLOCK_LFCLKSRC_EXTERNAL_Pos)),
    /**
     * External 32 kHz full swing signal. Used only with the LFCLKSRC register.
     * For the others @ref NRF_CLOCK_LFCLK_Xtal is returned for this setting.
     */
    NRF_CLOCK_LFCLK_Xtal_Full_Swing = (CLOCK_LFCLKSRC_SRC_Xtal |
        (CLOCK_LFCLKSRC_BYPASS_Enabled   << CLOCK_LFCLKSRC_BYPASS_Pos) |
        (CLOCK_LFCLKSRC_EXTERNAL_Enabled << CLOCK_LFCLKSRC_EXTERNAL_Pos)),
#endif // defined(NRF_CLOCK_USE_EXTERNAL_LFCLK_SOURCES) || defined(__NRFX_DOXYGEN__)
} nrf_clock_lfclk_t;

/** @brief High-frequency clock sources. */
typedef enum
{
#if defined(CLOCK_HFCLKSTAT_SRC_RC) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_HFCLK_LOW_ACCURACY  = CLOCK_HFCLKSTAT_SRC_RC,  /**< Internal 16 MHz RC oscillator. */
#endif
#if defined(CLOCK_HFCLKSTAT_SRC_Xtal) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_HFCLK_HIGH_ACCURACY = CLOCK_HFCLKSTAT_SRC_Xtal /**< External 16 MHz/32 MHz crystal oscillator. */
#else
    NRF_CLOCK_HFCLK_HIGH_ACCURACY = CLOCK_HFCLKSTAT_SRC_HFXO /**< External 32 MHz crystal oscillator. */
#endif
} nrf_clock_hfclk_t;

/**
 * @brief Trigger status of task LFCLKSTART/HFCLKSTART.
 * @details Used by LFCLKRUN and HFCLKRUN registers.
 */
typedef enum
{
    NRF_CLOCK_START_TASK_NOT_TRIGGERED = CLOCK_LFCLKRUN_STATUS_NotTriggered, /**< Task LFCLKSTART/HFCLKSTART has not been triggered. */
    NRF_CLOCK_START_TASK_TRIGGERED     = CLOCK_LFCLKRUN_STATUS_Triggered     /**< Task LFCLKSTART/HFCLKSTART has been triggered. */
} nrf_clock_start_task_status_t;

/** @brief Interrupts. */
typedef enum
{
    NRF_CLOCK_INT_HF_STARTED_MASK = CLOCK_INTENSET_HFCLKSTARTED_Msk, /**< Interrupt on HFCLKSTARTED event. */
    NRF_CLOCK_INT_LF_STARTED_MASK = CLOCK_INTENSET_LFCLKSTARTED_Msk, /**< Interrupt on LFCLKSTARTED event. */
#if (NRF_CLOCK_HAS_CALIBRATION) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_INT_DONE_MASK       = CLOCK_INTENSET_DONE_Msk,         /**< Interrupt on DONE event. */
    NRF_CLOCK_INT_CTTO_MASK       = CLOCK_INTENSET_CTTO_Msk,         /**< Interrupt on CTTO event. */
#endif
#if defined(CLOCK_INTENSET_CTSTARTED_Msk) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_INT_CTSTARTED_MASK  = CLOCK_INTENSET_CTSTARTED_Msk,    /**< Interrupt on CTSTARTED event. */
    NRF_CLOCK_INT_CTSTOPPED_MASK  = CLOCK_INTENSET_CTSTOPPED_Msk     /**< Interrupt on CTSTOPPED event. */
#endif
} nrf_clock_int_mask_t;

/**
 * @brief Tasks.
 *
 * @details The NRF_CLOCK_TASK_LFCLKSTOP task cannot be set when the low-frequency clock is not running.
 * The NRF_CLOCK_TASK_HFCLKSTOP task cannot be set when the high-frequency clock is not running.
 */
typedef enum
{
    NRF_CLOCK_TASK_HFCLKSTART = offsetof(NRF_CLOCK_Type, TASKS_HFCLKSTART), /**< Start HFCLK clock source.*/
    NRF_CLOCK_TASK_HFCLKSTOP  = offsetof(NRF_CLOCK_Type, TASKS_HFCLKSTOP),  /**< Stop HFCLK clock source.*/
    NRF_CLOCK_TASK_LFCLKSTART = offsetof(NRF_CLOCK_Type, TASKS_LFCLKSTART), /**< Start LFCLK clock source.*/
    NRF_CLOCK_TASK_LFCLKSTOP  = offsetof(NRF_CLOCK_Type, TASKS_LFCLKSTOP),  /**< Stop LFCLK clock source.*/
#if (NRF_CLOCK_HAS_CALIBRATION) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_TASK_CAL        = offsetof(NRF_CLOCK_Type, TASKS_CAL),        /**< Start calibration of LFCLK RC oscillator.*/
    NRF_CLOCK_TASK_CTSTART    = offsetof(NRF_CLOCK_Type, TASKS_CTSTART),    /**< Start calibration timer.*/
    NRF_CLOCK_TASK_CTSTOP     = offsetof(NRF_CLOCK_Type, TASKS_CTSTOP)      /**< Stop calibration timer.*/
#endif
} nrf_clock_task_t;

/** @brief Events. */
typedef enum
{
    NRF_CLOCK_EVENT_HFCLKSTARTED = offsetof(NRF_CLOCK_Type, EVENTS_HFCLKSTARTED), /**< HFCLK oscillator started.*/
    NRF_CLOCK_EVENT_LFCLKSTARTED = offsetof(NRF_CLOCK_Type, EVENTS_LFCLKSTARTED), /**< LFCLK oscillator started.*/
#if (NRF_CLOCK_HAS_CALIBRATION) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_EVENT_DONE         = offsetof(NRF_CLOCK_Type, EVENTS_DONE),         /**< Calibration of LFCLK RC oscillator completed.*/
    NRF_CLOCK_EVENT_CTTO         = offsetof(NRF_CLOCK_Type, EVENTS_CTTO),         /**< Calibration timer time-out.*/
#endif
#if defined(CLOCK_INTENSET_CTSTARTED_Msk) || defined(__NRFX_DOXYGEN__)
    NRF_CLOCK_EVENT_CTSTARTED    = offsetof(NRF_CLOCK_Type, EVENTS_CTSTARTED),    /**< Calibration timer started.*/
    NRF_CLOCK_EVENT_CTSTOPPED    = offsetof(NRF_CLOCK_Type, EVENTS_CTSTOPPED)     /**< Calibration timer stopped.*/
#endif
} nrf_clock_event_t;

/**
 * @brief Function for enabling the specified interrupt.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
NRF_STATIC_INLINE void nrf_clock_int_enable(NRF_CLOCK_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling the specified interrupt.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
NRF_STATIC_INLINE void nrf_clock_int_disable(NRF_CLOCK_Type * p_reg, uint32_t mask);

/**
 * @brief Function for checking if the specified interrupts are enabled.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be checked.
 *
 * @return Mask of enabled interrupts.
 */
NRF_STATIC_INLINE uint32_t nrf_clock_int_enable_check(NRF_CLOCK_Type const * p_reg, uint32_t mask);

/**
 * @brief Function for retrieving the address of the specified task.
 * @details This function can be used by the PPI module.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  CLOCK Task.
 *
 * @return Address of the requested task register.
 */
NRF_STATIC_INLINE uint32_t nrf_clock_task_address_get(NRF_CLOCK_Type const * p_reg,
                                                      nrf_clock_task_t       task);

/**
 * @brief Function for setting the specified task.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  Task to be activated.
 */
NRF_STATIC_INLINE void nrf_clock_task_trigger(NRF_CLOCK_Type * p_reg, nrf_clock_task_t task);

/**
 * @brief Function for retrieving the address of the specified event.
 * @details This function can be used by the PPI module.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event CLOCK Event.
 *
 * @return Address of the specified event register.
 */
NRF_STATIC_INLINE uint32_t nrf_clock_event_address_get(NRF_CLOCK_Type const * p_reg,
                                                       nrf_clock_event_t      event);

/**
 * @brief Function for clearing the specified event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to clear.
 */
NRF_STATIC_INLINE void nrf_clock_event_clear(NRF_CLOCK_Type * p_reg, nrf_clock_event_t event);

/**
 * @brief Function for retrieving the state of the specified event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to be checked.
 *
 * @retval true  The event has been generated.
 * @retval false The event has not been generated.
 */
NRF_STATIC_INLINE bool nrf_clock_event_check(NRF_CLOCK_Type const * p_reg, nrf_clock_event_t event);

/**
 * @brief Function for changing the low-frequency clock source.
 * @details This function cannot be called when the low-frequency clock is running.
 *
 * @param[in] p_reg  Pointer to the structure of registers of the peripheral.
 * @param[in] source New low-frequency clock source.
 */
NRF_STATIC_INLINE void nrf_clock_lf_src_set(NRF_CLOCK_Type * p_reg, nrf_clock_lfclk_t source);

/**
 * @brief Function for retrieving the selected source for the low-frequency clock.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval NRF_CLOCK_LFCLK_RC    The internal 32 kHz RC oscillator
 *                               is the selected source for the low-frequency clock.
 * @retval NRF_CLOCK_LFCLK_Xtal  An external 32 kHz crystal oscillator
 *                               is the selected source for the low-frequency clock.
 * @retval NRF_CLOCK_LFCLK_Synth The internal 32 kHz synthesizer from
 *                               the HFCLK is the selected source for the low-frequency clock.
 */
NRF_STATIC_INLINE nrf_clock_lfclk_t nrf_clock_lf_src_get(NRF_CLOCK_Type const * p_reg);

/**
 * @brief Function for retrieving the active source of the low-frequency clock.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval NRF_CLOCK_LFCLK_RC    The internal 32 kHz RC oscillator
 *                               is the active source of the low-frequency clock.
 * @retval NRF_CLOCK_LFCLK_Xtal  An external 32 kHz crystal oscillator
 *                               is the active source of the low-frequency clock.
 * @retval NRF_CLOCK_LFCLK_Synth The internal 32 kHz synthesizer from
 *                               the HFCLK is the active source of the low-frequency clock.
 */
NRF_STATIC_INLINE nrf_clock_lfclk_t nrf_clock_lf_actv_src_get(NRF_CLOCK_Type const * p_reg);

/**
 * @brief Function for retrieving the clock source for the LFCLK clock when
 *        the task LKCLKSTART is triggered.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval NRF_CLOCK_LFCLK_RC    The internal 32 kHz RC oscillator
 *                               is running and generating the LFCLK clock.
 * @retval NRF_CLOCK_LFCLK_Xtal  An external 32 kHz crystal oscillator
 *                               is running and generating the LFCLK clock.
 * @retval NRF_CLOCK_LFCLK_Synth The internal 32 kHz synthesizer from
 *                               the HFCLK is running and generating the LFCLK clock.
 */
NRF_STATIC_INLINE nrf_clock_lfclk_t nrf_clock_lf_srccopy_get(NRF_CLOCK_Type const * p_reg);

/**
 * @brief Function for retrieving the state of the LFCLK clock.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval false The LFCLK clock is not running.
 * @retval true  The LFCLK clock is running.
 */
NRF_STATIC_INLINE bool nrf_clock_lf_is_running(NRF_CLOCK_Type const * p_reg);

/**
 * @brief Function for retrieving the trigger status of the task LFCLKSTART.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval NRF_CLOCK_START_TASK_NOT_TRIGGERED The task LFCLKSTART has not been triggered.
 * @retval NRF_CLOCK_START_TASK_TRIGGERED     The task LFCLKSTART has been triggered.
 */
NRF_STATIC_INLINE
nrf_clock_start_task_status_t nrf_clock_lf_start_task_status_get(NRF_CLOCK_Type const * p_reg);

/**
 * @brief Function for retrieving the active source of the high-frequency clock.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval NRF_CLOCK_HFCLK_LOW_ACCURACY  The internal RC oscillator is the active
 *                                       source of the high-frequency clock.
 * @retval NRF_CLOCK_HFCLK_HIGH_ACCURACY An external crystal oscillator is the active
 *                                       source of the high-frequency clock.
 */
NRF_STATIC_INLINE nrf_clock_hfclk_t nrf_clock_hf_src_get(NRF_CLOCK_Type const * p_reg);

/**
 * @brief Function for retrieving the state of the HFCLK clock.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] clk_src Clock source to be checked.
 *
 * @retval false The HFCLK clock is not running.
 * @retval true  The HFCLK clock is running.
 */
NRF_STATIC_INLINE bool nrf_clock_hf_is_running(NRF_CLOCK_Type const * p_reg,
                                               nrf_clock_hfclk_t      clk_src);

/**
 * @brief Function for retrieving the trigger status of the task HFCLKSTART.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @retval NRF_CLOCK_START_TASK_NOT_TRIGGERED The task HFCLKSTART has not been triggered.
 * @retval NRF_CLOCK_START_TASK_TRIGGERED     The task HFCLKSTART has been triggered.
 */
NRF_STATIC_INLINE
nrf_clock_start_task_status_t nrf_clock_hf_start_task_status_get(NRF_CLOCK_Type const * p_reg);

#if (NRF_CLOCK_HAS_CALIBRATION) || defined(__NRFX_DOXYGEN__)
/**
 * @brief Function for changing the calibration timer interval.
 *
 * @param[in] p_reg    Pointer to the structure of registers of the peripheral.
 * @param[in] interval New calibration timer interval in 0.25 s resolution
 *                     (range: 0.25 seconds to 31.75 seconds).
 */
NRF_STATIC_INLINE void nrf_clock_cal_timer_timeout_set(NRF_CLOCK_Type * p_reg, uint32_t interval);
#endif

#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
/**
 * @brief Function for setting the subscribe configuration for a given
 *        CLOCK task.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] task    Task for which to set the configuration.
 * @param[in] channel Channel through which to subscribe events.
 */
NRF_STATIC_INLINE void nrf_clock_subscribe_set(NRF_CLOCK_Type * p_reg,
                                               nrf_clock_task_t task,
                                               uint8_t          channel);

/**
 * @brief Function for clearing the subscribe configuration for a given
 *        CLOCK task.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  Task for which to clear the configuration.
 */
NRF_STATIC_INLINE void nrf_clock_subscribe_clear(NRF_CLOCK_Type * p_reg, nrf_clock_task_t task);

/**
 * @brief Function for setting the publish configuration for a given
 *        CLOCK event.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] event   Event for which to set the configuration.
 * @param[in] channel Channel through which to publish the event.
 */
NRF_STATIC_INLINE void nrf_clock_publish_set(NRF_CLOCK_Type *  p_reg,
                                             nrf_clock_event_t event,
                                             uint8_t           channel);

/**
 * @brief Function for clearing the publish configuration for a given
 *        CLOCK event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event for which to clear the configuration.
 */
NRF_STATIC_INLINE void nrf_clock_publish_clear(NRF_CLOCK_Type * p_reg, nrf_clock_event_t event);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)

#ifndef NRF_DECLARE_ONLY

NRF_STATIC_INLINE void nrf_clock_int_enable(NRF_CLOCK_Type * p_reg, uint32_t mask)
{
    p_reg->INTENSET = mask;
}

NRF_STATIC_INLINE void nrf_clock_int_disable(NRF_CLOCK_Type * p_reg, uint32_t mask)
{
    p_reg->INTENCLR = mask;
}

NRF_STATIC_INLINE uint32_t nrf_clock_int_enable_check(NRF_CLOCK_Type const * p_reg, uint32_t mask)
{
    return p_reg->INTENSET & mask;
}

NRF_STATIC_INLINE uint32_t nrf_clock_task_address_get(NRF_CLOCK_Type const * p_reg,
                                                      nrf_clock_task_t       task)
{
    return (uint32_t)((uint8_t *)p_reg + (uint32_t)task);
}

NRF_STATIC_INLINE void nrf_clock_task_trigger(NRF_CLOCK_Type * p_reg, nrf_clock_task_t task)
{
    *((volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)task)) = 0x1UL;
}

NRF_STATIC_INLINE uint32_t nrf_clock_event_address_get(NRF_CLOCK_Type const * p_reg,
                                                       nrf_clock_event_t      event)
{
    return (uint32_t)((uint8_t *)p_reg + (uint32_t)event);
}

NRF_STATIC_INLINE void nrf_clock_event_clear(NRF_CLOCK_Type * p_reg, nrf_clock_event_t event)
{
    *((volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)event)) = 0x0UL;
#if __CORTEX_M == 0x04
    volatile uint32_t dummy = *((volatile uint32_t *)((uint8_t *)p_reg + (uint32_t)event));
    (void)dummy;
#endif
}

NRF_STATIC_INLINE bool nrf_clock_event_check(NRF_CLOCK_Type const * p_reg, nrf_clock_event_t event)
{
    return (bool)*((volatile uint32_t *)((uint8_t *)p_reg + event));
}

NRF_STATIC_INLINE void nrf_clock_lf_src_set(NRF_CLOCK_Type * p_reg, nrf_clock_lfclk_t source)
{
    p_reg->LFCLKSRC = (uint32_t)(source);
}

NRF_STATIC_INLINE nrf_clock_lfclk_t nrf_clock_lf_src_get(NRF_CLOCK_Type const * p_reg)
{
    return (nrf_clock_lfclk_t)(p_reg->LFCLKSRC);
}

NRF_STATIC_INLINE nrf_clock_lfclk_t nrf_clock_lf_actv_src_get(NRF_CLOCK_Type const * p_reg)
{
    return (nrf_clock_lfclk_t)((p_reg->LFCLKSTAT & CLOCK_LFCLKSTAT_SRC_Msk)
                                >> CLOCK_LFCLKSTAT_SRC_Pos);
}

NRF_STATIC_INLINE nrf_clock_lfclk_t nrf_clock_lf_srccopy_get(NRF_CLOCK_Type const * p_reg)
{
    return (nrf_clock_lfclk_t)((p_reg->LFCLKSRCCOPY & CLOCK_LFCLKSRCCOPY_SRC_Msk)
                                >> CLOCK_LFCLKSRCCOPY_SRC_Pos);
}

NRF_STATIC_INLINE bool nrf_clock_lf_is_running(NRF_CLOCK_Type const * p_reg)
{
    return ((p_reg->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) >> CLOCK_LFCLKSTAT_STATE_Pos);
}

NRF_STATIC_INLINE
nrf_clock_start_task_status_t nrf_clock_lf_start_task_status_get(NRF_CLOCK_Type const * p_reg)
{
    return (nrf_clock_start_task_status_t)((p_reg->LFCLKRUN & CLOCK_LFCLKRUN_STATUS_Msk)
                                            >> CLOCK_LFCLKRUN_STATUS_Pos);
}

NRF_STATIC_INLINE nrf_clock_hfclk_t nrf_clock_hf_src_get(NRF_CLOCK_Type const * p_reg)
{
    return (nrf_clock_hfclk_t)((p_reg->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Msk)
                                >> CLOCK_HFCLKSTAT_SRC_Pos);
}

NRF_STATIC_INLINE bool nrf_clock_hf_is_running(NRF_CLOCK_Type const * p_reg,
                                               nrf_clock_hfclk_t      clk_src)
{
    return (p_reg->HFCLKSTAT & (CLOCK_HFCLKSTAT_STATE_Msk | CLOCK_HFCLKSTAT_SRC_Msk)) ==
            (CLOCK_HFCLKSTAT_STATE_Msk | (clk_src << CLOCK_HFCLKSTAT_SRC_Pos));
}

NRF_STATIC_INLINE
nrf_clock_start_task_status_t nrf_clock_hf_start_task_status_get(NRF_CLOCK_Type const * p_reg)
{
    return (nrf_clock_start_task_status_t)((p_reg->HFCLKRUN & CLOCK_HFCLKRUN_STATUS_Msk)
                                            >> CLOCK_HFCLKRUN_STATUS_Pos);
}

#if (NRF_CLOCK_HAS_CALIBRATION)
NRF_STATIC_INLINE void nrf_clock_cal_timer_timeout_set(NRF_CLOCK_Type * p_reg, uint32_t interval)
{
    p_reg->CTIV = ((interval << CLOCK_CTIV_CTIV_Pos) & CLOCK_CTIV_CTIV_Msk);
}
#endif

#if defined(DPPI_PRESENT)
NRF_STATIC_INLINE void nrf_clock_subscribe_set(NRF_CLOCK_Type * p_reg,
                                               nrf_clock_task_t task,
                                               uint8_t          channel)
{
    *((volatile uint32_t *) ((uint8_t *)p_reg+ (uint32_t)task + 0x80uL)) =
            ((uint32_t)channel | CLOCK_SUBSCRIBE_HFCLKSTART_EN_Msk);
}

NRF_STATIC_INLINE void nrf_clock_subscribe_clear(NRF_CLOCK_Type * p_reg, nrf_clock_task_t task)
{
    *((volatile uint32_t *) ((uint8_t *)p_reg + (uint32_t)task + 0x80uL)) = 0;
}

NRF_STATIC_INLINE void nrf_clock_publish_set(NRF_CLOCK_Type *  p_reg,
                                             nrf_clock_event_t event,
                                             uint8_t           channel)
{
    *((volatile uint32_t *) ((uint8_t *)p_reg + (uint32_t)event + 0x80uL)) =
            ((uint32_t)channel | CLOCK_PUBLISH_HFCLKSTARTED_EN_Msk);
}

NRF_STATIC_INLINE void nrf_clock_publish_clear(NRF_CLOCK_Type * p_reg, nrf_clock_event_t event)
{
    *((volatile uint32_t *) ((uint8_t *)p_reg + (uint32_t)event + 0x80uL)) = 0;
}
#endif // defined(DPPI_PRESENT)

#endif // NRF_DECLARE_ONLY

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_CLOCK_H__
