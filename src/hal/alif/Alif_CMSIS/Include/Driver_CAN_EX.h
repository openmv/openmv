/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     Driver_CAN_EX.h
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     29-Jan-2024
 * @brief    Extension of CMSIS Driver_CAN.h
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef DRIVER_CAN_EX_H_
#define DRIVER_CAN_EX_H_

#include "Driver_CAN.h"

/****** CAN Control Function Operation codes *****/
#define ARM_CAN_SET_SPECIFICATION                   (5UL << ARM_CAN_CONTROL_Pos)  ///< Set Spec: ISO/Non-ISO. Default: ISO
#define ARM_CAN_SET_RBUF_OVERFLOW_MODE              (6UL << ARM_CAN_CONTROL_Pos)  ///< Set mode: overwrite old msg or Don't store new msg
#define ARM_CAN_SET_RBUF_STORAGE_FORMAT             (7UL << ARM_CAN_CONTROL_Pos)  ///< Rx buf storage format. Default: Normal msgs
#define ARM_CAN_SET_RBUF_ALMOST_FULL_WARN_LIMIT     (8UL << ARM_CAN_CONTROL_Pos)  ///< Rx buf almost full warning limit arg: 1-15. Default: 15
#define ARM_CAN_SET_TRANSMISSION_MODE               (9UL << ARM_CAN_CONTROL_Pos)  ///< Normal Transmission buffer mode; Fifo/Priority. Not for primary buffer
#define ARM_CAN_SET_PRIMARY_TBUF                    (10UL << ARM_CAN_CONTROL_Pos) ///< Set Primary Tx buffer
#define ARM_CAN_ABORT_PRIMARY_TBUF_MESSAGE_SEND     (11UL << ARM_CAN_CONTROL_Pos) ///< Abort Primary buffer message transmission
#define ARM_CAN_CONTROL_PRIMARY_TBUF_RETRANSMISSION (12UL << ARM_CAN_CONTROL_Pos) ///< Enable/disable primary tbuf's automatic retransmission; arg: 0 = disable, 1 = enable (default state)
#define ARM_CAN_SET_TIMER_COUNTER                   (13UL << ARM_CAN_CONTROL_Pos) ///< Set the initial value Timer counter; arg: value
#define ARM_CAN_CONTROL_TIMER_COUNTER               (14UL << ARM_CAN_CONTROL_Pos) ///< Manipulate Timer counter; arg: Start, Stop, Clear
#define ARM_CAN_ENABLE_TIMESTAMP                    (15UL << ARM_CAN_CONTROL_Pos) ///< Enable CAN msg timestamp; arg: timestamp position- SOF/EOF
#define ARM_CAN_DISABLE_TIMESTAMP                   (16UL << ARM_CAN_CONTROL_Pos) ///< Disable CAN msg timestamp; arg: NULL
#define ARM_CAN_GET_TX_TIMESTAMP                    (17UL << ARM_CAN_CONTROL_Pos) ///< Get CAN Tx msg timestamp; arg: Address of variable to store it
#define ARM_CAN_GET_RX_TIMESTAMP                    (18UL << ARM_CAN_CONTROL_Pos) ///< Get CAN Rx msg timestamp; arg: Address of variable to store it

/* CAN Control arguments ISO/Non-ISO modes */
#define ARM_CAN_SPECIFICATION_NON_ISO               1U           ///< Bosch (Non-ISO) Specification
#define ARM_CAN_SPECIFICATION_ISO                   2U           ///< ISO (11898-1:2015) Specification

/* Control arguments for RBUF Overflow mode */
#define ARM_CAN_RBUF_OVERWRITE_OLD_MSG              1U
#define ARM_CAN_RBUF_DISCARD_NEW_MSG                2U

/* Control arguments for RBUF Storage format */
#define ARM_CAN_RBUF_STORAGE_NORMAL_MSG             1U           ///< Stores only normal messages
#define ARM_CAN_RBUF_STORAGE_ALL_MSG                2U           ///< Stores both correct and error data frames

/* Control arguments for Transmission mode */
#define ARM_CAN_SET_TRANSMISSION_MODE_FIFO          1U           ///< Fifo mode; only for normal buffer
#define ARM_CAN_SET_TRANSMISSION_MODE_PRIORITY      2U           ///< Priority mode; only for normal

/* Object filter configuration arguments decoding.
 * User doesn't have to use it*/
#define ARM_CAN_OBJECT_ID(id)                       (id & 0x1FFFFFFFUL)

/* Arguments for Object filter configuration.
 * These arguments should be added to the
 * parameter "id" of ObjectSetFilter() api */
#define ARM_CAN_OBJECT_FILTER_STD_FRAMES            (1U << 30U)
#define ARM_CAN_OBJECT_FILTER_EXT_FRAMES            (3U << 29U)
#define ARM_CAN_OBJECT_FILTER_ALL_FRAMES            (0U)

/* Arguments for Timer Counter Controlling */
#define ARM_CAN_TIMER_COUNTER_START                 (1U << 1U)   ///< Start the counter
#define ARM_CAN_TIMER_COUNTER_STOP                  (1U << 2U)   ///< Stop the counter
#define ARM_CAN_TIMER_COUNTER_CLEAR                 (1U << 3U)   ///< Clear the counter

/* Arguments for CAN Timestamp position */
#define CAN_TIMESTAMP_POSITION_SOF                  (1U << 1U)   ///< Timestamp at start of the frame
#define CAN_TIMESTAMP_POSITION_EOF                  (1U << 2U)   ///< Timestamp at end of the frame

/****** Extended CAN Events *****/
#define ARM_CAN_EVENT_RBUF_ALMOST_FULL              (1UL << 3)   ///< Rx buffer is almost full
#define ARM_CAN_EVENT_ARBITRATION_LOST              (1UL << 4)   ///< Arbitration lost during msg transmission
#define ARM_CAN_EVENT_PRIMARY_TBUF_SEND_COMPLETE    (1UL << 5)   ///< Primary Tx buffer send complete

#endif /* DRIVER_CAN_EX_H_ */
