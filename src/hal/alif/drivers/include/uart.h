/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 \brief struct UART_Type:- Register map for UART
 */
typedef struct {                                     /*!< UART Register Structure                                                   */

  union {
    volatile const  uint32_t UART_RBR;               /*!< (@ 0x00000000) Receive Buffer Register                                    */
    volatile uint32_t UART_DLL;                      /*!< (@ 0x00000000) Divisor Latch Low Register                                 */
    volatile uint32_t UART_THR;                      /*!< (@ 0x00000000) Transmit Holding Register                                  */
  };

  union {
    volatile uint32_t UART_DLH;                      /*!< (@ 0x00000004) Divisor Latch High Register                                */
    volatile uint32_t UART_IER;                      /*!< (@ 0x00000004) Interrupt Enable Register                                  */
  };

  union {
    volatile uint32_t UART_FCR;                      /*!< (@ 0x00000008) FIFO Control Register                                      */
    volatile const  uint32_t UART_IIR;               /*!< (@ 0x00000008) Interrupt Identification Register                          */
  };

    volatile uint32_t  UART_LCR;                     /*!< (@ 0x0000000C) Line Control Register                                      */
    volatile uint32_t  UART_MCR;                     /*!< (@ 0x00000010) Modem Control Register                                     */
    volatile const  uint32_t  UART_LSR;              /*!< (@ 0x00000014) Line Status Register                                       */
    volatile const  uint32_t  UART_MSR;              /*!< (@ 0x00000018) Modem Status Register                                      */
    volatile uint32_t  UART_SCR;                     /*!< (@ 0x0000001C) Scratchpad Register                                        */
    volatile const  uint32_t  RESERVED[4];

  union {
    volatile const  uint32_t UART_SRBR[16];          /*!< (@ 0x00000030) Shadow Receive Buffer Register (n)                         */
    volatile uint32_t UART_STHR[16];                 /*!< (@ 0x00000030) Shadow Transmit Holding Register (n)                       */
  };

    volatile uint32_t  UART_FAR;                     /*!< (@ 0x00000070) FIFO Access Register                                       */
    volatile const  uint32_t  UART_TFR;              /*!< (@ 0x00000074) Tx FIFO Read Register                                      */
    volatile uint32_t  UART_RFW;                     /*!< (@ 0x00000078) Rx FIFO Write Register                                     */
    volatile const  uint32_t  UART_USR;              /*!< (@ 0x0000007C) UART Status Register                                       */
    volatile const  uint32_t  UART_TFL;              /*!< (@ 0x00000080) Tx FIFO Level Register                                     */
    volatile const  uint32_t  UART_RFL;              /*!< (@ 0x00000084) Rx FIFO Level Register                                     */
    volatile uint32_t  UART_SRR;                     /*!< (@ 0x00000088) Software Reset Register                                    */
    volatile uint32_t  UART_SRTS;                    /*!< (@ 0x0000008C) Shadow Request to Send Register                            */
    volatile uint32_t  UART_SBCR;                    /*!< (@ 0x00000090) Shadow Break Control Register                              */
    volatile uint32_t  UART_SDMAM;                   /*!< (@ 0x00000094) Shadow DMA Mode Register                                   */
    volatile uint32_t  UART_SFE;                     /*!< (@ 0x00000098) Shadow FIFO Enable Register                                */
    volatile uint32_t  UART_SRT;                     /*!< (@ 0x0000009C) Shadow RCVR Trigger Register                               */
    volatile uint32_t  UART_STET;                    /*!< (@ 0x000000A0) Shadow Tx Empty Trigger Register                           */
    volatile uint32_t  UART_HTX;                     /*!< (@ 0x000000A4) Halt Tx Register                                           */
    volatile uint32_t  UART_DMASA;                   /*!< (@ 0x000000A8) DMA Software Acknowledge Register                          */
    volatile uint32_t  UART_TCR;                     /*!< (@ 0x000000AC) Transceiver Control Register                               */
    volatile uint32_t  UART_DE_EN;                   /*!< (@ 0x000000B0) Driver Output Enable Register                              */
    volatile uint32_t  UART_RE_EN;                   /*!< (@ 0x000000B4) Receiver Output Enable Register                            */
    volatile uint32_t  UART_DET;                     /*!< (@ 0x000000B8) Driver Output Enable Timing Register                       */
    volatile uint32_t  UART_TAT;                     /*!< (@ 0x000000BC) Turnaround Timing Register                                 */
    volatile uint32_t  UART_DLF;                     /*!< (@ 0x000000C0) Divisor Latch Fraction Register                            */
    volatile uint32_t  UART_RAR;                     /*!< (@ 0x000000C4) Receive Address Register                                   */
    volatile uint32_t  UART_TAR;                     /*!< (@ 0x000000C8) Transmit Address Register                                  */
    volatile uint32_t  UART_LCR_EXT;                 /*!< (@ 0x000000CC) Line Extended Control Register                             */
    volatile const  uint32_t  RESERVED1;
    volatile uint32_t  UART_REG_TIMEOUT_RST;         /*!< (@ 0x000000D4) Timeout Counter Reset Value Register                       */
    volatile const  uint32_t  RESERVED2[7];
    volatile const  uint32_t  UART_CPR;              /*!< (@ 0x000000F4) Module Configuration Register                              */
    volatile const  uint32_t  UART_UCV;              /*!< (@ 0x000000F8) Reserved                                                   */
    volatile const  uint32_t  UART_CTR;              /*!< (@ 0x000000FC) Reserved                                                   */
} UART_Type;                                         /*!< Size = 256 (0x100)                                                        */


