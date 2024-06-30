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
 * @file     i2s.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     23-05-2023
 * @brief    Low Level Header file for I2S
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef I2S_H_
#define I2S_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef  __cplusplus
}
#endif

/**
  * @brief I2S (I2S)
  */

typedef struct {                                      /*!< I2S Structure                                                  */
    volatile uint32_t           I2S_IER;              /*!< (@ 0x00000000) I2S Enable Register                             */
    volatile uint32_t           I2S_IRER;             /*!< (@ 0x00000004) I2S Receiver Block Enable Register              */
    volatile uint32_t           I2S_ITER;             /*!< (@ 0x00000008) I2S Transmitter Block Enable Register           */
    volatile uint32_t           I2S_CER;              /*!< (@ 0x0000000C) Clock Enable Register                           */
    volatile uint32_t           I2S_CCR;              /*!< (@ 0x00000010) Clock Configuration Register                    */
    volatile uint32_t           I2S_RXFFR;            /*!< (@ 0x00000014) Receiver Block FIFO Reset Register              */
    volatile uint32_t           I2S_TXFFR;            /*!< (@ 0x00000018) Transmitter Block FIFO Reset Register           */
    volatile const uint32_t     RESERVED;

    union {
        volatile const uint32_t I2S_LRBR0;            /*!< (@ 0x00000020) Left Receive Buffer Register 0                  */
        volatile uint32_t       I2S_LTHR0;            /*!< (@ 0x00000020) Left Transmit Holding Register 0                */
    };

    union {
      volatile const uint32_t   I2S_RRBR0;            /*!< (@ 0x00000024) Right Receive Buffer Register 0               */
      volatile uint32_t         I2S_RTHR0;            /*!< (@ 0x00000024) Right Transmit Holding Register 0   */
    };
    volatile uint32_t             I2S_RER0;           /*!< (@ 0x00000028) Receive Enable Register 0                       */
    volatile uint32_t             I2S_TER0;           /*!< (@ 0x0000002C) Transmit Enable Register 0                      */
    volatile uint32_t             I2S_RCR0;           /*!< (@ 0x00000030) Receive Configuration Register 0                */
    volatile uint32_t             I2S_TCR0;           /*!< (@ 0x00000034) Transmit Configuration Register 0               */
    volatile const uint32_t       I2S_ISR0;           /*!< (@ 0x00000038) Interrupt Status Register 0                     */
    volatile uint32_t             I2S_IMR0;           /*!< (@ 0x0000003C) Interrupt Mask Register 0                       */
    volatile const uint32_t       I2S_ROR0;           /*!< (@ 0x00000040) Receive Overrun Register 0                      */
    volatile const uint32_t       I2S_TOR0;           /*!< (@ 0x00000044) Transmit Overrun Register 0                     */
    volatile uint32_t             I2S_RFCR0;          /*!< (@ 0x00000048) Receive FIFO Configuration Register 0           */
    volatile uint32_t             I2S_TFCR0;          /*!< (@ 0x0000004C) Transmit FIFO Configuration Register 0          */
    volatile uint32_t             I2S_RFF0;           /*!< (@ 0x00000050) Receive FIFO Flush Register 0                   */
    volatile uint32_t             I2S_TFF0;           /*!< (@ 0x00000054) Transmit FIFO Flush Register 0                  */
    volatile const uint32_t       RESERVED1[90];
    volatile const uint32_t       I2S_RXDMA;          /*!< (@ 0x000001C0) Receiver Block DMA Register                     */
    volatile const uint32_t       RESERVED2;
    volatile uint32_t             I2S_TXDMA;          /*!< (@ 0x000001C8) Transmitter Block DMA Register                  */
    volatile const uint32_t       RESERVED3[9];
    volatile const uint32_t       I2S_COMP_PARAM_2;   /*!< (@ 0x000001F0) Module Configuration Register 2                 */
    volatile const uint32_t       I2S_COMP_PARAM_1;   /*!< (@ 0x000001F4) Module Configuration Register 1                 */
    volatile const uint32_t       I2S_COMP_VERSION;   /*!< (@ 0x000001F8) Reserved                                        */
    volatile const uint32_t       I2S_COMP_TYPE;      /*!< (@ 0x000001FC) Reserved                                        */
    volatile uint32_t             I2S_DMACR;          /*!< (@ 0x00000200) DMA Control Register                            */
} I2S_Type;                                           /*!< Size = 516 (0x204)                                             */

