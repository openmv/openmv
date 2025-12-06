/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef I2C_H_
#define I2C_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/* i2c register set */
typedef struct {
    volatile uint32_t  I2C_CON;                      /*!< (@ 0x00000000) Control Register                                           */
    volatile uint32_t  I2C_TAR;                      /*!< (@ 0x00000004) Target Address Register                                    */
    volatile uint32_t  I2C_SAR;                      /*!< (@ 0x00000008) Slave Address Register                                     */
    volatile uint32_t ic_hs_maddr;                   /* (0x0c) : I2C HS Master Mode Code address          */
    volatile uint32_t  I2C_DATA_CMD;                 /*!< (@ 0x00000010) Rx/Tx Data Buffer and Command Register                     */

    volatile uint32_t I2C_SS_SCL_HCNT;               /*!< (@ 0x00000014) Standard Speed SCL High Count Register                     */
    volatile uint32_t I2C_SS_SCL_LCNT;               /* (0x18) : Standard Speed I2C clock SCL Low Count   */
    volatile uint32_t I2C_FS_SCL_HCNT;               /*!< (@ 0x0000001C) Fast Mode or Fast Mode Plus SCL High Count Register        */
    volatile uint32_t I2C_FS_SCL_LCNT;               /*!< (@ 0x00000020) Fast Mode or Fast Mode Plus SCL Low Count Register         */
    volatile uint32_t ic_hs_scl_hcnt;                /* (0x24) : High Speed I2C clock SCL Low Count       */
    volatile uint32_t ic_hs_scl_lcnt;                /* (0x28) : High Speed I2C clock SCL Low Count       */

    volatile const uint32_t  I2C_INTR_STAT;          /*!< (@ 0x0000002C) Interrupt Status Register                                  */
    volatile       uint32_t  I2C_INTR_MASK;          /*!< (@ 0x00000030) Interrupt Mask Register                                    */
    volatile const uint32_t  I2C_RAW_INTR_STAT;      /*!< (@ 0x00000034) Raw Interrupt Status Register                              */

    volatile       uint32_t  I2C_RX_TL;              /*!< (@ 0x00000038) Receive FIFO Threshold Register                            */
    volatile       uint32_t  I2C_TX_TL;              /*!< (@ 0x0000003C) Transmit FIFO Threshold Register                           */

    volatile const   uint32_t  I2C_CLR_INTR;         /*!< (@ 0x00000040) Clear Combined and Individual Interrupt Register           */
    volatile const   uint32_t  I2C_CLR_RX_UNDER;     /*!< (@ 0x00000044) Clear RX_UNDER Interrupt Register                          */
    volatile const   uint32_t  I2C_CLR_RX_OVER;      /*!< (@ 0x00000048) Clear RX_OVER Interrupt Register                           */
    volatile const   uint32_t  I2C_CLR_TX_OVER;      /*!< (@ 0x0000004C) Clear TX_OVER Interrupt Register                           */
    volatile const   uint32_t  I2C_CLR_RD_REQ;       /*!< (@ 0x00000050) Clear RD_REQ Interrupt Register                            */
    volatile const   uint32_t  I2C_CLR_TX_ABRT;      /*!< (@ 0x00000054) Clear TX_ABRT Interrupt Register                           */
    volatile const   uint32_t  I2C_CLR_RX_DONE;      /*!< (@ 0x00000058) Clear RX_DONE Interrupt Register                           */
    volatile const   uint32_t  I2C_CLR_ACTIVITY;     /*!< (@ 0x0000005C) Clear ACTIVITY Interrupt Register                          */
    volatile const   uint32_t  I2C_CLR_STOP_DET;     /*!< (@ 0x00000060) Clear STOP_DET Interrupt Register                          */
    volatile const   uint32_t  I2C_CLR_START_DET;    /*!< (@ 0x00000064) Clear START_DET Interrupt Register                         */
    volatile const   uint32_t  I2C_CLR_GEN_CALL;     /*!< (@ 0x00000068) Clear GEN_CALL Interrupt Register                          */

    volatile       uint32_t  I2C_ENABLE;             /*!< (@ 0x0000006C) Enable Register                                            */
    volatile const uint32_t  I2C_STATUS;             /*!< (@ 0x00000070) Status Register                                            */
    volatile const uint32_t  I2C_TXFLR;              /*!< (@ 0x00000074) Transmit FIFO Level Register                               */
    volatile const uint32_t  I2C_RXFLR;              /*!< (@ 0x00000078) Receive FIFO Level Register                                */
    volatile uint32_t  I2C_SDA_HOLD;                 /*!< (@ 0x0000007C) SDA Hold Time Length Register                              */
    volatile const uint32_t  I2C_TX_ABRT_SOURCE;     /*!< (@ 0x00000080) Transmit Abort Source Register                             */
    volatile uint32_t ic_slv_data_nack_only;         /* (0x84) : Generate SLV_DATA_NACK Register          */

    volatile uint32_t  I2C_DMA_CR;                   /*!< (@ 0x00000088) DMA Control Register                                       */
    volatile uint32_t  I2C_DMA_TDLR;                 /*!< (@ 0x0000008C) DMA Transmit Data Level Register                           */
    volatile uint32_t  I2C_DMA_RDLR;                 /*!< (@ 0x00000090) DMA Transmit Data Level Register                           */

    volatile uint32_t  I2C_SDA_SETUP;                /*!< (@ 0x00000094) SDA Setup Register                                         */
    volatile uint32_t  I2C_ACK_GENERAL_CALL;         /*!< (@ 0x00000098) ACK General Call Register                                  */
    volatile const uint32_t  I2C_ENABLE_STATUS;      /*!< (@ 0x0000009C) Enable Status Register                                     */

    volatile uint32_t  I2C_FS_SPKLEN;                /*!< (@ 0x000000A0) SS, FS or FM+ Spike Suppression Limit Register             */
    volatile uint32_t reserved[2];
    volatile uint32_t  I2C_SCL_STUCK_AT_LOW_TIMEOUT; /*!< (@ 0x000000AC) SCL Stuck at Low Timeout Register                          */
    volatile uint32_t  I2C_SDA_STUCK_AT_LOW_TIMEOUT; /*!< (@ 0x000000B0) SDA Stuck at Low Timeout Register                          */
    volatile const uint32_t  I2C_CLR_SCL_STUCK_DET;  /*!< (@ 0x000000B4) Clear SCL Stuck at Low Detect Interrupt Register           */
    volatile const uint32_t  I2C_DEVICE_ID;          /*!< (@ 0x000000B8) Device-ID Register                                         */
    volatile const uint32_t RESERVED4[13];
    volatile uint32_t  I2C_REG_TIMEOUT_RST;          /*!< (@ 0x000000F0) Timeout Counter Reset Value Register                       */
    volatile const uint32_t  I2C_COMP_PARAM_1;       /*!< (@ 0x000000F4) Module Configuration Register 1                            */
    volatile const uint32_t  I2C_COMP_VERSION;       /*!< (@ 0x000000F8) Reserved                                                   */
    volatile const uint32_t  I2C_COMP_TYPE;          /*!< (@ 0x000000FC) Reserved                                                   */
} I2C_Type;

