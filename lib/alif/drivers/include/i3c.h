/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef I3C_H_
#define I3C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief I3C_Type:- Register map for I3C
  */
typedef struct {                                     /*!< (@ 0x49034000) I3C Structure                                              */
  volatile uint32_t  I3C_DEVICE_CTRL;                /*!< (@ 0x00000000) Device Control Register                                    */
  volatile uint32_t  I3C_DEVICE_ADDR;                /*!< (@ 0x00000004) Device Address Register                                    */
  volatile const  uint32_t  I3C_HW_CAPABILITY;       /*!< (@ 0x00000008) Hardware Capability Register                               */
  volatile  uint32_t  I3C_COMMAND_QUEUE_PORT;        /*!< (@ 0x0000000C) Command Queue Port Register                                */
  volatile const  uint32_t  I3C_RESPONSE_QUEUE_PORT; /*!< (@ 0x00000010) Response Queue Port Register                               */

  union {
    volatile const  uint32_t I3C_RX_DATA_PORT;       /*!< (@ 0x00000014) Receive Data Port Register                                 */
    volatile  uint32_t I3C_TX_DATA_PORT;             /*!< (@ 0x00000014) Transmit Data Port Register                                */
  };

  union {
    volatile const  uint32_t I3C_IBI_QUEUE_DATA;   /*!< (@ 0x00000018) In-Band Interrupt Queue Data Register                        */
    volatile const  uint32_t I3C_IBI_QUEUE_STATUS; /*!< (@ 0x00000018) In-Band Interrupt Queue Status Register                      */
  };
  volatile uint32_t  I3C_QUEUE_THLD_CTRL;          /*!< (@ 0x0000001C) Queue Threshold Control Register                             */
  volatile uint32_t  I3C_DATA_BUFFER_THLD_CTRL;    /*!< (@ 0x00000020) Data Buffer Threshold Control Register                       */
  volatile uint32_t  I3C_IBI_QUEUE_CTRL;           /*!< (@ 0x00000024) IBI Queue Control Register                                   */
  volatile const  uint32_t  RESERVED;
  volatile uint32_t  I3C_IBI_MR_REQ_REJECT;        /*!< (@ 0x0000002C) IBI MR Request Rejection Control Register                    */
  volatile uint32_t  I3C_IBI_SIR_REQ_REJECT;       /*!< (@ 0x00000030) IBI SIR Request Rejection Control Register                   */
  volatile uint32_t  I3C_RESET_CTRL;               /*!< (@ 0x00000034) Reset Control Register                                       */
  volatile uint32_t  I3C_SLV_EVENT_STATUS;         /*!< (@ 0x00000038) Slave Event Status Register                                  */
  volatile uint32_t  I3C_INTR_STATUS;              /*!< (@ 0x0000003C) Interrupt Status Register                                    */
  volatile uint32_t  I3C_INTR_STATUS_EN;           /*!< (@ 0x00000040) Interrupt Status Enable Register                             */
  volatile uint32_t  I3C_INTR_SIGNAL_EN;           /*!< (@ 0x00000044) Interrupt Signal Enable Register                             */
  volatile uint32_t  I3C_INTR_FORCE;               /*!< (@ 0x00000048) Interrupt Force Enable Register                              */
  volatile const  uint32_t  I3C_QUEUE_STATUS_LEVEL;       /*!< (@ 0x0000004C) Queue Status Level Register                           */
  volatile const  uint32_t  I3C_DATA_BUFFER_STATUS_LEVEL; /*!< (@ 0x00000050) Data Buffer Status Level Register                     */
  volatile const  uint32_t  I3C_PRESENT_STATE;            /*!< (@ 0x00000054) Present State Register                                */
  volatile const  uint32_t  I3C_CCC_DEVICE_STATUS;        /*!< (@ 0x00000058) Device Operating Status Register                      */
  volatile const  uint32_t  I3C_DEVICE_ADDR_TABLE_POINTER;/*!< (@ 0x0000005C) Pointer for Device Address Table Registers            */
  volatile uint32_t  I3C_DEV_CHAR_TABLE_POINTER;          /*!< (@ 0x00000060) Pointer for Device Characteristics Table Register     */
  volatile const  uint32_t  RESERVED1[2];
  volatile const  uint32_t  I3C_VENDOR_SPECIFIC_REG_POINTER;/*!< (@ 0x0000006C) Pointer for Vendor Specific Registers               */
  volatile uint32_t  I3C_SLV_MIPI_ID_VALUE;        /*!< (@ 0x00000070) Provisional ID Register                                      */
  volatile uint32_t  I3C_SLV_PID_VALUE;            /*!< (@ 0x00000074) Provisional ID Register                                      */
  volatile uint32_t  I3C_SLV_CHAR_CTRL;            /*!< (@ 0x00000078) I3C Slave Characteristic Register                            */
  volatile const  uint32_t  I3C_SLV_MAX_LEN;         /*!< (@ 0x0000007C) I3C Max Write/Read Length Register                         */
  volatile const  uint32_t  I3C_MAX_READ_TURNAROUND; /*!< (@ 0x00000080) MXDS Maximum Read Turnaround Time Register                 */
  volatile uint32_t  I3C_MAX_DATA_SPEED;           /*!< (@ 0x00000084) MXDS Maximum Data Speed Register                             */
  volatile const  uint32_t  RESERVED2;
  volatile uint32_t  I3C_SLV_INTR_REQ;             /*!< (@ 0x0000008C) Slave Interrupt Request Register                             */
  volatile const  uint32_t  RESERVED3[8];
  volatile uint32_t  I3C_DEVICE_CTRL_EXTENDED;     /*!< (@ 0x000000B0) Device Control Extended Register                             */
  volatile uint32_t  I3C_SCL_I3C_OD_TIMING;        /*!< (@ 0x000000B4) SCL I3C Open Drain Timing Register                           */
  volatile uint32_t  I3C_SCL_I3C_PP_TIMING;        /*!< (@ 0x000000B8) SCL I3C Push Pull Timing Register                            */
  volatile uint32_t  I3C_SCL_I2C_FM_TIMING;        /*!< (@ 0x000000BC) SCL I2C Fast Mode Timing Register                            */
  volatile uint32_t  I3C_SCL_I2C_FMP_TIMING;       /*!< (@ 0x000000C0) SCL I2C Fast Mode Plus Timing Register                       */
  volatile const  uint32_t  RESERVED4;
  volatile uint32_t  I3C_SCL_EXT_LCNT_TIMING;      /*!< (@ 0x000000C8) SCL Extended Low Count Timing Register                       */
  volatile uint32_t  I3C_SCL_EXT_TERMN_LCNT_TIMING;/*!< (@ 0x000000CC) SCL Termination Bit Low Count Timing Register                */
  volatile uint32_t  I3C_SDA_HOLD_SWITCH_DLY_TIMING;/*!< (@ 0x000000D0) SDA Hold and Mode Switch Delay Timing Register              */
  volatile uint32_t  I3C_BUS_FREE_AVAIL_TIMING;    /*!< (@ 0x000000D4) Bus Free Timing Register                                     */
  volatile uint32_t  I3C_BUS_IDLE_TIMING;          /*!< (@ 0x000000D8) Bus Idle Timing Register                                     */
  volatile uint32_t  I3C_SCL_LOW_MST_EXT_TIMEOUT;  /*!< (@ 0x000000DC) SCL Low Master Extended Timeout Register                     */
  volatile const  uint32_t  I3C_VER_ID;                /*!< (@ 0x000000E0) Reserved                                                 */
  volatile const  uint32_t  I3C_VER_TYPE;              /*!< (@ 0x000000E4) Reserved                                                 */
  volatile const  uint32_t  I3C_QUEUE_SIZE_CAPABILITY; /*!< (@ 0x000000E8) I3C Queue Size Capability Register                       */
  volatile const  uint32_t  RESERVED5[69];

  union {
    volatile const  uint32_t I3C_DEV_CHAR_TABLE1_LOC1; /*!< (@ 0x00000200) Device Characteristic Table Location 1 Register          */
    volatile const  uint32_t I3C_SEC_DEV_CHAR_TABLE1;  /*!< (@ 0x00000200) Secondary Master Device Characteristic Table
                                                                           Location Register                                        */
  };
  volatile const  uint32_t  I3C_DEV_CHAR_TABLE1_LOC2;  /*!< (@ 0x00000204) Device Characteristic Table Location 2 Register          */
  volatile const  uint32_t  I3C_DEV_CHAR_TABLE1_LOC3;  /*!< (@ 0x00000208) Device Characteristic Table Location 3 Register          */
  volatile const  uint32_t  I3C_DEV_CHAR_TABLE1_LOC4;  /*!< (@ 0x0000020C) Device Characteristic Table Location 4 Register          */
  volatile const  uint32_t  RESERVED6[4];
  volatile uint32_t  I3C_DEV_ADDR_TABLE_LOC1;          /*!< (@ 0x00000220) Device Address Table Location 1 Register                 */
} I3C_Type;

/* common helpers */
#define BIT(nr)                           (1UL << (nr))

#define GENMASK(h, l) \
                                          (((~(0U)) - ((1U) << (l)) + 1) & \
                                          (~(0U) >> (32 - 1 - (h))))

#define DIV_ROUND_UP(n, d)                (((n) + (d) - 1) / (d))

/* Maximum 8 Slave Devices are supported
 * (\ref register DEVICE_ADDR_TABLE_POINTER) */
#define I3C_MAX_DEVS                      8U