/* UART register bit definitions --------------------------- */

/* IER: interrupt enable register */
#define UART_IER_ENABLE_RECEIVED_DATA_AVAILABLE         (0x01)
#define UART_IER_ENABLE_TRANSMIT_HOLD_REG_EMPTY         (0x02)
#define UART_IER_ENABLE_RECEIVER_LINE_STATUS            (0x04)
#define UART_IER_ENABLE_MODEM_STATUS                    (0x08)
#define UART_IER_PTIME                                  (0x80)

/* IIR: interrupt identity register */
#define UART_IIR_INTERRUPT_PENDING                      (0x01)
#define UART_IIR_MASK                                   (0x0E)
#define UART_IIR_FIFO_ENABLE_STATUS                     (0xC0)

/* interrupt IIR_MASK values */
#define UART_IIR_MODEM_STATUS                           (0x00)
#define UART_IIR_TRANSMIT_HOLDING_REG_EMPTY             (0x02)
#define UART_IIR_RECEIVED_DATA_AVAILABLE                (0x04)
#define UART_IIR_RECEIVER_LINE_STATUS                   (0x06)
#define UART_IIR_CHARACTER_TIMEOUT                      (0x0C)
#define UART_IIR_INTERRUPT_ID_MASK                      (0x0f)

/* FCR: FIFO control register */
#define UART_FCR_FIFO_ENABLE                            (0x01)
#define UART_FCR_RCVR_FIFO_RESET                        (0x02)
#define UART_FCR_TRANSMIT_FIFO_RESET                    (0x04)
#define UART_FCR_DMAM_MODE1                             (0x08)
#define UART_FCR_RCVR_TRIGGER                           (0xC0)

/* LCR: line control register */
#define UART_LCR_DATA_LENGTH_MASK                       (0x03)
#define UART_LCR_STOP_BIT_MASK                          (0x04)
#define UART_LCR_PARITY_MASK                            (0x38)
#define UART_LCR_DATA_PARITY_STOP_MASK                  (0x3F)
#define UART_LCR_STICK_PARITY                           (0x20)
#define UART_LCR_BREAK                                  (0x40)
#define UART_LCR_DLAB                                   (0x80)

/* data length values */
#define UART_LCR_DATA_LENGTH_5                          (0x00)
#define UART_LCR_DATA_LENGTH_6                          (0x01)
#define UART_LCR_DATA_LENGTH_7                          (0x02)
#define UART_LCR_DATA_LENGTH_8                          (0x03)

/* stop bit values */
#define UART_LCR_STOP_1BIT                              (0x00)
#define UART_LCR_STOP_1_5BIT                            (0x04)
#define UART_LCR_STOP_2BIT                              (0x04)

/* Parity bit values */
#define UART_LCR_PARITY_NONE                            (0x00)
#define UART_LCR_PARITY_ODD                             (0x08)
#define UART_LCR_PARITY_EVEN                            (0x18)
#define UART_LCR_PARITY_STICK_LOGIC1                    (0x28)
#define UART_LCR_PARITY_STICK_LOGIC0                    (0x38)

/* MCR: modem control register */
#define UART_MCR_DTR                                    (0x01)
#define UART_MCR_RTS                                    (0x02)
#define UART_MCR_LOOPBACK                               (0x10)
#define UART_MCR_AFCE                                   (0x20)
#define UART_MCR_SIRE                                   (0x40)

/* LSR: line status register */
#define UART_LSR_RCVR_DATA_READY                        (0x01)
#define UART_LSR_OVERRUN_ERR                            (0x02)
#define UART_LSR_PARITY_ERR                             (0x04)
#define UART_LSR_FRAME_ERR                              (0x08)
#define UART_LSR_BREAK_INTERRUPT                        (0x10)
#define UART_LSR_TRANSMIT_HOLDING_REG_EMPTY             (0x20)
#define UART_LSR_TRANSMITTER_EMPTY                      (0x40)
#define UART_LSR_RECEIVER_FIFO_ERR                      (0x80)

/* MSR: modem status register */
#define UART_MSR_DCTS                                   (0x01)
#define UART_MSR_DDSR                                   (0x02)
#define UART_MSR_TERI                                   (0x04)
#define UART_MSR_DDCD                                   (0x08)
#define UART_MSR_CTS                                    (0x10)
#define UART_MSR_DSR                                    (0x20)
#define UART_MSR_RI                                     (0x40)
#define UART_MSR_DCD                                    (0x80)

/* USR: uart status register */
#define UART_USR_TRANSMIT_FIFO_NOT_FULL                 (0x02)
#define UART_USR_TRANSMIT_FIFO_EMPTY                    (0x04)
#define UART_USR_RECEIVE_FIFO_NOT_EMPTY                 (0x08)
#define UART_USR_RECEIVE_FIFO_FULL                      (0x10)

/* SFE: shadow FIFO enable register */
#define UART_SFE_SHADOW_FIFO_ENABLE                     (0x01)

/* SRR: software reset register */
#define UART_SRR_UART_RESET                             (0x01)
#define UART_SRR_RCVR_FIFO_RESET                        (0x02)
#define UART_SRR_TRANSMIT_FIFO_RESET                    (0x04)