/*!< FIFO Depth for Tx & Rx  */
#define I2C_FIFO_DEPTH                              32

#define I2C_SLAVE_10BIT_ADDR_MODE                   (1 << 3)          /* 10 bit address mode for slave mode */
#define I2C_MASTER_10BIT_ADDR_MODE                  (1 << 12)         /* 10 bit address mode for master mode */

/* Enable I2C */
#define I2C_IC_ENABLE_I2C_ENABLE                    (1)

/* Disable I2C */
#define I2C_IC_ENABLE_I2C_DISABLE                   (0)

/* i2c IC_ENABLE_STATUS Bits */
#define I2C_IC_ENABLE_STATUS_IC_EN                  (1 << 0)

/* i2c Status Register Fields. */
#define I2C_IC_STATUS_ACTIVITY                      (0x01)      /* (1 << 0) */
#define I2C_IC_STATUS_TRANSMIT_FIFO_NOT_FULL        (0x02)      /* (1 << 1) */
#define I2C_IC_STATUS_TFE                           (0x04)      /* (1 << 2) */
#define I2C_IC_STATUS_RECEIVE_FIFO_NOT_EMPTY        (0x08)      /* (1 << 3) */
#define I2C_IC_STATUS_RFF                           (0x10)      /* (1 << 4) */
#define I2C_IC_STATUS_MASTER_ACT                    (0x20)      /* (1 << 5) */
#define I2C_IC_STATUS_SLAVE_ACT                     (0x40)      /* (1 << 6) */

/* Perform a write request */
#define I2C_IC_DATA_CMD_WRITE_REQ                   (0)
/* Perform a read request */
#define I2C_IC_DATA_CMD_READ_REQ                    (1 << 8)

/* Speed modes of IC_CON */
#define I2C_IC_CON_SPEED_MASK                       (0x6)
#define I2C_IC_CON_SPEED_STANDARD                   (0x2)
#define I2C_IC_CON_SPEED_FAST                       (0x4)
#define I2C_IC_CON_SPEED_HIGH                       (0x6)

