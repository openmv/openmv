/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef DRIVER_I3C_H_
#define DRIVER_I3C_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_I3C_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(7,2)     /* API version */

/****** I3C Control Codes *****/

/* I3C Control Codes: Bus mode */
#define I3C_MASTER_SET_BUS_MODE                         (1UL << 0)  ///< Set bus mode to pure i3c, mixed i3c + i2c fast etc.
#define I3C_SLAVE_SET_ADDR                              (1UL << 1)  ///< Set slave addr and initialize slave; arg: slave address
#define I3C_MASTER_SET_SLAVE_NACK_RETRY_COUNT           (1UL << 2)  ///< Set retry count to communicate to the slave that nacked the comm. Ref: Slave Nack retry count Control arguments
#define I3C_SET_SDA_TX_HOLD_TIME                        (1UL << 3)  ///< Set SDA Hold time(in terms of the CORE_CLK periods) for FM, FM+, SDR and DDR mode of operations; arg:<1-7>
#define I3C_MASTER_ABORT_MESSAGE_TRANSFER               (1UL << 4)  ///< Abort the current Message transfer
#define I3C_MASTER_SETUP_HOT_JOIN_ACCEPTANCE            (1UL << 5)  ///< Accept/Reject the Hot Join request from slaves; arg: 1-Accept, 0-Reject
#define I3C_MASTER_SETUP_MR_ACCEPTANCE                  (1UL << 6)  ///< Accept/Reject the Master request from slaves; arg: 1-Accept, 0-Reject
#define I3C_SLAVE_REQUEST_IBI_BUS_MASTERSHIP            (1UL << 7)  ///< Request for Bus Mastership
#define I3C_SET_DEVICE_CHARACTERISTICS                  (1UL << 8)  ///< Set Device Characteristics (DCR) value; arg: 0-255
#define I3C_MASTER_INIT                                 (1UL << 9)  ///< Initialises the device as master
#define I3C_MASTER_SETUP_SIR_ACCEPTANCE                 (1UL << 10) ///< Accept/Reject the SIR from slaves; arg: 1-Accept, 0-Reject
#define I3C_SLAVE_SET_IBI_SIR                           (1UL << 11) ///< Set IBI Slave interrupt Request
#define I3C_SLAVE_SET_PID                               (1UL << 12) ///< Set MIPI I3C Slave's 48-bit Provisional ID

/* I3C Control Codes: Bus mode arguments */
#define I3C_BUS_SLOW_MODE                               (0x00UL)    ///< Slow bus mode for pure i3c devices - For slave addressing
#define I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS    (0x01UL)    ///< Mixed i3c + i2c device, Speed: Fast Mode Plus   1 MBPS
#define I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS   (0x02UL)    ///< Mixed i3c + i2c device, Speed: Fast Mode      400 KBPS
#define I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS   (0x03UL)    ///< Mixed i3c + i2c device, Speed: Standard Mode  100 KBPS
#define I3C_BUS_MODE_MIXED_LIMITED                      (0x04UL)
#define I3C_BUS_NORMAL_MODE                             (0x05UL)    ///< Normal bus mode for pure i3c devices - For actual data comm

/* I3C Control arguments: For Slave Nack retry count */
#define I3C_SLAVE_NACK_RETRY_COUNT_Pos                  8U
#define I3C_SLAVE_NACK_RETRY_COUNT_Msk                  (3U << I3C_SLAVE_NACK_RETRY_COUNT_Pos)
#define I3C_SLAVE_NACK_RETRY_COUNT(x)                   ((x << I3C_SLAVE_NACK_RETRY_COUNT_Pos) & I3C_SLAVE_NACK_RETRY_COUNT_Msk) ///< x: 0-3
#define I3C_SLAVE_ADDR(x)                               (x & (0x7FU))

#define I3C_BUS_MAX_DEVS                                0x8