/* Clock rates and periods */
#define I3C_BUS_I2C_FM_PLUS_SCL_RATE      1000000
#define I3C_BUS_I2C_FM_SCL_RATE           400000
#define I3C_BUS_I2C_SS_SCL_RATE           100000
#define I3C_BUS_I2C_SS_TLOW_MIN_NS        4700
#define I3C_BUS_I2C_FM_TLOW_MIN_NS        1300
#define I3C_BUS_I2C_FMP_TLOW_MIN_NS       500

/* Below macros are timing requirements to
 * to connect with slower I3C slaves.
 * PP and OD timing is 2MHz
 */
#define I3C_SLOW_BUS_TLOW_OD_NS           300
#define I3C_SLOW_BUS_THIGH_NS             200

/* Below macros are timing requirements
 * for normal communication.
 * OD timing is 4MHz
 */
#define I3C_NORMAL_BUS_TLOW_OD_NS         200
#define I3C_NORMAL_BUS_THIGH_NS           41

#define I3C_BUS_MAX_I3C_SCL_RATE          12900000
#define I3C_BUS_SDR0_SCL_RATE             12500000
#define I3C_BUS_SDR1_SCL_RATE             8000000
#define I3C_BUS_SDR2_SCL_RATE             6000000
#define I3C_BUS_SDR3_SCL_RATE             4000000
#define I3C_BUS_SDR4_SCL_RATE             2000000
#define I3C_SCL_I3C_TIMING_CNT_MIN        5

/* ref clock frequency 1 GHz to get core-period */
#define REF_CLK_RATE                      1000000000

#define I3C_MAX_DATA_BUF_LOC              64U                         /* 64 Locations */
#define I3C_MAX_DATA_BUF_SIZE             (I3C_MAX_DATA_BUF_LOC * 4U) /* 64 words */

/* Macro for Master to retry max when the slave nacks */
#define I3C_SLAVE_NACK_RETRY_COUNT_MAX    0x3U

#define I3C_SDA_TX_HOLD_TIME_MAX          0x7U

#define I3C_SPEED_HDR_DDR                 0x6U
#define I3C_SPEED_SDR0                    0x0U
#define I3C_NEXT_SLAVE_ADDR_OFFSET        0x9U

#define I3C_HOT_JOIN_ID                   0x4U /* MIPI std val: 2. Here it's stored in bit 2 of reg. So val:4 */

#define I3C_DEVICE_ROLE_SECONDARY_MASTER  0x3U
#define I3C_MASTER_DYNAMIC_ADDR           0x8U


/* transaction ids for tx and rx, CCC set and get,
 *  and Dynamic Address Assignment. */
#define I3C_CCC_SET_TID                   0x1
#define I3C_CCC_GET_TID                   0x2
#define I3C_MST_TX_TID                    0x3
#define I3C_MST_RX_TID                    0x4
#define I3C_ADDR_ASSIGN_TID               0x5
#define I3C_SLV_TX_TID                    0X6
#define I3C_SLV_RX_TID                    0X8
#define I3C_SLV_DEFSLVS_TID               0xF

/* Macros for address positions in Device Address Table */
#define I3C_DAT_STATIC_ADDR_Msk           0xFFU
#define I3C_DAT_DYNAMIC_ADDR_Msk          0x00FF0000U
#define I3C_DAT_DYNAMIC_ADDR_Pos          0x10U
#define I3C_DAT_DEV_NACK_RETRY_CNT_Pos    0x1DU /* Position: 29th bit*/
#define I3C_DAT_DEV_NACK_RETRY_CNT_Msk    (0x3U << I3C_DAT_DEV_NACK_RETRY_CNT_Pos)

#define I3C_DCT_DYNAMIC_ADDR_OFFSET       0xCU
#define I3C_DCT_DYNAMIC_ADDR_Msk          0x7FU
#define I3C_DCT_CHAR_OFFSET               0x8U
#define I3C_DCT_DCR_Msk                   GENMASK(7, 0)
#define I3C_DCT_BCR_Pos                   8U
#define I3C_DCT_BCR_Msk                   GENMASK(15, I3C_DCT_BCR_Pos)
#define I3C_DCT_PID_DCR_Msk               GENMASK(11, 0)
#define I3C_DCT_INST_ID_Pos               12U
#define I3C_DCT_INST_ID_Msk               GENMASK(15, 12)

#define I3C_DCT_PART_ID_Msk               GENMASK(15, 0)
#define I3C_DCT_ID_SEL_Pos                16U
#define I3C_DCT_ID_SEL_Msk                0x10000U
#define I3C_DCT_MIPI_MFG_ID_Pos           17U
#define I3C_DCT_MIPI_MFG_ID_Msk           GENMASK(31, I3C_DCT_MIPI_MFG_ID_Pos)

#define I3C_DEVICE_CTRL_ENABLE                      BIT(31)
#define I3C_DEVICE_CTRL_RESUME                      BIT(30)
#define I3C_DEVICE_CTRL_ABORT                       BIT(29)
#define I3C_DEVICE_CTRL_DMA_ENABLE                  BIT(28)
#define I3C_DEVICE_CTRL_ADAPTIVE_I2C_I3C            BIT(27)
#define I3C_DEVICE_CTRL_HOT_JOIN_CTRL               BIT(8)   /* 1: Nack, 0: Ack */
#define I3C_DEVICE_CTRL_I2C_SLAVE_PRESENT           BIT(7)

#define I3C_DEVICE_ADDR_DYNAMIC_ADDR_VALID          BIT(31)
#define I3C_DEVICE_ADDR_DYNAMIC_ADDR_Pos            16U
#define I3C_DEVICE_ADDR_DYNAMIC_ADDR_Msk            GENMASK(22, I3C_DEVICE_ADDR_DYNAMIC_ADDR_Pos)
#define I3C_DEVICE_ADDR_DYNAMIC_ADDR(x)             (((x) << I3C_DEVICE_ADDR_DYNAMIC_ADDR_Pos) & \
                                                            I3C_DEVICE_ADDR_DYNAMIC_ADDR_Msk)
#define I3C_DEVICE_ADDR_STATIC_ADDR_VALID           BIT(15)
#define I3C_DEVICE_ADDR_STATIC_ADDR_Pos             0U
#define I3C_DEVICE_ADDR_STATIC_ADDR_Msk             GENMASK(6, I3C_DEVICE_ADDR_STATIC_ADDR_Pos)


#define I3C_HW_CAPABILITY_DEVICE_ROLE_CONFIG_Msk    GENMASK(2, 0)

#define I3C_COMMAND_QUEUE_PORT_PEC                  BIT(31)
#define I3C_COMMAND_QUEUE_PORT_TOC                  BIT(30)
#define I3C_COMMAND_QUEUE_PORT_READ_TRANSFER        BIT(28)
#define I3C_COMMAND_QUEUE_PORT_SDAP                 BIT(27)
#define I3C_COMMAND_QUEUE_PORT_ROC                  BIT(26)
#define I3C_COMMAND_QUEUE_PORT_DBP                  BIT(25)
#define I3C_COMMAND_QUEUE_PORT_SPEED(x)             (((x) << 21) & GENMASK(23, 21))
#define I3C_COMMAND_QUEUE_PORT_DEV_INDEX(x)         (((x) << 16) & GENMASK(20, 16))
#define I3C_COMMAND_QUEUE_PORT_CP                   BIT(15)
#define I3C_COMMAND_QUEUE_PORT_CMD(x)               (((x) << 7) & GENMASK(14, 7))
#define I3C_COMMAND_QUEUE_PORT_TID(x)               (((x) << 3) & GENMASK(6, 3))
#define I3C_COMMAND_QUEUE_PORT_SLV_PORT_TID(x)           (((x) << 3) & GENMASK(5, 3))

#define I3C_COMMAND_QUEUE_PORT_ARG_DATA_LEN(x)      (((x) << 16) & GENMASK(31, 16))
#define I3C_COMMAND_QUEUE_PORT_ARG_DATA_LEN_MAX     65536
#define I3C_COMMAND_QUEUE_PORT_TRANSFER_ARG         0x01
#define I3C_COMMAND_QUEUE_PORT_ARG_DATA_DB(x)       (((x) << 8) & GENMASK(15, 8))

#define I3C_COMMAND_QUEUE_PORT_DEV_COUNT(x)         (((x) << 21) & GENMASK(25, 21))
#define I3C_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD       0x03

#define I3C_RESPONSE_QUEUE_PORT_ERR_STATUS(x)           (((x) & GENMASK(31, 28)) >> 28)
#define I3C_RESPONSE_QUEUE_PORT_TID(x)                  (((x) & GENMASK(27, 24)) >> 24)
#define I3C_RESPONSE_QUEUE_PORT_ERR_NONE                0
#define I3C_RESPONSE_QUEUE_PORT_ERR_CRC                 1
#define I3C_RESPONSE_QUEUE_PORT_ERR_PARITY              2
#define I3C_RESPONSE_QUEUE_PORT_ERR_FRAME               3
#define I3C_RESPONSE_QUEUE_PORT_ERR_IBA_NACK            4
#define I3C_RESPONSE_QUEUE_PORT_ERR_ADDRESS_NACK        5
#define I3C_RESPONSE_QUEUE_PORT_ERR_OVER_UNDER_FLOW     6
#define I3C_RESPONSE_QUEUE_PORT_ERR_XFER_ABORT          8
#define I3C_RESPONSE_QUEUE_PORT_ERR_I2C_W_NACK          9
#define I3C_RESPONSE_QUEUE_PORT_ERR_PEC_OR_EARLY_TERM   0xA

