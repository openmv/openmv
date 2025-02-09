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
 * @file     utimer.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     02-April-2023
 * @brief    Low Level header file for UTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef UTIMER_H_
#define UTIMER_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "stdbool.h"

/*****************  Bit definition for TIMER_RegInfo:cntr_start_src_0 register  ******************/
#define CNTR_SRC0_TRIG0_RISING                      ((uint32_t)0x00000001)
#define CNTR_SRC0_TRIG0_FALLING                     ((uint32_t)0x00000002)
#define CNTR_SRC0_TRIG1_RISING                      ((uint32_t)0x00000004)
#define CNTR_SRC0_TRIG1_FALLING                     ((uint32_t)0x00000008)
#define CNTR_SRC0_TRIG2_RISING                      ((uint32_t)0x00000010)
#define CNTR_SRC0_TRIG2_FALLING                     ((uint32_t)0x00000020)
#define CNTR_SRC0_TRIG3_RISING                      ((uint32_t)0x00000040)
#define CNTR_SRC0_TRIG3_FALLING                     ((uint32_t)0x00000080)
#define CNTR_SRC0_TRIG4_RISING                      ((uint32_t)0x00000100)
#define CNTR_SRC0_TRIG4_FALLING                     ((uint32_t)0x00000200)
#define CNTR_SRC0_TRIG5_RISING                      ((uint32_t)0x00000400)
#define CNTR_SRC0_TRIG5_FALLING                     ((uint32_t)0x00000800)
#define CNTR_SRC0_TRIG6_RISING                      ((uint32_t)0x00001000)
#define CNTR_SRC0_TRIG6_FALLING                     ((uint32_t)0x00002000)
#define CNTR_SRC0_TRIG7_RISING                      ((uint32_t)0x00004000)
#define CNTR_SRC0_TRIG7_FALLING                     ((uint32_t)0x00008000)
#define CNTR_SRC0_TRIG8_RISING                      ((uint32_t)0x00010000)
#define CNTR_SRC0_TRIG8_FALLING                     ((uint32_t)0x00020000)
#define CNTR_SRC0_TRIG9_RISING                      ((uint32_t)0x00040000)
#define CNTR_SRC0_TRIG9_FALLING                     ((uint32_t)0x00080000)
#define CNTR_SRC0_TRIG10_RISING                     ((uint32_t)0x00100000)
#define CNTR_SRC0_TRIG10_FALLING                    ((uint32_t)0x00200000)
#define CNTR_SRC0_TRIG11_RISING                     ((uint32_t)0x00400000)
#define CNTR_SRC0_TRIG11_FALLING                    ((uint32_t)0x00800000)
#define CNTR_SRC0_TRIG12_RISING                     ((uint32_t)0x01000000)
#define CNTR_SRC0_TRIG12_FALLING                    ((uint32_t)0x02000000)
#define CNTR_SRC0_TRIG13_RISING                     ((uint32_t)0x04000000)
#define CNTR_SRC0_TRIG13_FALLING                    ((uint32_t)0x08000000)
#define CNTR_SRC0_TRIG14_RISING                     ((uint32_t)0x10000000)
#define CNTR_SRC0_TRIG14_FALLING                    ((uint32_t)0x20000000)
#define CNTR_SRC0_TRIG15_RISING                     ((uint32_t)0x40000000)
#define CNTR_SRC0_TRIG15_FALLING                    ((uint32_t)0x80000000)

/*****************  Bit definition for TIMER_RegInfo:cntr_start_src_1 register  ******************/
#define CNTR_SRC1_DRIVE_A_RISING_B_0                ((uint32_t)0x00000001)
#define CNTR_SRC1_DRIVE_A_RISING_B_1                ((uint32_t)0x00000002)
#define CNTR_SRC1_DRIVE_A_FALLING_B_0               ((uint32_t)0x00000004)
#define CNTR_SRC1_DRIVE_A_FALLING_B_1               ((uint32_t)0x00000008)
#define CNTR_SRC1_DRIVE_B_RISING_A_0                ((uint32_t)0x00000010)
#define CNTR_SRC1_DRIVE_B_RISING_A_1                ((uint32_t)0x00000020)
#define CNTR_SRC1_DRIVE_B_FALLING_A_0               ((uint32_t)0x00000040)
#define CNTR_SRC1_DRIVE_B_FALLING_A_1               ((uint32_t)0x00000080)
#define CNTR_SRC1_PGM_EN                            ((uint32_t)0x80000000)

/*****************  Bit definition for TIMER_RegInfo:cntr_pause_src register  ******************/
#define CNTR_PAUSE_SRC_0_HIGH_EN                    ((uint32_t)0x00000001)
#define CNTR_PAUSE_SRC_0_LOW_EN                     ((uint32_t)0x00000002)
#define CNTR_PAUSE_SRC_1_HIGH_EN                    ((uint32_t)0x00000004)
#define CNTR_PAUSE_SRC_1_LOW_EN                     ((uint32_t)0x00000008)

/*****************  Bit definition for TIMER_RegInfo:cntr_ctrl register  ******************/
#define CNTR_CTRL_EN                                ((uint32_t)0x00000001)
#define CNTR_CTRL_START                             ((uint32_t)0x00000002)
#define CNTR_CTRL_SAWTOOTH_ONE_SHOT                 ((uint32_t)0x00000004)
#define CNTR_CTRL_TRIANGLE_BUF_TROUGH               ((uint32_t)0x00000010)
#define CNTR_CTRL_TRIANGLE_BUF_TROUGH_CREST         ((uint32_t)0x00000014)
#define CNTR_CTRL_TRIANGLE_ONE_SHOT                 ((uint32_t)0x00000018)
#define CNTR_CTRL_DIR_DOWN                          ((uint32_t)0x00000100)

/*****************  Bit definition for TIMER_RegInfo:filter_ctrl register  ******************/
#define CNTR_CTRL_EN                                ((uint32_t)0x00000001)

