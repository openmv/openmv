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
 * @file     Driver_UTIMER.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     02-April-2023
 * @brief    CMSIS-Driver for UTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef __DRIVER_UTIMER_H__
#define __DRIVER_UTIMER_H__

#include "Driver_Common.h"

typedef void (*ARM_UTIMER_SignalEvent_t) (uint8_t event);   /**< Initialization UTIMER call back function declaration >*/

/**< UTIMER Events >*/
#define ARM_UTIMER_EVENT_CAPTURE_A                  1U    /**< UTIMER Capture A event >*/
#define ARM_UTIMER_EVENT_CAPTURE_B                  2U    /**< UTIMER Capture B event >*/
#define ARM_UTIMER_EVENT_COMPARE_A                  3U    /**< UTIMER Compare A event >*/
#define ARM_UTIMER_EVENT_COMPARE_B                  4U    /**< UTIMER Compare B event >*/
#define ARM_UTIMER_EVENT_COMPARE_A_BUF1             5U    /**< UTIMER Compare A Buf1 event >*/
#define ARM_UTIMER_EVENT_COMPARE_A_BUF2             6U    /**< UTIMER Compare A Buf2 event >*/
#define ARM_UTIMER_EVENT_COMPARE_B_BUF1             7U    /**< UTIMER Compare B Buf1 event >*/
#define ARM_UTIMER_EVENT_COMPARE_B_BUF2             8U    /**< UTIMER Compare B Buf2 event >*/
#define ARM_UTIMER_EVENT_UNDER_FLOW                 9U    /**< UTIMER Underflow event >*/
#define ARM_UTIMER_EVENT_OVER_FLOW                  10U   /**< UTIMER Overflow event >*/

/**< UTIMER Channel declaration >*/
#define ARM_UTIMER_CHANNEL0                         0U    /**< UTIMER Channel 0 >*/
#define ARM_UTIMER_CHANNEL1                         1U    /**< UTIMER Channel 1 >*/
#define ARM_UTIMER_CHANNEL2                         2U    /**< UTIMER Channel 2 >*/
#define ARM_UTIMER_CHANNEL3                         3U    /**< UTIMER Channel 3 >*/
#define ARM_UTIMER_CHANNEL4                         4U    /**< UTIMER Channel 4 >*/
#define ARM_UTIMER_CHANNEL5                         5U    /**< UTIMER Channel 5 >*/
#define ARM_UTIMER_CHANNEL6                         6U    /**< UTIMER Channel 6 >*/
#define ARM_UTIMER_CHANNEL7                         7U    /**< UTIMER Channel 7 >*/
#define ARM_UTIMER_CHANNEL8                         8U    /**< UTIMER Channel 8 >*/
#define ARM_UTIMER_CHANNEL9                         9U    /**< UTIMER Channel 9 >*/
#define ARM_UTIMER_CHANNEL10                        10U   /**< UTIMER Channel 10 >*/
#define ARM_UTIMER_CHANNEL11                        11U   /**< UTIMER Channel 11 >*/
#define ARM_UTIMER_CHANNEL12                        12U   /**< QEC Channel 0 >*/
#define ARM_UTIMER_CHANNEL13                        13U   /**< QEC Channel 1 >*/
#define ARM_UTIMER_CHANNEL14                        14U   /**< QEC Channel 2 >*/
#define ARM_UTIMER_CHANNEL15                        15U   /**< QEC Channel 3 >*/

#define ARM_UTIMER_COUNTER_CLEAR                    1U    /* UTIMER counter clear enable at counter stop */
#define ARM_UTIMER_COUNTER_NOT_CLEAR                0U    /* UTIMER counter clear disabled at counter stop */

/**
 * enum ARM_UTIMER_MODE.
 * Driver UTIMER modes.
 */
typedef enum _ARM_UTIMER_MODE {
    ARM_UTIMER_MODE_BASIC,                          /**< UTIMER Channel mode configure to Basic mode >*/
    ARM_UTIMER_MODE_BUFFERING,                      /**< UTIMER Channel mode configure to Buffering mode >*/
    ARM_UTIMER_MODE_TRIGGERING,                     /**< UTIMER Channel mode configure to Triggering mode >*/
    ARM_UTIMER_MODE_CAPTURING,                      /**< UTIMER Channel mode configure to Capturing mode >*/
    ARM_UTIMER_MODE_COMPARING,                      /**< UTIMER Channel mode configure to Comparing mode >*/
    ARM_UTIMER_MODE_DEAD_TIME                       /**< UTIMER Channel mode configure to Dead Timer mode >*/
} ARM_UTIMER_MODE;