#define I3C_RESPONSE_QUEUE_PORT_TID(x)                  (((x) & GENMASK(27, 24)) >> 24)
#define I3C_RESPONSE_QUEUE_PORT_DATA_LEN(x)             ((x) & GENMASK(15, 0))

#define I3C_IBI_QUEUE_STATUS_IBI_STS                    (1U << 28U)
#define I3C_IBI_QUEUE_STATUS_IBI_ID(x)                  ((x & GENMASK(0xFU, 8U)) >> 8U)
#define I3C_IBI_QUEUE_STATUS_IBI_ID_RW_Pos              (1U)
#define I3C_IBI_QUEUE_STATUS_IBI_ID_RW(x)               (x & 1U)

#define I3C_QUEUE_THLD_CTRL_IBI_STATUS_THLD_Msk         GENMASK(31, 24)
#define I3C_QUEUE_THLD_CTRL_RESP_BUF_THLD_Msk           GENMASK(15, 8)
#define I3C_QUEUE_THLD_CTRL_RESP_BUF_THLD(x)            (((x) - 1) << 8)

#define I3C_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_Msk       GENMASK(10, 8)
#define I3C_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_Msk GENMASK(2, 0)

#define I3C_IBI_QUEUE_CTRL_NOTIFY_SIR_REJECTED          (1U << 3U)
#define I3C_IBI_QUEUE_CTRL_NOTIFY_MR_REJECTED           (1U << 1U)
#define I3C_IBI_QUEUE_CTRL_NOTIFY_HJ_REJECTED           (1U << 0U)

#define I3C_IBI_MR_REQ_REJECT_MR_REQ_REJECT             GENMASK(31, 0)
#define I3C_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT           GENMASK(31, 0)

#define I3C_RESET_CTRL_IBI_QUEUE_RST                    BIT(5)
#define I3C_RESET_CTRL_RX_FIFO_RST                      BIT(4)
#define I3C_RESET_CTRL_TX_FIFO_RST                      BIT(3)
#define I3C_RESET_CTRL_RESP_QUEUE_RST                   BIT(2)
#define I3C_RESET_CTRL_CMD_QUEUE_RST                    BIT(1)
#define I3C_RESET_CTRL_SOFT_RST                         BIT(0)

#define I3C_SLV_EVENT_STATUS_MWL_UPDATED                BIT(7)
#define I3C_SLV_EVENT_STATUS_MRL_UPDATED                BIT(6)
#define I3C_SLV_EVENT_STATUS_HJ_EN                      BIT(3)
#define I3C_SLV_EVENT_STATUS_MR_EN                      BIT(1)
#define I3C_SLV_EVENT_STATUS_SIR_EN                     BIT(0)

#define I3C_INTR_STATUS_BUS_RESET_DONE_STS              BIT(15)
#define I3C_INTR_STATUS_BUSOWNER_UPDATED_STS            BIT(13)
#define I3C_INTR_STATUS_IBI_UPDATED_STS                 BIT(12)
#define I3C_INTR_STATUS_READ_REQ_RECV_STS               BIT(11)
#define I3C_INTR_STATUS_DEFSLV_STS                      BIT(10)
#define I3C_INTR_STATUS_TRANSFER_ERR_STS                BIT(9)
#define I3C_INTR_STATUS_DYN_ADDR_ASSGN_STS              BIT(8)
#define I3C_INTR_STATUS_CCC_UPDATED_STS                 BIT(6)
#define I3C_INTR_STATUS_TRANSFER_ABORT_STS              BIT(5)
#define I3C_INTR_STATUS_RESP_READY_STS                  BIT(4)
#define I3C_INTR_STATUS_CMD_QUEUE_READY_STS             BIT(3)
#define I3C_INTR_STATUS_IBI_THLD_STS                    BIT(2)
#define I3C_INTR_STATUS_RX_THLD_STS                     BIT(1)
#define I3C_INTR_STATUS_TX_THLD_STS                     BIT(0)
#define I3C_INTR_STATUS_ALL                             (I3C_INTR_STATUS_BUSOWNER_UPDATED_STS |   \
                                                        I3C_INTR_STATUS_IBI_UPDATED_STS       |   \
                                                        I3C_INTR_STATUS_READ_REQ_RECV_STS     |   \
                                                        I3C_INTR_STATUS_DEFSLV_STS            |   \
                                                        I3C_INTR_STATUS_TRANSFER_ERR_STS      |   \
                                                        I3C_INTR_STATUS_DYN_ADDR_ASSGN_STS    |   \
                                                        I3C_INTR_STATUS_CCC_UPDATED_STS       |   \
                                                        I3C_INTR_STATUS_TRANSFER_ABORT_STS    |   \
                                                        I3C_INTR_STATUS_RESP_READY_STS        |   \
                                                        I3C_INTR_STATUS_CMD_QUEUE_READY_STS   |   \
                                                        I3C_INTR_STATUS_IBI_THLD_STS          |   \
                                                        I3C_INTR_STATUS_RX_THLD_STS           |   \
                                                        I3C_INTR_STATUS_TX_THLD_STS)

#define I3C_INTR_STATUS_EN_BUS_RESET_DONE_STS_EN        BIT(15)
#define I3C_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN      BIT(13)
#define I3C_INTR_STATUS_EN_IBI_UPDATED_STS_EN           BIT(12)
#define I3C_INTR_STATUS_EN_READ_REQ_RECV_STS_EN         BIT(11)
#define I3C_INTR_STATUS_EN_DEFSLV_STS_EN                BIT(10)
#define I3C_INTR_STATUS_EN_TRANSFER_ERR_STS_EN          BIT(9)
#define I3C_INTR_STATUS_EN_DYN_ADDR_ASSGN_STS_EN        BIT(8)
#define I3C_INTR_STATUS_EN_CCC_UPDATED_STS_EN           BIT(6)
#define I3C_INTR_STATUS_EN_TRANSFER_ABORT_STS_EN        BIT(5)
#define I3C_INTR_STATUS_EN_RESP_READY_STS_EN            BIT(4)
#define I3C_INTR_STATUS_EN_CMD_QUEUE_READY_STS_EN       BIT(3)
#define I3C_INTR_STATUS_EN_IBI_THLD_STS_EN              BIT(2)
#define I3C_INTR_STATUS_EN_RX_THLD_STS_EN               BIT(1)
#define I3C_INTR_STATUS_EN_TX_THLD_STS_EN               BIT(0)

#define I3C_MASTER_INTR_EN_MASK                         (I3C_INTR_STATUS_EN_TRANSFER_ERR_STS_EN     |   \
                                                        I3C_INTR_STATUS_EN_IBI_THLD_STS_EN          |   \
                                                        I3C_INTR_STATUS_EN_TRANSFER_ABORT_STS_EN    |   \
                                                        I3C_INTR_STATUS_EN_RESP_READY_STS_EN)

#define I3C_SLAVE_INTR_EN_MASK                          (I3C_INTR_STATUS_EN_TRANSFER_ERR_STS_EN     |   \
                                                        I3C_INTR_STATUS_EN_DYN_ADDR_ASSGN_STS_EN    |   \
                                                        I3C_INTR_STATUS_EN_CCC_UPDATED_STS_EN       |   \
                                                        I3C_INTR_STATUS_EN_RESP_READY_STS_EN        |   \
                                                        I3C_INTR_STATUS_EN_DEFSLV_STS_EN)

#define I3C_QUEUE_STATUS_LEVEL_IBI_BUF_BLR(x)           (((x) & GENMASK(23, 16)) >> 16)
#define I3C_QUEUE_STATUS_LEVEL_RESP_BUF_BLR(x)          (((x) & GENMASK(15, 8)) >> 8)
#define I3C_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_Msk         GENMASK(15, 8)

#define I3C_DATA_BUFFER_STATUS_LEVEL_RX_BUF_BLR_Msk                   (GENMASK(23, 16))
#define I3C_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC_Msk             (GENMASK(7, 0))

#define I3C_PRESENT_STATE_MASTER_IDLE                                 (1U << 28U)
#define I3C_PRESENT_STATE_CURRENT_MASTER                              (1U << 2U)

#define I3C_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_Pos        16U
#define I3C_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_Msk GENMASK(15, 0)

#define I3C_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_Pos    19U
#define I3C_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_Msk    GENMASK(22, 19)
#define I3C_DEV_CHAR_TABLE_POINTER_P_DEV_CHAR_TABLE_START_ADDR_Msk    GENMASK(11, 0)

#define I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_Pos   1U
#define I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_Msk   GENMASK(15, \
                                                    I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_Pos)
#define I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID(x)    ((x & I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_Msk) >> \
                                                          I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_Pos)
#define I3C_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL       BIT(0)

#define I3C_SLV_PID_VALUE_SLV_PART_ID_Pos           16U
#define I3C_SLV_PID_VALUE_SLV_PART_ID_Msk           GENMASK(31, I3C_SLV_PID_VALUE_SLV_PART_ID_Pos)
#define I3C_SLV_PID_VALUE_SLV_PART_ID(x)            ((x & I3C_SLV_PID_VALUE_SLV_PART_ID_Msk) >>  \
                                                          I3C_SLV_PID_VALUE_SLV_PART_ID_Pos)