/*****************  Bit definition for TIMER_RegInfo:compare_ctrl register  ******************/
#define COMPARE_CTRL_DRV_MATCH_0                    (uint32_t) (0x00000001)
#define COMPARE_CTRL_DRV_MATCH_1                    (uint32_t) (0x00000002)
#define COMPARE_CTRL_DRV_MATCH                      (uint32_t) (COMPARE_CTRL_DRV_MATCH_1|COMPARE_CTRL_DRV_MATCH_0)
#define COMPARE_CTRL_DRV_CYCLE_END_0                (uint32_t) (0x00000004)
#define COMPARE_CTRL_DRV_CYCLE_END_1                (uint32_t) (0x00000008)
#define COMPARE_CTRL_DRV_CYCLE_END                  (uint32_t) (COMPARE_CTRL_DRV_CYCLE_END_1|COMPARE_CTRL_DRV_CYCLE_END_0)
#define COMPARE_CTRL_DRV_START_VAL                  (uint32_t) (0x00000010)
#define COMPARE_CTRL_DRV_STOP_VAL                   (uint32_t) (0x00000040)
#define COMPARE_CTRL_DRV_START_STOP_LEVEL           (uint32_t) (0x00000080)
#define COMPARE_CTRL_DRV_DRIVER_EN                  (uint32_t) (0x00000100)
#define COMPARE_CTRL_DRV_DISABLE_VAL                (uint32_t) (0x00000200)
#define COMPARE_CTRL_DRV_COMPARE_EN                 (uint32_t) (0x00000800)
#define COMPARE_CTRL_DRV_COMPARE_TRIG_EN            (uint32_t) (0x00001000)
#define COMPARE_CTRL_DRV_DMA_CLEAR_EN               (uint32_t) (0x00002000)

/*****************  Bit definition for TIMER_RegInfo:buf_op_ctrl register  ******************/
#define BUF_OP_CTRL_CAPTURE_BUF_EN                  ((uint32_t)0x00000001)
#define BUF_OP_CTRL_CNTR_BUF_EN                     ((uint32_t)0x00000002)
#define BUF_OP_CTRL_COMPARE_BUF_EN                  ((uint32_t)0x00000004)
#define BUF_OP_CTRL_CAPTURE_A_BUF_OP_BIT0           ((uint32_t)0x00010000)
#define BUF_OP_CTRL_CAPTURE_A_BUF_OP_BIT1           ((uint32_t)0x00020000)
#define BUF_OP_CTRL_CAPTURE_A_BUF_OP                ((uint32_t)(BUF_OP_CTRL_CAPTURE_A_BUF_OP_BIT0|BUF_OP_CTRL_CAPTURE_A_BUF_OP_BIT1))
#define BUF_OP_CTRL_CAPTURE_B_BUF_OP_BIT0           ((uint32_t)0x00040000)
#define BUF_OP_CTRL_CAPTURE_B_BUF_OP_BIT1           ((uint32_t)0x00080000)
#define BUF_OP_CTRL_CAPTURE_B_BUF_OP                ((uint32_t)(BUF_OP_CTRL_CAPTURE_B_BUF_OP_BIT0|BUF_OP_CTRL_CAPTURE_B_BUF_OP_BIT1))
#define BUF_OP_CTRL_CNTR_BUF_OP_BIT0                ((uint32_t)0x00100000)
#define BUF_OP_CTRL_CNTR_BUF_OP_BIT1                ((uint32_t)0x00200000)
#define BUF_OP_CTRL_CNTR_BUF_OP                     ((uint32_t)(BUF_OP_CTRL_CNTR_BUF_OP_BIT0|BUF_OP_CTRL_CNTR_BUF_OP_BIT1))
#define BUF_OP_CTRL_FORCE_COMPARE_BUF_OP            ((uint32_t)0x00400000)
#define BUF_OP_CTRL_COMPARE_A_BUF_EVENT_BIT0        ((uint32_t)0x01000000)
#define BUF_OP_CTRL_COMPARE_A_BUF_EVENT_BIT1        ((uint32_t)0x02000000)
#define BUF_OP_CTRL_COMPARE_A_BUF_EVENT             ((uint32_t)(BUF_OP_CTRL_COMPARE_A_BUF_EVENT_BIT0|BUF_OP_CTRL_COMPARE_A_BUF_EVENT_BIT1))
#define BUF_OP_CTRL_COMPARE_A_BUF_OP                ((uint32_t)0x04000000)
#define BUF_OP_CTRL_COMPARE_B_BUF_EVENT_BIT0        ((uint32_t)0x10000000)
#define BUF_OP_CTRL_COMPARE_B_BUF_EVENT_BIT1        ((uint32_t)0x20000000)
#define BUF_OP_CTRL_COMPARE_B_BUF_EVENT             ((uint32_t)(BUF_OP_CTRL_COMPARE_B_BUF_EVENT_BIT0|BUF_OP_CTRL_COMPARE_B_BUF_EVENT_BIT1))
#define BUF_OP_CTRL_COMPARE_B_BUF_OP                ((uint32_t)0x40000000)

/*****************  Bit definition for TIMER_RegInfo:chan_status register  ******************/
#define CHAN_STATUS_CAPTURE_A                       ((uint32_t)0x00000001)
#define CHAN_STATUS_CAPTURE_B                       ((uint32_t)0x00000002)
#define CHAN_STATUS_UNDER_FLOW                      ((uint32_t)0x00000040)
#define CHAN_STATUS_OVER_FLOW                       ((uint32_t)0x00000080)
#define CHAN_STATUS_CNTR_RUNNING                    ((uint32_t)0x00004000)
#define CHAN_STATUS_CNTR_DIR                        ((uint32_t)0x00008000)
#define CHAN_STATUS_COMPARE_A_UP                    ((uint32_t)0x00010000)
#define CHAN_STATUS_COMPARE_A_DOWN                  ((uint32_t)0x00020000)
#define CHAN_STATUS_COMPARE_B_UP                    ((uint32_t)0x00040000)
#define CHAN_STATUS_COMPARE_B_DOWN                  ((uint32_t)0x00080000)
#define CHAN_STATUS_DRV_A                           ((uint32_t)0x08000000)
#define CHAN_STATUS_DRV_B                           ((uint32_t)0x10000000)
#define CHAN_STATUS_DRV_A_B_1                       ((uint32_t)0x20000040)
#define CHAN_STATUS_DRV_A_B_0                       ((uint32_t)0x40000080)