/* Working mode of IC_CON */
#define I2C_IC_CON_MST_SLV_MODE_MASK                (0x41)
#define I2C_IC_CON_ENABLE_MASTER_MODE               (0x41)
#define I2C_IC_CON_ENA_SLAVE_MODE                   (0)

/* I2C interrupt control */
#define I2C_IC_INT_DISABLE_ALL                      (0x0)

/* Interrupt Register Fields */
#define I2C_IC_INTR_STAT_GEN_CALL                   (1 << 11)
#define I2C_IC_INTR_STAT_START_DET                  (1 << 10)
#define I2C_IC_INTR_STAT_STOP_DET                   (1 << 9)
#define I2C_IC_INTR_STAT_ACTIVITY                   (1 << 8)
#define I2C_IC_INTR_STAT_RX_DONE                    (1 << 7)

#define I2C_IC_INTR_STAT_TX_ABRT                    (1 << 6)    /* raw interrupt status */
#define I2C_IC_INTR_STAT_RD_REQ                     (1 << 5)
#define I2C_IC_INTR_STAT_TX_EMPTY                   (1 << 4)
#define I2C_IC_INTR_STAT_TX_OVER                    (1 << 3)    /* raw interrupt status */

#define I2C_IC_INTR_STAT_RX_FULL                    (1 << 2)
#define I2C_IC_INTR_STAT_RX_OVER                    (1 << 1)    /* raw interrupt status */
#define I2C_IC_INTR_STAT_RX_UNDER                   (1 << 0)    /* raw interrupt status */

/* Interrupt enable mask as master */
#define I2C_IC_INT_MST_TX_ENABLE                    (I2C_IC_INTR_STAT_TX_EMPTY| \
                                                     I2C_IC_INTR_STAT_TX_OVER | \
                                                     I2C_IC_INTR_STAT_TX_ABRT | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

#define I2C_IC_INT_DMA_MST_TX_ENABLE                (I2C_IC_INTR_STAT_TX_OVER  | \
                                                     I2C_IC_INTR_STAT_TX_ABRT  | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

#define I2C_IC_INT_MST_RX_ENABLE                    (I2C_IC_INTR_STAT_TX_EMPTY | \
                                                     I2C_IC_INTR_STAT_RX_FULL  | \
                                                     I2C_IC_INTR_STAT_RX_OVER  | \
                                                     I2C_IC_INTR_STAT_RX_UNDER | \
                                                     I2C_IC_INTR_STAT_TX_ABRT  | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

#define I2C_IC_INT_DMA_MST_RX_ENABLE                (I2C_IC_INTR_STAT_TX_EMPTY | \
                                                     I2C_IC_INTR_STAT_RX_OVER  | \
                                                     I2C_IC_INTR_STAT_RX_UNDER | \
                                                     I2C_IC_INTR_STAT_TX_ABRT  | \
                                                     I2C_IC_INTR_STAT_STOP_DET)
/* Interrupt enable mask as slave */
#define I2C_IC_INT_SLV_TX_ENABLE                    (I2C_IC_INTR_STAT_RD_REQ  | \
                                                     I2C_IC_INTR_STAT_TX_ABRT | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

#define I2C_IC_INT_DMA_SLV_TX_ENABLE                (I2C_IC_INTR_STAT_TX_ABRT  | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

#define I2C_IC_INT_SLV_RX_ENABLE                    (I2C_IC_INTR_STAT_RX_FULL  | \
                                                     I2C_IC_INTR_STAT_RX_OVER  | \
                                                     I2C_IC_INTR_STAT_RX_UNDER | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

#define I2C_IC_INT_DMA_SLV_RX_ENABLE                (I2C_IC_INTR_STAT_RX_OVER  | \
                                                     I2C_IC_INTR_STAT_RX_UNDER | \
                                                     I2C_IC_INTR_STAT_STOP_DET)

/* I2C_TX_ABRT_SOURCE Register Bit Fields */
#define I2C_IC_TX_ABRT_7B_ADDR_NOACK                (1 << 0)
#define I2C_IC_TX_ABRT_10ADDR1_NOACK                (1 << 1)
#define I2C_IC_TX_ABRT_10ADDR2_NOACK                (1 << 2)

#define I2C_IC_TX_ABRT_TXDATA_NOACK                 (1 << 3)