/****** I3C Event *****/
#define ARM_I3C_EVENT_TRANSFER_DONE                     (1UL << 0)  ///< Event Success
#define ARM_I3C_EVENT_TRANSFER_ERROR                    (1UL << 1)  ///< Master and slave Transmit/Receive Error
#define ARM_I3C_EVENT_SLV_DYN_ADDR_ASSGN                (1UL << 2)  ///< Slave Dynamic Address Assigned(only for Slave mode)
#define ARM_I3C_EVENT_MESSAGE_TRANSFER_ABORT            (1UL << 3)  ///< Message transfer is aborted
#define ARM_I3C_EVENT_IBI_HOT_JOIN_REQ                  (1UL << 4)  ///< Hot-Join request received from new slave
#define ARM_I3C_EVENT_IBI_MASTERSHIP_REQ                (1UL << 5)  ///< Slave requested to become bus Master
#define ARM_I3C_EVENT_BUSOWNER_UPDATED                  (1UL << 6)  ///< Bus Owner (Master) updated
#define ARM_I3C_EVENT_SLAVE_CCC_UPDATED                 (1UL << 7)  ///< Slave CCC updated by master
#define ARM_I3C_EVENT_IBI_SLV_INTR_REQ                  (1UL << 8)  ///< Slave requested through SIR
#define ARM_I3C_EVENT_SLAVE_LIST                        (1UL << 9)  ///< Targets info rcvd through DEFSLVS CCC. Applicable only for secondary masters

/****** I3C Error Status codes *****/
#define ARM_I3C_LEC_NO_ERROR                            (0U)             ///< Last error code: No error
#define ARM_I3C_LEC_CRC_ERROR                           (1U)             ///< Last error code: CRC error
#define ARM_I3C_LEC_PARITY_ERROR                        (2U)             ///< Last error code: Parity error
#define ARM_I3C_LEC_FRAME_ERROR                         (3U)             ///< Last error code: Frame error
#define ARM_I3C_LEC_IBA_NACK_ERROR                      (4U)             ///< Last error code: Nack for I3C broadcast address error
#define ARM_I3C_LEC_ADDR_NACK_ERROR                     (5U)             ///< Last error code: Nack for I3C slave address error
#define ARM_I3C_LEC_BUF_UNDER_OVERFLOW_ERROR            (6U)             ///< Last error code: Buffer Overflow/underflow error
#define ARM_I3C_LEC_TRANSFER_ABORT_ERROR                (7U)             ///< Last error code: Transfer abort error
#define ARM_I3C_LEC_I2C_SLAVE_NACK_ERROR                (8U)             ///< Last error code: I2C slave NACK for current write transfer error
#define ARM_I3C_LEC_PEC_BYTE_ERROR                      (9U)             ///< Last error code: PEC byte check error
#define ARM_I3C_LEC_EARLY_TERMINATION_ERROR             (10U)            ///< Last error code: Master early termination error

/* I3C CCC (Common Command Codes) related definitions */
#define I3C_CCC_DIRECT                                  BIT(7)

#define I3C_CCC_ID(id, broadcast)                       ((id) | ((broadcast) ? 0 : I3C_CCC_DIRECT))

/* Commands valid in both broadcast and unicast modes */
#define I3C_CCC_ENEC(broadcast)                         I3C_CCC_ID(0x0, broadcast)
#define I3C_CCC_DISEC(broadcast)                        I3C_CCC_ID(0x1, broadcast)
#define I3C_CCC_ENTAS(as, broadcast)                    I3C_CCC_ID(0x2 + (as), broadcast)
#define I3C_CCC_RSTDAA(broadcast)                       I3C_CCC_ID(0x6, broadcast)
#define I3C_CCC_SETMWL(broadcast)                       I3C_CCC_ID(0x9, broadcast)
#define I3C_CCC_SETMRL(broadcast)                       I3C_CCC_ID(0xa, broadcast)
#define I3C_CCC_SETXTIME(broadcast)                     ((broadcast) ? 0x28 : 0x98)
#define I3C_CCC_VENDOR(id, broadcast)                   ((id) + ((broadcast) ? 0x61 : 0xe0))