/*****************  Bit definition for TIMER_RegInfo:chan_interrupt register  ******************/
#define CHAN_INTERRUPT_CAPTURE_A_MASK               ((uint32_t)0x00000001)
#define CHAN_INTERRUPT_CAPTURE_B_MASK               ((uint32_t)0x00000002)
#define CHAN_INTERRUPT_COMPARE_A_BUF1_MASK          ((uint32_t)0x00000004)
#define CHAN_INTERRUPT_COMPARE_A_BUF2_MASK          ((uint32_t)0x00000008)
#define CHAN_INTERRUPT_COMPARE_B_BUF1_MASK          ((uint32_t)0x00000010)
#define CHAN_INTERRUPT_COMPARE_B_BUF2_MASK          ((uint32_t)0x00000020)
#define CHAN_INTERRUPT_UNDER_FLOW_MASK              ((uint32_t)0x00000040)
#define CHAN_INTERRUPT_OVER_FLOW_MASK               ((uint32_t)0x00000080)


/*****************  Bit definition for TIMER_RegInfo:duty_cycle_ctrl register  ******************/
#define DUTY_CYCLE_CTRL_DC_ENABLE_A                 ((uint32_t)0x00000001)
#define DUTY_CYCLE_CTRL_DC_FORCE_A                  ((uint32_t)0x00000002)
#define DUTY_CYCLE_CTRL_DC_SETTING_A                ((uint32_t)0x0000000A)
#define DUTY_CYCLE_CTRL_DC_OVERFLOW_A               ((uint32_t)0x00000010)
#define DUTY_CYCLE_CTRL_DC_UNDERFLOW_A              ((uint32_t)0x00000020)
#define DUTY_CYCLE_CTRL_DC_ENABLE_B                 ((uint32_t)0x00000100)
#define DUTY_CYCLE_CTRL_DC_FORCE_B                  ((uint32_t)0x00000200)
#define DUTY_CYCLE_CTRL_DC_SETTING_B                ((uint32_t)0x00000A00)
#define DUTY_CYCLE_CTRL_DC_OVERFLOW_B               ((uint32_t)0x00001000)
#define DUTY_CYCLE_CTRL_DC_UNDERFLOW_B              ((uint32_t)0x00002000)

/*****************  Bit definition for TIMER_RegInfo:dead_time_ctrl register  ******************/
#define DEAD_TIME_CTRL_DT_EN                        ((uint32_t)0x00000001)
#define DEAD_TIME_CTRL_DT_BUF_EN                    ((uint32_t)0x00000002)

/*****************  Bit definition for TIMER_RegInfo:int_cntr_ctrl register  ******************/
#define INT_CNTR_CTRL_INT_CNTR_EN                   ((uint32_t)0x00010000)

/*****************  Bit definition for TIMER_RegInfo:fault_ctrl register  ******************/
#define FAULT_CTRL_FAULT_EN_TRIG0                   ((uint32_t)0x00000001)
#define FAULT_CTRL_FAULT_EN_TRIG1                   ((uint32_t)0x00000002)
#define FAULT_CTRL_FAULT_EN_TRIG2                   ((uint32_t)0x00000004)
#define FAULT_CTRL_FAULT_EN_TRIG3                   ((uint32_t)0x00000008)
#define FAULT_CTRL_FAULT_EN                         ((uint32_t)(FAULT_CTRL_FAULT_EN_A_TRIG0|FAULT_CTRL_FAULT_EN_A_TRIG1| \
                                                                FAULT_CTRL_FAULT_EN_A_TRIG2|FAULT_CTRL_FAULT_EN_A_TRIG3))
#define FAULT_CTRL_FAULT_POL_HIGH_TRIG0             ((uint32_t)0x00000010)
#define FAULT_CTRL_FAULT_POL_HIGH_TRIG1             ((uint32_t)0x00000020)
#define FAULT_CTRL_FAULT_POL_HIGH_TRIG2             ((uint32_t)0x00000040)
#define FAULT_CTRL_FAULT_POL_HIGH_TRIG3             ((uint32_t)0x00000080)
#define FAULT_CTRL_FAULT_TYPE                       ((uint32_t)0x00000100)

/*****************  Bit definition for TIMER_RegInfo:glb_cntr_start register  ******************/
#define GLB_CNTR_START                              ((uint32_t)0x0000FFFF)

/*****************  Bit definition for TIMER_RegInfo:glb_cntr_stop register  ******************/
#define GLB_CNTR_STOP                               ((uint32_t)0x0000FFFF)

/*****************  Bit definition for TIMER_RegInfo:glb_cntr_clear register  ******************/
#define GLB_CNTR_CLEAR                              ((uint32_t)0x000007FF)

/*****************  Bit definition for TIMER_RegInfo:glb_cntr_running register  ******************/
#define GLB_CNTR_RUNNING                            ((uint32_t)0x0000FFFF)

/*****************  Bit definition for TIMER_RegInfo:glb_driver_oen register  ******************/
#define GLB_DRIVER_CHAN_A_OEN                       (1U)
#define GLB_DRIVER_CHAN_B_OEN                       (2U)
#define GLB_DRIVER_CHAN_OEN                         (3U)

/*****************  Bit definition for TIMER_RegInfo:glb_clk_en register  ******************/
#define GLB_CLK_EN                                  ((uint32_t)0x0000FFFF)


/**
 * enum UTIMER_TRIGGER_FOR.
 * UTIMER trigger target.
 */
