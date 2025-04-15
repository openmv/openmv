/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************/

/*
 * Copyright (c) 2013-2020 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * $Date:        31. March 2020
 * $Revision:    V2.3
 *
 * Project:      SPI (Serial Peripheral Interface) Driver definitions
 */

/* History:
 *  Version 2.3
 *    Removed Simplex Mode (deprecated)
 *    Removed volatile from ARM_SPI_STATUS
 *  Version 2.2
 *    ARM_SPI_STATUS made volatile
 *  Version 2.1
 *    Renamed status flag "tx_rx_busy" to "busy"
 *  Version 2.0
 *    New simplified driver:
 *      complexity moved to upper layer (especially data handling)
 *      more unified API for different communication interfaces
 *    Added:
 *      Slave Mode
 *      Half-duplex Modes
 *      Configurable number of data bits
 *      Support for TI Mode and Microwire
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.01
 *    Added "send_done_event" to Capabilities
 *  Version 1.00
 *    Initial release
 */

/**************************************************************************//**
 * @file     Driver_OSPI.h
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     21-Oct-2022
 * @brief    OSPI Driver header file derived from CMSIS Driver_SPI.h
 ******************************************************************************/

#ifndef DRIVER_OSPI_H_
#define DRIVER_OSPI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_OSPI_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

#define _ARM_Driver_OSPI_(n)                   Driver_OSPI##n
#define  ARM_Driver_OSPI_(n)                   _ARM_Driver_OSPI_(n)

/****** OSPI Control Codes *****/

#define ARM_OSPI_CONTROL_POS                   0
#define ARM_OSPI_CONTROL_MSK                   (0xFFUL << ARM_OSPI_CONTROL_POS)

/*----- OSPI Control Codes: Mode -----*/

#define ARM_OSPI_MODE_INACTIVE                 (0x00UL << ARM_OSPI_CONTROL_POS)     ///< OSPI Inactive
#define ARM_OSPI_MODE_MASTER                   (0x01UL << ARM_OSPI_CONTROL_POS)     ///< OSPI Master (Output on MOSI, Input on MISO); arg = Bus Speed in bps
#define ARM_OSPI_MODE_SLAVE                    (0x02UL << ARM_OSPI_CONTROL_POS)     ///< OSPI Slave  (Output on MISO, Input on MOSI)

/*----- OSPI Control Codes: Mode Parameters: Frame Format -----*/

#define ARM_OSPI_FRAME_FORMAT_POS              8
#define ARM_OSPI_FRAME_FORMAT_MSK              (7UL << ARM_OSPI_FRAME_FORMAT_POS)
#define ARM_OSPI_CPOL0_CPHA0                   (0UL << ARM_OSPI_FRAME_FORMAT_POS)   ///< Clock Polarity 0, Clock Phase 0 (default)
#define ARM_OSPI_CPOL0_CPHA1                   (1UL << ARM_OSPI_FRAME_FORMAT_POS)   ///< Clock Polarity 0, Clock Phase 1
#define ARM_OSPI_CPOL1_CPHA0                   (2UL << ARM_OSPI_FRAME_FORMAT_POS)   ///< Clock Polarity 1, Clock Phase 0
#define ARM_OSPI_CPOL1_CPHA1                   (3UL << ARM_OSPI_FRAME_FORMAT_POS)   ///< Clock Polarity 1, Clock Phase 1
#define ARM_OSPI_TI_SSI                        (4UL << ARM_OSPI_FRAME_FORMAT_POS)   ///< Texas Instruments Frame Format
#define ARM_OSPI_MICROWIRE                     (5UL << ARM_OSPI_FRAME_FORMAT_POS)   ///< National Semiconductor Microwire Frame Format

/*----- OSPI Control Codes: Mode Parameters: Data Bits -----*/

#define ARM_OSPI_DATA_BITS_POS                 12
#define ARM_OSPI_DATA_BITS_MSK                 (0x3FUL << ARM_OSPI_DATA_BITS_POS)
#define ARM_OSPI_DATA_BITS(n)                  (((n) & 0x3FUL) << ARM_OSPI_DATA_BITS_POS) ///< Number of Data bits