/**
 * enum ARM_UTIMER_COUNTER_DIR.
 * Driver UTIMER counter direction.
 */
typedef enum _ARM_UTIMER_COUNTER_DIR {
    ARM_UTIMER_COUNTER_UP,                          /**< UTIMER Channel counter direction up >*/
    ARM_UTIMER_COUNTER_DOWN,                        /**< UTIMER Channel counter direction down >*/
    ARM_UTIMER_COUNTER_TRIANGLE                     /**< UTIMER Channel counter direction triangle >*/
} ARM_UTIMER_COUNTER_DIR;

/**
 * enum ARM_UTIMER_COUNTER.
 * Driver UTIMER counters.
 */
typedef enum _ARM_UTIMER_COUNTER {
    ARM_UTIMER_CNTR,                                /**< UTIMER counter >*/
    ARM_UTIMER_CNTR_PTR,                            /**< UTIMER pointer counter >*/
    ARM_UTIMER_CNTR_PTR_BUF1,                       /**< UTIMER pointer buffer 1 counter >*/
    ARM_UTIMER_CNTR_PTR_BUF2,                       /**< UTIMER pointer buffer 2 counter >*/
    ARM_UTIMER_DT_UP,                               /**< UTIMER Dead Time UP >*/
    ARM_UTIMER_DT_UP_BUF1,                          /**< UTIMER Dead Time UP buffer 1 >*/
    ARM_UTIMER_DT_DOWN,                             /**< UTIMER Dead Time DOWN >*/
    ARM_UTIMER_DT_DOWN_BUF1,                        /**< UTIMER Dead Time DOWN buffer 1 >*/
    ARM_UTIMER_COMPARE_A,                           /**< UTIMER compare A >*/
    ARM_UTIMER_COMPARE_B,                           /**< UTIMER compare B >*/
    ARM_UTIMER_COMPARE_A_BUF1,                      /**< UTIMER compare A buffer 1 >*/
    ARM_UTIMER_COMPARE_B_BUF1,                      /**< UTIMER compare B buffer 1 >*/
    ARM_UTIMER_COMPARE_A_BUF2,                      /**< UTIMER compare A buffer 2 >*/
    ARM_UTIMER_COMPARE_B_BUF2,                      /**< UTIMER compare B buffer 1 >*/
    ARM_UTIMER_CAPTURE_A,                           /**< UTIMER capture A  >*/
    ARM_UTIMER_CAPTURE_B,                           /**< UTIMER capture B >*/
    ARM_UTIMER_CAPTURE_A_BUF1,                      /**< UTIMER capture A buffer 1 >*/
    ARM_UTIMER_CAPTURE_B_BUF1,                      /**< UTIMER capture B buffer 1 >*/
    ARM_UTIMER_CAPTURE_A_BUF2,                      /**< UTIMER capture A buffer 2 >*/
    ARM_UTIMER_CAPTURE_B_BUF2                       /**< UTIMER capture B buffer 2 >*/
} ARM_UTIMER_COUNTER;

/**
 * enum ARM_UTIMER_TRIGGER_SRC.
 * Driver UTIMER external event sources.
 */
typedef enum _ARM_UTIMER_TRIGGER_SRC {
    ARM_UTIMER_SRC_0,                               /**< global triggers >*/
    ARM_UTIMER_SRC_1,                               /**< channel events >*/
    ARM_UTIMER_FAULT_TRIGGER,                       /**< fault triggers >*/
    ARM_UTIMER_CNTR_PAUSE_TRIGGER                   /**< counter pause triggers >*/
} ARM_UTIMER_TRIGGER_SRC;

/**
 * enum ARM_UTIMER_TRIGGER_TARGET.
 * Driver UTIMER trigger targets.
 */
typedef enum _ARM_UTIMER_TRIGGER_TARGET {
    ARM_UTIMER_TRIGGER_START,                       /**< Trigger target UTIMER Channel counter start >*/
    ARM_UTIMER_TRIGGER_STOP,                        /**< Trigger target UTIMER Channel counter stop >*/
    ARM_UTIMER_TRIGGER_CLEAR,                       /**< Trigger target UTIMER Channel counter clear >*/
    ARM_UTIMER_TRIGGER_UPCOUNT,                     /**< Trigger target UTIMER Channel Up count >*/
    ARM_UTIMER_TRIGGER_DOWNCOUNT,                   /**< Trigger target UTIMER Channel Down count >*/
    ARM_UTIMER_TRIGGER_CAPTURE_A,                   /**< Trigger target UTIMER Channel to capture counter value in Drive A >*/
    ARM_UTIMER_TRIGGER_CAPTURE_B,                   /**< Trigger target UTIMER Channel to capture counter value in Drive B >*/
    ARM_UTIMER_TRIGGER_DMA_CLEAR_A,                 /**< Trigger target UTIMER Channel to clear Drive A DMA action >*/
    ARM_UTIMER_TRIGGER_DMA_CLEAR_B                  /**< Trigger target UTIMER Channel to clear Drive B DMA action >*/
} ARM_UTIMER_TRIGGER_TARGET;

