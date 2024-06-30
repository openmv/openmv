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
    volatile const  uint32_t I3C_IBI_QUEUE_DATA;   /*!< (@ 0x00000018) In-Band Interrupt Queue Data Register                      */
    volatile const  uint32_t I3C_IBI_QUEUE_STATUS; /*!< (@ 0x00000018) In-Band Interrupt Queue Status Register                    */
  };
  volatile uint32_t  I3C_QUEUE_THLD_CTRL;          /*!< (@ 0x0000001C) Queue Threshold Control Register                           */
  volatile uint32_t  I3C_DATA_BUFFER_THLD_CTRL;    /*!< (@ 0x00000020) Data Buffer Threshold Control Register                     */
  volatile uint32_t  I3C_IBI_QUEUE_CTRL;           /*!< (@ 0x00000024) IBI Queue Control Register                                 */
  volatile const  uint32_t  RESERVED;
  volatile uint32_t  I3C_IBI_MR_REQ_REJECT;        /*!< (@ 0x0000002C) IBI MR Request Rejection Control Register                  */
  volatile uint32_t  I3C_IBI_SIR_REQ_REJECT;       /*!< (@ 0x00000030) IBI SIR Request Rejection Control Register                 */
  volatile uint32_t  I3C_RESET_CTRL;               /*!< (@ 0x00000034) Reset Control Register                                     */
  volatile uint32_t  I3C_SLV_EVENT_STATUS;         /*!< (@ 0x00000038) Slave Event Status Register                                */
  volatile uint32_t  I3C_INTR_STATUS;              /*!< (@ 0x0000003C) Interrupt Status Register                                  */
  volatile uint32_t  I3C_INTR_STATUS_EN;           /*!< (@ 0x00000040) Interrupt Status Enable Register                           */
  volatile uint32_t  I3C_INTR_SIGNAL_EN;           /*!< (@ 0x00000044) Interrupt Signal Enable Register                           */
  volatile uint32_t  I3C_INTR_FORCE;               /*!< (@ 0x00000048) Interrupt Force Enable Register                            */
  volatile const  uint32_t  I3C_QUEUE_STATUS_LEVEL;       /*!< (@ 0x0000004C) Queue Status Level Register                                */
  volatile const  uint32_t  I3C_DATA_BUFFER_STATUS_LEVEL; /*!< (@ 0x00000050) Data Buffer Status Level Register                          */
  volatile const  uint32_t  I3C_PRESENT_STATE;            /*!< (@ 0x00000054) Present State Register                                     */
  volatile const  uint32_t  I3C_CCC_DEVICE_STATUS;        /*!< (@ 0x00000058) Device Operating Status Register                           */
  volatile const  uint32_t  I3C_DEVICE_ADDR_TABLE_POINTER;/*!< (@ 0x0000005C) Pointer for Device Address Table Registers                 */
  volatile uint32_t  I3C_DEV_CHAR_TABLE_POINTER;          /*!< (@ 0x00000060) Pointer for Device Characteristics Table Register          */
  volatile const  uint32_t  RESERVED1[2];
  volatile const  uint32_t  I3C_VENDOR_SPECIFIC_REG_POINTER;/*!< (@ 0x0000006C) Pointer for Vendor Specific Registers                    */
  volatile uint32_t  I3C_SLV_MIPI_ID_VALUE;        /*!< (@ 0x00000070) Provisional ID Register                                    */
  volatile uint32_t  I3C_SLV_PID_VALUE;            /*!< (@ 0x00000074) Provisional ID Register                                    */
  volatile uint32_t  I3C_SLV_CHAR_CTRL;            /*!< (@ 0x00000078) I3C Slave Characteristic Register                          */
  volatile const  uint32_t  I3C_SLV_MAX_LEN;         /*!< (@ 0x0000007C) I3C Max Write/Read Length Register                         */
  volatile const  uint32_t  I3C_MAX_READ_TURNAROUND; /*!< (@ 0x00000080) MXDS Maximum Read Turnaround Time Register                 */
  volatile uint32_t  I3C_MAX_DATA_SPEED;           /*!< (@ 0x00000084) MXDS Maximum Data Speed Register                           */
  volatile const  uint32_t  RESERVED2;
  volatile uint32_t  I3C_SLV_INTR_REQ;             /*!< (@ 0x0000008C) Slave Interrupt Request Register                           */
  volatile const  uint32_t  RESERVED3[8];
  volatile uint32_t  I3C_DEVICE_CTRL_EXTENDED;     /*!< (@ 0x000000B0) Device Control Extended Register                           */
  volatile uint32_t  I3C_SCL_I3C_OD_TIMING;        /*!< (@ 0x000000B4) SCL I3C Open Drain Timing Register                         */
  volatile uint32_t  I3C_SCL_I3C_PP_TIMING;        /*!< (@ 0x000000B8) SCL I3C Push Pull Timing Register                          */
  volatile uint32_t  I3C_SCL_I2C_FM_TIMING;        /*!< (@ 0x000000BC) SCL I2C Fast Mode Timing Register                          */
  volatile uint32_t  I3C_SCL_I2C_FMP_TIMING;       /*!< (@ 0x000000C0) SCL I2C Fast Mode Plus Timing Register                     */
  volatile const  uint32_t  RESERVED4;
  volatile uint32_t  I3C_SCL_EXT_LCNT_TIMING;      /*!< (@ 0x000000C8) SCL Extended Low Count Timing Register                     */
  volatile uint32_t  I3C_SCL_EXT_TERMN_LCNT_TIMING;/*!< (@ 0x000000CC) SCL Termination Bit Low Count Timing Register              */
  volatile uint32_t  I3C_SDA_HOLD_SWITCH_DLY_TIMING;/*!< (@ 0x000000D0) SDA Hold and Mode Switch Delay Timing Register            */
  volatile uint32_t  I3C_BUS_FREE_AVAIL_TIMING;    /*!< (@ 0x000000D4) Bus Free Timing Register                                   */
  volatile uint32_t  I3C_BUS_IDLE_TIMING;          /*!< (@ 0x000000D8) Bus Idle Timing Register                                   */
  volatile uint32_t  I3C_SCL_LOW_MST_EXT_TIMEOUT;  /*!< (@ 0x000000DC) SCL Low Master Extended Timeout Register                   */
  volatile const  uint32_t  I3C_VER_ID;                /*!< (@ 0x000000E0) Reserved                                                   */
  volatile const  uint32_t  I3C_VER_TYPE;              /*!< (@ 0x000000E4) Reserved                                                   */
  volatile const  uint32_t  I3C_QUEUE_SIZE_CAPABILITY; /*!< (@ 0x000000E8) I3C Queue Size Capability Register                         */
  volatile const  uint32_t  RESERVED5[69];

  union {
    volatile const  uint32_t I3C_DEV_CHAR_TABLE1_LOC1; /*!< (@ 0x00000200) Device Characteristic Table Location 1 Register            */
    volatile const  uint32_t I3C_SEC_DEV_CHAR_TABLE1;  /*!< (@ 0x00000200) Secondary Master Device Characteristic Table
                                                                           Location Register                                          */
  };
  volatile const  uint32_t  I3C_DEV_CHAR_TABLE1_LOC2;  /*!< (@ 0x00000204) Device Characteristic Table Location 2 Register            */
  volatile const  uint32_t  I3C_DEV_CHAR_TABLE1_LOC3;  /*!< (@ 0x00000208) Device Characteristic Table Location 3 Register            */
  volatile const  uint32_t  I3C_DEV_CHAR_TABLE1_LOC4;  /*!< (@ 0x0000020C) Device Characteristic Table Location 4 Register            */
  volatile const  uint32_t  RESERVED6[4];
  volatile uint32_t  I3C_DEV_ADDR_TABLE_LOC1;          /*!< (@ 0x00000220) Device Address Table Location 1 Register                   */
} I3C_Type;