#define I3C_SLV_PID_VALUE_SLV_INST_ID_Pos           12U
#define I3C_SLV_PID_VALUE_SLV_INST_ID_Msk           GENMASK(15, I3C_SLV_PID_VALUE_SLV_INST_ID_Pos)
#define I3C_SLV_PID_VALUE_SLV_INST_ID(x)            ((x & I3C_SLV_PID_VALUE_SLV_INST_ID_Msk) >>  \
                                                          I3C_SLV_PID_VALUE_SLV_INST_ID_Pos)
#define I3C_SLV_PID_VALUE_SLV_PID_DCR_Msk           GENMASK(11, 0)
#define I3C_SLV_PID_VALUE_SLV_PID_DCR(x)            (x & I3C_SLV_PID_VALUE_SLV_PID_DCR_Msk)

#define I3C_SLV_CHAR_CTRL_DCR_Pos                   8U
#define I3C_SLV_CHAR_CTRL_DCR_Msk                   GENMASK(15, I3C_SLV_CHAR_CTRL_DCR_Pos)
#define I3C_SLV_CHAR_CTRL_DCR(x)                    ((x & I3C_SLV_CHAR_CTRL_DCR_Msk) >>          \
                                                          I3C_SLV_CHAR_CTRL_DCR_Pos)

#define I3C_SLV_CHAR_CTRL_DEVICE_ROLE_MASTER        (1U << 6U)

#define I3C_SLV_CHAR_CTRL_BCR_Msk                   GENMASK(7, 0)
#define I3C_SLV_CHAR_CTRL_BCR(x)                    (x & I3C_SLV_CHAR_CTRL_BCR_Msk)

#define I3C_SLV_MAX_LEN_MRL_Pos                     16U
#define I3C_SLV_MAX_LEN_MRL_Msk                     GENMASK(31, I3C_SLV_MAX_LEN_MRL_Pos)
#define I3C_SLV_MAX_LEN_MWL_Msk                     GENMASK(15, 0)

#define I3C_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN_Msk   GENMASK(23, 0)
#define I3C_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN(x)    (x & I3C_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN_Msk)

#define I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_Pos       8U
#define I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_Msk       GENMASK(10, I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_Pos)
#define I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED(x)        ((x & I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_Msk) >> \
                                                             I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED_Pos)

#define I3C_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED_Msk       GENMASK(2, 0)
#define I3C_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED(x)        (x & I3C_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED_Msk)

#define I3C_SLV_INTR_REQ_IBI_STS_Pos                    8U
#define I3C_SLV_INTR_REQ_IBI_STS_Msk                    GENMASK(9, 8)
#define I3C_SLV_INTR_REQ_IBI_STS_MST_ACKED              1U
#define I3C_SLV_INTR_REQ_IBI_STS_NATMPTED               3U
#define I3C_SLV_INTR_REQ_MR                             (1U << 3U)
#define I3C_SLV_INTR_REQ_SIR                            (1U << 0U)
#define I3C_SLV_INTR_REQ_SIR_CTRL                       ~(3U << 1U)

#define I3C_DEVICE_CTRL_EXTENDED_REQMST_ACK_CTRL        (1U << 3U)
#define I3C_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_Msk GENMASK(1, 0)
#define I3C_DEVICE_CTRL_EXTENDED_DEV_OP_MODE_SLV        (1)

#define I3C_SCL_I3C_OD_TIMING_I3C_OD_HCNT(x)            (((x) << 16) & GENMASK(23, 16))
#define I3C_SCL_I3C_OD_TIMING_I3C_OD_LCNT(x)            ((x) & GENMASK(7, 0))

#define I3C_SCL_I3C_PP_TIMING_I3C_PP_HCNT(x)            (((x) << 16) & GENMASK(23, 16))
#define I3C_SCL_I3C_PP_TIMING_I3C_PP_LCNT(x)            ((x) & GENMASK(7, 0))

#define I3C_SCL_I2C_FM_TIMING_I2C_FM_HCNT(x)            (((x) << 16) & GENMASK(31, 16))
#define I3C_SCL_I2C_FM_TIMING_I2C_FM_LCNT(x)            ((x) & GENMASK(15, 0))

#define I3C_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT(x)          (((x) << 16) & GENMASK(23, 16))
#define I3C_SCL_I2C_FMP_TIMING_I2C_FMP_LCNT(x)          ((x) & GENMASK(15, 0))

#define I3C_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4(x)       (((x) << 24) & GENMASK(31, 24))
#define I3C_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3(x)       (((x) << 16) & GENMASK(23, 16))
#define I3C_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2(x)       (((x) << 8) & GENMASK(15, 8))
#define I3C_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1(x)       ((x) & GENMASK(7, 0))

#define I3C_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_Pos  0x10U
#define I3C_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_Msk         \
        (0x7U << I3C_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_Pos)

/* Bus available time of 1us in ns */
#define I3C_BUS_AVAILABLE_TIME_NS         1000U
#define I3C_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME(x) (((x) << 16) & GENMASK(31, 16))
#define I3C_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME(x)      ((x) & GENMASK(15, 0))

/* Bus Idle time of 1ms in ns */
#define I3C_BUS_IDLE_TIME_NS                            1000000U
#define I3C_BUS_IDLE_TIMING_BUS_IDLE_TIME(x)            ((x) & GENMASK(19, 0))

#define I3C_SEC_DEV_CHAR_TABLE1_STATIC_ADDR_Pos         1U
#define I3C_SEC_DEV_CHAR_TABLE1_STATIC_ADDR_Msk                 \
        GENMASK(7, I3C_SEC_DEV_CHAR_TABLE1_STATIC_ADDR_Pos)
#define I3C_SEC_DEV_CHAR_TABLE1_STATIC_ADDR(x)          (((x) & \
        I3C_SEC_DEV_CHAR_TABLE1_STATIC_ADDR_Msk)  >>            \
        I3C_SEC_DEV_CHAR_TABLE1_STATIC_ADDR_Pos)

#define I3C_SEC_DEV_CHAR_TABLE1_BCR_TYPE_Pos            8U
#define I3C_SEC_DEV_CHAR_TABLE1_BCR_TYPE_Msk                    \
        GENMASK(15, I3C_SEC_DEV_CHAR_TABLE1_BCR_TYPE_Pos)
#define I3C_SEC_DEV_CHAR_TABLE1_BCR_TYPE(x)             (((x) & \
        I3C_SEC_DEV_CHAR_TABLE1_BCR_TYPE_Msk)     >>            \
        I3C_SEC_DEV_CHAR_TABLE1_BCR_TYPE_Pos)

#define I3C_SEC_DEV_CHAR_TABLE1_DCR_TYPE_Pos            16U
#define I3C_SEC_DEV_CHAR_TABLE1_DCR_TYPE_Msk                    \
        GENMASK(23, (I3C_SEC_DEV_CHAR_TABLE1_DCR_TYPE_Pos - 1U))
#define I3C_SEC_DEV_CHAR_TABLE1_DCR_TYPE(x)             (((x) & \
        I3C_SEC_DEV_CHAR_TABLE1_DCR_TYPE_Msk)     >>            \
        I3C_SEC_DEV_CHAR_TABLE1_DCR_TYPE_Pos)

#define I3C_SEC_DEV_CHAR_TABLE1_DYNAMIC_ADDR_Pos        25U
#define I3C_SEC_DEV_CHAR_TABLE1_DYNAMIC_ADDR_Msk                \
        GENMASK(31, I3C_SEC_DEV_CHAR_TABLE1_DYNAMIC_ADDR_Pos)
#define I3C_SEC_DEV_CHAR_TABLE1_DYNAMIC_ADDR(x)         (((x) & \
        I3C_SEC_DEV_CHAR_TABLE1_DYNAMIC_ADDR_Msk) >>            \
        I3C_SEC_DEV_CHAR_TABLE1_DYNAMIC_ADDR_Pos)

#define I3C_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEV          BIT(31)
#define I3C_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR(x)     (((x) << 16) & GENMASK(23, 16))
#define I3C_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR(x)      ((x) & GENMASK(6, 0))

/**
 \brief I3C Control Codes: I2C Bus Speed mode
 */
typedef enum
{
    I3C_I2C_SPEED_MODE_FMP_1_MBPS  = 0,   /* Speed: Fast Mode Plus    1 MBPS */
    I3C_I2C_SPEED_MODE_FM_400_KBPS = 1,   /* Speed: Fast Mode       400 KBPS */
    I3C_I2C_SPEED_MODE_SS_100_KBPS = 2,   /* Speed: Standard Mode   100 KBPS */
    I3C_I2C_SPEED_MODE_LIMITED     = 3    /* Speed: Limited                  */
} I3C_I2C_SPEED_MODE;