/* SRT: shadow receiver trigger register */
#define UART_SRT_TRIGGER_1_CHAR_IN_FIFO                 (0x00)
#define UART_SRT_TRIGGER_FIFO_1_BY_4_FULL               (0x01)
#define UART_SRT_TRIGGER_FIFO_1_BY_2_FULL               (0x02)
#define UART_SRT_TRIGGER_FIFO_2_LESS_THAN_FULL          (0x03)

/* STET: Shadow TX empty register */
#define UART_STET_FIFO_EMPTY                            (0x00)
#define UART_STET_2_CHARS_IN_FIFO                       (0x01)
#define UART_STET_1_BY_4_FULL                           (0x02)
#define UART_STET_1_BY_2_FULL                           (0x03)

/* CPR: component parameter register */
#define UART_CPR_FIFO_STAT                              (1 << 10)
#define UART_CPR_FIFO_MODE_OFFSET                       (16)
#define UART_CPR_FIFO_MODE_MASK                         (0xFF)
#define UART_CPR_FIFO_MODE                              (0xFF0000)
#define UART_CPR_SHADOW_MODE                            (1 << 11)

/* DLF: divisor latch fraction register */
#define UART_DLF_SIZE                                   (0x04)

/* UART FIFO depth for Tx & Rx */
#define UART_FIFO_DEPTH                                 (32)

/* defines for uart baudrates */
#define UART_BAUDRATE_9600                              (9600)      /* uart baudrate 9600bps   */
#define UART_BAUDRATE_115200                            (115200)    /* uart baudrate 115200bps */
#define UART_BAUDRATE_230400                            (230400)    /* uart baudrate 230400bps */
#define UART_BAUDRATE_460800                            (460800)    /* uart baudrate 460800bps */
#define UART_BAUDRATE_921600                            (921600)    /* uart baudrate 921600bps */

/* RS485 control registers.------------------------------ ---*/

/* RS485 Mode Control.               */
#define UART_RS485_MODE_DISABLE                         (0x00)
#define UART_RS485_MODE_ENABLE                          (0x01)

/* TCR: transceiver control register */
#define UART_TCR_RS485_DISABLE                          (0x00)
#define UART_TCR_RS485_ENABLE                           (0x01)

/* RE_POL: Receiver Enable Polarity  */
#define UART_TCR_RE_POL_ACTIVE_LOW                      (0x00)
#define UART_TCR_RE_POL_ACTIVE_HIGH                     (0x02)

/* DE_POL: Driver Enable Polarity    */
#define UART_TCR_DE_POL_ACTIVE_LOW                      (0x00)
#define UART_TCR_DE_POL_ACTIVE_HIGH                     (0x04)

/* Transfer Modes */
#define UART_TCR_XFER_MODE_MASK                         (0x18)
#define UART_TCR_XFER_MODE_FULL_DUPLEX                  (0x00)
#define UART_TCR_XFER_MODE_SW_CONTROL_HALF_DUPLEX       (0x08)
#define UART_TCR_XFER_MODE_HW_CONTROL_HALF_DUPLEX       (0x10)

/* DE_EN: driver output enable register   */
#define UART_DE_EN_DISABLE                              (0x00)   /* de-assert de signal */
#define UART_DE_EN_ENABLE                               (0x01)   /* assert    de signal */

/* RE_EN: receiver output enable register */
#define UART_RE_EN_DISABLE                              (0x00)   /* de-assert re signal */
#define UART_RE_EN_ENABLE                               (0x01)   /* assert    re signal */

/* DET: driver output enable timing register */
#define UART_DET_TIME_MASK                              (0xFF)   /* 8 bits allocated for assertion and de-assertion time. */
#define UART_DET_DE_DEASSERTION_TIME_BIT_SHIFT          (16)     /* bit-shift for DE de-assertion time.                   */

/* TAT: turn-around timing register */
#define UART_TAT_TIME_MASK                              (0xFFFF) /* 16 bits allocated for RE to DE and DE to RE time.     */
#define UART_TAT_RE_TO_DE_TIME_BIT_SHIFT                (16)     /* bit-shift for RE to DE time.                          */


/**
 * UART Control Codes: Mode Parameters: Data Bits Types Enum
 */
typedef enum
{
    UART_DATA_BITS_5  = 0,      /* 5 Data bits */
    UART_DATA_BITS_6  = 1,      /* 6 Data bits */
    UART_DATA_BITS_7  = 2,      /* 7 Data bits */
    UART_DATA_BITS_8  = 3       /* 8 Data bits */
} UART_DATA_BITS;

/**
 * UART Control Codes: Mode Parameters: Parity Types Enum
 */
typedef enum
{
    UART_PARITY_NONE  = 0,      /* No Parity   */
    UART_PARITY_EVEN  = 1,      /* Even Parity */
    UART_PARITY_ODD   = 2       /* Odd Parity  */
} UART_PARITY;

/**
 * UART Control Codes: Mode Parameters: Stop Bits Types Enum
 */
typedef enum
{
    UART_STOP_BITS_1  = 0,      /* 1 Stop bit  */
    UART_STOP_BITS_2  = 1       /* 2 Stop bits */
} UART_STOP_BITS;

/**
 * UART Control Codes: Mode Parameters: Flow Control Types Enum
 */
typedef enum
{
    UART_FLOW_CONTROL_NONE    = 0,      /* No Flow Control  */
    UART_FLOW_CONTROL_RTS     = 1,      /* RTS Flow Control */
    UART_FLOW_CONTROL_CTS     = 2,      /* CTS Flow Control */
    UART_FLOW_CONTROL_RTS_CTS = 3       /* RTS/CTS Flow Control */
} UART_FLOW_CONTROL;