/*----- OSPI Control Codes: Mode Parameters: Bit Order -----*/

#define ARM_OSPI_BIT_ORDER_POS                 18
#define ARM_OSPI_BIT_ORDER_MSK                 (1UL << ARM_OSPI_BIT_ORDER_POS)
#define ARM_OSPI_MSB_LSB                       (0UL << ARM_OSPI_BIT_ORDER_POS)      ///< OSPI Bit order from MSB to LSB (default)
#define ARM_OSPI_LSB_MSB                       (1UL << ARM_OSPI_BIT_ORDER_POS)      ///< OSPI Bit order from LSB to MSB

/*----- OSPI Control Codes: Mode Parameters: Slave Select Mode -----*/

#define ARM_OSPI_SS_MASTER_MODE_POS            19
#define ARM_OSPI_SS_MASTER_MODE_MSK            (3UL << ARM_OSPI_SS_MASTER_MODE_POS)
#define ARM_OSPI_SS_MASTER_UNUSED              (0UL << ARM_OSPI_SS_MASTER_MODE_POS) ///< OSPI Slave Select when Master: Not used (default)
#define ARM_OSPI_SS_MASTER_SW                  (1UL << ARM_OSPI_SS_MASTER_MODE_POS) ///< OSPI Slave Select when Master: Software controlled
#define ARM_OSPI_SS_MASTER_HW_OUTPUT           (2UL << ARM_OSPI_SS_MASTER_MODE_POS) ///< OSPI Slave Select when Master: Hardware controlled Output
#define ARM_OSPI_SS_MASTER_HW_INPUT            (3UL << ARM_OSPI_SS_MASTER_MODE_POS) ///< OSPI Slave Select when Master: Hardware monitored Input
#define ARM_OSPI_SS_SLAVE_MODE_POS             21
#define ARM_OSPI_SS_SLAVE_MODE_MSK             (1UL << ARM_OSPI_SS_SLAVE_MODE_POS)
#define ARM_OSPI_SS_SLAVE_HW                   (0UL << ARM_OSPI_SS_SLAVE_MODE_POS)  ///< OSPI Slave Select when Slave: Hardware monitored (default)
#define ARM_OSPI_SS_SLAVE_SW                   (1UL << ARM_OSPI_SS_SLAVE_MODE_POS)  ///< OSPI Slave Select when Slave: Software controlled

/*----- OSPI Control Codes: Miscellaneous Controls  -----*/

#define ARM_OSPI_SET_BUS_SPEED                 (0x10UL << ARM_OSPI_CONTROL_POS)     ///< Set Bus Speed in bps; arg = value
#define ARM_OSPI_GET_BUS_SPEED                 (0x11UL << ARM_OSPI_CONTROL_POS)     ///< Get Bus Speed in bps
#define ARM_OSPI_SET_DEFAULT_TX_VALUE          (0x12UL << ARM_OSPI_CONTROL_POS)     ///< Set default Transmit value; arg = value
#define ARM_OSPI_CONTROL_SS                    (0x13UL << ARM_OSPI_CONTROL_POS)     ///< Control Slave Select; arg: 0=inactive, 1=active
#define ARM_OSPI_ABORT_TRANSFER                (0x14UL << ARM_OSPI_CONTROL_POS)     ///< Abort current data transfer
#define ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE    (0x15UL << ARM_OSPI_CONTROL_POS)
#define ARM_OSPI_SET_FRAME_FORMAT              (0x16UL << ARM_OSPI_CONTROL_POS)
#define ARM_OSPI_SET_DDR_MODE                  (0x17UL << ARM_OSPI_CONTROL_POS)

/*----- OSPI Custom Control codes -----*/

#define ARM_OSPI_ADDR_LENGTH_0_BITS            0x0
#define ARM_OSPI_ADDR_LENGTH_8_BITS            0x2
#define ARM_OSPI_ADDR_LENGTH_24_BITS           0x6
#define ARM_OSPI_ADDR_LENGTH_32_BITS           0x8

#define ARM_OSPI_DDR_DISABLE                   0x0
#define ARM_OSPI_DDR_ENABLE                    0x1