typedef enum _UTIMER_TRIGGER_TARGET {
    UTIMER_TRIGGER_START,                       /**< Trigger target UTIMER Channel counter start >*/
    UTIMER_TRIGGER_STOP,                        /**< Trigger target UTIMER Channel counter stop >*/
    UTIMER_TRIGGER_CLEAR,                       /**< Trigger target UTIMER Channel counter clear >*/
    UTIMER_TRIGGER_UPCOUNT,                     /**< Trigger target UTIMER Channel Up count >*/
    UTIMER_TRIGGER_DOWNCOUNT,                   /**< Trigger target UTIMER Channel Down count >*/
    UTIMER_TRIGGER_CAPTURE_A,                   /**< Trigger target UTIMER Channel to capture counter value in Drive A >*/
    UTIMER_TRIGGER_CAPTURE_B,                   /**< Trigger target UTIMER Channel to capture counter value in Drive B >*/
    UTIMER_TRIGGER_DMA_CLEAR_A,                 /**< Trigger target UTIMER Channel to clear Drive A DMA action >*/
    UTIMER_TRIGGER_DMA_CLEAR_B                  /**< Trigger target UTIMER Channel to clear Drive B DMA action >*/
} UTIMER_TRIGGER_TARGET;

/**
 * enum UTIMER_TRIGGER_SRC.
 * UTIMER external event source type.
 */
typedef enum _UTIMER_TRIGGER_SRC {
    UTIMER_SRC_0,                              /**< global triggers >*/
    UTIMER_SRC_1,                              /**< channel events >*/
    UTIMER_FAULT_TRIGGER,                      /**< fault triggers >*/
    UTIMER_CNTR_PAUSE_TRIGGER                  /**< counter pause triggers >*/
} UTIMER_TRIGGER_SRC;

/**
 * enum UTIMER_COUNTER.
 * UTIMER counters.
 */
typedef enum _UTIMER_COUNTER {
    UTIMER_CNTR,                                /**< UTIMER counter >*/
    UTIMER_CNTR_PTR,                            /**< UTIMER pointer counter >*/
    UTIMER_CNTR_PTR_BUF1,                       /**< UTIMER pointer buffer 1 counter >*/
    UTIMER_CNTR_PTR_BUF2,                       /**< UTIMER pointer buffer 2 counter >*/
    UTIMER_DT_UP,                               /**< UTIMER Dead Time UP >*/
    UTIMER_DT_UP_BUF1,                          /**< UTIMER Dead Time UP buffer 1 >*/
    UTIMER_DT_DOWN,                             /**< UTIMER Dead Time DOWN >*/
    UTIMER_DT_DOWN_BUF1,                        /**< UTIMER Dead Time DOWN buffer 1 >*/
    UTIMER_COMPARE_A,                           /**< UTIMER compare A >*/
    UTIMER_COMPARE_B,                           /**< UTIMER compare B >*/
    UTIMER_COMPARE_A_BUF1,                      /**< UTIMER compare A buffer 1 >*/
    UTIMER_COMPARE_B_BUF1,                      /**< UTIMER compare B buffer 1 >*/
    UTIMER_COMPARE_A_BUF2,                      /**< UTIMER compare A buffer 2 >*/
    UTIMER_COMPARE_B_BUF2,                      /**< UTIMER compare B buffer 1 >*/
    UTIMER_CAPTURE_A,                           /**< UTIMER capture A  >*/
    UTIMER_CAPTURE_B,                           /**< UTIMER capture B >*/
    UTIMER_CAPTURE_A_BUF1,                      /**< UTIMER capture A buffer 1 >*/
    UTIMER_CAPTURE_B_BUF1,                      /**< UTIMER capture B buffer 1 >*/
    UTIMER_CAPTURE_A_BUF2,                      /**< UTIMER capture A buffer 2 >*/
    UTIMER_CAPTURE_B_BUF2,                      /**< UTIMER capture B buffer 2 >*/
} UTIMER_COUNTER;

/**
 * enum UTIMER_MODE.
 * UTIMER modes.
 */
typedef enum _UTIMER_MODE {
    UTIMER_MODE_BASIC,                                  /**< UTIMER Channel mode configure to Basic mode >*/
    UTIMER_MODE_BUFFERING,                              /**< UTIMER Channel mode configure to Buffering mode >*/
    UTIMER_MODE_TRIGGERING,                             /**< UTIMER Channel mode configure to Triggering mode >*/
    UTIMER_MODE_CAPTURING,                              /**< UTIMER Channel mode configure to Capturing mode >*/
    UTIMER_MODE_COMPARING,                              /**< UTIMER Channel mode configure to Comparing mode >*/
    UTIMER_MODE_DEAD_TIME,                              /**< UTIMER Channel mode configure to Dead Timer mode >*/
} UTIMER_MODE;

/**
 * enum UTIMER_COUNTER_TYPE.
 * UTIMER counter type.
 */
typedef enum _UTIMER_COUNTER_DIR {
    UTIMER_COUNTER_UP,                                  /**< UTIMER Channel counter direction up >*/
    UTIMER_COUNTER_DOWN,                                /**< UTIMER Channel counter direction down >*/
    UTIMER_COUNTER_TRIANGLE                             /**< UTIMER Channel counter direction triangle >*/
} UTIMER_COUNTER_DIR;


/** \brief UTIMER trigger configuration. */
typedef struct _UTIMER_TRIGGER_CONFIG {
    UTIMER_TRIGGER_TARGET      trigger_target;            /**< UTIMER trigger target >*/
    UTIMER_TRIGGER_SRC         src_type;                  /**< UTIMER trigger source type >*/
    uint32_t                   trigger_type;              /**< UTIMER trigger type >*/
} UTIMER_TRIGGER_CONFIG ;

/**
  * @brief UTIMER_UTIMER_CHANNEL_CFG [UTIMER_CHANNEL_CFG]
  */