/**
 * UART Device TX Empty Trigger (TET) Types Enum
 * This is used to select the empty threshold level
 * in the transmitter FIFO at which the THRE
 * Interrupts will be generated.
 */
typedef enum
{
    UART_TX_FIFO_EMPTY        = 0,      /* FIFO Empty            */
    UART_TX_FIFO_CHAR_2       = 1,      /* 2 characters in FIFO  */
    UART_TX_FIFO_QUARTER_FULL = 2,      /* FIFO 1/4 full         */
    UART_TX_FIFO_HALF_FULL    = 3       /* FIFO 1/2 full         */
} UART_TX_TRIGGER;

/**
 * UART Device RX Trigger (RT) Types Enum
 * This is used to select the trigger level
 * in the receiver FIFO at which the Received Data Available
 * Interrupt will be generated.
 */
typedef enum
{
    UART_RX_ONE_CHAR_IN_FIFO   = 0,     /* 1 character in FIFO   */
    UART_RX_FIFO_QUARTER_FULL  = 1,     /* FIFO 1/4 Full         */
    UART_RX_FIFO_HALF_FULL     = 2,     /* FIFO 1/2 full         */
    UART_RX_FIFO_TWO_LESS_FULL = 3      /* FIFO 2 less than full */
} UART_RX_TRIGGER;

/* UART RS485 transfer modes */
typedef enum
{
    UART_RS485_FULL_DULPLX_MODE            = 0,    /* RS485 full duplex mode                    */
    UART_RS485_SW_CONTROL_HALF_DULPLX_MODE = 1,    /* RS485 software control half duplex mode   */
    UART_RS485_HW_CONTROL_HALF_DULPLX_MODE = 2,    /* RS485 hardware control half duplex mode   */
} UART_RS485_TRANSFER_MODE;

/**
 * enum UART_TRANSFER_STATUS.
 * Status of an ongoing UART transfer.
 */
typedef enum
{
    UART_TRANSFER_STATUS_NONE              = 0,           /**< Transfer status none                           */
    UART_TRANSFER_STATUS_SEND_COMPLETE     = (1UL << 0),  /**< Transfer status Send completed                 */
    UART_TRANSFER_STATUS_RECEIVE_COMPLETE  = (1UL << 1),  /**< Transfer status Receive completed              */
    UART_TRANSFER_STATUS_RX_TIMEOUT        = (1UL << 2),  /**< Transfer status Receive character timeout      */
    UART_TRANSFER_STATUS_ERROR             = (1UL << 3),  /**< Transfer status Error                          */
    UART_TRANSFER_STATUS_ERROR_RX_OVERRUN  = (1UL << 4),  /**< Transfer status Error: Receive Overrun error   */
    UART_TRANSFER_STATUS_ERROR_RX_PARITY   = (1UL << 5),  /**< Transfer status Error: Receive Parity  error   */
    UART_TRANSFER_STATUS_ERROR_RX_FRAMING  = (1UL << 6),  /**< Transfer status Error: Receive Framing error   */
    UART_TRANSFER_STATUS_ERROR_RX_BREAK    = (1UL << 7),  /**< Transfer status Error: Receive Break Interrupt */
} UART_TRANSFER_STATUS;

/* UART Transfer Information (Run-Time) */
typedef struct _UART_TRANSFER
{
    const uint8_t                    *tx_buf;                   /* Pointer to out data buffer                   */
    uint32_t                          tx_total_num;             /* Total number of data to be send              */
    volatile uint32_t                 tx_curr_cnt;              /* Current number of data sent from total num   */
    uint8_t                          *rx_buf;                   /* Pointer to in data buffer                    */
    uint32_t                          rx_total_num;             /* Total number of data to be received          */
    volatile uint32_t                 rx_curr_cnt;              /* Number of data received                      */
    volatile UART_TRANSFER_STATUS     status;                   /* transfer status                              */
} UART_TRANSFER;


/**
 * @fn      void uart_software_reset (UART_Type *uart)
 * @brief   uart software reset
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_software_reset (UART_Type *uart)
{
    /* use shadow register srr software reset register */
    uart->UART_SRR = UART_SRR_UART_RESET|UART_SRR_RCVR_FIFO_RESET|UART_SRR_TRANSMIT_FIFO_RESET;
}

/**
  \fn          void* uart_get_dma_tx_addr(UART_Type *uart)
  \brief       Return the DMA Tx Address(UART THR)
  \param[in]   uart   Pointer to UART register map
  \return      \ref  Return the address
*/
static inline void* uart_get_dma_tx_addr(UART_Type *uart)
{
    return ((void *)&(uart->UART_THR));
}

/**
  \fn          void* uart_get_dma_rx_addr(UART_Type *uart)
  \brief       Return the DMA Rx Address(UART RBR)
  \param[in]   uart   Pointer to UART register map
  \return      \ref  Return the address
*/
static inline void* uart_get_dma_rx_addr(UART_Type *uart)
{
    return ((void *)&(uart->UART_RBR));
}