/*!< Number of bytes for 16/32bit resolution*/
#define I2S_16BIT_BUF_TYPE_BYTES        2
#define I2S_32BIT_BUF_TYPE_BYTES        4

/*!< Max Audio Resolution for Tx/Rx Channel */
#define I2S_RX_WORDSIZE_MAX             32
#define I2S_TX_WORDSIZE_MAX             32

/*!< FIFO Depth for Tx & Rx  */
#define I2S_FIFO_DEPTH                  16
#define I2S_FIFO_TRIGGER_LEVEL_MAX      15

/* Register fields and masks */

/* I2S IER.IEN: Global Enable */
#define I2S_IER_IEN                     0x1U

/* I2S IRER.RXEN: RX block Enable */
#define I2S_IRER_RXEN                   0x1U

/* I2S ITER.TXEN: TX block Enable */
#define I2S_ITER_TXEN                   0x1U

/* I2S CER.CLKEN: Clock Enable */
#define I2S_CER_CLKEN                   0x1U

/* I2S CCR.SCLKG: Clock Gating */
#define I2S_CCR_SCLKG_Pos               0U
#define I2S_CCR_SCLKG_Msk               (0x7U << I2S_CCR_SCLKG_Pos)

/* I2S CCR.WSS: Word Select length */
#define I2S_CCR_WSS_Pos                 3U
#define I2S_CCR_WSS_Msk                 (0x3U << I2S_CCR_WSS_Pos)

/* I2S RXFFR.RXFFR: Rx Block FIFO Reset */
#define I2S_RXFFR_RXFFR                 0x1U

/* I2S TXFFR.TXFFR: Tx Block FIFO Reset */
#define I2S_TXFFR_TXFFR                 0x1U

/* I2S RER.RXCHEN: Rx Channel Enable */
#define I2S_RER_RXCHEN                  0x1U

/* I2S TER.TXCHEN: Tx Channel Enable */
#define I2S_TER_TXCHEN                  0x1U

/* I2S RCR.WLEN: Data Resolution of Rx */
#define I2S_RCR_WLEN_Pos                0U
#define I2S_RCR_WLEN_Msk                (0x7U << I2S_RCR_WLEN_Pos)

/* I2S TCR.WLEN: Data Resolution of Tx */
#define I2S_TCR_WLEN_Pos                0U
#define I2S_TCR_WLEN_Msk                (0x7U << I2S_TCR_WLEN_Pos)

/* I2S ISR.RXDA: Status of Rx Data Available interrupt */
#define I2S_ISR_RXDA                    0x1U

/* I2S ISR.RXFO: Status of Data Overrun interrupt of Rx */
#define I2S_ISR_RXFO                    (0x1U << 1)

/* I2S ISR.TXFE: Status of Tx Empty Trigger Interrupt */
#define I2S_ISR_TXFE                    (0x1U << 4)

/* I2S ISR.TXFO: Status of Data Overrun Interrupt for Tx */
#define I2S_ISR_TXFO                    (0x1U << 5)

/* I2S IMR.RXDAM: Mask Rx Data Available interrupt */
#define I2S_IMR_RXDAM                   0x1U

/* I2S IMR.RXFOM: Mask Data Overrun interrupt of Rx */
#define I2S_IMR_RXFOM                   (0x1U << 1)

/* I2S IMR.TXFEM: Mask Tx Empty Interrupt */
#define I2S_IMR_TXFEM                   (0x1U << 4)

/* I2S IMR.TXFOM: Mask Data Overrun Interrupt for Tx */
#define I2S_IMR_TXFOM                   (0x1U << 5)