#define ARM_OSPI_FRF_STANDRAD                  0x0    /* 0x0 Standard OSPI Format */
#define ARM_OSPI_FRF_DUAL                      0x1    /* 0x1 Dual OSPI Format */
#define ARM_OSPI_FRF_QUAD                      0x2    /* 0X2 Quad OSPI Format */
#define ARM_OSPI_FRF_OCTAL                     0x3    /* 0X2 Octal OSPI Format */

#define ARM_OSPI_ADDR_LENGTH_POS               0x0
#define ARM_OSPI_WAIT_CYCLE_POS                0x8

/*---- OSPI Slave Select Signal definitions ----*/

#define ARM_OSPI_SS_INACTIVE                   0UL                 //< OSPI Slave Select Signal Inactive
#define ARM_OSPI_SS_ACTIVE                     1UL                 ///< OSPI Slave Select Signal Active

/*----- OSPI specific error codes ----*/

#define ARM_OSPI_ERROR_MODE                    (ARM_DRIVER_ERROR_SPECIFIC - 1)     ///< Specified Mode not supported
#define ARM_OSPI_ERROR_FRAME_FORMAT            (ARM_DRIVER_ERROR_SPECIFIC - 2)     ///< Specified Frame Format not supported
#define ARM_OSPI_ERROR_DATA_BITS               (ARM_DRIVER_ERROR_SPECIFIC - 3)     ///< Specified number of Data bits not supported
#define ARM_OSPI_ERROR_BIT_ORDER               (ARM_DRIVER_ERROR_SPECIFIC - 4)     ///< Specified Bit order not supported
#define ARM_OSPI_ERROR_SS_MODE                 (ARM_DRIVER_ERROR_SPECIFIC - 5)     ///< Specified Slave Select Mode not supported

/**
\brief OSPI Status
*/
typedef struct _ARM_OSPI_STATUS {
    uint32_t busy       : 1;              ///< Transmitter/Receiver busy flag
    uint32_t data_lost  : 1;              ///< Data lost: Receive overflow / Transmit underflow (cleared on start of transfer operation)
    uint32_t mode_fault : 1;              ///< Mode fault detected; optional (cleared on start of transfer operation)
    uint32_t reserved   : 29;
} ARM_OSPI_STATUS;

/****** OSPI Event *****/
#define ARM_OSPI_EVENT_TRANSFER_COMPLETE (1UL << 0)  ///< Data Transfer completed
#define ARM_OSPI_EVENT_DATA_LOST         (1UL << 1)  ///< Data lost: Receive overflow / Transmit underflow
#define ARM_OSPI_EVENT_MODE_FAULT        (1UL << 2)  ///< Master Mode Fault (SS deactivated when Master)

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_OSPI_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_OSPI_CAPABILITIES ARM_OSPI_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_OSPI_CAPABILITIES

  \fn          int32_t ARM_OSPI_Initialize (ARM_OSPI_SignalEvent_t cb_event)
  \brief       Initialize OSPI Interface.
  \param[in]   cb_event  Pointer to \ref ARM_OSPI_SignalEvent
  \return      \ref execution_status

  \fn          int32_t ARM_OSPI_Uninitialize (void)
  \brief       De-initialize OSPI Interface.
  \return      \ref execution_status

  \fn          int32_t ARM_OSPI_PowerControl (ARM_POWER_STATE state)
  \brief       Control OSPI Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status

  \fn          int32_t ARM_OSPI_Send (const void *data, uint32_t num)
  \brief       Start sending data to OSPI transmitter.
  \param[in]   data  Pointer to buffer with data to send to OSPI transmitter
  \param[in]   num   Number of data items to send
  \return      \ref execution_status

  \fn          int32_t ARM_OSPI_Receive (void *data, uint32_t num)
  \brief       Start receiving data from OSPI receiver.
  \param[out]  data  Pointer to buffer for data to receive from OSPI receiver
  \param[in]   num   Number of data items to receive
  \return      \ref execution_status

  \fn          int32_t ARM_OSPI_Transfer (const void *data_out,
                                               void *data_in,
                                         uint32_t    num)
  \brief       Start sending/receiving data to/from OSPI transmitter/receiver.
  \param[in]   data_out  Pointer to buffer with data to send to OSPI transmitter
  \param[out]  data_in   Pointer to buffer for data to receive from OSPI receiver
  \param[in]   num       Number of data items to transfer
  \return      \ref execution_status

  \fn          uint32_t ARM_OSPI_GetDataCount (void)
  \brief       Get transferred data count.
  \return      number of data items transferred

  \fn          int32_t ARM_OSPI_Control (uint32_t control, uint32_t arg)
  \brief       Control OSPI Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \return      common \ref execution_status and driver specific \ref spi_execution_status

  \fn          ARM_OSPI_STATUS ARM_OSPI_GetStatus (void)
  \brief       Get OSPI status.
  \return      OSPI status \ref ARM_OSPI_STATUS

  \fn          void ARM_OSPI_SignalEvent (uint32_t event)
  \brief       Signal OSPI Events.
  \param[in]   event \ref OSPI_events notification mask
  \return      none