/* common helpers */
#define BIT(nr)                           (1UL << (nr))

#define GENMASK(h, l) \
                                          (((~(0U)) - ((1U) << (l)) + 1) & \
                                          (~(0U) >> (32 - 1 - (h))))

#define DIV_ROUND_UP(n, d)                (((n) + (d) - 1) / (d))

/* Maximum 8 Slave Devices are supported
 * (\ref register DEVICE_ADDR_TABLE_POINTER) */
#define MAX_DEVS                          8

/* Clock rates and periods */
#define I3C_BUS_MAX_I3C_SCL_RATE          12900000
#define I3C_BUS_TYP_I3C_SCL_RATE          12500000
#define I3C_BUS_I2C_FM_PLUS_SCL_RATE      1000000
#define I3C_BUS_I2C_FM_SCL_RATE           400000
#define I3C_BUS_I2C_SS_SCL_RATE           100000
#define I3C_BUS_TLOW_OD_MIN_NS            200

/* ref clock frequency 1 GHz to get core-period */
#define REF_CLK_RATE                      1000000000

/* transaction ids for tx and rx, CCC set and get,
 *  and Dynamic Address Assignment. */
#define I3C_CCC_SET_TID                   0x1
#define I3C_CCC_GET_TID                   0x2
#define I3C_MST_TX_TID                    0x3
#define I3C_MST_RX_TID                    0x4
#define I3C_ADDR_ASSIGN_TID               0x5
#define I3C_SLV_TX_TID                    0X6
#define I3C_SLV_RX_TID                    0X8

