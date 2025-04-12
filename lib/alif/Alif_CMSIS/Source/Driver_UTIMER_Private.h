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
 * @file     Driver_UTIMER_Private.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     02-April-2023
 * @brief    Header file for UTIMER.
 * @bug      None.
 * @Note	 None
 ******************************************************************************/

#ifndef DRIVER_UTIMER_PRIVATE_H_
#define DRIVER_UTIMER_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_UTIMER.h"
#include "utimer.h"

#define ARM_UTIMER_MAX_CHANNEL                      15U
#define ARM_UTIMER_TOTAL_CHANNELS                   16U

#define UTIMER_MODE_ENABLE                          1U
#define QEC_MODE_ENABLE                             0U

#define CHAN_INTERRUPT_CAPTURE_A                    (1U << 0)
#define CHAN_INTERRUPT_CAPTURE_B                    (1U << 1)
#define CHAN_INTERRUPT_COMPARE_A_BUF1               (1U << 2)
#define CHAN_INTERRUPT_COMPARE_A_BUF2               (1U << 3)
#define CHAN_INTERRUPT_COMPARE_B_BUF1               (1U << 4)
#define CHAN_INTERRUPT_COMPARE_B_BUF2               (1U << 5)
#define CHAN_INTERRUPT_UNDER_FLOW                   (1U << 6)
#define CHAN_INTERRUPT_OVER_FLOW                    (1U << 7)

#define UTIMER_CAPTURE_A_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 0U)
#define UTIMER_CAPTURE_B_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 1U)
#define UTIMER_CAPTURE_C_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 2U)
#define UTIMER_CAPTURE_D_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 3U)
#define UTIMER_CAPTURE_E_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 4U)
#define UTIMER_CAPTURE_F_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 5U)
#define UTIMER_UNDERFLOW_IRQ_BASE                   (UTIMER_IRQ0_IRQn + 6U)
#define UTIMER_OVERFLOW_IRQ_BASE                    (UTIMER_IRQ0_IRQn + 7U)

#define UTIMER_CAPTURE_A_IRQ(channel)               (UTIMER_CAPTURE_A_IRQ_BASE + (channel*8U))
#define UTIMER_CAPTURE_B_IRQ(channel)               (UTIMER_CAPTURE_B_IRQ_BASE + (channel*8U))
#define UTIMER_CAPTURE_C_IRQ(channel)               (UTIMER_CAPTURE_C_IRQ_BASE + (channel*8U))
#define UTIMER_CAPTURE_D_IRQ(channel)               (UTIMER_CAPTURE_D_IRQ_BASE + (channel*8U))
#define UTIMER_CAPTURE_E_IRQ(channel)               (UTIMER_CAPTURE_E_IRQ_BASE + (channel*8U))
#define UTIMER_CAPTURE_F_IRQ(channel)               (UTIMER_CAPTURE_F_IRQ_BASE + (channel*8U))
#define UTIMER_UNDERFLOW_IRQ(channel)               (UTIMER_UNDERFLOW_IRQ_BASE + (channel*8U))
#define UTIMER_OVERFLOW_IRQ(channel)                (UTIMER_OVERFLOW_IRQ_BASE + (channel*8U))

#define QEC_CAPTURE_A_IRQ_BASE                      (QEC0_CMPA_IRQ_IRQn + 0U)
#define QEC_CAPTURE_B_IRQ_BASE                      (QEC0_CMPA_IRQ_IRQn + 1U)

#define QEC_CAPTURE_A_IRQ(channel)                  (QEC_CAPTURE_A_IRQ_BASE + ((channel - ARM_UTIMER_CHANNEL12)*2U))
#define QEC_CAPTURE_B_IRQ(channel)                  (QEC_CAPTURE_B_IRQ_BASE + ((channel - ARM_UTIMER_CHANNEL12)*2U))

/** \brief UTIMER driver state. */
typedef struct _UTIMER_DRV_STATE {
    uint32_t initialized : 1;    /* Driver initialized */
    uint32_t powered     : 1;    /* Driver powered */
    uint32_t configured  : 1;    /* Driver counter configured */
    uint32_t triggered   : 1;    /* Driver event triggered */
    uint32_t started     : 1;    /* Driver counter started */
    uint32_t reserved    : 27;   /* Reserved bits */
}UTIMER_DRV_STATE;