/**
 * enum ARM_UTIMER_TRIGGER.
 * Driver UTIMER trigger types.
 */
typedef enum _ARM_UTIMER_TRIGGER {
    ARM_UTIMER_SRC0_TRIG0_RISING,                    /**< UTIMER/QEC trigger 0 rising edge >*/
    ARM_UTIMER_SRC0_TRIG0_FALLING,                   /**< UTIMER/QEC trigger 0 falling edge >*/
    ARM_UTIMER_SRC0_TRIG1_RISING,                    /**< UTIMER/QEC trigger 1 rising edge >*/
    ARM_UTIMER_SRC0_TRIG1_FALLING,                   /**< UTIMER/QEC trigger 1 falling edge >*/
    ARM_UTIMER_SRC0_TRIG2_RISING,                    /**< UTIMER/QEC trigger 2 rising edge >*/
    ARM_UTIMER_SRC0_TRIG2_FALLING,                   /**< UTIMER/QEC trigger 2 falling edge >*/
    ARM_UTIMER_SRC0_TRIG3_RISING,                    /**< UTIMER/QEC trigger 3 rising edge >*/
    ARM_UTIMER_SRC0_TRIG3_FALLING,                   /**< UTIMER/QEC trigger 3 falling edge >*/
    ARM_UTIMER_SRC0_TRIG4_RISING,                    /**< UTIMER/QEC trigger 4 rising edge >*/
    ARM_UTIMER_SRC0_TRIG4_FALLING,                   /**< UTIMER/QEC trigger 4 falling edge >*/
    ARM_UTIMER_SRC0_TRIG5_RISING,                    /**< UTIMER/QEC trigger 5 rising edge >*/
    ARM_UTIMER_SRC0_TRIG5_FALLING,                   /**< UTIMER/QEC trigger 5 falling edge >*/
    ARM_UTIMER_SRC0_TRIG6_RISING,                    /**< UTIMER/QEC trigger 6 rising edge >*/
    ARM_UTIMER_SRC0_TRIG6_FALLING,                   /**< UTIMER/QEC trigger 6 falling edge >*/
    ARM_UTIMER_SRC0_TRIG7_RISING,                    /**< UTIMER/QEC trigger 7 rising edge >*/
    ARM_UTIMER_SRC0_TRIG7_FALLING,                   /**< UTIMER/QEC trigger 7 falling edge >*/
    ARM_UTIMER_SRC0_TRIG8_RISING,                    /**< UTIMER/QEC trigger 8 rising edge >*/
    ARM_UTIMER_SRC0_TRIG8_FALLING,                   /**< UTIMER/QEC trigger 8 falling edge >*/
    ARM_UTIMER_SRC0_TRIG9_RISING,                    /**< UTIMER/QEC trigger 9 rising edge >*/
    ARM_UTIMER_SRC0_TRIG9_FALLING,                   /**< UTIMER/QEC trigger 9 falling edge >*/
    ARM_UTIMER_SRC0_TRIG10_RISING,                   /**< UTIMER/QEC trigger 10 rising edge >*/
    ARM_UTIMER_SRC0_TRIG10_FALLING,                  /**< UTIMER/QEC trigger 10 falling edge >*/
    ARM_UTIMER_SRC0_TRIG11_RISING,                   /**< UTIMER/QEC trigger 11 rising edge >*/
    ARM_UTIMER_SRC0_TRIG11_FALLING,                  /**< UTIMER/QEC trigger 11 falling edge >*/
    ARM_UTIMER_SRC0_TRIG12_RISING,                   /**< UTIMER trigger 12 rising edge >*/
    ARM_UTIMER_SRC0_TRIG12_FALLING,                  /**< UTIMER trigger 12 falling edge >*/
    ARM_UTIMER_SRC0_TRIG13_RISING,                   /**< UTIMER trigger 13 rising edge >*/
    ARM_UTIMER_SRC0_TRIG13_FALLING,                  /**< UTIMER trigger 13 falling edge >*/
    ARM_UTIMER_SRC0_TRIG14_RISING,                   /**< UTIMER trigger 14 rising edge >*/
    ARM_UTIMER_SRC0_TRIG14_FALLING,                  /**< UTIMER trigger 14 falling edge >*/
    ARM_UTIMER_SRC0_TRIG15_RISING,                   /**< UTIMER trigger 15 rising edge >*/
    ARM_UTIMER_SRC0_TRIG15_FALLING,                  /**< UTIMER trigger 15 falling edge >*/
    ARM_UTIMER_SRC1_DRIVE_A_RISING_B_0,              /**< UTIMER Channel inputs: rising edge on driver A, 0 on driver B >*/
    ARM_UTIMER_SRC1_DRIVE_A_RISING_B_1,              /**< UTIMER Channel inputs: rising edge on driver A, 1 on driver B >*/
    ARM_UTIMER_SRC1_DRIVE_A_FALLING_B_0,             /**< UTIMER Channel inputs: falling edge on driver A, 0 on driver B >*/
    ARM_UTIMER_SRC1_DRIVE_A_FALLING_B_1,             /**< UTIMER Channel inputs: falling edge on driver A, 1 on driver B >*/
    ARM_UTIMER_SRC1_DRIVE_B_RISING_A_0,              /**< UTIMER Channel inputs: rising edge on driver B, 0 on driver A >*/
    ARM_UTIMER_SRC1_DRIVE_B_RISING_A_1,              /**< UTIMER Channel inputs: rising edge on driver B, 1 on driver A >*/
    ARM_UTIMER_SRC1_DRIVE_B_FALLING_A_0,             /**< UTIMER Channel inputs: falling edge on driver B, 0 on driver A >*/
    ARM_UTIMER_SRC1_DRIVE_B_FALLING_A_1,             /**< UTIMER Channel inputs: falling edge on driver B, 0 on driver A >*/
    ARM_UTIMER_FAULT_TRIG0_RISING,                   /**< Fault trigger 0 rising edge >*/
    ARM_UTIMER_FAULT_TRIG0_FALLING,                  /**< Fault trigger 0 falling edge >*/
    ARM_UTIMER_FAULT_TRIG1_RISING,                   /**< Fault trigger 1 rising edge >*/
    ARM_UTIMER_FAULT_TRIG1_FALLING,                  /**< Fault trigger 1 falling edge >*/
    ARM_UTIMER_FAULT_TRIG2_RISING,                   /**< Fault trigger 2 rising edge >*/
    ARM_UTIMER_FAULT_TRIG2_FALLING,                  /**< Fault trigger 2 falling edge >*/
    ARM_UTIMER_FAULT_TRIG3_RISING,                   /**< Fault trigger 3 rising edge >*/
    ARM_UTIMER_FAULT_TRIG3_FALLING,                  /**< Fault trigger 3 falling edge >*/
    ARM_UTIMER_PAUSE_SRC_0_HIGH,                     /**< Pause source 0 high >*/
    ARM_UTIMER_PAUSE_SRC_0_LOW,                      /**< Pause source 0 low >*/
    ARM_UTIMER_PAUSE_SRC_1_HIGH,                     /**< Pause source 1 high >*/
    ARM_UTIMER_PAUSE_SRC_1_LOW                       /**< Pause source 1 low >*/
} ARM_UTIMER_TRIGGER;