#define DEVICE_CTRL                       0x0
#define DEV_CTRL_ENABLE                   BIT(31)
#define DEV_CTRL_RESUME                   BIT(30)
#define DEV_CTRL_DMA_ENABLE               BIT(28)
#define DEV_CTRL_HOT_JOIN_NACK            BIT(8)
#define DEV_CTRL_I2C_SLAVE_PRESENT        BIT(7)

#define DEVICE_ADDR                       0x4
#define DEV_ADDR_DYNAMIC_ADDR_VALID       BIT(31)
#define DEV_ADDR_DYNAMIC(x)               (((x) << 16) & GENMASK(22, 16))

#define HW_CAPABILITY                     0x8
#define COMMAND_QUEUE_PORT                0xc
#define COMMAND_PORT_TOC                  BIT(30)
#define COMMAND_PORT_READ_TRANSFER        BIT(28)
#define COMMAND_PORT_SDAP                 BIT(27)
#define COMMAND_PORT_ROC                  BIT(26)
#define COMMAND_PORT_SPEED(x)             (((x) << 21) & GENMASK(23, 21))
#define COMMAND_PORT_DEV_INDEX(x)         (((x) << 16) & GENMASK(20, 16))
#define COMMAND_PORT_CP                   BIT(15)
#define COMMAND_PORT_CMD(x)               (((x) << 7) & GENMASK(14, 7))
#define COMMAND_PORT_TID(x)               (((x) << 3) & GENMASK(6, 3))
#define COMMAND_SLV_PORT_TID(x)           (((x) << 3) & GENMASK(5, 3))

#define COMMAND_PORT_ARG_DATA_LEN(x)      (((x) << 16) & GENMASK(31, 16))
#define COMMAND_PORT_ARG_DATA_LEN_MAX     65536
#define COMMAND_PORT_TRANSFER_ARG         0x01

#define COMMAND_PORT_SDA_DATA_BYTE_3(x)   (((x) << 24) & GENMASK(31, 24))
#define COMMAND_PORT_SDA_DATA_BYTE_2(x)   (((x) << 16) & GENMASK(23, 16))
#define COMMAND_PORT_SDA_DATA_BYTE_1(x)   (((x) << 8) & GENMASK(15, 8))
#define COMMAND_PORT_SDA_BYTE_STRB_3      BIT(5)
#define COMMAND_PORT_SDA_BYTE_STRB_2      BIT(4)
#define COMMAND_PORT_SDA_BYTE_STRB_1      BIT(3)
#define COMMAND_PORT_SHORT_DATA_ARG       0x02