/* I2S ROR.RXCHO: Clear Rx Data Overrun interrupt */
#define I2S_ROR_RXCHO                   0x1U

/* I2S TOR.TXCHO: Clear Tx Data Overrun interrupt */
#define I2S_TOR_TXCHO                   0x1U

/* I2S RFCR.RXCHDT: Rx FIFO Trigger Level */
#define I2S_RFCR_RXCHDT_Pos             0U
#define I2S_RFCR_RXCHDT_Msk             (0xFU << I2S_RFCR_RXCHDT_Pos)

/* I2S TFCR.TXCHET: Tx FIFO Trigger Level */
#define I2S_TFCR_TXCHET_Pos             0U
#define I2S_TFCR_TXCHET_Msk             (0xFU << I2S_TFCR_TXCHET_Pos)

/* I2S RFF.RXCHFR: Rx Channel FIFO Reset */
#define I2S_RFF_RXCHFR_Msk              0x1U

/* I2S TFF.TXCHFR: Tx Channel FIFO Reset */
#define I2S_TFF_TXCHFR_Msk              0x1U

/* I2S DMACR.DMAEN_RXBLOCK: DMA Enable for Rx Block */
#define I2S_DMACR_DMAEN_RXBLOCK         (0x1U << 16)

/* I2S DMACR.DMAEN_TXBLOCK: DMA Enable for Tx Block */
#define I2S_DMACR_DMAEN_TXBLOCK         (0x1U << 17)

/* I2S DMA TX and RX register offset */
#define I2S_RXDMA_OFFSET                (0x1C0)
#define I2S_TXDMA_OFFSET                (0x1C8)

/**
 * enum I2S_WSS
 * I2S Word Select Size
 */
typedef enum _I2S_WSS {
    I2S_WSS_SCLK_CYCLES_16,         /*!< 16 sclk cycles */
    I2S_WSS_SCLK_CYCLES_24,         /*!< 24 sclk cycles */
    I2S_WSS_SCLK_CYCLES_32,         /*!< 32 sclk cycles */

    I2S_WSS_SCLK_CYCLES_MAX
} I2S_WSS;

/**
 * enum I2S_SCLKG
 * I2S Serial Clock gating
 */
typedef enum _I2S_SCLKG {
    I2S_SCLKG_NONE,                 /*!< Gating is disabled          */
    I2S_SCLKG_CLOCK_CYCLES_12,      /*!< Gating after 12 sclk cycles */
    I2S_SCLKG_CLOCK_CYCLES_16,      /*!< Gating after 16 sclk cycles */
    I2S_SCLKG_CLOCK_CYCLES_20,      /*!< Gating after 20 sclk cycles */
    I2S_SCLKG_CLOCK_CYCLES_24,      /*!< Gating after 24 sclk cycles */

    I2S_SCLKG_CLOCK_CYCLES_MAX
} I2S_SCLKG;

/**
 * enum I2S_WLEN
 * I2S Word length. Specifies the data resolution of the receiver.
 */
typedef enum _I2S_WLEN {
    I2S_WLEN_RES_NONE,              /*!< Word length ignored    */
    I2S_WLEN_RES_12_BIT,            /*!< 12 bit Data Resolution */
    I2S_WLEN_RES_16_BIT,            /*!< 16 bit Data Resolution */
    I2S_WLEN_RES_20_BIT,            /*!< 20 bit Data Resolution */
    I2S_WLEN_RES_24_BIT,            /*!< 24 bit Data Resolution */
    I2S_WLEN_RES_32_BIT,            /*!< 32 bit Data Resolution */
} I2S_WLEN;

/**
 * enum I2S_TRANSFER_STATUS.
 * Status of an ongoing I2S transfer.
 */