typedef struct {
    volatile        uint32_t  UTIMER_START_0_SRC;           /*!< (@ 0x00000000) Channel (n) Counter Start Source 0 Register          */
    volatile        uint32_t  UTIMER_START_1_SRC;           /*!< (@ 0x00000004) Channel (n) Counter Start Source 1 Register          */
    volatile        uint32_t  UTIMER_STOP_0_SRC;            /*!< (@ 0x00000008) Channel (n) Counter Stop Source 0 Register           */
    volatile        uint32_t  UTIMER_STOP_1_SRC;            /*!< (@ 0x0000000C) Channel (n) Counter Stop Source 1 Register           */
    volatile        uint32_t  UTIMER_CLEAR_0_SRC;           /*!< (@ 0x00000010) Channel (n) Counter Clear Source 0 Register          */
    volatile        uint32_t  UTIMER_CLEAR_1_SRC;           /*!< (@ 0x00000014) Channel (n) Counter Clear Source 1 Register          */
    volatile        uint32_t  UTIMER_UP_0_SRC;              /*!< (@ 0x00000018) Channel (n) Counter Up Count Source 0 Register       */
    volatile        uint32_t  UTIMER_UP_1_SRC;              /*!< (@ 0x0000001C) Channel (n) Counter Up Count Source 1 Register       */
    volatile        uint32_t  UTIMER_DOWN_0_SRC;            /*!< (@ 0x00000020) Channel (n) Counter Down Count Source 0 Register     */
    volatile        uint32_t  UTIMER_DOWN_1_SRC;            /*!< (@ 0x00000024) Channel (n) Counter Down Count Source 1 Register     */
    volatile        uint32_t  UTIMER_TRIG_CAPTURE_SRC_A_0;  /*!< (@ 0x00000028) Channel (n) Trigger Capture Source A 0 Register      */
    volatile        uint32_t  UTIMER_TRIG_CAPTURE_SRC_A_1;  /*!< (@ 0x0000002C) Channel (n) Trigger Capture Source A 1 Register      */
    volatile        uint32_t  UTIMER_TRIG_CAPTURE_SRC_B_0;  /*!< (@ 0x00000030) Channel (n) Trigger Capture Source B 0 Register      */
    volatile        uint32_t  UTIMER_TRIG_CAPTURE_SRC_B_1;  /*!< (@ 0x00000034) Channel (n) Trigger Capture Source B 1 Register      */
    volatile        uint32_t  UTIMER_DMA_CLEAR_SRC_A_0;     /*!< (@ 0x00000038) Channel (n) DMA Clear Source A 0 Register            */
    volatile        uint32_t  UTIMER_DMA_CLEAR_SRC_A_1;     /*!< (@ 0x0000003C) Channel (n) DMA Clear Source A 1 Register            */
    volatile        uint32_t  UTIMER_DMA_CLEAR_SRC_B_0;     /*!< (@ 0x00000040) Channel (n) DMA Clear Source B 0 Register            */
    volatile        uint32_t  UTIMER_DMA_CLEAR_SRC_B_1;     /*!< (@ 0x00000044) Channel (n) DMA Clear Source B 1 Register            */
    volatile        uint32_t  UTIMER_CNTR_PAUSE_SRC;        /*!< (@ 0x00000048) Channel (n) Counter Pause Source Register            */
    volatile const  uint32_t  RESERVED[13];
    volatile        uint32_t  UTIMER_CNTR_CTRL;             /*!< (@ 0x00000080) Channel (n) Counter Control Register                 */
    volatile        uint32_t  UTIMER_FILTER_CTRL_A;         /*!< (@ 0x00000084) Channel (n) Filter Control A Register                */
    volatile        uint32_t  UTIMER_FILTER_CTRL_B;         /*!< (@ 0x00000088) Channel (n) Filter Control B Register                */
    volatile        uint32_t  UTIMER_COMPARE_CTRL_A;        /*!< (@ 0x0000008C) Channel (n) Compare Control A Register               */
    volatile        uint32_t  UTIMER_COMPARE_CTRL_B;        /*!< (@ 0x00000090) Channel (n) Compare Control B Register               */
    volatile        uint32_t  UTIMER_BUF_OP_CTRL;           /*!< (@ 0x00000094) Channel (n) Buffer Operation Control Register        */
    volatile const  uint32_t  RESERVED1[2];
    volatile        uint32_t  UTIMER_CNTR;                  /*!< (@ 0x000000A0) Channel (n) Counter Register                         */
    volatile        uint32_t  UTIMER_CNTR_PTR;              /*!< (@ 0x000000A4) Channel (n) Counter Pointer Register                 */
    volatile        uint32_t  UTIMER_CNTR_PTR_BUF1;         /*!< (@ 0x000000A8) Channel (n) Counter Pointer Buffer 1 Register        */
    volatile        uint32_t  UTIMER_CNTR_PTR_BUF2;         /*!< (@ 0x000000AC) Channel (n) Counter Pointer Buffer 2 Register        */
    volatile        uint32_t  UTIMER_CAPTURE_A;             /*!< (@ 0x000000B0) Channel (n) Capture A Register                       */
    volatile        uint32_t  UTIMER_CAPTURE_A_BUF1;        /*!< (@ 0x000000B4) Channel (n) Capture A Buffer 1 Register              */
    volatile        uint32_t  UTIMER_CAPTURE_A_BUF2;        /*!< (@ 0x000000B8) Channel (n) Capture A Buffer 2 Register              */
    volatile const  uint32_t  RESERVED2;
    volatile        uint32_t  UTIMER_CAPTURE_B;             /*!< (@ 0x000000C0) Channel (n) Capture B Register                       */
    volatile        uint32_t  UTIMER_CAPTURE_B_BUF1;        /*!< (@ 0x000000C4) Channel (n) Capture B Buffer 1 Register              */
    volatile        uint32_t  UTIMER_CAPTURE_B_BUF2;        /*!< (@ 0x000000C8) Channel (n) Capture B Buffer 2 Register              */
    volatile const  uint32_t  RESERVED3;
    volatile        uint32_t  UTIMER_COMPARE_A;             /*!< (@ 0x000000D0) Channel (n) Compare A Register                       */
    volatile        uint32_t  UTIMER_COMPARE_A_BUF1;        /*!< (@ 0x000000D4) Channel (n) Compare A Buffer 1 Register              */
    volatile        uint32_t  UTIMER_COMPARE_A_BUF2;        /*!< (@ 0x000000D8) Channel (n) Compare A Buffer 2 Register              */
    volatile const  uint32_t  RESERVED4;
    volatile        uint32_t  UTIMER_COMPARE_B;             /*!< (@ 0x000000E0) Channel (n) Compare B Register                       */
    volatile        uint32_t  UTIMER_COMPARE_B_BUF1;        /*!< (@ 0x000000E4) Channel (n) Compare B Buffer 1 Register              */
    volatile        uint32_t  UTIMER_COMPARE_B_BUF2;        /*!< (@ 0x000000E8) Channel (n) Compare B Buffer 2 Register              */
    volatile const  uint32_t  RESERVED5;
    volatile        uint32_t  UTIMER_DT_UP;                 /*!< (@ 0x000000F0) Channel (n) Dead-time Up Register                    */
    volatile        uint32_t  UTIMER_DT_UP_BUF1;            /*!< (@ 0x000000F4) Channel (n) Dead-time Up Buffer 1 Register           */
    volatile        uint32_t  UTIMER_DT_DOWN;               /*!< (@ 0x000000F8) Channel (n) Dead-time Down Register                  */
    volatile        uint32_t  UTIMER_DT_DOWN_BUF1;          /*!< (@ 0x000000FC) Channel (n) Dead-time Down Buffer 1 Register         */
    volatile const  uint32_t  RESERVED6[5];
    volatile        uint32_t  UTIMER_CHAN_STATUS;           /*!< (@ 0x00000114) Channel (n) Status Register                          */
    volatile        uint32_t  UTIMER_CHAN_INTERRUPT;        /*!< (@ 0x00000118) Channel (n) Interrupt Control Register               */
    volatile        uint32_t  UTIMER_CHAN_INTERRUPT_MASK;   /*!< (@ 0x0000011C) Channel (n) Interrupt Mask Register                  */
    volatile        uint32_t  UTIMER_DUTY_CYCLE_CTRL;       /*!< (@ 0x00000120) Channel (n) Duty Cycle Control Register              */
    volatile        uint32_t  UTIMER_DEAD_TIME_CTRL;        /*!< (@ 0x00000124) Channel (n) Dead-time Control Register               */
    volatile const  uint32_t  RESERVED7[2];
    volatile        uint32_t  UTIMER_INT_CNTR_CTRL;         /*!< (@ 0x00000130) Channel (n) Interrupt Counter Control Register       */
    volatile        uint32_t  UTIMER_FAULT_CTRL;            /*!< (@ 0x00000134) Channel (n) Fault Control Register                   */
    volatile const  uint32_t  RESERVED8[946];
} UTIMER_UTIMER_CHANNEL_CFG_Type;                           /*!< Size = 4096 (0x1000)                                                */