/* Broadcast-only commands */
#define I3C_CCC_ENTDAA                                  I3C_CCC_ID(0x7, true)
#define I3C_CCC_DEFSLVS                                 I3C_CCC_ID(0x8, true)
#define I3C_CCC_ENTTM                                   I3C_CCC_ID(0xb, true)
#define I3C_CCC_ENTHDR(x)                               I3C_CCC_ID(0x20 + (x), true)
#define I3C_CCC_SETAASA                                 I3C_CCC_ID(0x29, true)

/* Unicast-only commands */
#define I3C_CCC_SETDASA                                 I3C_CCC_ID(0x7, false)
#define I3C_CCC_SETNEWDA                                I3C_CCC_ID(0x8, false)
#define I3C_CCC_GETMWL                                  I3C_CCC_ID(0xb, false)
#define I3C_CCC_GETMRL                                  I3C_CCC_ID(0xc, false)
#define I3C_CCC_GETPID                                  I3C_CCC_ID(0xd, false)
#define I3C_CCC_GETBCR                                  I3C_CCC_ID(0xe, false)
#define I3C_CCC_GETDCR                                  I3C_CCC_ID(0xf, false)
#define I3C_CCC_GETSTATUS                               I3C_CCC_ID(0x10, false)
#define I3C_CCC_GETACCMST                               I3C_CCC_ID(0x11, false)
#define I3C_CCC_SETBRGTGT                               I3C_CCC_ID(0x13, false)
#define I3C_CCC_GETMXDS                                 I3C_CCC_ID(0x14, false)
#define I3C_CCC_GETHDRCAP                               I3C_CCC_ID(0x15, false)
#define I3C_CCC_GETXTIME                                I3C_CCC_ID(0x19, false)

/* List of some Defining byte values */
#define I3C_CCC_DEF_BYTE_SYNC_TICK                      0x7F
#define I3C_CCC_DEF_BYTE_DELAY_TIME                     0xBF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE0                    0xDF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE1                    0xEF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE2                    0xF7
#define I3C_CCC_DEF_BYTE_ASYNC_MODE3                    0xFB
#define I3C_CCC_DEF_BYTE_ASYNC_TRIG                     0xFD
#define I3C_CCC_DEF_BYTE_TPH                            0x3F
#define I3C_CCC_DEF_BYTE_TU                             0x9F
#define I3C_CCC_DEF_BYTE_ODR                            0x8F

/**
\brief I3C Command
*/
typedef struct _ARM_I3C_CMD {
  uint8_t   rw;
  uint8_t   cmd_id;
  uint8_t   def_byte;
  uint16_t  len;
  uint8_t  *data;
  uint8_t   addr;
} ARM_I3C_CMD;

/**
\brief I3C Status
*/
typedef struct _ARM_I3C_STATUS {
  uint32_t busy            :1;             ///< Busy flag
  uint32_t mode            :1;             ///< Mode: 0=Slave, 1=Master
  uint32_t ibi_slv_addr    :8;             ///< Address of last IBI slave
  uint32_t last_error_code :4;             ///< Last occurred error
  uint32_t defslv_cnt      :8;             ///< Slave count from last DEFSLVS (Applicable for Sec masters only)
  uint32_t reserved        :10;
} ARM_I3C_STATUS;

/**
\brief I3C device characteristics
*/
typedef struct _ARM_I3C_DEV_CHAR {
    uint8_t         static_addr;                ///< Static address
    uint8_t         bcr;                        ///< Bus characteristics
    uint8_t         dcr;                        ///< Device characteristics
    uint8_t         dynamic_addr;               ///< Dynamic address
} ARM_I3C_DEV_CHAR;