typedef enum _I2S_TRANSFER_STATUS {
    I2S_TRANSFER_STATUS_NONE          = (0U),       /**< Transfer status none         */
    I2S_TRANSFER_STATUS_TX_COMPLETE   = (1U << 0),  /**< TX Transfer status complete  */
    I2S_TRANSFER_STATUS_RX_COMPLETE   = (1U << 1),  /**< RX Transfer status complete  */
    I2S_TRANSFER_STATUS_RX_OVERFLOW   = (1U << 2),  /**< Transfer status Rx overflow  */
    I2S_TRANSFER_STATUS_TX_OVERFLOW   = (1U << 3)   /**< Transfer status Rx overflow  */
} I2S_TRANSFER_STATUS;

typedef struct _i2s_transfer_t {
    volatile uint32_t               tx_current_cnt;     /**< Current Tx Transfer count                                  */
    volatile uint32_t               rx_current_cnt;     /**< Current Rx Transfer count                                  */
    uint32_t                        tx_total_cnt;       /**< Tx Total count to transfer                                 */
    uint32_t                        rx_total_cnt;       /**< Rx Total count to transfer                                 */
    const void                      *tx_buff;           /**< Pointer to TX buffer                                       */
    void                            *rx_buff;           /**< Pointer to Rx buffer                                       */
    volatile uint32_t               status;             /**< I2S_TRANSFER_STATUS Transfer status                        */
    bool                            mono_mode;          /**< Mono mode enable status                                    */
} i2s_transfer_t;

/**
  \fn          void i2s_enable(I2S_Type *i2s)
  \brief       Enable I2S
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_enable(I2S_Type *i2s)
{
    i2s->I2S_IER = I2S_IER_IEN;
}

/**
  \fn          void i2s_disable(I2S_Type *i2s)
  \brief       Disable I2S
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_disable(I2S_Type *i2s)
{
    i2s->I2S_IER &= ~I2S_IER_IEN;
}

/**
  \fn          void i2s_rxblock_enable(I2S_Type *i2s)
  \brief       Enable I2S Receiver Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_rxblock_enable(I2S_Type *i2s)
{
    i2s->I2S_IRER = I2S_IRER_RXEN;
}

/**
  \fn          void i2s_rxblock_disable(I2S_Type *i2s)
  \brief       Disable I2S Receiver Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_rxblock_disable(I2S_Type *i2s)
{
    i2s->I2S_IRER &= ~I2S_IRER_RXEN;
}

/**
  \fn          void i2s_txblock_enable(I2S_Type *i2s)
  \brief       Enable I2S Transmitter Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_txblock_enable(I2S_Type *i2s)
{
    i2s->I2S_ITER = I2S_ITER_TXEN;
}

/**
  \fn          void i2s_txblock_disable(I2S_Type *i2s)
  \brief       Disable I2S Transmitter Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_txblock_disable(I2S_Type *i2s)
{
    i2s->I2S_ITER &= ~I2S_ITER_TXEN;
}

/**
  \fn          void i2s_clock_enable(I2S_Type *i2s)
                                     I2S_SCLKG sclkg,
                                     I2S_WSS wss_len)
  \brief       Enable Clock in Master Mode.
  \param[in]   i2s      Pointer to I2S register map
  \param[in]   sclkg    sclk gating value
  \param[in]   wss_len  word select length
*/
static inline void i2s_clock_enable(I2S_Type *i2s,
                                    I2S_SCLKG sclkg,
                                    I2S_WSS wss_len)
{
    i2s->I2S_CCR = (((sclkg << I2S_CCR_SCLKG_Pos) & I2S_CCR_SCLKG_Msk) |
                    ((wss_len << I2S_CCR_WSS_Pos) & I2S_CCR_WSS_Msk));

    i2s->I2S_CER = I2S_CER_CLKEN;
}