/**
  * @brief UTIMER (UTIMER)
  */
typedef struct {
    volatile        uint32_t  UTIMER_GLB_CNTR_START;        /*!< (@ 0x00000000) Channels Global Counter Start Register               */
    volatile        uint32_t  UTIMER_GLB_CNTR_STOP;         /*!< (@ 0x00000004) Channels Global Counter Stop Register                */
    volatile        uint32_t  UTIMER_GLB_CNTR_CLEAR;        /*!< (@ 0x00000008) Channels Global Counter Clear Register               */
    volatile const  uint32_t  UTIMER_GLB_CNTR_RUNNING;      /*!< (@ 0x0000000C) Channels Global Counter Running Status Register      */
    volatile        uint32_t  UTIMER_GLB_DRIVER_OEN;        /*!< (@ 0x00000010) Channels Driver Output Enable Register               */
    volatile const  uint32_t  RESERVED[3];
    volatile        uint32_t  UTIMER_GLB_CLOCK_ENABLE;      /*!< (@ 0x00000020) Channels Clock Enable Register                       */
    volatile const  uint32_t  RESERVED1[1015];
    volatile        UTIMER_UTIMER_CHANNEL_CFG_Type UTIMER_CHANNEL_CFG[16];/*!< (@ 0x00001000) [0..15]                                */
} UTIMER_Type;                                              /*!< Size = 69632 (0x11000)                                              */

typedef struct _utimer_channel_config
{
    bool     utimer_mode;                            /**< SET: UTIMER(channel 0-11), CLEAR: QEC(channel 12-15) >*/
    bool     driver_A;                               /**< output drive type A, SET: enabled, CLEAR: disabled >*/
    bool     driver_B;                               /**< output drive type B, SET: enabled, CLEAR: disabled >*/
    bool     dma_ctrl;                               /**< SET: Enable DMA control, CLEAR: Disable DMA control >*/
    bool     fault_type;                             /**< For fault triggers: SET: low until counter stop, CLEAR: until overflow/underflow event >*/
    bool     fixed_buffer;                           /**< SET: Enable Fixed buffer feature, CLEAR: Disable Fixed buffer feature in compare mode >*/
    bool     driver_a_start_state;                   /**< initial state of output driver A, SET: Driver state is HIGH, CLEAR: Driver state is LOW >*/
    bool     driver_a_stop_state;                    /**< end state of output driver A, SET: Driver state is HIGH, CLEAR: Driver state is LOW >*/
    bool     buffering_type;                         /**< Buffering type, SET: Double, CLEAR: Single >*/
    bool     driver_b_start_state;                   /**< initial state of output driver B, SET: Driver state is HIGH, CLEAR: Driver state is LOW >*/
    bool     driver_b_stop_state;                    /**< end state of output driver B, SET: Driver state is HIGH, CLEAR: Driver state is LOW >*/
    bool     comp_buffer_at_crest;                   /**< buffering at crest transfer >*/
    bool     comp_buffer_at_trough;                  /**< buffering at trough transfer >*/
    bool     buffer_operation;                       /**< buffer operation enable for capture/compare/dt modes >*/
    bool     buf_trough_n_crest;                     /**< buffer config for triangle timer- SET: buffer at crest&trough, CLEAR: buffer at trough >*/
    uint8_t  capt_buffer_type_A;                     /**< Only for Capture mode: Buffering type for Drive A Single or Double >*/
    uint8_t  capt_buffer_type_B;                     /**< Only for Capture mode: Buffering type for Drive B Single or Double >*/
    uint8_t  driver_a_at_comp_match;                 /**< COMPARE: state of driver A output on count matches with compare buffer values >*/
    uint8_t  driver_a_at_cycle_end;                  /**< state of driver A output on end of cycle, counter_overflow/counter_underflow >*/
    uint8_t  driver_b_at_comp_match;                 /**< COMPARE: state of driver B output on count matches with compare buffer values >*/
    uint8_t  driver_b_at_cycle_end;                  /**< state of driver B output on end of cycle, counter_overflow/counter_underflow >*/
    uint8_t  dc_value;                               /**< UTIMER duty cycle value: 0/1: compare match or 2: 0% or 3: 100% >*/
} utimer_channel_config;