#define COMMAND_PORT_DEV_COUNT(x)         (((x) << 21) & GENMASK(25, 21))
#define COMMAND_PORT_ADDR_ASSGN_CMD       0x03

#define RESPONSE_QUEUE_PORT               0x10
#define RESPONSE_PORT_ERR_STATUS(x)       (((x) & GENMASK(31, 28)) >> 28)
#define RESPONSE_PORT_TID(x)              (((x) & GENMASK(27, 24)) >> 24)
#define RESPONSE_NO_ERROR                 0
#define RESPONSE_ERROR_CRC                1
#define RESPONSE_ERROR_PARITY             2
#define RESPONSE_ERROR_FRAME              3
#define RESPONSE_ERROR_IBA_NACK           4
#define RESPONSE_ERROR_ADDRESS_NACK       5
#define RESPONSE_ERROR_OVER_UNDER_FLOW    6
#define RESPONSE_ERROR_TRANSF_ABORT       8
#define RESPONSE_ERROR_I2C_W_NACK_ERR     9
#define RESPONSE_PORT_TID(x)              (((x) & GENMASK(27, 24)) >> 24)
#define RESPONSE_PORT_DATA_LEN(x)         ((x) & GENMASK(15, 0))

#define RX_TX_DATA_PORT                   0x14
#define IBI_QUEUE_STATUS                  0x18
#define QUEUE_THLD_CTRL                   0x1c
#define QUEUE_THLD_CTRL_RESP_BUF_MASK     GENMASK(15, 8)
#define QUEUE_THLD_CTRL_RESP_BUF(x)       (((x) - 1) << 8)

#define DATA_BUFFER_THLD_CTRL             0x20
#define DATA_BUFFER_THLD_CTRL_RX_BUF      GENMASK(10, 8)
#define DATA_BUFFER_THLD_CTRL_TX_BUF      GENMASK(2, 0)

#define IBI_QUEUE_CTRL                    0x24
#define IBI_MR_REQ_REJECT                 0x2C
#define IBI_SIR_REQ_REJECT                0x30
#define IBI_REQ_REJECT_ALL                GENMASK(31, 0)

#define RESET_CTRL                        0x34
#define RESET_CTRL_IBI_QUEUE              BIT(5)
#define RESET_CTRL_RX_FIFO                BIT(4)
#define RESET_CTRL_TX_FIFO                BIT(3)
#define RESET_CTRL_RESP_QUEUE             BIT(2)
#define RESET_CTRL_CMD_QUEUE              BIT(1)
#define RESET_CTRL_SOFT                   BIT(0)

#define SLV_EVENT_CTRL                    0x38
#define INTR_STATUS                       0x3c
#define INTR_STATUS_EN                    0x40
#define INTR_SIGNAL_EN                    0x44
#define INTR_FORCE                        0x48
#define INTR_BUSOWNER_UPDATE_STAT         BIT(13)
#define INTR_IBI_UPDATED_STAT             BIT(12)
#define INTR_READ_REQ_RECV_STAT           BIT(11)
#define INTR_DEFSLV_STAT                  BIT(10)
#define INTR_TRANSFER_ERR_STAT            BIT(9)
#define INTR_DYN_ADDR_ASSGN_STAT          BIT(8)
#define INTR_CCC_UPDATED_STAT             BIT(6)
#define INTR_TRANSFER_ABORT_STAT          BIT(5)
#define INTR_RESP_READY_STAT              BIT(4)
#define INTR_CMD_QUEUE_READY_STAT         BIT(3)
#define INTR_IBI_THLD_STAT                BIT(2)
#define INTR_RX_THLD_STAT                 BIT(1)
#define INTR_TX_THLD_STAT                 BIT(0)
#define INTR_ALL                          (INTR_BUSOWNER_UPDATE_STAT  |   \
                                          INTR_IBI_UPDATED_STAT       |   \
                                          INTR_READ_REQ_RECV_STAT     |   \
                                          INTR_DEFSLV_STAT            |   \
                                          INTR_TRANSFER_ERR_STAT      |   \
                                          INTR_DYN_ADDR_ASSGN_STAT    |   \
                                          INTR_CCC_UPDATED_STAT       |   \
                                          INTR_TRANSFER_ABORT_STAT    |   \
                                          INTR_RESP_READY_STAT        |   \
                                          INTR_CMD_QUEUE_READY_STAT   |   \
                                          INTR_IBI_THLD_STAT          |   \
                                          INTR_TX_THLD_STAT           |   \
                                          INTR_RX_THLD_STAT)