/**
\brief I3C Device Provisional ID info
\ The data is as per the MIPI SPEC
*/
typedef struct _ARM_I3C_SLV_PID {
    uint32_t dcr         :12;              ///< Slave DCR
    uint32_t inst_id     :4;               ///< Slave Instance ID
    uint32_t part_id     :16;              ///< Slave Part ID
    uint32_t pid_sel     :1;               ///< Slave PID selection; 0:Vendor fixed, 1: Random
    uint32_t mipi_mfg_id :15;              ///< Slave MIPI Manufacturer ID
    uint32_t reserved    :16;
} ARM_I3C_SLV_PID;

/**
\brief I3C Device Primary information
*/
typedef struct _ARM_I3C_DEV_PRIME_INFO {
    ARM_I3C_DEV_CHAR   dev_char;           ///< Device characteristics
    ARM_I3C_SLV_PID    pid;                ///< Device PID
} ARM_I3C_DEV_PRIME_INFO;

/**
\brief I3C Device information
\ The data is as per the MIPI SPEC
*/
typedef struct _ARM_I3C_DEVICE_INFO {
    ARM_I3C_DEV_PRIME_INFO  prime_info;             ///< Device Primary info
    uint16_t                max_read_len;           ///< Maximum Read length
    uint16_t                max_write_len;          ///< Maximum Write length
    uint32_t                max_read_turnaround;    ///< Maximum Read turnaround time
    uint8_t                 max_read_speed;         ///< Maximum Read speed
    uint8_t                 max_write_speed;        ///< Maximum write speed
    uint16_t                reserved;
} ARM_I3C_DEVICE_INFO;