/**
  \fn          void i2s_clock_disable(I2S_Type *i2s)
  \brief       Disable I2S Clock in Master Mode
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_clock_disable(I2S_Type *i2s)
{
    i2s->I2S_CER &= ~I2S_CER_CLKEN;
}

/**
  \fn          void i2s_rxchannel_enable(I2S_Type *i2s)
  \brief       Enable I2S Receiver Channel
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_rxchannel_enable(I2S_Type *i2s)
{
    i2s->I2S_RER0 = I2S_RER_RXCHEN;
}

/**
  \fn          void i2s_rxchannel_disable(I2S_Type *i2s)
  \brief       Disable I2S Receiver Channel
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_rxchannel_disable(I2S_Type *i2s)
{
    i2s->I2S_RER0 &= ~I2S_RER_RXCHEN;
}

/**
  \fn          void i2s_txchannel_enable(I2S_Type *i2s)
  \brief       Enable I2S Transmit Channel
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_txchannel_enable(I2S_Type *i2s)
{
    i2s->I2S_TER0 = I2S_TER_TXCHEN;
}

/**
  \fn          void i2s_txchannel_disable(I2S_Type *i2s)
  \brief       Disable I2S Transmit Channel
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_txchannel_disable(I2S_Type *i2s)
{
    i2s->I2S_TER0 &= ~I2S_TER_TXCHEN;
}

/**
  \fn          void i2s_set_rx_wlen(I2S_Type *i2s, I2S_WLEN wlen)
  \brief       Set Wlen in Receive Configuration Register
               Should be called with RXCHEN disabled.
  \param[in]   i2s   Pointer to I2S register map
  \param[in]   wlen  word length
*/
static inline void i2s_set_rx_wlen(I2S_Type *i2s, I2S_WLEN wlen)
{
    i2s->I2S_RCR0 = (wlen << I2S_RCR_WLEN_Pos) & I2S_RCR_WLEN_Msk;
}

/**
  \fn          void i2s_set_tx_wlen(I2S_Type *i2s, I2S_WLEN wlen)
  \brief       Set Wlen in Transmit Configuration Register
               Should be called with TXCHEN disabled.
  \param[in]   i2s   Pointer to I2S register map
  \param[in]   wlen  word length
*/
static inline void i2s_set_tx_wlen(I2S_Type *i2s, I2S_WLEN wlen)
{
    i2s->I2S_TCR0 = (wlen << I2S_TCR_WLEN_Pos) & I2S_TCR_WLEN_Msk;
}