#define INTR_MASTER_MASK                  (INTR_TRANSFER_ERR_STAT     |   \
                                          INTR_RESP_READY_STAT)

#define INTR_SLAVE_MASK                   (INTR_TRANSFER_ERR_STAT     |   \
                                          INTR_DYN_ADDR_ASSGN_STAT    |   \
                                          INTR_RESP_READY_STAT)

#define MR_REQ_REJECT                     (0X1)
#define REQMST_ACK_CTRL_AS_NACK           (1 << 3)
#define DYN_ADDR_ASSGN_STS                (1 << 8)
#define STATIC_ADDR_VAILD                 (1 << 15)
#define ADAPTIVE_I2C_I3C                  (1 << 27)

#define QUEUE_STATUS_LEVEL                0x4c
#define QUEUE_STATUS_IBI_STATUS_CNT(x)    (((x) & GENMASK(28, 24)) >> 24)
#define QUEUE_STATUS_IBI_BUF_BLR(x)       (((x) & GENMASK(23, 16)) >> 16)
#define QUEUE_STATUS_LEVEL_RESP(x)        (((x) & GENMASK(15, 8)) >> 8)
#define QUEUE_STATUS_LEVEL_CMD(x)         ((x) & GENMASK(7, 0))

#define DATA_BUFFER_STATUS_LEVEL          0x50
#define DATA_BUFFER_STATUS_LEVEL_TX(x)    ((x) & GENMASK(7, 0))

#define PRESENT_STATE                     0x54
#define CCC_DEVICE_STATUS                 0x58

#define DEVICE_ADDR_TABLE_POINTER         0x5c
#define DEVICE_ADDR_TABLE_DEPTH(x)        (((x) & GENMASK(31, 16)) >> 16)
#define DEVICE_ADDR_TABLE_ADDR(x)         ((x) & GENMASK(7, 0))

#define DEV_CHAR_TABLE_POINTER            0x60
#define VENDOR_SPECIFIC_REG_POINTER       0x6c
#define SLV_PID_VALUE                     0x74
#define SLV_CHAR_CTRL                     0x78
#define SLV_MAX_LEN                       0x7c
#define MAX_READ_TURNAROUND               0x80
#define MAX_DATA_SPEED                    0x84
#define SLV_DEBUG_STATUS                  0x88
#define SLV_INTR_REQ                      0x8c
#define DEVICE_CTRL_EXTENDED              0xb0
#define SCL_I3C_OD_TIMING                 0xb4
#define SCL_I3C_PP_TIMING                 0xb8
#define SCL_I3C_TIMING_HCNT(x)            (((x) << 16) & GENMASK(23, 16))
#define SCL_I3C_TIMING_LCNT(x)            ((x) & GENMASK(7, 0))
#define SCL_I3C_TIMING_CNT_MIN            5

#define SCL_I2C_FM_TIMING                 0xbc
#define SCL_I2C_FM_TIMING_HCNT(x)         (((x) << 16) & GENMASK(31, 16))
#define SCL_I2C_FM_TIMING_LCNT(x)         ((x) & GENMASK(15, 0))