#define I2C_IC_TX_ABRT_GCALL_NOACK                  (1 << 4)
#define I2C_IC_TX_ABRT_GCALL_READ                   (1 << 5)
#define I2C_IC_TX_ABRT_HS_ACKDET                    (1 << 6)
#define I2C_IC_TX_ABRT_SBYTE_ACKDET                 (1 << 7)
#define I2C_IC_TX_ABRT_HS_NORSTRT                   (1 << 8)
#define I2C_IC_TX_ABRT_SBYTE_NORSTRT                (1 << 9)
#define I2C_IC_TX_ABRT_10B_RD_NORSTRT               (1 << 10)
#define I2C_IC_TX_ABRT_MASTER_DIS                   (1 << 11)

#define I2C_IC_TX_ABRT_ARB_LOST                     (1 << 12)
#define I2C_IC_TX_ABRT_SLVFLUSH_TXFIFO              (1 << 13)
#define I2C_IC_TX_ABRT_SLV_ARBLOST                  (1 << 14)
#define I2C_IC_TX_ABRT_SLVRD_INTX                   (1 << 15)

/* Combined bits for i2c abort source as master */
#define I2C_MST_ABRT_ADDR_NOACK                     (I2C_IC_TX_ABRT_7B_ADDR_NOACK|I2C_IC_TX_ABRT_10ADDR1_NOACK|I2C_IC_TX_ABRT_10ADDR2_NOACK)
#define I2C_MST_ABRT_LOST_BUS                       (I2C_IC_TX_ABRT_ARB_LOST)
#define I2C_MST_ABRT_DATA_NOACK                     (I2C_IC_TX_ABRT_TXDATA_NOACK)

/* Combined bits for i2c abort source as slave */
#define I2C_SLV_ABRT_LOST_BUS                       (I2C_IC_TX_ABRT_ARB_LOST|I2C_IC_TX_ABRT_SLV_ARBLOST)

/* Enabling of I2C Tx and Rx transfer through DMA */
#define I2C_DMACR_TX_DMA_ENABLE                     (1 << 1)
#define I2C_DMACR_RX_DMA_ENABLE                     (1 << 0)

/* register configuration ------------------------------------------------------------------------------------------------------------- */
#ifndef I2C_ALLOW_RESTART
#define I2C_ALLOW_RESTART                           (1)    /* allow restart configuration */
#endif

#ifndef I2C_DYNAMIC_TAR_UPDATE_SUPPORT
#define I2C_DYNAMIC_TAR_UPDATE_SUPPORT              (0)    /* Dynamic target address update support */
#endif

/* Fields of IC_CON register */
/*  I2C IP Config Dependencies. */
#if I2C_ALLOW_RESTART
#define I2C_IC_CON_MASTER_RESTART_EN                (1 << 5)
#else
#define I2C_IC_CON_MASTER_RESTART_EN                (0x00)
#endif

#define I2C_SPECIAL_START_BYTE                      0
#if I2C_SPECIAL_START_BYTE
#define I2C_IC_TAR_SPECIAL                          (1 << 11)
#define I2C_IC_TAR_GC_OR_START                      (1 << 10)
#else
#define I2C_IC_TAR_SPECIAL                          (0x00)
#define I2C_IC_TAR_GC_OR_START                      (0x00)
#endif

/* Field of IC_ENABLE_STATUS register*/
#define I2C_ENABLE_STATUS_IC_EN                     (1 << 0)

/* register configuration ---------------------------------------------------------------------------------------- */
#define I2C_IC_TAR_7BIT_ADDR_MASK                   (0x7F)    /* 7bit  I2C address mask for target address register  */
#define I2C_IC_SAR_7BIT_ADDR_MASK                   (0x7F)    /* 7bit  I2C address mask for slave  address register  */
#define I2C_IC_TAR_10BIT_ADDR_MASK                  (0x3FF)   /* 10bit I2C address mask for target address register  */
#define I2C_IC_SAR_10BIT_ADDR_MASK                  (0x3FF)   /* 10bit I2C address mask for slave  address register  */

#define I2C_FS_SPIKE_LENGTH_NS                (50)
#define I2C_HS_SPIKE_LENGTH_NS                (10)

#define I2C_MIN_SS_SCL_LCNT(spklen)     ((spklen)+7)
#define I2C_MIN_FS_SCL_LCNT(spklen)     ((spklen)+7)

#define I2C_MIN_SS_SCL_HCNT(spklen)     ((spklen)+5)
#define I2C_MIN_FS_SCL_HCNT(spklen)     ((spklen)+5)

#define I2C_MIN_SS_HIGH_TIME_NS         (4400)
#define I2C_MIN_SS_LOW_TIME_NS          (5200)