/** \brief UTIMER channel specific configurations. */
typedef struct _UTIMER_CHANNEL_INFO
{
    utimer_channel_config      ch_config;                   /**< Pointer to channel configurations >*/
    bool                       dc_enable;                   /**< UTIMER duty cycle SET: ENABLED or CLEAR: DISABLED >*/
    uint8_t                    capture_A_irq_priority;      /**< IRQ Priority for Compare/Capture A match event >*/
    uint8_t                    capture_B_irq_priority;      /**< IRQ Priority for Compare/Capture B match event >*/
    uint8_t                    capture_C_irq_priority;      /**< IRQ Priority for Compare/Capture A Buffer 1 match event >*/
    uint8_t                    capture_D_irq_priority;      /**< IRQ Priority for Compare/Capture A Buffer 2 match event >*/
    uint8_t                    capture_E_irq_priority;      /**< IRQ Priority for Compare/Capture B Buffer 1 match event >*/
    uint8_t                    capture_F_irq_priority;      /**< IRQ Priority for Compare/Capture B Buffer 2 match event >*/
    uint8_t                    over_flow_irq_priority;      /**< IRQ Priority for Over Flow event >*/
    uint8_t                    under_flow_irq_priority;     /**< IRQ Priority for Under Flow event >*/
    ARM_UTIMER_MODE            channel_mode_backup;         /**< UTIMER Channel mode back up >*/
    ARM_UTIMER_COUNTER_DIR     channel_counter_dir_backup;  /**< UTIMER Channel counter direction back up >*/
    UTIMER_DRV_STATE           state;                       /**< UTIMER channel status flag >*/
    ARM_UTIMER_SignalEvent_t   CB_function_ptr;             /**< Holding CB functions >*/
} UTIMER_CHANNEL_INFO;

/** \brief UTIMER resource. */
typedef struct _UTIMER_RESOURCES
{
    UTIMER_Type *regs;                /**< Pointer to UTIMER registers >*/
    UTIMER_CHANNEL_INFO ch_info[ARM_UTIMER_TOTAL_CHANNELS]; /**< Pointer to Info structure of UTIMER >*/
} UTIMER_RESOURCES;

/**
 * @fn      static inline uint32_t UTIMER_Get_TriggerType (ARM_UTIMER_TRIGGER trigger)
 * @brief   Get trigger type for UTIMER.
 * @note    none.
 * @param   trigger : trigger type.
 * @retval  trigger value
 */