/**
  \fn          void i2s_clear_rx_overrun(I2S_Type *i2s)
  \brief       Clear Receiver FIFO Data Overrun Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_clear_rx_overrun(I2S_Type *i2s)
{
    (void) (i2s->I2S_ROR0);
}

/**
  \fn          void i2s_clear_tx_overrun(I2S_Type *i2s)
  \brief       Clear Transmit FIFO Data Overrun Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_clear_tx_overrun(I2S_Type *i2s)
{
    (void) (i2s->I2S_TOR0);
}

/**
  \fn          void i2s_set_rx_triggerLevel(I2S_Type *i2s,
                                            uint8_t trigger_level)
  \brief       Set the Trigger Level in RxFIFO
               The channel must be disabled before doing this
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_set_rx_triggerLevel(I2S_Type *i2s,
                                           uint8_t trigger_level)
{
    i2s->I2S_RFCR0 = trigger_level;
}

/**
  \fn          void i2s_set_tx_triggerLevel(I2S_Type *i2s
                                            uint8_t trigger_level)
  \brief       Set the Trigger Level in TxFIFO
               The channel must be disabled before doing this
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_set_tx_triggerLevel(I2S_Type *i2s,
                                           uint8_t trigger_level)
{
    i2s->I2S_TFCR0 = trigger_level;
}

/**
  \fn          void i2s_reset_rx_fifo(I2S_Type *i2s)
  \brief       Reset the Rx Channel FIFO
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_reset_rx_fifo(I2S_Type *i2s)
{
    i2s->I2S_RFF0 = 0x1;
}

/**
  \fn          void i2s_reset_tx_fifo(I2S_Type *i2s)
  \brief       Reset the Tx Channel FIFO
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_reset_tx_fifo(I2S_Type *i2s)
{
    i2s->I2S_TFF0 = 0x1;
}

/**
  \fn          void i2s_rx_dma_enable(I2S_Type *i2s)
  \brief       Enable I2S Rx DMA Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_rx_dma_enable(I2S_Type *i2s)
{
    i2s->I2S_DMACR |= I2S_DMACR_DMAEN_RXBLOCK;
}

/**
  \fn          void i2s_rx_dma_disable(I2S_Type *i2s)
  \brief       Disable I2S Rx DMA Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_rx_dma_disable(I2S_Type *i2s)
{
    i2s->I2S_DMACR &= ~I2S_DMACR_DMAEN_RXBLOCK;
}

/**
  \fn          void i2s_tx_dma_enable(I2S_Type *i2s)
  \brief       Enable I2S Tx DMA Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_tx_dma_enable(I2S_Type *i2s)
{
    i2s->I2S_DMACR |= I2S_DMACR_DMAEN_TXBLOCK;
}

/**
  \fn          void i2s_tx_dma_disable(I2S_Type *i2s)
  \brief       Disable I2S Tx DMA Block
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_tx_dma_disable(I2S_Type *i2s)
{
    i2s->I2S_DMACR &= ~I2S_DMACR_DMAEN_TXBLOCK;
}

/**
  \fn          void i2s_enable_tx_interrupt(I2S_Type *i2s)
  \brief       Enable I2S Tx Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_enable_tx_interrupt(I2S_Type *i2s)
{
    uint32_t imr;

    imr = i2s->I2S_IMR0;
    imr &= ~(I2S_IMR_TXFEM | I2S_IMR_TXFOM);

    i2s->I2S_IMR0 = imr;
}

/**
  \fn          void i2s_disable_tx_interrupt(I2S_Type *i2s)
  \brief       Disable I2S Tx Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_disable_tx_interrupt(I2S_Type *i2s)
{
    uint32_t imr;

    imr = i2s->I2S_IMR0;
    imr |= (I2S_IMR_TXFEM | I2S_IMR_TXFOM);

    i2s->I2S_IMR0 = imr;
}

/**
  \fn          void i2s_enable_rx_interrupt(I2S_Type *i2s)
  \brief       Enable I2S Rx Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_enable_rx_interrupt(I2S_Type *i2s)
{
    uint32_t imr;

    imr = i2s->I2S_IMR0;
    imr &= ~(I2S_IMR_RXDAM | I2S_IMR_RXFOM);

    i2s->I2S_IMR0 = imr;
}

/**
  \fn          void i2s_disable_rx_interrupt(I2S_Type *i2s)
  \brief       Disable I2S Rx Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_disable_rx_interrupt(I2S_Type *i2s)
{
    uint32_t imr;

    imr = i2s->I2S_IMR0;
    imr |= (I2S_IMR_RXDAM | I2S_IMR_RXFOM);

    i2s->I2S_IMR0 = imr;
}

/**
  \fn          void i2s_enable_rx_overflow_interrupt(I2S_Type *i2s)
  \brief       Enable I2S Rx Overflow Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_enable_rx_overflow_interrupt(I2S_Type *i2s)
{
    uint32_t imr;

    imr = i2s->I2S_IMR0;
    imr &= ~I2S_IMR_RXFOM;

    i2s->I2S_IMR0 = imr;
}

/**
  \fn          void i2s_disable_rx_overflow_interrupt(I2S_Type *i2s)
  \brief       Disable I2S Rx Overflow Interrupt
  \param[in]   i2s  Pointer to I2S register map
*/
static inline void i2s_disable_rx_overflow_interrupt(I2S_Type *i2s)
{
    uint32_t imr;

    imr = i2s->I2S_IMR0;
    imr |= I2S_IMR_RXFOM;

    i2s->I2S_IMR0 = imr;
}

/**
  \fn          void* i2s_get_dma_tx_addr(I2S_Type *i2s)
  \brief       Return the DMA Tx Address
  \param[in]   i2s   Pointer to I2S register map
  \return      \ref  Return the address
*/
static inline void* i2s_get_dma_tx_addr(I2S_Type *i2s)
{
    return ((uint8_t*)i2s + I2S_TXDMA_OFFSET);
}