/**
 * @fn      int32_t uart_set_tx_trigger (UART_Type       *uart,
                                         UART_TX_TRIGGER  tx_trigger)
 * @brief   set uart transmitter trigger level
 *          This is used to select the empty threshold level in the transmitter FIFO
 *          at which the THRE Interrupts will be generated.
 * @note    \ref UART_TX_TRIGGER enum
 * @param   uart        : Pointer to uart register set structure
 * @param   tx_trigger  : enum UART_TX_TRIGGER
 * @retval  none
 */
static inline void uart_set_tx_trigger (UART_Type       *uart,
                                        UART_TX_TRIGGER  tx_trigger)
{
    /* update stet shadow TX empty trigger register */
    uart->UART_STET = tx_trigger;
}

/**
 * @fn      uint8_t uart_get_decoded_tx_trigger (UART_Type *uart)
 * @brief   get decoded uart transmitter trigger level value
 * @param   uart        : Pointer to uart register set structure
 * @retval  decoded uart transmitter trigger level value
 */
static inline uint8_t uart_get_decoded_tx_trigger (UART_Type *uart)
{
    UART_TX_TRIGGER tx_trigger = 0;
    uint8_t decoded_tx_trigger = 0;

    /* get TX trigger from stet shadow TX empty trigger register */
    tx_trigger = uart->UART_STET;

    switch(tx_trigger)
    {
    case UART_TX_FIFO_EMPTY:
        decoded_tx_trigger = 0;
        break;

    case UART_TX_FIFO_CHAR_2:
        decoded_tx_trigger = 2;
        break;

    case UART_TX_FIFO_QUARTER_FULL:
        decoded_tx_trigger = UART_FIFO_DEPTH/4;
        break;

    case UART_TX_FIFO_HALF_FULL:
        decoded_tx_trigger = UART_FIFO_DEPTH/2;
        break;
    }

    return decoded_tx_trigger;
}

/**
 * @fn      int32_t uart_set_rx_trigger (UART_Type       *uart,
                                         UART_RX_TRIGGER  rx_trigger)
 * @brief   set uart receiver trigger level
 *          This is used to select the trigger level in the receiver FIFO
 *          at which the Received Data Available Interrupt will be generated.
 * @note    \ref UART_RX_TRIGGER enum
 * @param   uart        : Pointer to uart register set structure
 * @param   rx_trigger  : enum UART_RX_TRIGGER
 * @retval  none
 */
static inline void uart_set_rx_trigger (UART_Type       *uart,
                                        UART_RX_TRIGGER  rx_trigger)
{
    /* update srt shadow receiver trigger register */
    uart->UART_SRT = rx_trigger;
}

/**
 * @fn      uint8_t uart_get_decoded_rx_trigger (UART_Type *uart)
 * @brief   get decoded uart receiver trigger level value
 * @param   uart        : Pointer to uart register set structure
 * @retval  decoded uart receiver trigger level value
 */
static inline uint8_t uart_get_decoded_rx_trigger (UART_Type *uart)
{
    UART_RX_TRIGGER rx_trigger = 0;
    uint8_t decoded_rx_trigger = 0;

    /* update srt shadow receiver trigger register */
    rx_trigger = uart->UART_SRT;

    switch(rx_trigger)
    {
    case UART_RX_ONE_CHAR_IN_FIFO:
        decoded_rx_trigger = 1;
        break;

    case UART_RX_FIFO_QUARTER_FULL:
        decoded_rx_trigger = UART_FIFO_DEPTH/4;
        break;

    case UART_RX_FIFO_HALF_FULL:
        decoded_rx_trigger = UART_FIFO_DEPTH/2;
        break;

    case UART_RX_FIFO_TWO_LESS_FULL:
        decoded_rx_trigger = UART_FIFO_DEPTH - 2;
        break;
    }

    return decoded_rx_trigger;
}

/**
 * @fn      void uart_set_break_control (UART_Type *uart)
 * @brief   set uart break control
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_set_break_control (UART_Type *uart)
{
    /* set break_control bit in lcr line control register. */
    uart->UART_LCR |= UART_LCR_BREAK;
}

/**
 * @fn      void uart_clear_break_control (UART_Type *uart)
 * @brief   clear uart break control
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_clear_break_control (UART_Type *uart)
{
    /* clear break_control bit in lcr line control register. */
    uart->UART_LCR &= ~UART_LCR_BREAK;
}

/**
 * @fn      void uart_enable_fifo (UART_Type *uart)
 * @brief   enable uart fifo
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_enable_fifo (UART_Type *uart)
{
    /* enable uart fifo fcr FIFO control register */
    uart->UART_FCR = UART_FCR_FIFO_ENABLE;
}

/**
 * @fn      void uart_select_dma_mode1 (UART_Type *uart)
 * @brief   select uart dma mode1
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_select_dma_mode1 (UART_Type *uart)
{
    uart->UART_FCR |= UART_FCR_DMAM_MODE1;
}

/**
 * @fn      void uart_reset_txfifo (UART_Type *uart)
 * @brief   reset transmit fifo
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_reset_txfifo (UART_Type *uart)
{
    /* set XMIT_FIFO_Reset bit in shadow register
     * srr software reset register */
    uart->UART_SRR = UART_SRR_TRANSMIT_FIFO_RESET;
}

/**
 * @fn      void uart_reset_rxfifo (UART_Type *uart)
 * @brief   reset receiver fifo
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_reset_rxfifo (UART_Type *uart)
{
    /* set RCVR_FIFO_Reset bit in shadow register
     * srr software reset register */
    uart->UART_SRR = UART_SRR_RCVR_FIFO_RESET;
}