#define I2C_MIN_FS_HIGH_TIME_NS         (790)
#define I2C_MIN_FS_LOW_TIME_NS          (1600)

#define I2C_MIN_FS_PLUS_HIGH_TIME_NS    (290)
#define I2C_MIN_FS_PLUS_LOW_TIME_NS     (550)

/* Macros for write-read mode */
#define I2C_WRITE_READ_MODE_EN               0x80U
#define I2C_WRITE_READ_TAR_REG_ADDR_SIZE_Msk 0xFU
#define I2C_WRITE_READ_TAR_REG_ADDR_SIZE_Pos 0x0U
#define I2C_WRITE_READ_TAR_REG_ADDR_SIZE(x)  (x & I2C_WRITE_READ_TAR_REG_ADDR_SIZE_Msk >> I2C_WRITE_READ_TAR_REG_ADDR_SIZE_Pos)

/* I2C Bus possible speed modes */
typedef enum i2c_speed_mode
{
    I2C_SPEED_STANDARD = 1,     /* Bidirectional, Standard-mode (Sm), with a bit rate up to 100 kbit/s               */
    I2C_SPEED_FAST     = 2,     /* Bidirectional, Fast-mode (Fm), with a bit rate up to 400 kbit/s                   */
    I2C_SPEED_FASTPLUS = 3      /* Bidirectional, Fast-mode Plus (Fm+), with a bit rate up to 1 Mbit/s               */
} i2c_speed_mode_t;

/* I2C Error State */
typedef enum i2c_error_state
{
    I2C_ERR_NONE           = 0,     /* Currently in I2C device free state                                           */
    I2C_ERR_LOST_BUS       = 1,     /* Master or slave lost bus during operation                                    */
    I2C_ERR_ADDR_NOACK     = 2,     /* Slave address is sent but not addressed by any slave devices                 */
    I2C_ERR_DATA_NOACK     = 3,     /* Data in transfer is not acked when it should be acked                        */
    I2C_ERR_TIMEOUT        = 4,     /* Transfer timeout, no more data is received or sent                           */
    I2C_ERR_MSTSTOP        = 5,     /* Slave received a STOP condition from master device                           */
    I2C_ERR_UNDEF          = 6,     /* Undefined error cases                                                        */
    I2C_ERR_GCALL          = 7,     /* General call detected after slave receiver address from master               */
    I2C_ERR_10B_RD_NORSTRT = 8      /* Master in Receive mode during 10 bit addressing but comm restart is disabled */
} i2c_error_state_t;

/* I2C next Condition */
typedef enum i2c_next_condtion
{
    I2C_MODE_STOP       = 0,    /* Send a STOP condition after write/read operation     */
    I2C_MODE_RESTART    = 1     /* Send a RESTART condition after write/read operation  */
} i2c_next_condtion_t;

/* I2C Addressing Mode */
typedef enum i2c_address_mode
{
    I2C_7BIT_ADDRESS    = 0,    /* Use 7bit address mode  */
    I2C_10BIT_ADDRESS   = 1     /* Use 10bit address mode */
} i2c_address_mode_t;

/* I2C transfer state */
typedef enum _I2C_TRANSFER_STATE
{
    I2C_TRANSFER_NONE   = 0,    /* Transfer state none      */
    I2C_TRANSFER_MST_TX = 1,    /* Transfer state master tx */
    I2C_TRANSFER_MST_RX = 2,    /* Transfer state master rx */
    I2C_TRANSFER_SLV_TX = 3,    /* Transfer state slave tx  */
    I2C_TRANSFER_SLV_RX = 4     /* Transfer state slave rx  */
}I2C_TRANSFER_STATE;

/**
 * enum I2C_TRANSFER_STATUS.
 * Status of an ongoing I2C transfer.
 */
typedef enum _I2C_TRANSFER_STATUS {
    I2C_TRANSFER_STATUS_NONE              = 0,           /**< Transfer status none             */
    I2C_TRANSFER_STATUS_DONE              = (1 << 0),    /**< Transfer status done             */
    I2C_TRANSFER_STATUS_INCOMPLETE        = (1 << 1),    /**< Transfer status incomplete       */
    I2C_TRANSFER_STATUS_SLAVE_TRANSMIT    = (1 << 2),    /**< Transfer status slave transmit   */
    I2C_TRANSFER_STATUS_SLAVE_RECEIVE     = (1 << 3),    /**< Transfer status slave receieve   */
    I2C_TRANSFER_STATUS_ADDRESS_NACK      = (1 << 4),    /**< Transfer status address nack     */
    I2C_TRANSFER_STATUS_GENERAL_CALL      = (1 << 5),    /**< Transfer status general call     */
    I2C_TRANSFER_STATUS_ARBITRATION_LOST  = (1 << 6),    /**< Transfer status arbitration lost */
    I2C_TRANSFER_STATUS_BUS_ERROR         = (1 << 7),    /**< Transfer status bus error        */
    I2C_TRANSFER_STATUS_BUS_CLEAR         = (1 << 8),    /**< Transfer status bus clear        */
} I2C_TRANSFER_STATUS;