/**
\brief I3C Device type
*/
typedef enum _ARM_I3C_DEVICE_TYPE {
    ARM_I3C_DEVICE_TYPE_I3C,                ///< i3c device
    ARM_I3C_DEVICE_TYPE_I2C                 ///< legacy i2c device
} ARM_I3C_DEVICE_TYPE;

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION GetVersion (void)
  \brief       Get i3c driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_I3C_CAPABILITIES GetCapabilities (void)
  \brief       Get i3c driver capabilities.
  \return      \ref ARM_I3C_CAPABILITIES

  \fn          ARM_I3C_STATUS GetStatus (void)
  \brief       Get i3c driver status.
  \return      \ref ARM_I3C_STATUS

  \fn          ARM_I3C_DEVICE_INFO GetDeviceInfo (void)
  \brief       Get i3c device information.
  \return      \ref ARM_I3C_DEVICE_INFO

  \fn          int32_t Initialize (ARM_I3C_SignalEvent_t cb_event)
  \brief       Initialize I3C Interface.
  \param[in]   cb_event  Pointer to \ref ARM_I3C_SignalEvent
  \return      \ref execution_status

  \fn          int32_t Uninitialize (void)
  \brief       De-initialize I3C Interface.
  \return      \ref execution_status

  \fn          int32_t PowerControl (ARM_POWER_STATE state)
  \brief       Control I3C Interface Power.
  \param[in]   state     Power state
  \return      \ref execution_status

  \fn          int32_t MasterTransmit (uint8_t addr, const uint8_t *data, uint16_t len)
  \brief       Start transmitting data as I3C Master.
  \param[in]   addr      Assigned Slave Address;
                          Dynamic Address for i3c, Static Address for i2c slave device
  \param[in]   data      Pointer to buffer with data to transmit to I3C Slave
  \param[in]   len       Number of data bytes to transmit
  \return      \ref execution_status

  \fn          int32_t MasterReceive (uint8_t addr, uint8_t *data, uint16_t len)
  \brief       Start receiving data as I3C Master.
  \param[in]   addr      Assigned Slave Address;
                         Dynamic Address for i3c, Static Address for i2c slave device
  \param[out]  data      Pointer to buffer for data to receive from I3C Slave
  \param[in]   len       Number of data bytes to receive
  \return      \ref execution_status

  \fn          int32_t SlaveTransmit(struct i3c_dev *dev, const  uint8_t *data, uint16_t len)
  \brief       Start transmitting data as I3C Slave.
  \param[in]   data      Pointer to buffer with data to transmit to I3C master
  \param[in]   len       Number of data bytes to transmit
  \return      \ref execution_status

  \fn          int32_t SlaveReceive(struct i3c_dev *dev, uint8_t *data, uint32_t len)
  \brief       Start receiving data as I3C Master.
  \param[out]  data      Pointer to buffer for data to receive from I3C master
  \param[in]   len       Number of data bytes to receive
  \return      \ref execution_status

  \fn          int32_t Control (uint32_t control, uint32_t arg)
  \brief       Control I3C mastaer and slave Interface.
  \param[in]   control   Operation
  \param[in]   arg       Argument of operation (optional)
  \return      \ref execution_status

  \fn          int32_t MasterSendCommand (ARM_I3C_CMD *cmd)
  \brief       Send a I3C Command
  \param[in]   cmd       The I3C_CMD to be sent
  \return      \ref execution status

  \fn          int32_t MasterAssignDA (ARM_I3C_CMD *addr_cmd)
  \brief       Assign dynamic address to the i3c slave using SETDASA and ENTDAA
               Note: Only required for i3c slave devices;
                     i2c slave device uses static address for communication \ref I3Cx_AttachSlvDev.
  \param[in]   addr_cmd  Command for addressing
  \return      \ref execution status

  \fn          int32_t AttachSlvDev (const ARM_I3C_DEVICE_TYPE dev_type,
                                     const uint8_t addr)
  \brief       Attach i3c/i2c device to the i3c bus.
  \param[in]   dev_type  Device type - i3c/i2c
  \param[in]   addr      dynamic addr of i3c device/static addr of i2c device
  \return      \ref execution_status

  \fn          int32_t Detachdev (uint8_t addr)
  \brief       Detach already attached i2c/i3c device from the i3c bus.
  \param[in]   addr      static  address of already attached i2c device
                                 OR
                         dynamic address of already attached i3c device
  \return      \ref execution_status

  \fn          int32_t GetSlaveList (uint8_t *addr_list, uint8_t *count)
  \brief       Gets list of slaves already attached
  \param[in]   addr_list Address list of i3c slavesus
  \param[in]   count     Count of i3c slaves
  \return      \ref execution_status

  \fn          int32_t GetSlaveDynAddr  (const uint8_t static_addr, uint8_t *dynamic_addr)
  \brief       Detach already attached i2c/i3c device from the i3c bus.
  \param[in]   static_addr  static  address of already attached i3c device
  \param[in]   dynamic_addr dynamic address of already attached i3c device
  \return      \ref execution_status

  \fn          int32_t GetSlvsInfo  (void* data, const uint8_t value)
  \brief       Gets the received Slaves information
  \param[in]   data  ARM_I3C_DEV_CHAR/ARM_I3C_DEV_PRIME_INFO information store
  \param[in]   value Either Slaves count or Slave address
  \return      \ref execution_status
*/

typedef void (*ARM_I3C_SignalEvent_t) (uint32_t event);  ///< Pointer to \ref I3C_SignalEvent : Signal I3C Event.

/**
\brief I3C Driver Capabilities.
*/
typedef struct _ARM_I3C_CAPABILITIES {
    uint32_t legacy_i2c_dev   : 1;           ///< legacy i2c device support
    uint32_t adaptive_mode    : 1;           ///< Slave I2C/I3C Adaptive mode support during boot up phase
    uint32_t ibi              : 1;           ///< In-Band Interrupts support
    uint32_t ibi_payload      : 1;           ///< In-Band Interrupt with Payload support
    uint32_t sec_master       : 1;           ///< Secondary Master Configuration support
    uint32_t hdr_ddr0         : 1;           ///< HDR DDR0 mode support
    uint32_t hdr_tsp          : 1;           ///< HDR Ternary Symbol Pure-Bus mode support
    uint32_t hdr_tsl          : 1;           ///< HDR Ternary Symbol Legacy-Inclusive-Bus mode support
    uint32_t reserved         : 24;          ///< Reserved (must be zero)
} ARM_I3C_CAPABILITIES;