/**
 * @fn      void uart_enable_tx_irq (UART_Type *uart)
 * @brief   enable transmitter interrupt
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_enable_tx_irq (UART_Type *uart)
{
    /* enable transmit_holding_register_empty bit in
     * ier interrupt enable register */
    uart->UART_IER |= UART_IER_ENABLE_TRANSMIT_HOLD_REG_EMPTY;
}

/**
 * @fn      void uart_disable_tx_irq (UART_Type *uart)
 * @brief   disable transmit interrupt
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_disable_tx_irq (UART_Type *uart)
{
    /* disable transmit_holding_register_empty bit in
     * ier interrupt enable register */
    uart->UART_IER &= ~UART_IER_ENABLE_TRANSMIT_HOLD_REG_EMPTY;
}

/**
 * @fn      void uart_enable_rx_irq (UART_Type *uart)
 * @brief   enable receiver interrupt
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_enable_rx_irq (UART_Type *uart)
{
    /* enable receiver interrupt */
    /* enable receive_data_available_interrupt bit in
     * ier interrupt enable register */
    uart->UART_IER |= UART_IER_ENABLE_RECEIVED_DATA_AVAILABLE;

    /* also enable receiver line status interrupt. */
    uart->UART_IER |= UART_IER_ENABLE_RECEIVER_LINE_STATUS;
}

/**
 * @fn      void uart_disable_rx_irq (UART_Type *uart)
 * @brief   disable receiver interrupt
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_disable_rx_irq (UART_Type *uart)
{
    /* disable receiver interrupt */
    /* disable receive_data_available_interrupt bit in
     * ier interrupt enable register */
    uart->UART_IER &= ~UART_IER_ENABLE_RECEIVED_DATA_AVAILABLE;

    /* also disable receiver line status interrupt. */
    uart->UART_IER &= ~UART_IER_ENABLE_RECEIVER_LINE_STATUS;
}

/* --------------------------------- RS485 Functions ---------------------------*/

/*  @Note:
 *    As per PINMUX,
 *      UART0-UART3 instances has RS232 with RTS/CTS functionality and
 *      UART4-UART7 instances has RS232 without RTS/CTS and RS485 functionality.
 */

/* @Note: Observations with RS485 Transfer Modes:
 * 1.) RS485_HW_CONTROL_HALF_DULPLX_MODE: Able to send and receive data properly.
 *      -Both DE and RE signal can be enable at a time,
 *         as Hardware will take care of switching between Drive Enable and
 *         Receive Enable signal.
 * 2.) RS485_SW_CONTROL_HALF_DULPLX_MODE: Able to send and receive data properly
 *     with below conditions.
 *      -Drive Enable and Receive Enable signal are mutually exclusive.
 *      -Both DE and RE signal can not be enable at a time.
 *      -Proper Tuning between TX and RX is required with proper delay between TX-RX.
 *      -@Note: Currently Driver APIs are not exposed for SW Control Half Duplex mode.
 * 3.) RS485_FULL_DULPLX_MODE: Not tested.
 */

/**
 * @fn      void uart_rs485_enable (UART_Type *uart)
 * @brief   enable uart RS485 mode with default configuration as per RTE_device.h
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_rs485_enable (UART_Type *uart)
{
    /* reset TCR transceiver control register. */
    uart->UART_TCR = 0;

    /* enable RS485 mode in TCR transceiver control register. */
    uart->UART_TCR |= UART_TCR_RS485_ENABLE;

    /* de polarity: active high, re polarity: active low/high. */
    uart->UART_TCR |= UART_TCR_DE_POL_ACTIVE_HIGH;
    uart->UART_TCR |= UART_TCR_RE_POL_ACTIVE_HIGH;
}

/**
 * @fn      void uart_rs485_disable (UART_Type *uart)
 * @brief   disable uart RS485 mode
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_rs485_disable (UART_Type *uart)
{
    /* disable RS485 mode in TCR transceiver control register. */
    uart->UART_TCR &= ~UART_TCR_RS485_ENABLE;
}

/**
 * @fn      void uart_rs485_set_transfer_mode (UART_Type *uart, UART_RS485_TRANSFER_MODE mode)
 * @brief   set RS485 transfer modes \ref UART_RS485_TRANSFER_MODE
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @param   mode: Available RS485 transfer modes
 *              - UART_RS485_FULL_DULPLX_MODE            : Not tested as Hardware is not supporting Full duplex mode.
 *              - UART_RS485_SW_CONTROL_HALF_DULPLX_MODE : Currently Driver APIs are not exposed for SW Control Half Duplex mode.
 *              - UART_RS485_HW_CONTROL_HALF_DULPLX_MODE : Tested and Verified, Able to send and receive data properly.
 * @retval  none
 */
static inline void uart_rs485_set_transfer_mode (UART_Type *uart, UART_RS485_TRANSFER_MODE mode)
{
    /* clear Transfer modes bits[4:3] */
    uart->UART_TCR &= (~UART_TCR_XFER_MODE_MASK);

    if(mode == UART_RS485_FULL_DULPLX_MODE)            uart->UART_TCR |= UART_TCR_XFER_MODE_FULL_DUPLEX;
    if(mode == UART_RS485_SW_CONTROL_HALF_DULPLX_MODE) uart->UART_TCR |= UART_TCR_XFER_MODE_SW_CONTROL_HALF_DUPLEX;
    if(mode == UART_RS485_HW_CONTROL_HALF_DULPLX_MODE) uart->UART_TCR |= UART_TCR_XFER_MODE_HW_CONTROL_HALF_DUPLEX;
}