#define SCL_I2C_FMP_TIMING                0xc0
#define SCL_I2C_FMP_TIMING_HCNT(x)        (((x) << 16) & GENMASK(23, 16))
#define SCL_I2C_FMP_TIMING_LCNT(x)        ((x) & GENMASK(15, 0))

#define SCL_EXT_LCNT_TIMING               0xc8
#define SCL_EXT_LCNT_4(x)                 (((x) << 24) & GENMASK(31, 24))
#define SCL_EXT_LCNT_3(x)                 (((x) << 16) & GENMASK(23, 16))
#define SCL_EXT_LCNT_2(x)                 (((x) << 8) & GENMASK(15, 8))
#define SCL_EXT_LCNT_1(x)                 ((x) & GENMASK(7, 0))

#define SCL_EXT_TERMN_LCNT_TIMING         0xcc
#define BUS_FREE_TIMING                   0xd4
#define BUS_I3C_MST_FREE(x)               ((x) & GENMASK(15, 0))

#define BUS_IDLE_TIMING                   0xd8
#define I3C_VER_ID                        0xe0
#define I3C_VER_TYPE                      0xe4
#define EXTENDED_CAPABILITY               0xe8
#define SLAVE_CONFIG                      0xec

#define DEV_ADDR_TABLE_LEGACY_I2C_DEV     BIT(31)
#define DEV_ADDR_TABLE_DYNAMIC_ADDR(x)    (((x) << 16) & GENMASK(23, 16))
#define DEV_ADDR_TABLE_STATIC_ADDR(x)     ((x) & GENMASK(6, 0))
#define DEV_ADDR_TABLE_LOC(start, idx)    ((start) + ((idx) << 2))

#define I3C_BUS_SDR1_SCL_RATE             8000000
#define I3C_BUS_SDR2_SCL_RATE             6000000
#define I3C_BUS_SDR3_SCL_RATE             4000000
#define I3C_BUS_SDR4_SCL_RATE             2000000
#define I3C_BUS_I2C_SS_TLOW_MIN_NS        4700
#define I3C_BUS_I2C_FM_TLOW_MIN_NS        1300
#define I3C_BUS_I2C_FMP_TLOW_MIN_NS       500
#define I3C_BUS_THIGH_MAX_NS              41

#define DEV_OPERATION_MODE_AS_SLV         (1)

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
typedef enum
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
  I3C_XFER_STATUS_ERROR_ADDR_ASSIGN   = (1UL << 12), /**< Transfer status error  Address Assign */
} I3C_XFER_STATUS;

/**
\brief I3C Device Transfer
*/
typedef struct _I3C_XFER
{
  uint32_t                  cmd_hi;  /* value to be programmed first  into C QUEUE */
  uint32_t                  cmd_lo;  /* value to be programmed second into C QUEUE */
  uint16_t                  tx_len;  /* len of data to be programmed into TX_PORT  */
  uint16_t                  rx_len;  /* len of received data                       */
  const void               *tx_buf;  /* buf address where tx data resides          */
  void                     *rx_buf;  /* pointer where rx data needs to be kept     */
  volatile I3C_XFER_STATUS  status;  /* transfer status                            */
  volatile uint8_t          error;   /* error if any for this transfer             */
}I3C_XFER;

/**
  \fn          void i3c_dma_enable(I3C_Type *i3c)
  \brief       enable i3c dma
  \param[in]   i3c   Pointer to i3c register map
  \return      none
*/
static inline void i3c_dma_enable(I3C_Type *i3c)
{
  i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL | DEV_CTRL_DMA_ENABLE;
}