static inline uint32_t UTIMER_Get_TriggerType (ARM_UTIMER_TRIGGER trigger)
{
    uint32_t value;

    switch (trigger)
    {
        case ARM_UTIMER_SRC0_TRIG0_RISING:
        {
            value = CNTR_SRC0_TRIG0_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG0_FALLING:
        {
            value = CNTR_SRC0_TRIG0_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG1_RISING:
        {
            value = CNTR_SRC0_TRIG1_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG1_FALLING:
        {
            value = CNTR_SRC0_TRIG1_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG2_RISING:
        {
            value = CNTR_SRC0_TRIG2_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG2_FALLING:
        {
            value = CNTR_SRC0_TRIG2_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG3_RISING:
        {
            value = CNTR_SRC0_TRIG3_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG3_FALLING:
        {
            value = CNTR_SRC0_TRIG3_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG4_RISING:
        {
            value = CNTR_SRC0_TRIG4_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG4_FALLING:
        {
            value = CNTR_SRC0_TRIG4_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG5_RISING:
        {
            value = CNTR_SRC0_TRIG5_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG5_FALLING:
        {
            value = CNTR_SRC0_TRIG5_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG6_RISING:
        {
            value = CNTR_SRC0_TRIG6_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG6_FALLING:
        {
            value = CNTR_SRC0_TRIG6_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG7_RISING:
        {
            value = CNTR_SRC0_TRIG7_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG7_FALLING:
        {
            value = CNTR_SRC0_TRIG7_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG8_RISING:
        {
            value = CNTR_SRC0_TRIG8_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG8_FALLING:
        {
            value = CNTR_SRC0_TRIG8_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG9_RISING:
        {
            value = CNTR_SRC0_TRIG9_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG9_FALLING:
        {
            value = CNTR_SRC0_TRIG9_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG10_RISING:
        {
            value = CNTR_SRC0_TRIG10_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG10_FALLING:
        {
            value = CNTR_SRC0_TRIG10_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG11_RISING:
        {
            value = CNTR_SRC0_TRIG11_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG11_FALLING:
        {
            value = CNTR_SRC0_TRIG11_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG12_RISING:
        {
            value = CNTR_SRC0_TRIG12_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG12_FALLING:
        {
            value = CNTR_SRC0_TRIG12_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG13_RISING:
        {
            value = CNTR_SRC0_TRIG13_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG13_FALLING:
        {
            value = CNTR_SRC0_TRIG13_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG14_RISING:
        {
            value = CNTR_SRC0_TRIG14_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG14_FALLING:
        {
            value = CNTR_SRC0_TRIG14_FALLING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG15_RISING:
        {
            value = CNTR_SRC0_TRIG15_RISING;
            break;
        }
        case ARM_UTIMER_SRC0_TRIG15_FALLING:
        {
            value = CNTR_SRC0_TRIG15_FALLING;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_A_RISING_B_0:
        {
            value = CNTR_SRC1_DRIVE_A_RISING_B_0;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_A_RISING_B_1:
        {
            value = CNTR_SRC1_DRIVE_A_RISING_B_1;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_A_FALLING_B_0:
        {
            value = CNTR_SRC1_DRIVE_A_FALLING_B_0;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_A_FALLING_B_1:
        {
            value = CNTR_SRC1_DRIVE_A_FALLING_B_1;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_B_RISING_A_0:
        {
            value = CNTR_SRC1_DRIVE_B_RISING_A_0;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_B_RISING_A_1:
        {
            value = CNTR_SRC1_DRIVE_B_RISING_A_1;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_B_FALLING_A_0:
        {
            value = CNTR_SRC1_DRIVE_B_FALLING_A_0;
            break;
        }
        case ARM_UTIMER_SRC1_DRIVE_B_FALLING_A_1:
        {
            value = CNTR_SRC1_DRIVE_B_FALLING_A_1;
            break;
        }
        case ARM_UTIMER_FAULT_TRIG0_RISING:
        {
            value = (FAULT_CTRL_FAULT_EN_TRIG0 | FAULT_CTRL_FAULT_POL_HIGH_TRIG0);
            break;
        }
        case ARM_UTIMER_FAULT_TRIG0_FALLING:
        {
            value = FAULT_CTRL_FAULT_EN_TRIG0;
            break;
        }
        case ARM_UTIMER_FAULT_TRIG1_RISING:
        {
            value = (FAULT_CTRL_FAULT_EN_TRIG1 | FAULT_CTRL_FAULT_POL_HIGH_TRIG1);
            break;
        }
        case ARM_UTIMER_FAULT_TRIG1_FALLING:
        {
            value = FAULT_CTRL_FAULT_EN_TRIG1;
            break;
        }
        case ARM_UTIMER_FAULT_TRIG2_RISING:
        {
            value = (FAULT_CTRL_FAULT_EN_TRIG2 | FAULT_CTRL_FAULT_POL_HIGH_TRIG2);
            break;
        }
        case ARM_UTIMER_FAULT_TRIG2_FALLING:
        {
            value = FAULT_CTRL_FAULT_EN_TRIG2;
            break;
        }
        case ARM_UTIMER_FAULT_TRIG3_RISING:
        {
            value = (FAULT_CTRL_FAULT_EN_TRIG3 | FAULT_CTRL_FAULT_POL_HIGH_TRIG3);
            break;
        }
        case ARM_UTIMER_FAULT_TRIG3_FALLING:
        {
            value = FAULT_CTRL_FAULT_EN_TRIG3;
            break;
        }
        case ARM_UTIMER_PAUSE_SRC_0_HIGH:
        {
            value = CNTR_PAUSE_SRC_0_HIGH_EN;
            break;
        }
        case ARM_UTIMER_PAUSE_SRC_0_LOW:
        {
            value = CNTR_PAUSE_SRC_0_LOW_EN;
            break;
        }
        case ARM_UTIMER_PAUSE_SRC_1_HIGH:
        {
            value = CNTR_PAUSE_SRC_1_HIGH_EN;
            break;
        }
        case ARM_UTIMER_PAUSE_SRC_1_LOW:
        {
            value = CNTR_PAUSE_SRC_1_LOW_EN;
            break;
        }
    }
    return value;
}

#endif /* DRIVER_UTIMER_PRIVATE_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