/**
  \fn           static inline void utimer_clock_enable (UTIMER_Type *utimer, uint8_t channel)
  \brief        Enable utimer channel clock
  \param[in]    utimer   : Pointer to utimer register block
  \param[in]    channel  : utimer channel number
  \return       none
*/
static inline void utimer_clock_enable (UTIMER_Type *utimer, uint8_t channel)
{
    utimer->UTIMER_GLB_CLOCK_ENABLE |= (1 << channel);
}

/**
  \fn           static inline void utimer_clock_disable (UTIMER_Type *utimer, uint8_t channel)
  \brief        Disable utimer channel clock
  \param[in]    utimer   : Pointer to utimer register block
  \param[in]    channel  : utimer channel number
  \return       none
*/
static inline void utimer_clock_disable (UTIMER_Type *utimer, uint8_t channel)
{
    utimer->UTIMER_GLB_CLOCK_ENABLE &= ~(1 << channel);
}

/**
  \fn           static inline void utimer_control_enable (UTIMER_Type *utimer, uint8_t channel)
  \brief        Enable utimer control
  \param[in]    utimer   : Pointer to utimer register block
  \param[in]    channel  : utimer channel number
  \return       none
*/
static inline void utimer_control_enable (UTIMER_Type *utimer, uint8_t channel)
{
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_START_1_SRC = CNTR_SRC1_PGM_EN;
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_STOP_1_SRC = CNTR_SRC1_PGM_EN;
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_CLEAR_1_SRC = CNTR_SRC1_PGM_EN;
}

/**
  \fn           static inline void utimer_control_disable (UTIMER_Type *utimer, uint8_t channel)
  \brief        Disable utimer control
  \param[in]    utimer   : Pointer to utimer register block
  \param[in]    channel  : utimer channel number
  \return       none
*/
static inline void utimer_control_disable (UTIMER_Type *utimer, uint8_t channel)
{
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_START_1_SRC = ~CNTR_SRC1_PGM_EN;
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_STOP_1_SRC = ~CNTR_SRC1_PGM_EN;
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_CLEAR_1_SRC = ~CNTR_SRC1_PGM_EN;
}

/**
  \fn           static inline void utimer_driver_output_enable (UTIMER_Type *utimer, uint8_t channel, utimer_channel_config *ch_config)
  \brief        Enable utimer channel driver output
  \param[in]    utimer    : Pointer to utimer register block
  \param[in]    channel   : utimer channel number
  \param[in]    ch_config : pointer for utimer channel configuration structure
  \return       none
*/
static inline void utimer_driver_output_enable (UTIMER_Type *utimer, uint8_t channel, utimer_channel_config *ch_config)
{
    if (ch_config->driver_A)
    {
        utimer->UTIMER_GLB_DRIVER_OEN &= ~(GLB_DRIVER_CHAN_A_OEN << (channel << 1));
    }
    if (ch_config->driver_B)
    {
        utimer->UTIMER_GLB_DRIVER_OEN &= ~(GLB_DRIVER_CHAN_B_OEN << (channel << 1));
    }
}

/**
  \fn           static inline void utimer_driver_output_disable (UTIMER_Type *utimer, uint8_t channel)
  \brief        Disable utimer channel driver output
  \param[in]    utimer    : Pointer to utimer register block
  \param[in]    channel   : utimer channel number
  \return       none
*/
static inline void utimer_driver_output_disable (UTIMER_Type *utimer, uint8_t channel)
{
        utimer->UTIMER_GLB_DRIVER_OEN |= ((GLB_DRIVER_CHAN_A_OEN | GLB_DRIVER_CHAN_B_OEN) << (channel << 1));
}

/**
  \fn           static inline void utimer_counter_start (UTIMER_Type *utimer, uint8_t channel)
  \brief        Start utimer channel counter
  \param[in]    utimer    : Pointer to utimer register block
  \param[in]    channel   : utimer channel number
  \return       none
*/
static inline void utimer_counter_start (UTIMER_Type *utimer, uint8_t channel)
{
    utimer->UTIMER_GLB_CNTR_START |= (1 << channel);
}

/**
  \fn           static inline bool utimer_counter_running (UTIMER_Type *utimer, uint8_t channel)
  \brief        Read state of utimer channel counter
  \param[in]    utimer    : Pointer to utimer register block
  \param[in]    channel   : utimer channel number
  \return       counter state
*/
static inline bool utimer_counter_running (UTIMER_Type *utimer, uint8_t channel)
{
    return (utimer->UTIMER_GLB_CNTR_RUNNING & (1 << channel)) ? 1 : 0;
}

/**
  \fn           static inline void utimer_counter_stop (UTIMER_Type *utimer, uint8_t channel, bool clear_count)
  \brief        Stop utimer channel counter with counter clear option
  \param[in]    utimer      : Pointer to utimer register block
  \param[in]    channel     : utimer channel number
  \param[in]    clear_count : counter clear option
  \return       none
*/
static inline void utimer_counter_stop (UTIMER_Type *utimer, uint8_t channel, bool clear_count)
{
    utimer->UTIMER_GLB_CNTR_STOP |= (1 << channel);
    if (clear_count)
    {
        utimer->UTIMER_GLB_CNTR_CLEAR |= (1 << channel);
    }
}

/**
  \fn           static inline void utimer_reset (UTIMER_Type *utimer, uint8_t channel)
  \brief        reset utimer channel configurations
  \param[in]    utimer      : Pointer to utimer register block
  \param[in]    channel     : utimer channel number
  \return       none
*/
static inline void utimer_reset (UTIMER_Type *utimer, uint8_t channel)
{
    utimer->UTIMER_GLB_CNTR_START &= ~(1U << channel);
    utimer->UTIMER_GLB_CNTR_STOP  &= ~(1U << channel);
    utimer->UTIMER_GLB_CNTR_CLEAR &= ~(1U << channel);
    utimer->UTIMER_GLB_CLOCK_ENABLE &= ~(1U << channel);
}