/* i2c Transfer Information (Run-Time) */
typedef struct i2c_transfer_info
{
  const uint8_t                *tx_buf;           /* Pointer to out data buffer                                              */
  uint32_t                      tx_total_num;     /* Total number of data to be send                                         */
  volatile uint32_t             tx_curr_cnt;      /* current Number of data sent from total num                              */
  uint8_t                      *rx_buf;           /* Pointer to in data buffer                                               */
  uint32_t                      rx_total_num;     /* Total number of data to be received                                     */
  volatile uint32_t             rx_curr_cnt;      /* Number of data received                                                 */
  volatile uint32_t             rx_curr_tx_index; /* current index Number which needs to send while receive.                 */
  volatile uint32_t             curr_cnt;         /* common current count update in ARM_I2C_GetDataCount function            */
  volatile uint32_t             tx_over;          /* i2c tx overflow count                                                   */
  volatile uint32_t             rx_over;          /* i2c rx overflow count                                                   */
  volatile int32_t              err_state;        /* \ref I2C_ERROR_STATE "current error state for i2c device"               */
  volatile I2C_TRANSFER_STATE   curr_stat;        /* \ref I2C_TRANSFER_STATE "current working state for i2c device"          */
  volatile uint32_t             next_cond;        /* \ref I2C_NEXT_CONDTION "next condition for master transmit or receive", \
                                                      possible values are STOP or RESTART, it should be STOP for first open  */
  volatile I2C_TRANSFER_STATUS  status;           /* \ref to I2C_TRANSFER_STATUS for data transfer state                         */
  volatile bool                 wr_mode;          /* write-read mode                                                         */
} i2c_transfer_info_t;


/**
 * @brief   Enable i2c device
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_enable(I2C_Type *i2c)
{
    i2c->I2C_ENABLE = I2C_IC_ENABLE_I2C_ENABLE;

    while(!(i2c->I2C_ENABLE_STATUS & I2C_ENABLE_STATUS_IC_EN));
}

/**
 * @brief   Disable i2c device
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_disable(I2C_Type *i2c)
{
    i2c->I2C_ENABLE = I2C_IC_ENABLE_I2C_DISABLE;

    while(i2c->I2C_ENABLE_STATUS & I2C_ENABLE_STATUS_IC_EN);
}

/**
 * @brief   Sets I2C bus speed
 * @note    none
 * @param   i2c   : Pointer to i2c register map
 * @param   speed : Bus speed
 * @retval  none
 */
static inline void i2c_set_bus_speed(I2C_Type *i2c, const uint8_t speed)
{
    i2c_disable(i2c);
    i2c->I2C_CON = ((i2c->I2C_CON & ~(I2C_IC_CON_SPEED_MASK)) | speed);
    i2c_enable(i2c);
}

/**
 * @brief   read data buffer's address
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  Address of I2C data buffer
 */
static inline volatile void* i2c_get_data_addr(I2C_Type *i2c)
{
    return ((volatile void*)&i2c->I2C_DATA_CMD);
}

/**
 * @brief   read data from RX FiFo Buffer
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  received data (8-bit)
 */
static inline uint8_t i2c_read_data_from_buffer(I2C_Type *i2c)
{
    return (i2c->I2C_DATA_CMD) & 0xFFU;
}

/**
 * @brief   enable(unmask) i2c interrupt (0-mask 1-unmask)
 * @note    none
 * @param   i2c  : Pointer to i2c register map
 * @param   mask : interrupt register bits which needs to be enable
 * @retval  none
 */
static inline void i2c_unmask_interrupt(I2C_Type *i2c, uint32_t mask)
{
    i2c->I2C_INTR_MASK |= mask;
}

/**
 * @brief   disable(mask) i2c interrupt 0-mask  1-unmask
 * @note    none
 * @param   i2c  : Pointer to i2c register map
 * @param   mask : interrupt register bits which needs to be disable
 * @retval  none
 */
static inline void i2c_mask_interrupt(I2C_Type *i2c, uint32_t mask)
{
    i2c->I2C_INTR_MASK &= ~mask;
}