/**
\brief Status of an ongoing I3C transfer.
*/
typedef enum _I3C_XFER_STATUS
{
    I3C_XFER_STATUS_NONE                = 0,           /**< Transfer status none                           */
    I3C_XFER_STATUS_DONE                = (1UL << 0),  /**< Transfer status done                           */
    I3C_XFER_STATUS_ERROR               = (1UL << 1),  /**< Transfer status error                          */
    I3C_XFER_STATUS_MST_TX_DONE         = (1UL << 2),  /**< Transfer status master transmit done           */
    I3C_XFER_STATUS_MST_RX_DONE         = (1UL << 3),  /**< Transfer status master receive done            */
    I3C_XFER_STATUS_SLV_TX_DONE         = (1UL << 4),  /**< Transfer status slave transmit done            */
    I3C_XFER_STATUS_SLV_RX_DONE         = (1UL << 5),  /**< Transfer status slave receive done             */
    I3C_XFER_STATUS_SLV_DYN_ADDR_ASSGN  = (1UL << 6),  /**< Transfer status slave dynamic address assigned */
    I3C_XFER_STATUS_CCC_SET_DONE        = (1UL << 7),  /**< Transfer status CCC set done                   */
    I3C_XFER_STATUS_CCC_GET_DONE        = (1UL << 8),  /**< Transfer status CCC get done                   */
    I3C_XFER_STATUS_ADDR_ASSIGN_DONE    = (1UL << 9),  /**< Transfer status Address Assign done            */
    I3C_XFER_STATUS_ERROR_TX            = (1UL << 10), /**< Transfer status error  Master/Slave TX and CCC SET */
    I3C_XFER_STATUS_ERROR_RX            = (1UL << 11), /**< Transfer status error  Master/Slave RX and CCC GET */
    I3C_XFER_STATUS_ERROR_ADDR_ASSIGN   = (1UL << 12), /**< Transfer status error  Address Assign          */
    I3C_XFER_STATUS_ERROR_XFER_ABORT    = (1UL << 13), /**< Transfer status Message Transfer Abort         */
    I3C_XFER_STATUS_SLV_HOT_JOIN_REQ    = (1UL << 14), /**< Transfer status Slave Hot join request received*/
    I3C_XFER_STATUS_IBI_MASTERSHIP_REQ  = (1UL << 15), /**< Transfer status Mastership request received    */
    I3C_XFER_STATUS_BUSOWNER_UPDATED    = (1UL << 16), /**< Transfer status Busowner updated               */
    I3C_XFER_STATUS_SLV_CCC_UPDATED     = (1UL << 17), /**< Transfer status slave CCC updated              */
    I3C_XFER_STATUS_IBI_SLV_INTR_REQ    = (1UL << 18), /**< Transfer status IBI Slave Intr request received*/
    I3C_XFER_STATUS_DEFSLV_LIST         = (1UL << 19)  /**< Transfer status Defslvs received               */
} I3C_XFER_STATUS;

/* brief I3C Transfer types */
typedef enum _I3C_XFER_TYPE
{
    I3C_XFER_TYPE_ADDR_ASSIGN         = 0x1U,
    I3C_XFER_CCC_SET                  = 0x2U,
    I3C_XFER_CCC_GET                  = 0x3U,
    I3C_XFER_TYPE_DATA                = 0x4U
}I3C_XFER_TYPE;

/* i3c message communication errors */
typedef enum _I3C_COMM_ERROR
{
    I3C_COMM_ERROR_NONE               = 0x0U,
    I3C_COMM_ERROR_CRC                = 0x1U,
    I3C_COMM_ERROR_PARITY             = 0x2U,
    I3C_COMM_ERROR_FRAME              = 0x3U,
    I3C_COMM_ERROR_IBA_NACK           = 0x4U,
    I3C_COMM_ERROR_ADDR_NACK          = 0x5U,
    I3C_COMM_ERROR_BUF_UNDR_OVR_FLW   = 0x6U,
    I3C_COMM_ERROR_XFER_ABORT         = 0x7U,
    I3C_COMM_ERROR_I2C_SLV_W_NACK    = 0x8U,
    I3C_COMM_ERROR_PEC_OR_EARLY_TERM  = 0x9U
}_I3C_COMM_ERROR;

/* brief I3C Transfer cmd with data type */
typedef struct _i3c_xfer_cmd_t
{
    I3C_XFER_TYPE   cmd_type;           /* xfer command type                          */
    uint8_t         cmd_id;             /* xfer cpmmand id                            */
    uint8_t         port_id;            /* Port ID                                    */
    uint8_t         addr_index;         /* DAT address index                          */
    uint8_t         addr_depth;         /* DAT address depth                          */
    uint8_t         def_byte;           /* Defining byte                              */
    uint32_t        cmd_hi;             /* value to be programmed first  into C QUEUE */
    uint32_t        cmd_lo;             /* value to be programmed second into C QUEUE */
    uint16_t        data_len;           /* Data length                                */
    uint8_t         speed;              /* xfer Speed */
}i3c_xfer_cmd_t;

/* brief I3C device provisional ID type */
typedef struct _i3c_slave_pid_t
{
    uint32_t dcr         :12;   /* slave DCR                                          */
    uint32_t inst_id     :4;    /* slave Instance ID                                  */
    uint32_t part_id     :16;   /* slave Part ID                                      */
    uint32_t pid_sel     :1;    /* slave PID selection; 0:Vendor fixed, 1: Random     */
    uint32_t mipi_mfg_id :15;   /* slave MIPI Manufacturer ID                         */
    uint32_t reserved    :16;
}i3c_slave_pid_t;

/* brief I3C device characteristics */
typedef struct _i3c_dev_char_t {
    uint8_t         static_addr;        /* Static address                             */
    uint8_t         bcr;                /* Bus characteristics                        */
    uint8_t         dcr;                /* Device characteristics                     */
    uint8_t         dynamic_addr;       /* Dynamic address                            */
}i3c_dev_char_t;

/* brief I3C device Prime information */
typedef struct _i3c_dev_prime_info_t {
    i3c_dev_char_t  dev_char;           /* Device Characteristics                     */
    i3c_slave_pid_t pid;                /* Provisional ID                             */
}i3c_dev_prime_info_t;

/**
\brief I3C Device Transfer
*/
typedef struct _i3c_xfer_t
{
  i3c_xfer_cmd_t            xfer_cmd;   /* Transfer command information to low level  */
  uint16_t                  tx_len;     /* len of data to be programmed into TX_PORT  */
  uint16_t                  rx_len;     /* len of received data                       */
  const void               *tx_buf;     /* buf address where tx data resides          */
  void                     *rx_buf;     /* pointer where rx data needs to be kept     */
  volatile I3C_XFER_STATUS  status;     /* transfer status                            */
  volatile uint8_t          error;      /* error if any for this transfer             */
  volatile uint8_t          addr;       /* Address of the slave                       */
}i3c_xfer_t;

/**
  \fn          static inline bool i3c_is_master(I3C_Type *i3c)
  \brief       Checks whether current instance is a master
  \param[in]   i3c   Pointer to i3c register map
  \return      none
*/
static inline bool i3c_is_master(I3C_Type *i3c)
{
    return((i3c->I3C_PRESENT_STATE & I3C_PRESENT_STATE_CURRENT_MASTER) != 0);
}

/**
  \fn          static inline void i3c_master_set_dynamic_addr(I3C_Type *i3c)
  \brief       Sets dynamic address of the master device
  \param[in]   i3c   Pointer to i3c register map
  \return      Dynamic address
*/
static inline void i3c_master_set_dynamic_addr(I3C_Type *i3c)
{
    /* set first non reserved address as the master's DA */
    i3c->I3C_DEVICE_ADDR = (I3C_DEVICE_ADDR_DYNAMIC_ADDR_VALID |
                           I3C_DEVICE_ADDR_DYNAMIC_ADDR(I3C_MASTER_DYNAMIC_ADDR));
}

/**
  \fn          static inline uint8_t i3c_get_static_addr(I3C_Type *i3c)
  \brief       returns static address of the device
  \param[in]   i3c   Pointer to i3c register map
  \return      Static address
*/
static inline uint8_t i3c_get_static_addr(I3C_Type *i3c)
{
    return (i3c->I3C_DEVICE_ADDR & I3C_DEVICE_ADDR_STATIC_ADDR_Msk);
}

/**
  \fn          static inline uint8_t i3c_get_dynamic_addr(I3C_Type *i3c)
  \brief       returns dynamic address of the device
  \param[in]   i3c   Pointer to i3c register map
  \return      Dynamic address
*/
static inline uint8_t i3c_get_dynamic_addr(I3C_Type *i3c)
{
    return ((i3c->I3C_DEVICE_ADDR & I3C_DEVICE_ADDR_DYNAMIC_ADDR_Msk) >>
             I3C_DEVICE_ADDR_DYNAMIC_ADDR_Pos);
}

/**
  \fn          static inline void i3c_set_dcr(I3C_Type *i3c)
  \brief       Sets Device characteristics of the device
  \param[in]   i3c   Pointer to i3c register map
  \return      none
*/
static inline void i3c_set_dcr(I3C_Type *i3c, const uint8_t val)
{
    uint32_t temp = i3c->I3C_SLV_CHAR_CTRL;

    temp &= ~I3C_SLV_CHAR_CTRL_DCR_Msk;

    temp |= ((val << I3C_SLV_CHAR_CTRL_DCR_Pos) &
                     I3C_SLV_CHAR_CTRL_DCR_Msk);

    i3c->I3C_SLV_CHAR_CTRL = temp;
}

/**
  \fn          static inline uint8_t i3c_get_dcr(I3C_Type *i3c)
  \brief       returns Device characteristics of the device
  \param[in]   i3c   Pointer to i3c register map
  \return      DCR
*/
static inline uint8_t i3c_get_dcr(I3C_Type *i3c)
{
    return I3C_SLV_CHAR_CTRL_DCR(i3c->I3C_SLV_CHAR_CTRL);
}

/**
  \fn          static inline uint8_t i3c_get_bcr(I3C_Type *i3c)
  \brief       returns Bus characteristics of the device
  \param[in]   i3c   Pointer to i3c register map
  \return      BCR
*/
static inline uint8_t i3c_get_bcr(I3C_Type *i3c)
{
    return I3C_SLV_CHAR_CTRL_BCR(i3c->I3C_SLV_CHAR_CTRL);
}