/**
  \fn          void i3c_dma_disable(I3C_Type *i3c)
  \brief       disable i3c dma
  \param[in]   i3c   Pointer to i3c register map
  \return      none
*/
static inline void i3c_dma_disable(I3C_Type *i3c)
{
    i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL & (~DEV_CTRL_DMA_ENABLE);
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
                           DATA_BUFFER_THLD_CTRL_TX_BUF);

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
                            DATA_BUFFER_THLD_CTRL_RX_BUF) >> 8);

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
  i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL | DEV_CTRL_RESUME;
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
  i3c->I3C_INTR_STATUS = INTR_TRANSFER_ERR_STAT;
}

/**
  \fn           uint32_t i3c_get_dat_addr(I3C_Type *i3c)
  \brief        get start address of Device Address Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       start address of Device Address Table
*/
static inline uint32_t i3c_get_dat_addr(I3C_Type *i3c)
{
  return (i3c->I3C_DEVICE_ADDR_TABLE_POINTER & 0xFFFF);
}

/**
  \fn           uint32_t i3c_get_dat_depth(I3C_Type *i3c)
  \brief        get depth of Device Address Table
  \param[in]    i3c     : Pointer to i3c register set structure
  \return       depth of Device Address Table
*/
static inline uint32_t i3c_get_dat_depth(I3C_Type *i3c)
{
  return (i3c->I3C_DEVICE_ADDR_TABLE_POINTER >> 16);
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
                                        uint32_t  pos,
                                        uint8_t   dyn_addr,
                                        uint8_t   sta_addr)
  \brief        add i3c/i2c slave to Device Address Table
  \param[in]    i3c      : Pointer to i3c register set structure
  \param[in]    pos      : DAT position
  \param[in]    dyn_addr : Slave dynamic address, only for i3c slave
  \param[in]    sta_addr : Slave static address
  \return       none
*/
static inline void i3c_add_slv_to_dat(I3C_Type *i3c,
                                      uint32_t  pos,
                                      uint8_t   dyn_addr,
                                      uint8_t   sta_addr)
{
  uint32_t val;

  if(dyn_addr)
  {
    /* i3c slave */
    val = DEV_ADDR_TABLE_DYNAMIC_ADDR(dyn_addr) |
          DEV_ADDR_TABLE_STATIC_ADDR(sta_addr);
  }
  else
  {
    /* i2c slave */
    val = DEV_ADDR_TABLE_STATIC_ADDR(sta_addr) |
          DEV_ADDR_TABLE_LEGACY_I2C_DEV;
  }

  i3c_update_dat(i3c, pos, val);
}

/**
  \fn           void i3c_remove_slv_from_dat(I3C_Type *i3c,
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
  \fn           void i3c_send_ccc_cmd(I3C_Type *i3c,
                                      uint32_t  ccc_cmd,
                                      uint32_t  index)
  \brief        send ccc command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    pos     : DAT Slave index
  \return       none
*/
static inline void i3c_send_ccc_cmd(I3C_Type *i3c,
                                    uint32_t  ccc_cmd,
                                    uint32_t  index)
{
  /* Issue ccc command */
  i3c->I3C_COMMAND_QUEUE_PORT =
        COMMAND_PORT_DEV_INDEX(index)        |
        COMMAND_PORT_CMD(ccc_cmd)            |
        COMMAND_PORT_TOC                     |
        COMMAND_PORT_ROC                     |
        COMMAND_PORT_DEV_COUNT(1)            |
        COMMAND_PORT_ADDR_ASSGN_CMD          |
        COMMAND_PORT_TID(I3C_ADDR_ASSIGN_TID);
}

/**
  \fn           void i3c_master_tx(I3C_Type *i3c,
                                   I3C_XFER *xfer,
                                   uint32_t  index,
                                   uint16_t  len)
  \brief        send master transmit command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    len     : Transmit length
  \return       none
*/
void i3c_master_tx(I3C_Type *i3c,
                   I3C_XFER *xfer,
                   uint32_t  index,
                   uint16_t  len);