*/

typedef void (*ARM_OSPI_SignalEvent_t) (uint32_t event);  ///< Pointer to \ref ARM_OSPI_SignalEvent : Signal OSPI Event.

/**
\brief OSPI Driver Capabilities.
*/
typedef struct _ARM_OSPI_CAPABILITIES {
    uint32_t simplex          : 1;        ///< supports Simplex Mode (Master and Slave) @deprecated Reserved (must be zero)
    uint32_t ti_ssi           : 1;        ///< supports TI Synchronous Serial Interface
    uint32_t microwire        : 1;        ///< supports Microwire Interface
    uint32_t event_mode_fault : 1;        ///< Signal Mode Fault event: \ref ARM_OSPI_EVENT_MODE_FAULT
    uint32_t reserved         : 28;       ///< Reserved (must be zero)
} ARM_OSPI_CAPABILITIES;


/**
\brief Access structure of the OSPI Driver.
*/
typedef struct _ARM_DRIVER_OSPI {
  ARM_DRIVER_VERSION    (*GetVersion)      (void);                             ///< Pointer to \ref ARM_OSPI_GetVersion : Get driver version.
  ARM_OSPI_CAPABILITIES (*GetCapabilities) (void);                             ///< Pointer to \ref ARM_OSPI_GetCapabilities : Get driver capabilities.
  int32_t               (*Initialize)      (ARM_OSPI_SignalEvent_t cb_event);   ///< Pointer to \ref ARM_OSPI_Initialize : Initialize OSPI Interface.
  int32_t               (*Uninitialize)    (void);                             ///< Pointer to \ref ARM_OSPI_Uninitialize : De-initialize OSPI Interface.
  int32_t               (*PowerControl)    (ARM_POWER_STATE state);            ///< Pointer to \ref ARM_OSPI_PowerControl : Control OSPI Interface Power.
  int32_t               (*Send)            (const void *data, uint32_t num);   ///< Pointer to \ref ARM_OSPI_Send : Start sending data to OSPI Interface.
  int32_t               (*Receive)         (      void *data, uint32_t num);   ///< Pointer to \ref ARM_OSPI_Receive : Start receiving data from OSPI Interface.
  int32_t               (*Transfer)        (const void *data_out,
                                                 void *data_in,
                                           uint32_t    num);                  ///< Pointer to \ref ARM_OSPI_Transfer : Start sending/receiving data to/from OSPI.
  uint32_t              (*GetDataCount)    (void);                             ///< Pointer to \ref ARM_OSPI_GetDataCount : Get transferred data count.
  int32_t               (*Control)         (uint32_t control, uint32_t arg);   ///< Pointer to \ref ARM_OSPI_Control : Control OSPI Interface.
  ARM_OSPI_STATUS       (*GetStatus)       (void);                             ///< Pointer to \ref ARM_OSPI_GetStatus : Get OSPI status.
} const ARM_DRIVER_OSPI;

#ifdef  __cplusplus
}
#endif
#endif /* DRIVER_OSPI_H_ */