/**
 * @fn      UART_RS485_TRANSFER_MODE uart_rs485_get_transfer_mode (UART_Type *uart)
 * @brief   get selected RS485 transfer mode \ref UART_RS485_TRANSFER_MODE
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @retval  selected RS485 transfer mode
 */
static inline UART_RS485_TRANSFER_MODE uart_rs485_get_transfer_mode (UART_Type *uart)
{
    return ((uart->UART_TCR & UART_TCR_XFER_MODE_MASK) >> 3);
}

/**
 * @fn      void uart_rs485_set_de_assertion_time (UART_Type *uart, uint32_t assertion_time)
 * @brief   set RS485 DE Assertion time for
 *          DET driver output enable timing register
 * @note    DE Assertion time: 8 bit only, DET register bits (7:0)
 * @param   uart            : Pointer to uart register set structure
 * @param   assertion_time  : 8-bit DE Assertion time
 * @retval  none
 */
static inline void uart_rs485_set_de_assertion_time (UART_Type *uart, uint32_t assertion_time)
{
    /* DE Assertion time: 8 bit only. */

    /* clear DE Assertion time: bits (7:0). */
    uart->UART_DET &= (~UART_DET_TIME_MASK);

    /* DE Assertion time: bits (7:0). */
    uart->UART_DET |= (assertion_time & UART_DET_TIME_MASK);
}

/**
 * @fn      void uart_rs485_set_de_deassertion_time (UART_Type *uart, uint32_t deassertion_time)
 * @brief   set RS485 DE De-Assertion time for
 *          DET driver output enable timing register
 * @note    DE Assertion time: 8 bit only, DET register bits (23:16)
 * @param   uart                : Pointer to uart register set structure
 * @param   deassertion_time    : 8-bit DE De-Assertion time
 * @retval  none
 */
static inline void  uart_rs485_set_de_deassertion_time (UART_Type *uart, uint32_t deassertion_time)
{
    /* DE De-Assertion time: 8 bit only. */

    /* clear DE De-Assertion time: bits (23:16). */
    uart->UART_DET &= ( ~ (UART_DET_TIME_MASK << UART_DET_DE_DEASSERTION_TIME_BIT_SHIFT) );

    /* DE De-Assertion time: bits (23:16). */
    uart->UART_DET |= ( (deassertion_time & UART_DET_TIME_MASK) << UART_DET_DE_DEASSERTION_TIME_BIT_SHIFT );
}

/**
 * @fn      void uart_rs485_set_de_to_re_turn_around_time (UART_Type *uart, uint32_t de_to_re_time)
 * @brief   set RS485 Driver Enable DE to Receive Enable RE Turn Around time for
 *          TAT turn-around timing register
 * @note    TAT DE to RE Turn Around time: 16 bit , TAT register bits (15:0)
 * @param   uart                : Pointer to uart register set structure
 * @param   de_to_re_time       : 16-bit DE to RE time
 * @retval  none
 */
static inline void uart_rs485_set_de_to_re_turn_around_time (UART_Type *uart, uint32_t de_to_re_time)
{
    /* Driver Enable DE to Receive Enable RE Turn Around time: 16 bit . */

    /* Clear TAT DE to RE Turn Around time bits (15:0). */
    uart->UART_TAT &= (~UART_TAT_TIME_MASK);

    /* TAT DE to RE Turn Around time bits (15:0). */
    uart->UART_TAT |= (de_to_re_time & UART_TAT_TIME_MASK);
}

/**
 * @fn      void uart_rs485_set_re_to_de_turn_around_time (UART_Type *uart, uint32_t re_to_de_time)
 * @brief   set RS485 Receive Enable RE to Driver Enable DE Turn Around time for
 *          TAT turn-around timing register
 * @note    TAT RE to DE Turn Around time: 16 bit , TAT register bits (31:16)
 * @param   uart            : Pointer to uart register set structure
 * @param   re_to_de_time   : 16-bit RE to DE time
 * @retval  none
 */
static inline void uart_rs485_set_re_to_de_turn_around_time (UART_Type *uart, uint32_t re_to_de_time)
{
    /* Receive Enable RE to Driver Enable DE Turn Around time: 16 bit . */

    /* Clear TAT RE to DE Turn Around time bits (31:16). */
    uart->UART_TAT &= ( ~ (UART_TAT_TIME_MASK << UART_TAT_RE_TO_DE_TIME_BIT_SHIFT) );

    /* TAT RE to DE Turn Around time bits (31:16). */
    uart->UART_TAT |= ( (re_to_de_time & UART_TAT_TIME_MASK) << UART_TAT_RE_TO_DE_TIME_BIT_SHIFT );
}

/**
 * @fn      void uart_rs485_enable_de_en (UART_Type *uart)
 * @brief   Enable RS485 DE Driver Enable Signal State
 * @note    none
 * @param   uart : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_rs485_enable_de_en (UART_Type *uart)
{
    uint32_t mode;

    /* Special check only for Software control Half-Duplex Mode
     * as DE and RE are mutually exclusive here. */

    /* Check rs485 transfer mode. */
    mode = uart_rs485_get_transfer_mode(uart);

    if(mode == UART_RS485_SW_CONTROL_HALF_DULPLX_MODE)
    {
        /* In Software control Half-Duplex Mode DE and RE are mutually exclusive.
         * so anyone either DE or RE can be enable at a time.
         */

        /* in S/W Half-Duplex Mode first disable RE signal before enabling DE signal. */
        /* disable RE. */
        uart->UART_RE_EN = UART_RE_EN_DISABLE;
    }

    /* enable DE Driver Enable signal. */
    uart->UART_DE_EN = UART_DE_EN_ENABLE;
}