/**
  \fn           static inline void utimer_clear_interrupt (UTIMER_Type *utimer, uint8_t channel, uint8_t interrupt)
  \brief        clear utimer channel interrupt
  \param[in]    utimer      : Pointer to utimer register block
  \param[in]    channel     : utimer channel number
  \param[in]    interrupt   : interrupt needs to be cleared
  \return       none
*/
static inline void utimer_clear_interrupt (UTIMER_Type *utimer, uint8_t channel, uint8_t interrupt)
{
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_CHAN_INTERRUPT |= interrupt;
}

/**
  \fn           static inline void utimer_unmask_interrupt (UTIMER_Type *utimer, uint8_t channel, uint8_t interrupt)
  \brief        unmask utimer channel interrupt
  \param[in]    utimer      : Pointer to utimer register block
  \param[in]    channel     : utimer channel number
  \param[in]    interrupt   : interrupt needs to be unmasked
  \return       none
*/
static inline void utimer_unmask_interrupt (UTIMER_Type *utimer, uint8_t channel, uint8_t interrupt)
{
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_CHAN_INTERRUPT_MASK &= ~interrupt;
}

/**
  \fn           static inline void utimer_mask_interrupt (UTIMER_Type *utimer, uint8_t channel, uint8_t interrupt)
  \brief        mask utimer channel interrupt
  \param[in]    utimer      : Pointer to utimer register block
  \param[in]    channel     : utimer channel number
  \param[in]    interrupt   : interrupt needs to be masked
  \return       none
*/
static inline void utimer_mask_interrupt (UTIMER_Type *utimer, uint8_t channel, uint8_t interrupt)
{
    utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_CHAN_INTERRUPT_MASK |= interrupt;
}

/**
  \fn           static inline void utimer_enable_duty_cycle (UTIMER_Type *utimer, uint8_t channel, utimer_channel_config *ch_config)
  \brief        enable utimer channel duty cycle
  \param[in]    utimer      : Pointer to utimer register block
  \param[in]    channel     : utimer channel number
  \param[in]    ch_config   : pointer for utimer channel configuration structure
  \return       none
*/
static inline void utimer_enable_duty_cycle (UTIMER_Type *utimer, uint8_t channel, utimer_channel_config *ch_config)
{
    if (ch_config->driver_A)
    {
        utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_DUTY_CYCLE_CTRL |= (DUTY_CYCLE_CTRL_DC_ENABLE_A |
                                              DUTY_CYCLE_CTRL_DC_FORCE_A |
                                              DUTY_CYCLE_CTRL_DC_UNDERFLOW_A |
                                              (ch_config->dc_value) << 2);
    }
    if (ch_config->driver_B)
    {
        utimer->UTIMER_CHANNEL_CFG[channel].UTIMER_DUTY_CYCLE_CTRL |= (DUTY_CYCLE_CTRL_DC_ENABLE_B |
                                              DUTY_CYCLE_CTRL_DC_FORCE_B |
                                              DUTY_CYCLE_CTRL_DC_UNDERFLOW_B |
                                              (ch_config->dc_value) << 10);
    }
}

/**
  \fn          void utimer_config_direction (UTIMER_Type *utimer, uint8_t channel, UTIMER_TYPE type, utimer_channel_config *ch_config)
  \brief       configure counter type for the UTIMER instance.
  \param[in]   utimer      Pointer to the UTIMER register map
  \param[in]   channel     channel number
  \param[in]   dir         counter direction
  \param[in]   ch_config   Pointer to the UTIMER channel specific config structure
  \return      none
*/
void utimer_config_direction (UTIMER_Type *utimer, uint8_t channel, UTIMER_COUNTER_DIR dir, utimer_channel_config *ch_config);

/**
  \fn          void utimer_config_mode (UTIMER_Type *utimer, uint8_t channel, UTIMER_MODE mode, utimer_channel_config *ch_config)
  \brief       configure counter mode for the UTIMER instance.
  \param[in]   utimer      Pointer to the UTIMER register map
  \param[in]   channel     channel number
  \param[in]   mode        counter mode
  \param[in]   ch_config   Pointer to the UTIMER channel specific config structure
  \return      none
*/
void utimer_config_mode (UTIMER_Type *utimer, uint8_t channel, UTIMER_MODE mode, utimer_channel_config *ch_config);

/**
  \fn          void utimer_set_count (UTIMER_Type *utimer, uint8_t channel, UTIMER_SET_COUNTER_TYPE counter_type, utimer_channel_config *ch_config)
  \brief       set counter value for the UTIMER instance.
  \param[in]   utimer          Pointer to the UTIMER register map
  \param[in]   channel         channel number
  \param[in]   counter_type    counter type
  \param[in]   value           counter value
  \return      none
*/
void utimer_set_count (UTIMER_Type *utimer, uint8_t channel, UTIMER_COUNTER counter_type, uint32_t value);

/**
  \fn          uint32_t utimer_get_count (UTIMER_Type *utimer, uint8_t channel, UTIMER_GET_COUNTER_TYPE counter, utimer_channel_config *ch_config)
  \brief       get counter direction for the UTIMER instance.
  \param[in]   utimer         Pointer to the UTIMER register map
  \param[in]   channel        channel number
  \param[in]   counter_type   counter type
  \return      current counter value
*/
uint32_t utimer_get_count (UTIMER_Type *utimer, uint8_t channel, UTIMER_COUNTER counter_type);

/**
  \fn          void utimer_config_trigger (UTIMER_Type *utimer, uint8_t channel, UT_TRIGGER_CONFIG *config_control, utimer_channel_config *ch_config)
  \brief       configure trigger for the UTIMER instance.
  \param[in]   utimer          Pointer to the UTIMER register map
  \param[in]   channel         channel number
  \param[in]   config_control  Pointer to a trigger configure type argument
  \param[in]   ch_config       Pointer to the UTIMER channel specific config structure
  \return      none
*/
void utimer_config_trigger (UTIMER_Type *utimer, uint8_t channel, UTIMER_TRIGGER_CONFIG *config_control, utimer_channel_config *ch_config);

#ifdef __cplusplus
}
#endif

#endif /* UTIMER_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