/**
 * @brief   clear all combined and individual i2c interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_clear_all_interrupt(I2C_Type *i2c)
{
    /* clear all combined and individual interrupt. */
    (void)i2c->I2C_CLR_INTR;
}

/**
 * @brief   Enable master tx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_master_enable_tx_interrupt(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_MST_TX_ENABLE);
}

/**
 * @brief   Enable master rx interrupt
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_master_enable_rx_interrupt(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_MST_RX_ENABLE);
}

/**
 * @brief   Disable master tx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_master_disable_tx_interrupt(I2C_Type *i2c)
{
    i2c_mask_interrupt(i2c, I2C_IC_INT_MST_TX_ENABLE);
}

/**
 * @brief   Disable master rx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_master_disable_rx_interrupt(I2C_Type *i2c)
{
    i2c_mask_interrupt(i2c, I2C_IC_INT_MST_RX_ENABLE);
}

/**
 * @brief   Enable slave tx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_slave_enable_tx_interrupt(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_SLV_TX_ENABLE);
}

/**
 * @brief   Enable slave rx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_slave_enable_rx_interrupt(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_SLV_RX_ENABLE);
}

/**
 * @brief   Disable slave tx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_slave_disable_tx_interrupt(I2C_Type *i2c)
{
    i2c_mask_interrupt(i2c, I2C_IC_INT_SLV_TX_ENABLE);
}

/**
 * @brief   Disable slave rx interrupt
 * @note    none
 * @param   i2c : Pointer to i2c register map
 * @retval  none
 */
static inline void i2c_slave_disable_rx_interrupt(I2C_Type *i2c)
{
    i2c_mask_interrupt(i2c, I2C_IC_INT_SLV_RX_ENABLE);
}

/**
 * @brief   check whether Restart Condition is enabled in I2C Master
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  1 Restart enabled, 0 not enabled
 */
static inline bool i2c_master_check_restart_cond(I2C_Type *i2c)
{
    return ((i2c->I2C_CON & I2C_IC_CON_MASTER_RESTART_EN) != 0);
}

/**
 * @brief   Enables Restart condition for I2C Master
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  None
 */
static inline void i2c_master_enable_restart_cond(I2C_Type *i2c)
{
    i2c->I2C_CON |= I2C_IC_CON_MASTER_RESTART_EN;
}

/**
 * @brief   Sets the Tx FIFO threshold value
 * @note    none
 * @param   i2c       : Pointer to i2c register map
 * @param   threshold : Tx Fifo threshold value
 * @retval  None
 */
static inline void i2c_set_tx_threshold(I2C_Type *i2c, const uint8_t threshold)
{
    i2c->I2C_TX_TL = threshold;
}

/**
 * @brief   Sets the Rx FIFO threshold value
 * @note    none
 * @param   i2c       : Pointer to i2c register map
 * @param   threshold : Rx Fifo threshold value
 * @retval  None
 */
static inline void i2c_set_rx_threshold(I2C_Type *i2c, const uint8_t threshold)
{
    i2c->I2C_RX_TL = threshold;
}

/**
 * @brief   Enables I2C Tx DMA channel
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  None
 */
static inline void i2c_enable_tx_dma(I2C_Type *i2c)
{
    i2c->I2C_DMA_CR |= I2C_DMACR_TX_DMA_ENABLE;
}

/**
 * @brief   Disables I2C Tx DMA channel
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  None
 */
static inline void i2c_disable_tx_dma(I2C_Type *i2c)
{
    i2c->I2C_DMA_CR &= (~I2C_DMACR_TX_DMA_ENABLE);
}

/**
 * @brief   Enables I2C Rx DMA channel
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  None
 */
static inline void i2c_enable_rx_dma(I2C_Type *i2c)
{
    i2c->I2C_DMA_CR |= I2C_DMACR_RX_DMA_ENABLE;
}

/**
 * @brief   Disables I2C Rx DMA channel
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  None
 */
static inline void i2c_disable_rx_dma(I2C_Type *i2c)
{
    i2c->I2C_DMA_CR &= (~I2C_DMACR_RX_DMA_ENABLE);
}

/**
 * @brief   Returns I2C Rx DMA enable status
 * @note    none
 * @param   i2c    : Pointer to i2c register map
 * @retval  None
 */
static inline bool i2c_is_rx_dma_enable(I2C_Type *i2c)
{
    return ((i2c->I2C_DMA_CR & I2C_DMACR_RX_DMA_ENABLE) != 0);
}