/**
 * @fn      void uart_rs485_disable_de_en (UART_Type *uart)
 * @brief   Disable RS485 DE Driver Enable Signal State
 * @note    none
 * @param   uart : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_rs485_disable_de_en (UART_Type *uart)
{
    /* Disable the DE Driver Enable signal. */
    uart->UART_DE_EN = UART_DE_EN_DISABLE;
}

/**
 * @fn      void uart_rs485_enable_re_en (UART_Type *uart)
 * @brief   Enable RS485 RE Receiver Enable Signal State
 * @note    none
 * @param   uart : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_rs485_enable_re_en (UART_Type *uart)
{
    uint32_t mode;

    /* Special check only for Software control Half-Duplex Mode
     * as DE and RE are mutually exclusive here. */

    /* Check rs485 transfer mode. */
    mode = uart_rs485_get_transfer_mode(uart);

    if(mode == UART_RS485_SW_CONTROL_HALF_DULPLX_MODE)
    {
        /* In Software control Half-Duplex Mode DE and RE are mutually exclusive.
         * so anyone either DE or RE can be enable at a time.
         */

        /* in S/W Half-Duplex Mode first disable DE signal before enabling RE signal. */
        /* disable DE. */
        uart->UART_DE_EN = UART_DE_EN_DISABLE;
    }

    /* enable RE Receiver Enable signal. */
    uart->UART_RE_EN = UART_RE_EN_ENABLE;
}

/**
 * @fn      void uart_rs485_disable_re_en (UART_Type *uart)
 * @brief   Disable RS485 RE Receiver Enable Signal State
 * @note    none
 * @param   uart    : Pointer to uart register set structure
 * @retval  none
 */
static inline void uart_rs485_disable_re_en (UART_Type *uart)
{
    /* Disable the RE Receiver Enable signal. */
    uart->UART_RE_EN = UART_RE_EN_DISABLE;
}

/* ---------------------- END of RS485 Functions ---------------------------- */


/**
 * @fn      void uart_set_baudrate (UART_Type *uart,uint32_t clk, uint32_t baudrate)
 * @brief   set uart baudrate
 * @note    added support for fraction in dlf divisor latch fraction register
 * @param   uart     : Pointer to uart register set structure
 * @param   clk      : clock
 * @param   baudrate : baudrate
 * @retval  none
 */
void uart_set_baudrate (UART_Type  *uart, uint32_t clk, uint32_t baudrate);

/**
 * @fn      void uart_set_data_parity_stop_bits(UART_Type       *uart,
                                                UART_DATA_BITS   data_bits,
                                                UART_PARITY      parity,
                                                UART_STOP_BITS   stop_bits)
 * @brief   set data, parity and stop bits.
 * @note    none
 * @param   uart      : Pointer to uart register set structure
 * @param   data_bits : data bits
 * @param   parity    : parity
 * @param   stop_bits : stop bits
 * @retval  none
 */
void uart_set_data_parity_stop_bits(UART_Type       *uart,
                                    UART_DATA_BITS   data_bits,
                                    UART_PARITY      parity,
                                    UART_STOP_BITS   stop_bits);

/**
 * @fn      void uart_set_flow_control(UART_Type         *uart,
                                       UART_FLOW_CONTROL  flow_control)
 * @brief   set flow control(RTS/CTS)
 * @note    none
 * @param   uart         : Pointer to uart register set structure
 * @param   flow_control : flow control (RTS/CTS)
 * @retval  none
 */
void uart_set_flow_control(UART_Type         *uart,
                           UART_FLOW_CONTROL  flow_control);

/**
 * @fn      void uart_send_blocking (UART_Type *uart, UART_TRANSFER *transfer)
 * @brief   uart send using blocking/polling method,
 *           this will block till uart sends all the data.
 * @param   uart     : Pointer to uart register set structure
 * @param   transfer : Pointer to uart transfer structure
 * @retval  none
 */
void uart_send_blocking (UART_Type *uart, UART_TRANSFER *transfer);

/**
 * @fn      void uart_receive_blocking (UART_Type *uart, UART_TRANSFER *transfer)
 * @brief   uart receive using blocking/polling method,
 *           this will block till uart receives all the data.
 * @param   uart     : Pointer to uart register set structure
 * @param   transfer : Pointer to uart transfer structure
 * @retval  none
 */
void uart_receive_blocking (UART_Type *uart, UART_TRANSFER *transfer);

/**
 * @fn      void uart_irq_handler (UART_Type *uart, UART_TRANSFER *transfer)
 * @brief   uart interrupt handler
 * @note    only one combined interrupt for
 *              -TX / RX
 *              -RX_Character_Timeout
 *              -Modem status
 *              -Receiver Line status
 *          in RX_Timeout case not clearing rx busy flag, it is up to user to decide whether
 *          to wait for remaining bytes or call the abort rx. \ref abort_rx
 * @param   uart     : Pointer to uart register set structure
 * @param   transfer : Pointer to uart transfer structure
 * @retval  none
 */
void uart_irq_handler (UART_Type *uart, UART_TRANSFER *transfer);

#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