/**
  \fn          static inline void i3c_slave_set_pid(I3C_Type *i3c,
                                                    const i3c_slave_pid_t slv_pid)
  \brief       Sets provisional ID of the slave device
  \param[in]   i3c       Pointer to i3c register map
  \param[in]   slv_pid   Slave's PID
  \return      None
*/
static inline void i3c_slave_set_pid(I3C_Type *i3c, const i3c_slave_pid_t slv_pid)
{
    /* Store 48-bits Provisional ID */
    i3c->I3C_SLV_MIPI_ID_VALUE = (slv_pid.pid_sel                    |
                                 (slv_pid.mipi_mfg_id               <<
                                  I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID_Pos));

    i3c->I3C_SLV_PID_VALUE     = (slv_pid.dcr                        |
                                 (slv_pid.inst_id                   <<
                                  I3C_SLV_PID_VALUE_SLV_INST_ID_Pos) |
                                 (slv_pid.part_id                   <<
                                  I3C_SLV_PID_VALUE_SLV_PART_ID_Pos));
}

/**
  \fn          static inline i3c_slave_pid_t i3c_slave_get_pid(I3C_Type *i3c)
  \brief       returns provisional ID of the slave device
  \param[in]   i3c   Pointer to i3c register map
  \return      Provisional ID
*/
static inline i3c_slave_pid_t i3c_slave_get_pid(I3C_Type *i3c)
{
    i3c_slave_pid_t pid;
    uint32_t temp   = i3c->I3C_SLV_MIPI_ID_VALUE;

    pid.pid_sel     = (temp & I3C_SLV_MIPI_ID_VALUE_SLV_PROV_ID_SEL);
    pid.mipi_mfg_id = I3C_SLV_MIPI_ID_VALUE_SLV_MIPI_MFG_ID(temp);

    temp            = i3c->I3C_SLV_PID_VALUE;

    pid.dcr         = I3C_SLV_PID_VALUE_SLV_PID_DCR(temp);
    pid.inst_id     = I3C_SLV_PID_VALUE_SLV_INST_ID(temp);
    pid.part_id     = I3C_SLV_PID_VALUE_SLV_PART_ID(temp);

    return pid;
}

/**
  \fn          static inline uint32_t i3c_slave_get_max_read_turn(I3C_Type *i3c)
  \brief       returns Maximum Read TurnAround time of slave
  \param[in]   i3c   Pointer to i3c register map
  \return      Max Rd turn around time
*/
static inline uint32_t i3c_slave_get_max_read_turn(I3C_Type *i3c)
{
    return I3C_MAX_READ_TURNAROUND_MXDS_MAX_RD_TURN(
           i3c->I3C_MAX_READ_TURNAROUND);
}

/**
  \fn          static inline uint32_t i3c_slave_get_max_read_speed(I3C_Type *i3c)
  \brief       returns Maximum Read speed of slave
  \param[in]   i3c   Pointer to i3c register map
  \return      Max Read speed
*/
static inline uint32_t i3c_slave_get_max_read_speed(I3C_Type *i3c)
{
    return I3C_MAX_DATA_SPEED_MXDS_MAX_RD_SPEED(
           i3c->I3C_MAX_DATA_SPEED);
}

/**
  \fn          static inline uint32_t i3c_slave_get_max_write_speed(I3C_Type *i3c)
  \brief       returns Maximum write speed of slave
  \param[in]   i3c   Pointer to i3c register map
  \return      Max write speed
*/
static inline uint32_t i3c_slave_get_max_write_speed(I3C_Type *i3c)
{
    return I3C_MAX_DATA_SPEED_MXDS_MAX_WR_SPEED(
           i3c->I3C_MAX_DATA_SPEED);
}

/**
  \fn          static inline uint16_t i3c_slave_get_max_read_len(I3C_Type *i3c)
  \brief       Gets maximum read lenth being supported
  \param[in]   i3c   Pointer to i3c register map
  \return      Max read length
*/
static inline uint16_t i3c_slave_get_max_read_len(I3C_Type *i3c)
{
    return ((uint16_t)((i3c->I3C_SLV_MAX_LEN & I3C_SLV_MAX_LEN_MRL_Msk) >>
                        I3C_SLV_MAX_LEN_MRL_Pos));
}

/**
  \fn          static inline uint16_t i3c_slave_get_max_write_len(I3C_Type *i3c)
  \brief       Gets maximum write lenth being supported as a slave
  \param[in]   i3c   Pointer to i3c register map
  \return      Max write length
*/
static inline uint16_t i3c_slave_get_max_write_len(I3C_Type *i3c)
{
    return ((uint16_t)(i3c->I3C_SLV_MAX_LEN & I3C_SLV_MAX_LEN_MWL_Msk));
}

/**
  \fn          void i3c_dma_enable(I3C_Type *i3c)
  \brief       enable i3c dma
  \param[in]   i3c   Pointer to i3c register map
  \return      none
*/
static inline void i3c_dma_enable(I3C_Type *i3c)
{
    i3c->I3C_DEVICE_CTRL |= I3C_DEVICE_CTRL_DMA_ENABLE;
}

/**
  \fn          void i3c_dma_disable(I3C_Type *i3c)
  \brief       disable i3c dma
  \param[in]   i3c   Pointer to i3c register map
  \return      none
*/
static inline void i3c_dma_disable(I3C_Type *i3c)
{
    i3c->I3C_DEVICE_CTRL &= (~I3C_DEVICE_CTRL_DMA_ENABLE);
}

/**
  \fn          void* i3c_get_dma_tx_addr(I3C_Type *i3c)
  \brief       Return the DMA Tx Address(TX FIFO)
  \param[in]   i3c   Pointer to i3c register map
  \return      Return the DMA Tx address
*/
static inline void* i3c_get_dma_tx_addr(I3C_Type *i3c)
{
    return ((void *)&(i3c->I3C_TX_DATA_PORT));
}

/**
  \fn          void* i3c_get_dma_rx_addr(I3C_Type *i3c)
  \brief       Return the DMA Rx Address(RX FIFO)
  \param[in]   i3c   Pointer to i3c register map
  \return      Return the DMA Rx address
*/
static inline void* i3c_get_dma_rx_addr(I3C_Type *i3c)
{
    return ((void *)&(i3c->I3C_RX_DATA_PORT));
}

/**
  \fn          uint8_t i3c_get_tx_empty_buf_thld(I3C_Type *i3c)
  \brief       Get TX empty buffer thresold value
  \param[in]   i3c   Pointer to i3c register map
  \return      TX empty buffer thresold value
*/
static inline uint8_t i3c_get_tx_empty_buf_thld(I3C_Type *i3c)
{
    uint8_t  tx_thld_val  = 0;
    uint32_t tx_empty_loc = (i3c->I3C_DATA_BUFFER_THLD_CTRL & \
                             I3C_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_Msk);

    /* as per datasheet each location is 4-bytes aligned.*/
    switch(tx_empty_loc)
    {
        case 0:
            tx_thld_val = 1;
            break;
        case 1:
            tx_thld_val = 4;
            break;
        case 2:
            tx_thld_val = 8;
            break;
        case 3:
            tx_thld_val = 16;
            break;
        case 4:
            tx_thld_val = 32;
            break;
        case 5:
            tx_thld_val = 64;
            break;
    }

    return tx_thld_val;
}

/**
  \fn          uint8_t i3c_get_rx_buf_thld(I3C_Type *i3c)
  \brief       Get RX buffer thresold value
  \param[in]   i3c   Pointer to i3c register map
  \return      RX buffer thresold value
*/
static inline uint8_t i3c_get_rx_buf_thld(I3C_Type *i3c)
{
    uint8_t  rx_thld_val  = 0;
    uint32_t rx_empty_loc = ((i3c->I3C_DATA_BUFFER_THLD_CTRL &  \
                              I3C_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_Msk) >> 8);

    /* as per datasheet each location is 4-bytes aligned.*/
    switch(rx_empty_loc)
    {
        case 0:
            rx_thld_val = 1;
            break;
        case 1:
            rx_thld_val = 4;
            break;
        case 2:
            rx_thld_val = 8;
            break;
        case 3:
            rx_thld_val = 16;
            break;
        case 4:
            rx_thld_val = 32;
            break;
        case 5:
            rx_thld_val = 64;
            break;
    }

    return rx_thld_val;
}

/**
  \fn           void i3c_resume(I3C_Type *i3c)
  \brief        resume i3c controller
                 (use in case of any error)
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_resume(I3C_Type *i3c)
{
    /* Resume i3c controller */
    i3c->I3C_DEVICE_CTRL |= I3C_DEVICE_CTRL_RESUME;
}

/**
  \fn           static inline void i3c_flush_all_buffers(I3C_Type *i3c)
  \brief        Flushes all the buffers (Queues and Fifos)
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_flush_all_buffers(I3C_Type *i3c)
{
    /* Flushes all data and command buffers */
    i3c->I3C_RESET_CTRL |= (I3C_RESET_CTRL_IBI_QUEUE_RST   |
                            I3C_RESET_CTRL_RX_FIFO_RST     |
                            I3C_RESET_CTRL_TX_FIFO_RST     |
                            I3C_RESET_CTRL_RESP_QUEUE_RST  |
                            I3C_RESET_CTRL_CMD_QUEUE_RST);
}

/**
  \fn           void i3c_abort_msg_transfer(I3C_Type *i3c)
  \brief        Aborts i3c message transfer as a master
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_abort_msg_transfer(I3C_Type *i3c)
{
    /* Aborts message transfer */
    i3c->I3C_DEVICE_CTRL |= I3C_DEVICE_CTRL_ABORT;
}