/**
  \fn           void i3c_master_rx(I3C_Type *i3c,
                                   I3C_XFER *xfer,
                                   uint32_t  index,
                                   uint16_t  len)
  \brief        send master receive command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    len     : Receive length
  \return       none
*/
void i3c_master_rx(I3C_Type *i3c,
                   I3C_XFER *xfer,
                   uint32_t  index,
                   uint16_t  len);

/**
  \fn           void i3c_slave_tx(I3C_Type *i3c,
                                  I3C_XFER *xfer,
                                  uint16_t  len)
  \brief        send slave transmit command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    len     : Transmit length
  \return       none
*/
void i3c_slave_tx(I3C_Type *i3c,
                  I3C_XFER *xfer,
                  uint16_t  len);

/**
  \fn           void i3c_slave_rx(I3C_Type *i3c,
                                  I3C_XFER *xfer)
  \brief        send slave receive command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_rx(I3C_Type *i3c,
                  I3C_XFER *xfer);

/**
  \fn           void i3c_ccc_set(I3C_Type *i3c,
                                 I3C_XFER *xfer,
                                 uint32_t  index,
                                 uint8_t   cmd_id,
                                 uint16_t  cmd_len)
  \brief        send CCC (Common Command Codes) SET command to
                 i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    cmd_id  : Command-ID \ref Driver_I3C.h
  \param[in]    cmd_len : Command length
  \return       none
*/
void i3c_ccc_set(I3C_Type *i3c,
                 I3C_XFER *xfer,
                 uint32_t  index,
                 uint8_t   cmd_id,
                 uint16_t  cmd_len);

/**
  \fn           void i3c_ccc_get(I3C_Type *i3c,
                                 I3C_XFER *xfer,
                                 uint32_t  index,
                                 uint8_t   cmd_id,
                                 uint16_t  cmd_len)
  \brief        send CCC (Common Command Codes) GET command to
                 i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    cmd_id  : Command-ID \ref Driver_I3C.h
  \param[in]    cmd_len : Command length
  \return       none
*/
void i3c_ccc_get(I3C_Type *i3c,
                 I3C_XFER *xfer,
                 uint32_t  index,
                 uint8_t   cmd_id,
                 uint16_t  cmd_len);

/**
  \fn           void i3c_clk_cfg(I3C_Type *i3c,
                                 uint32_t  core_clk)
  \brief        i3c clock configuration for i3c slave device
  \param[in]    i3c       : Pointer to i3c register set structure
  \param[in]    core_clk  : core clock
  \return       none
*/
void i3c_clk_cfg(I3C_Type *i3c,
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
                  - Free all the position from
                     DAT(Device Address Table)
                  - Clear Command Queue and Data buffer Queue
                  - Enable interrupt for
                      Response Queue Ready and
                      Transfer error status
                  - Enable i3c controller
  \param[in]    i3c  : Pointer to i3c register
                        set structure
  \return       none
*/
void i3c_master_init(I3C_Type *i3c);

/**
  \fn           void i3c_slave_init(I3C_Type *i3c,
                                    uint8_t   slv_addr)
  \brief        Initialize i3c slave.
                 This function will :
                  - set slave address
                  - set control to adaptive i2c and i3c
                  - Enable interrupt for
                     Response Queue Ready and
                     Transfer error status
                     dynamic address assignment
                  - set secondary master as slave mode
                  - Enable i3c controller
  \param[in]    i3c       : Pointer to i3c register
                             set structure
  \param[in]    slv_addr  : Slave own Address
  \return       none
*/
void i3c_slave_init(I3C_Type *i3c,
                    uint8_t   slv_addr);

/**
  \fn           void i3c_irq_handler(I3C_Type *i3c,
                                     I3C_XFER *xfer)
  \brief        i3c interrupt service routine
  \param[in]    i3c  : Pointer to i3c register
                        set structure
  \param[in]    xfer : Pointer to i3c transfer
                        structure
  \return       none
*/
void i3c_irq_handler(I3C_Type *i3c,
                     I3C_XFER *xfer);


#ifdef __cplusplus
}
#endif

#endif /* I3C_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