/**
  \fn          void* i2s_get_dma_rx_addr(I2S_Type *i2s)
  \brief       Return the DMA Rx Address
  \param[in]   i2s   Pointer to I2S register map
  \return      \ref  Return the address
*/
static inline void* i2s_get_dma_rx_addr(I2S_Type *i2s)
{
    return ((uint8_t*)i2s + I2S_RXDMA_OFFSET);
}

/**
  \fn          uint32_t i2s_get_sclk_frequency(uint32_t sample_rate,
                                               I2S_WSS wss_len)
  \brief       Calculate the sclk frequency
  \param[in]   sample_rate  Audio sample rate
  \param[in]   wss_len      Word Select Size
  \return      \ref         Serial clock frequency
*/
static inline uint32_t i2s_get_sclk_frequency(uint32_t sample_rate,
                                              I2S_WSS wss_len)
{
    const uint32_t num_sclk_cycles[I2S_WSS_SCLK_CYCLES_MAX] = {16, 24, 32};
    uint32_t sclk_freq;

    /* Calculate sclk = 2 x no of sclk cycles in wss_out x sample Rate */
    sclk_freq = 2 * num_sclk_cycles[wss_len] * sample_rate;

    return sclk_freq;
}

/**
  \fn          void i2s_send(I2S_Type *i2s)
  \brief       Prepare the I2S instance for transmission
  \param[in]   i2s  Pointer to the I2S register map
  \return      none
*/
static inline void i2s_send(I2S_Type *i2s)
{
    /* Clear Overrun interrupt if any */
    i2s_clear_tx_overrun(i2s);

    /* Enable Tx Channel */
    i2s_txchannel_enable(i2s);

    /* Enable Tx Interrupt */
    i2s_enable_tx_interrupt(i2s);
}

/**
  \fn          void i2s_receive(I2S_Type *i2s)
  \brief       Prepare the I2S instance for reception
  \param[in]   i2s  Pointer to the I2S register map
  \return      none
*/
static inline void i2s_receive(I2S_Type *i2s)
{
    /* Clear Overrun interrupt if any */
    i2s_clear_rx_overrun(i2s);

    /* Enable Rx Channel */
    i2s_enable_rx_interrupt(i2s);

    /* Enable Rx Channel */
    i2s_rxchannel_enable(i2s);
}

/**
  \fn          void i2s_dma_receive(I2S_Type *i2s)
  \brief       Prepare the I2S instance for reception using DMA
  \param[in]   i2s  Pointer to the I2S register map
  \return      none
*/
static inline void i2s_dma_receive(I2S_Type *i2s)
{
    /* Clear Overrun interrupt if any */
    i2s_clear_rx_overrun(i2s);

    /* Enable the Rx Overflow interrupt */
    i2s_enable_rx_overflow_interrupt(i2s);

    /* Enable Rx Channel */
    i2s_rxchannel_enable(i2s);
}

/**
  \fn          void i2s_dma_send(I2S_Type *i2s)
  \brief       Prepare the I2S instance for transmission using DMA
  \param[in]   i2s  Pointer to the I2S register map
  \return      none
*/
static inline void i2s_dma_send(I2S_Type *i2s)
{
    /* Clear Overrun interrupt if any */
    i2s_clear_tx_overrun(i2s);

    /* Enable Tx Channel */
    i2s_txchannel_enable(i2s);
}

/**
  \fn          void i2s_tx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Handle interrupts for the I2S Tx.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  The transfer structure for the I2S instance
  \return      none
*/
void i2s_tx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer);

/**
  \fn          void i2s_rx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Handle interrupts for the I2S Rx.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  The transfer structure for the I2S instance
  \return      none
*/
void i2s_rx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer);

/**
  \fn          void i2s_send_blocking(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Execute a blocking I2S send described by the transfer structure.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void i2s_send_blocking(I2S_Type *i2s, i2s_transfer_t *transfer);

/**
  \fn          void i2s_receive_blocking(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Execute a blocking I2S receive described by the transfer structure.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void i2s_receive_blocking(I2S_Type *i2s, i2s_transfer_t *transfer);

#ifdef  __cplusplus
}
#endif

#endif /* I2S_H_ */