/**
  \fn           void i3c_clear_xfer_error(I3C_Type *i3c)
  \brief        clear transfer error interrupt status
                 (used in case of any error)
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_clear_xfer_error(I3C_Type *i3c)
{
    /* clear i3c interrupt transfer error status. */
    i3c->I3C_INTR_STATUS = I3C_INTR_STATUS_TRANSFER_ERR_STS;
}

/**
  \fn           uint32_t i3c_get_dct_addr(I3C_Type *i3c)
  \brief        get start address of Device Characteristics Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       start address of Device Characteristics Table
*/
static inline uint32_t i3c_get_dct_addr(I3C_Type *i3c)
{
    return (i3c->I3C_DEV_CHAR_TABLE_POINTER &
            I3C_DEV_CHAR_TABLE_POINTER_P_DEV_CHAR_TABLE_START_ADDR_Msk);
}

/**
  \fn           uint32_t i3c_get_dct_cur_idx(I3C_Type *i3c)
  \brief        get depth of Device Characteristics Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       depth of Device Characteristics Table
*/
static inline uint32_t i3c_get_dct_cur_idx(I3C_Type *i3c)
{
    return ((i3c->I3C_DEV_CHAR_TABLE_POINTER &
            I3C_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_Msk) >>
            I3C_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX_Pos);
}

/**
  \fn           uint32_t i3c_get_dat_addr(I3C_Type *i3c)
  \brief        get start address of Device Address Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       start address of Device Address Table
*/
static inline uint32_t i3c_get_dat_addr(I3C_Type *i3c)
{
    return (i3c->I3C_DEVICE_ADDR_TABLE_POINTER &
            I3C_DEVICE_ADDR_TABLE_POINTER_P_DEV_ADDR_TABLE_START_ADDR_Msk);
}

/**
  \fn           uint32_t i3c_get_dat_depth(I3C_Type *i3c)
  \brief        get depth of Device Address Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       depth of Device Address Table
*/
static inline uint32_t i3c_get_dat_depth(I3C_Type *i3c)
{
    return (i3c->I3C_DEVICE_ADDR_TABLE_POINTER >>
            I3C_DEVICE_ADDR_TABLE_POINTER_DEV_ADDR_TABLE_DEPTH_Pos);
}

/**
  \fn           void i3c_update_dat(I3C_Type *i3c,
                                    uint32_t  pos,
                                    uint32_t  val)
  \brief        update Device Address Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    pos     : DAT position
  \param[in]    val     : Value needs to be written to DAT position
  \return       none
*/
static inline void i3c_update_dat(I3C_Type *i3c,
                                  uint32_t  pos,
                                  uint32_t  val)
{
    uint32_t datp = i3c_get_dat_addr(i3c);
    uint32_t dat_addr = 0;

    /* DAT address = i3c Base + DAT Base + (Pos * 4) */
    dat_addr = (uint32_t)i3c + datp + (pos << 2);
    *((volatile uint32_t *) (dat_addr)) = val;
}

/**
  \fn           void i3c_add_slv_to_dat(I3C_Type *i3c,
                                        const uint32_t  pos,
                                        const uint8_t   dyn_addr,
                                        const uint8_t   sta_addr)
  \brief        add i3c/i2c slave to Device Address Table
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    pos      : DAT position
  \param[in]    dyn_addr : Slave dynamic address, only for i3c slave
  \param[in]    sta_addr : Slave static address
  \return       none
*/
static inline void i3c_add_slv_to_dat(I3C_Type *i3c,
                                      const uint32_t  pos,
                                      const uint8_t   dyn_addr,
                                      const uint8_t   sta_addr)
{
    uint32_t val;

    if(dyn_addr)
    {
        /* i3c slave */
        val = I3C_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR(dyn_addr) |
              I3C_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR(sta_addr);
    }
    else
    {
        /* i2c slave */
        val = I3C_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR(sta_addr) |
              I3C_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEV;
    }

    i3c_update_dat(i3c, pos, val);
}

/**
  \fn           static inline void i3c_remove_slv_from_dat(I3C_Type *i3c,
                                                           uint32_t  pos)
  \brief        remove slave from Device Address Table
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    pos      : DAT position
  \return       none
*/
static inline void i3c_remove_slv_from_dat(I3C_Type *i3c,
                                           uint32_t  pos)
{
    i3c_update_dat(i3c, pos, 0);
}

/**
  \fn           static inline void i3c_update_slv_addr_in_dat(I3C_Type *i3c,
                                                              uint32_t  pos,
                                                              uint8_t   dyn_addr)
  \brief        Change i3c slave address at specific position
                in Device Address Table
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    pos      : DAT position
  \param[in]    dyn_addr : Slave dynamic address
  \return       none
*/
static inline void i3c_update_slv_addr_in_dat(I3C_Type *i3c,
                                              uint32_t  pos,
                                              uint8_t   dyn_addr)
{
    uint32_t val;

    /* i3c slave */
    val = I3C_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR(dyn_addr);

    i3c_update_dat(i3c, pos, val);
}

/**
  \fn           static inline void (I3C_Type *i3c,
                                    const uint8_t pos,
                                    const uint8_t retry_cnt)
  \brief        Sets nack retry count for the slave present
                given pos of DAT
  \param[in]    i3c       : Pointer to i3c register set structure
  \param[in]    pos       : DAT position
  \param[in]    retry_cnt : Retry Count
  \return       none
*/
static inline void i3c_set_slave_nack_retry_cnt(I3C_Type *i3c,
                                                const uint8_t pos,
                                                const uint8_t retry_cnt)
{
    uint32_t datp      = i3c_get_dat_addr(i3c);
    uint32_t dat_addr  = 0U;
    uint32_t dest_addr = 0U;

    /* DAT address = i3c Base + DAT Base + (Pos * 4) */
    dat_addr = (uint32_t)i3c + datp + (pos << 2);
    dest_addr = *((volatile uint32_t *) (dat_addr));

    /* Update the Device Nack retry count bits */
    dest_addr &= ~I3C_DAT_DEV_NACK_RETRY_CNT_Msk;
    dest_addr |= ((retry_cnt << I3C_DAT_DEV_NACK_RETRY_CNT_Pos) &
                   I3C_DAT_DEV_NACK_RETRY_CNT_Msk);

    *((volatile uint32_t *) (dat_addr)) = dest_addr;
}

/**
  \fn           void i3c_master_enable_interrupts(I3C_Type *i3c)
  \brief        enables master interrupts
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_master_enable_interrupts(I3C_Type *i3c)
{
    i3c->I3C_INTR_STATUS    = I3C_INTR_STATUS_ALL;
    i3c->I3C_INTR_STATUS_EN = I3C_MASTER_INTR_EN_MASK;
    i3c->I3C_INTR_SIGNAL_EN = I3C_MASTER_INTR_EN_MASK;
}

/**
  \fn           static inline void i3c_slave_enable_interrupts(I3C_Type *i3c)
  \brief        enables slave interrupts
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_slave_enable_interrupts(I3C_Type *i3c)
{
    i3c->I3C_INTR_STATUS    = I3C_INTR_STATUS_ALL;
    i3c->I3C_INTR_STATUS_EN = I3C_SLAVE_INTR_EN_MASK;
    i3c->I3C_INTR_SIGNAL_EN = I3C_SLAVE_INTR_EN_MASK;
}

/**
  \fn           static inline void i3c_set_sda_tx_hold_time(I3C_Type *i3c,
                                                            const uint8_t val)
  \brief        Sets SDA Tx hold time in terms of Core clock period
  \param[in]    i3c : Pointer to i3c register set structure
  \return       none
*/
static inline void i3c_set_sda_tx_hold_time(I3C_Type *i3c,
                                            const uint8_t val)
{
    i3c->I3C_SDA_HOLD_SWITCH_DLY_TIMING = ((val <<
         I3C_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_Pos) &
         I3C_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD_Msk);
}

/**
  \fn           static inline void i3c_master_setup_hot_join_ctrl(I3C_Type *i3c,
  \                                                               const bool enable)
  \brief        Controls hot-join control at master side
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    enable  : flag to enable/disable hot-join
  \return       none
*/
static inline void i3c_master_setup_hot_join_ctrl(I3C_Type *i3c,
                                                  const bool enable)
{
    if(enable)
    {
        /* Accepts Hot join request from slaves */
        i3c->I3C_DEVICE_CTRL    &= ~I3C_DEVICE_CTRL_HOT_JOIN_CTRL;
    }
    else
    {
        /* Denies Hot join request from slaves and
         * also doesn't notify about rejection */
        i3c->I3C_DEVICE_CTRL    |= I3C_DEVICE_CTRL_HOT_JOIN_CTRL;
        i3c->I3C_IBI_QUEUE_CTRL &= ~I3C_IBI_QUEUE_CTRL_NOTIFY_HJ_REJECTED;
    }
}