/** \brief UTIMER trigger configuration. */
typedef struct _ARM_UTITMER_TRIGGER_CONFIG {
    ARM_UTIMER_TRIGGER_TARGET     triggerTarget;      /**< UTIMER trigger target >*/
    ARM_UTIMER_TRIGGER_SRC        triggerSrc;         /**< UTIMER trigger source >*/
    ARM_UTIMER_TRIGGER            trigger;            /**< UTIMER triggers >*/
} ARM_UTIMER_TRIGGER_CONFIG;

/**
\fn         int32_t ARM_UTIMER_Initialize (uint8_t channel, ARM_UTIMER_CBEvent CB_event)
\brief      Initialize UTIMER interface and register signal(call back) functions.
\param[in]  channel :Utimer have total 12 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  CB_event: call back function.
\param[out] int32_t : execution_status.

\fn         int32_t ARM_UTIMER_PowerControl (uint8_t channel, ARM_POWER_STATE state)
\brief      Control UTIMER channel interface power.
\param[in]  channel : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  state   : Power state
                    - \ref ARM_POWER_OFF :  power off: no operation possible
                    - \ref ARM_POWER_LOW :  low power mode: retain state, detect and signal wake-up events
                    - \ref ARM_POWER_FULL : power on: full operation at maximum performance
\param[out] int32_t : execution_status.

\fn         int32_t ARM_UTIMER_ConfigCounter (uint8_t channel, ARM_UTIMER_MODE mode, ARM_UTIMER_COUNTER_DIR dir)
\brief      Configure utimer mode and type.
\param[in]  channel     : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  mode        : Utimer mode.
 param[in]  dir         : Utimer counter direction.
 param[out] int32_t     : execution_status.

\fn         int32_t ARM_UTIMER_SetCount (uint8_t channel, ARM_UTIMER_COUNTER counter, uint32_t arg)
\brief      Set UTIMER counter value.
\param[in]  channel     : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  counter     : counter type.
 param[in]  value       : counter value.
 param[out] int32_t     : execution_status.

\fn         uint32_t ARM_UTIMER_GetCount (uint8_t channel, ARM_UTIMER_COUNTER counter)
\brief      Get UTIMER counter value.
\param[in]  channel     : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  counter     : counter type.
 param[out] uint32_t    : counter value.

\fn         int32_t ARM_UTIMER_ConfigTrigger (uint8_t channel, ARM_UTIMER_TRIGGER_CONFIG *arg)
\brief      Configure the UTMER Channel for trigger operation.
\param[in]  channel     : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  arg         : Pointer to an argument of type UTIMER_TRIGGER_CONFIG.
\param[out] int32_t     : execution_status.

\fn         int32_t ARM_UTIMER_Start (uint8_t channel)
\brief      Start the UTMER Channel counting.
\param[in]  channel : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[out] int32_t : execution_status.

\fn         int32_t ARM_UTIMER_Stop (uint8_t channel, bool count_clear_option)
\brief      Stop the UTMER Channel running count.
\param[in]  channel : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[in]  count_clear_option : Utimer set = count clear, clear = count unclear
\param[out] int32_t : execution_status.

\fn         int32_t ARM_UTIMER_Uninitialize (uint8_t channel)
\brief      Un-initialize the named channel configuration
\param[in]  channel : Utimer have total 16 channel, configure by feeding TIMER_CHANNEL0,1,2,3....
\param[out] int32_t : execution_status.
*/