/**
\brief Access structure of the I3C Driver.
*/
typedef struct _ARM_DRIVER_I3C {
    ARM_DRIVER_VERSION   (*GetVersion)        (void);                                             ///< Pointer to \ref GetVersion        : Get I3C driver version.
    ARM_I3C_CAPABILITIES (*GetCapabilities)   (void);                                             ///< Pointer to \ref GetCapabilities   : Get I3C driver capabilities.
    ARM_I3C_STATUS       (*GetStatus)         (void);                                             ///< Pointer to \ref GetStatus         : Get I3C driver status.
    ARM_I3C_DEVICE_INFO  (*GetDeviceInfo)     (void);                                             ///< Pointer to \ref GetDeviceInfo     : Get I3C device information.
    int32_t              (*Initialize)        (ARM_I3C_SignalEvent_t cb_event);                   ///< Pointer to \ref Initialize        : Initialize    I3C Interface.
    int32_t              (*Uninitialize)      (void);                                             ///< Pointer to \ref Uninitialize      : De-initialize I3C Interface.
    int32_t              (*PowerControl)      (ARM_POWER_STATE state);                            ///< Pointer to \ref PowerControl      : Control I3C Interface Power.
    int32_t              (*MasterTransmit)    (uint8_t addr, const uint8_t *data, uint16_t len);  ///< Pointer to \ref MasterTransmit    : Start transmitting data as I3C Master.
    int32_t              (*MasterReceive)     (uint8_t addr, uint8_t *data, uint16_t len);        ///< Pointer to \ref MasterReceive     : Start receiving    data as I3C Master.
    int32_t              (*SlaveTransmit)     (const uint8_t *data, uint16_t len);                ///< Pointer to \ref MasterTransmit    : Start transmitting data as I3C Slave.
    int32_t              (*SlaveReceive)      (uint8_t *data, uint16_t len);                      ///< Pointer to \ref SlaveReceive      : Start receiving    data as I3C Slave.
    int32_t              (*Control)           (uint32_t control, uint32_t arg);                   ///< Pointer to \ref Control           : Control I3C Master and slave Interface.
    int32_t              (*MasterSendCommand) (ARM_I3C_CMD *cmd);                                 ///< Pointer to \ref MasterSendCommand : Send I3C CCC
    int32_t              (*MasterAssignDA)    (ARM_I3C_CMD *addr_cmd);                            ///< Pointer to \ref MasterAssignDA    : Assign I3C Dynamic Address
    int32_t              (*AttachSlvDev)      (ARM_I3C_DEVICE_TYPE dev_type, const uint8_t addr); ///< Pointer to \ref AttachSlvDev      : Attach i3c/i2c slave device to the i3c bus.
    int32_t              (*Detachdev)         (uint8_t addr);                                     ///< Pointer to \ref Detachdev         : Detach already attached i2c/i3c device from the i3c bus.
    int32_t              (*GetSlaveList)      (uint8_t *addr_list, uint8_t *count);               ///< Pointer to \ref GetSlaveList      : Fetch list of slaves attached to the bus.
    int32_t              (*GetSlaveDynAddr)   (const uint8_t static_addr, uint8_t *dynamic_addr); ///< Pointer to \ref GetSlaveDynAddr   : Fetch dynamic address of slave given with static address.
    int32_t              (*GetSlvsInfo)       (void* data, const uint8_t value);                  ///< Pointer to \ref GetSlvsInfo       : Fetches slave's characteristics.
} const ARM_DRIVER_I3C;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_I3C_H_ */