/**
  \fn           static inline void i3c_master_setup_mst_req_ctrl(I3C_Type *i3c,
  \                                                             const bool enable)
  \brief        Controls Masterhip request control at master side
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    enable  : flag to enable/disable Mastership request
  \return       none
*/
static inline void i3c_master_setup_mst_req_ctrl(I3C_Type *i3c,
                                                 const bool enable)
{
    if(enable)
    {
        /* Accepts Mastership request from slaves */
        i3c->I3C_IBI_MR_REQ_REJECT &= ~I3C_IBI_MR_REQ_REJECT_MR_REQ_REJECT;

        /* Enable updated ownership interrupt */
        i3c->I3C_INTR_STATUS_EN    |= I3C_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN;
        i3c->I3C_INTR_SIGNAL_EN    |= I3C_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN;
    }
    else
    {
        /* Denies Mastership request from slaves and
         * also doesn't notify about rejection */
        i3c->I3C_IBI_MR_REQ_REJECT |= I3C_IBI_MR_REQ_REJECT_MR_REQ_REJECT;
        i3c->I3C_IBI_QUEUE_CTRL    &= ~I3C_IBI_QUEUE_CTRL_NOTIFY_MR_REJECTED;

        /* Disable updated ownership interrupt */
        i3c->I3C_INTR_STATUS_EN    &= ~I3C_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN;
        i3c->I3C_INTR_SIGNAL_EN    &= ~I3C_INTR_STATUS_EN_BUSOWNER_UPDATED_STS_EN;
    }
}

/**
  \fn           static inline void i3c_master_setup_slv_intr_req_ctrl(I3C_Type *i3c,
  \                                                                   const bool enable)
  \brief        Controls Slave Interrupt request control at master side
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    enable  : flag to enable/disable SIR request acceptance
  \return       none
*/
static inline void i3c_master_setup_slv_intr_req_ctrl(I3C_Type *i3c,
                                                      const bool enable)
{
    if(enable)
    {
        /* Accepts Mastership request from slaves */
        i3c->I3C_IBI_SIR_REQ_REJECT &= ~I3C_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT;
    }
    else
    {
        /* Denies Mastership request from slaves and
         * also doesn't notify about rejection */
        i3c->I3C_IBI_SIR_REQ_REJECT |= I3C_IBI_SIR_REQ_REJECT_SIR_REQ_REJECT;
        i3c->I3C_IBI_QUEUE_CTRL     &= ~I3C_IBI_QUEUE_CTRL_NOTIFY_SIR_REJECTED;
    }
}

/**
  \fn           static inline void i3c_slave_setup_adaptive_mode(I3C_Type *i3c,
  \                                                              const bool enable)
  \brief        Controls I2C/I3c Adaptive mode for slave
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    enable  : flag to enable/disable Adaptive mode
  \return       none
*/
static inline void i3c_slave_setup_adaptive_mode(I3C_Type *i3c,
                                                 const bool enable)
{
    /* Enables I2C adaptive mode */
    i3c->I3C_DEVICE_CTRL     &= ~I3C_DEVICE_CTRL_ENABLE;

    if(enable)
    {
        /* Enables I2C adaptive mode */
        i3c->I3C_DEVICE_CTRL |= I3C_DEVICE_CTRL_ADAPTIVE_I2C_I3C;
    }
    else
    {
        /* Disables I2C adaptive mode */
        i3c->I3C_DEVICE_CTRL &= ~I3C_DEVICE_CTRL_ADAPTIVE_I2C_I3C;
    }

    /* Enables Hot-Join */
    i3c->I3C_SLV_EVENT_STATUS |= I3C_SLV_EVENT_STATUS_HJ_EN;
}

/**
  \fn           uint8_t i3c_get_slv_dyn_addr(I3C_Type *i3c,
                                             const uint8_t static_addr)
  \brief        Gets slave's dynamic from DAT for the
                given static address
  \param[in]    i3c         : Pointer to i3c register set structure
  \param[in]    static_addr : Slave's static address
  \return       none
*/
uint8_t i3c_get_slv_dyn_addr(I3C_Type *i3c, const uint8_t static_addr);

/**
  \fn           void i3c_sec_master_get_dct(I3C_Type *i3c,
  \                                         i3c_dev_char_t *data,
  \                                         const uint8_t slv_cnt)
  \brief        Get Device Characteritics Table of slaves.
  \             Note: Applicable only for secondary masters,
  \             not for bus master.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    data    : Device characteristics data
  \param[in]    slv_cnt : Slaves count
  \return       start address of Device Address Table
*/
void i3c_sec_master_get_dct(I3C_Type *i3c,
                            i3c_dev_char_t *data,
                            const uint8_t slv_cnt);

/**
  \fn           void i3c_master_get_dct(I3C_Type *i3c,
  \                                     i3c_dev_prime_info_t *data,
  \                                     const uint8_t addr)
  \brief        Get Device Characteritics Table of requested slave.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    data    : Device characteristics data
  \param[in]    addr    : Slave's dynamic address
  \return       None
*/
void i3c_master_get_dct(I3C_Type *i3c,
                        i3c_dev_prime_info_t *data,
                        const uint8_t addr);

/**
  \fn           void i3c_master_tx(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        send master transmit command to i3c bus.
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_master_tx(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_master_rx(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        receive master receive command to i3c bus.
  \param[in]    i3c       : Pointer to i3c register set structure
  \param[in]    xfer      : Pointer to i3c transfer structure
  \return       none
*/

void i3c_master_rx(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_master_tx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        perform data transmission in blocking mode.
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_master_tx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_master_rx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        perform data reception in blocking mode.
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_master_rx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_slave_tx(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        send slave transmit command to i3c bus.
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_tx(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_slave_rx(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        send slave receive command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_rx(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_slave_tx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        Performs slave data transmission in blocking mode.
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_tx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_slave_rx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        Performs slave data reception in blocking mode.
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_rx_blocking(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_send_xfer_cmd(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        performs master command transfer
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_send_xfer_cmd(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_send_xfer_cmd_blocking(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        performs command transfer in blocking mode
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    xfer     : Pointer to i3c transfer structure
  \return       none
*/
void i3c_send_xfer_cmd_blocking(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_slow_bus_clk_cfg(I3C_Type *i3c,
                                          uint32_t  core_clk)
  \brief        i3c slow bus clock configuration for i3c slave device
  \param[in]    i3c       : Pointer to i3c register set structure
  \param[in]    core_clk  : core clock
  \return       none
*/
void i3c_slow_bus_clk_cfg(I3C_Type *i3c,
                          uint32_t  core_clk);

/**
  \fn           void i3c_normal_bus_clk_cfg(I3C_Type *i3c,
                                            uint32_t  core_clk)
  \brief        i3c normal bus clock configuration for i3c slave device
  \param[in]    i3c       : Pointer to i3c register set structure
  \param[in]    core_clk  : core clock
  \return       none
*/
void i3c_normal_bus_clk_cfg(I3C_Type *i3c,
                            uint32_t  core_clk);

/**
  \fn           void i2c_clk_cfg(I3C_Type           *i3c,
                                 uint32_t            core_clk,
                                 I3C_I2C_SPEED_MODE  i2c_speed_mode)
  \brief        i3c clock configuration for legacy i2c slave device
  \param[in]    i3c             : Pointer to i3c register set structure
  \param[in]    core_clk        : core clock
  \param[in]    i2c_speed_mode  : i2c Speed mode
                 I3C_I2C_SPEED_MODE_FMP_1_MBPS  : Fast Mode Plus 1   MBPS
                 I3C_I2C_SPEED_MODE_FM_400_KBPS : Fast Mode      400 KBPS
                 I3C_I2C_SPEED_MODE_SS_100_KBPS : Standard Mode  100 KBPS
  \return        none
*/
void i2c_clk_cfg(I3C_Type          *i3c,
                 uint32_t           core_clk,
                 I3C_I2C_SPEED_MODE i2c_speed_mode);

/**
  \fn           void i3c_master_init(I3C_Type *i3c)
  \brief        Initialize i3c master.
                 This function will :
                  - Set mode as master
                  - Clear Command Queue and Data buffer Queue
                  - Enable i3c controller
  \param[in]    i3c  : Pointer to i3c register
                        set structure
  \return       none
*/
void i3c_master_init(I3C_Type *i3c);

/**
  \fn           void i3c_slave_init(I3C_Type *i3c,
                                    const uint8_t  slv_addr,
                                    const uint32_t core_clk)
  \brief        Initialize i3c slave.
                 This function will :
                  - set slave static address
                  - set secondary master as slave mode
                  - Enable i3c controller
  \param[in]    i3c       : Pointer to i3c register
                             set structure
  \param[in]    slv_addr  : Slave own Address
  \param[in]    core_clk  : core clock
  \return       none
*/
void i3c_slave_init(I3C_Type *i3c,
                    const uint8_t  slv_addr,
                    const uint32_t core_clk);

/**
  \fn           int32_t i3c_slave_req_bus_mastership(I3C_Type *i3c)
  \brief        Sends mastership request to master
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       exec status
*/
int32_t i3c_slave_req_bus_mastership(I3C_Type *i3c);

/**
  \fn           int32_t i3c_slave_tx_slv_intr_req(I3C_Type *i3c)
  \brief        Sends Slave Interrupt request to master
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       exec status
*/
int32_t i3c_slave_tx_slv_intr_req(I3C_Type *i3c);

/**
  \fn           void i3c_master_irq_handler(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        i3c interrupt service routine for master
  \param[in]    i3c  : Pointer to i3c register set structure
  \param[in]    xfer : Pointer to i3c transfer structure
  \return       none
*/
void i3c_master_irq_handler(I3C_Type *i3c, i3c_xfer_t *xfer);

/**
  \fn           void i3c_slave_irq_handler(I3C_Type *i3c, i3c_xfer_t *xfer)
  \brief        i3c interrupt service routine for slave
  \param[in]    i3c  : Pointer to i3c register set structure
  \param[in]    xfer : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_irq_handler(I3C_Type *i3c, i3c_xfer_t *xfer);

#ifdef __cplusplus
}
#endif

#endif /* I3C_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