typedef struct _ARM_DRIVER_UTIMER {
    int32_t               (*Initialize)              (uint8_t channel, ARM_UTIMER_SignalEvent_t cb_event);                    /**< Pointer to \ref ARM_UTIMER_Initialize    : Initialize the UTIMER Interface>*/
    int32_t               (*PowerControl)            (uint8_t channel, ARM_POWER_STATE state);                                /**< Pointer to \ref ARM_UTIMER_PowerControl  : Control UTIMER interface power>*/
    int32_t               (*ConfigCounter)           (uint8_t channel, ARM_UTIMER_MODE mode, ARM_UTIMER_COUNTER_DIR dir);     /**< Pointer to \ref ARM_UTIMER_ConfigCounter : Control timer type and mode for UTIMER channel>*/
    int32_t               (*SetCount)                (uint8_t channel, ARM_UTIMER_COUNTER counter, uint32_t value);           /**< Pointer to \ref ARM_UTIMER_SetCount      : Set count value for UTIMER channel>*/
    uint32_t              (*GetCount)                (uint8_t channel, ARM_UTIMER_COUNTER counter);                           /**< Pointer to \ref ARM_UTIMER_GetCount      : Get count value for UTIMER channel>*/
    int32_t               (*ConfigTrigger)           (uint8_t channel, ARM_UTIMER_TRIGGER_CONFIG *arg);                       /**< Pointer to \ref ARM_UTIMER_ConfigTrigger : configure the input triggers>*/
    int32_t               (*Start)                   (uint8_t channel);                                                       /**< Pointer to \ref ARM_UTIMER_Start         : Global starts the UTIMER channel>*/
    int32_t               (*Stop)                    (uint8_t channel, bool count_clear_option);                              /**< Pointer to \ref ARM_UTIMER_Stop          : Global stops  the UTIMER channel>*/
    int32_t               (*Uninitialize)            (uint8_t channel);                                                       /**< Pointer to \ref ARM_UTIMER_Uninitialize  : Uninitialized the UTIMER Interface>*/
} ARM_DRIVER_UTIMER;

#endif /*< __DRIVER_UTIMER_H__>*/