/**
 * @brief       Set DMA Transmit data level
 * @param       i2c : Pointer to the I2C register map
 * @retval      none
*/
static inline void i2c_set_dma_tx_level(I2C_Type *i2c, uint8_t data_level)
{
    i2c->I2C_DMA_TDLR = data_level;
}

/**
 * @brief   Set DMA Receive data level
 * @param   i2c : Pointer to the I2C register map
 * @retval  none
*/
static inline void i2c_set_dma_rx_level(I2C_Type *i2c, uint8_t data_level)
{
    i2c->I2C_DMA_RDLR = data_level;
}

/**
 * @brief
 * @param    i2c : Pointer to the I2C register map
 * @retval   none
*/
static inline void i2c_enable_dma_master_tx(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_DMA_MST_TX_ENABLE);
}

/**
 * @brief
 * @param    i2c : Pointer to the I2C register map
 * @retval   none
*/
static inline void i2c_enable_dma_master_rx(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_DMA_MST_RX_ENABLE);
}

/**
 * @brief    Set i2c slave for DMA receive
 * @param    i2c : Pointer to the I2C register map
 * @retval   none
*/
static inline void i2c_enable_dma_slave_tx(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_DMA_SLV_TX_ENABLE);
}

/**
 * @brief    Set i2c slave for DMA receive
 * @param    i2c : Pointer to the I2C register map
 * @retval   none
*/
static inline void i2c_enable_dma_slave_rx(I2C_Type *i2c)
{
    i2c_unmask_interrupt(i2c, I2C_IC_INT_DMA_SLV_RX_ENABLE);
}

/**
 * @brief    set i2c target address for slave device in master mode
 * @param    i2c       : Pointer to i2c resources structure
 * @param    address   : i2c 7-bit or 10-bit slave address
 * @param    addr_mode : Addressing mode (10Bit/7Bit)
 * @param    cur_state : Current transfer state (Master Tx/ Master Rx)
 * @retval   none
 */
void i2c_set_target_addr(I2C_Type *i2c, const uint32_t address,
                         const i2c_address_mode_t addr_mode,
                         const I2C_TRANSFER_STATE cur_state);

/**
 * @brief   Setup i2c master clock configuration
 * @note    none
 * @param   i2c          : Pointer to i2c register map
 * @param   clk_khz      : Clock
 * @param   speed_mode   : Speed
 *          ARM_I2C_BUS_SPEED_STANDARD /
 *          I2C_IC_CON_SPEED_FAST /
 *          ARM_I2C_BUS_SPEED_FAST_PLUS
 * @retval  none
 */
void i2c_master_set_clock(I2C_Type *i2c, const uint32_t clk_khz,
                          uint8_t speed_mode);

/**
 * @brief   initialize i2c master
 * @note    none
 * @param   i2c          : Pointer to i2c register map
 * @param   tar_addr     : target address
 * @retval  none
 */
void i2c_master_init(I2C_Type *i2c, const uint32_t tar_addr);

/**
 * @brief   initialize i2c slave
 * @note    none
 * @param   i2c          : Pointer to i2c register map
 * @param   slave_addr   : i2c slave address
 * param    addr_mode    : Addressing mode (10Bit/7Bit)
 * @retval  none
 */
void i2c_slave_init(I2C_Type *i2c, uint32_t slave_addr,
                    i2c_address_mode_t addr_mode);

/**
 * @brief    i2c master transmit data using interrupt method
 * @param    i2c      : Pointer to i2c register map
 * @param    transfer : Pointer to i2c_transfer_info_t
 * @retval   callback event
 */
void i2c_master_tx_isr(I2C_Type *i2c, i2c_transfer_info_t *transfer);

/**
 * @brief    i2c master receive data using interrupt method
 * @param    i2c      : Pointer to i2c register map
 * @param    transfer : Pointer to i2c_transfer_info_t
 * @retval   callback event
 */
void i2c_master_rx_isr(I2C_Type *i2c, i2c_transfer_info_t *transfer);

/**
 * @brief    i2c slave transmit data using interrupt method
 * @param    i2c      : Pointer to i2c register map
 * @param    transfer : Pointer to i2c_transfer_info_t
 * @retval   callback event
 */
void i2c_slave_tx_isr(I2C_Type *i2c, i2c_transfer_info_t *transfer);

/**
 * @brief    i2c slave receive data using interrupt method
 * @param    i2c      : Pointer to i2c register map
 * @param    transfer : Pointer to i2c_transfer_info_t
 * @retval   callback event
 */
void i2c_slave_rx_isr(I2C_Type *i2c, i2c_transfer_info_t *transfer);

#ifdef  __cplusplus
}
#endif

#endif /* I2C_H_ */
