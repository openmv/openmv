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
 * @file     spi.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     20-04-2023
 * @brief    Low Level Header file for SPI.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef SPI_H_
#define SPI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
  * @brief LPSPI (LPSPI)
  */

typedef struct {
    volatile        uint32_t  SPI_CTRLR0;            /*!< (@ 0x00000000) Control Register 0                                  */
    volatile        uint32_t  SPI_CTRLR1;            /*!< (@ 0x00000004) Control Register 1                                  */
    volatile        uint32_t  SPI_ENR;               /*!< (@ 0x00000008) SPI Enable Register                                 */
    volatile        uint32_t  SPI_MWCR;              /*!< (@ 0x0000000C) Microwire Control Register                          */
    volatile        uint32_t  SPI_SER;               /*!< (@ 0x00000010) Slave Enable Register                               */
    volatile        uint32_t  SPI_BAUDR;             /*!< (@ 0x00000014) Baud Rate Select Register                           */
    volatile        uint32_t  SPI_TXFTLR;            /*!< (@ 0x00000018) Transmit FIFO Threshold Level Register              */
    volatile        uint32_t  SPI_RXFTLR;            /*!< (@ 0x0000001C) Receive FIFO Threshold Level Register               */
    volatile const  uint32_t  SPI_TXFLR;             /*!< (@ 0x00000020) Transmit FIFO Level Register                        */
    volatile const  uint32_t  SPI_RXFLR;             /*!< (@ 0x00000024) Receive FIFO Level Register                         */
    volatile const  uint32_t  SPI_SR;                /*!< (@ 0x00000028) Status Register                                     */
    volatile        uint32_t  SPI_IMR;               /*!< (@ 0x0000002C) Interrupt Mask Register                             */
    volatile const  uint32_t  SPI_ISR;               /*!< (@ 0x00000030) Interrupt Status Register                           */
    volatile const  uint32_t  SPI_RISR;              /*!< (@ 0x00000034) Raw Interrupt Status Register                       */
    volatile const  uint32_t  SPI_TXOICR;            /*!< (@ 0x00000038) Transmit FIFO Overflow Interrupt Clear Register     */
    volatile const  uint32_t  SPI_RXOICR;            /*!< (@ 0x0000003C) Receive FIFO Overflow Interrupt Clear Register      */
    volatile const  uint32_t  SPI_RXUICR;            /*!< (@ 0x00000040) Receive FIFO Underflow Interrupt Clear Register     */
    volatile const  uint32_t  SPI_MSTICR;            /*!< (@ 0x00000044) Multi-Master Interrupt Clear Register               */
    volatile const  uint32_t  SPI_ICR;               /*!< (@ 0x00000048) Interrupt Clear Register                            */
    volatile        uint32_t  SPI_DMACR;             /*!< (@ 0x0000004C) DMA Control Register                                */
    volatile        uint32_t  SPI_DMATDLR;           /*!< (@ 0x00000050) DMA Transmit Data Level Register                    */
    volatile        uint32_t  SPI_DMARDLR;           /*!< (@ 0x00000054) DMA Receive Data Level Register                     */
    volatile const  uint32_t  SPI_IDR;               /*!< (@ 0x00000058) Reserved                                            */
    volatile const  uint32_t  SPI_VERSION_ID;        /*!< (@ 0x0000005C) Reserved                                            */
    volatile        uint32_t  SPI_DR[36];            /*!< (@ 0x00000060) SPI Data Register (n)                               */
    volatile        uint32_t  SPI_RX_SAMPLE_DELAY;   /*!< (@ 0x000000F0) RX Sample Delay Register                            */
} SPI_Type;                                          /*!< Size = 244 (0xf4)                                                         */

#define SPI_TX_FIFO_DEPTH                               16U
#define SPI_RX_FIFO_DEPTH                               16U

#define SPI_MAX_SLAVE_SELECT_PINS                       4U
#define SPI_SLAVE_SELECT_PIN_MASK                       0xFFFFFFF0U

#define SPI_DATA_FRAME_SIZE_MIN                         4U
#define SPI_DATA_FRAME_SIZE_MAX                         32U

#define SPI_ENABLE                                      1U
#define SPI_DISABLE                                     0U

/* SPI Control Register 0 (CTRLR0) bit Definition, Macros, Offsets and Masks
 * these include DFS, FRF, SCPH, SCPOL, TMOD, etc
 */
/* Data Frame DFS bit[4:0]*/
#define SPI_CTRLR0_DFS                                  0U
#define SPI_CTRLR0_DFS_MASK                             (0x1FU << SPI_CTRLR0_DFS)

/* Frame Format FRF bit[7:6] */
#define SPI_CTRLR0_FRF                                  6U
#define SPI_CTRLR0_FRF_MASK                             (0x3UL << SPI_CTRLR0_FRF)
#define SPI_CTRLR0_FRF_MOTOROLA                         0x00U     /* 0x0 Motorola SPI Frame Format */
#define SPI_CTRLR0_FRF_TI                               0x40U     /* 0x1 Texas Instruments SSP Frame Format */
#define SPI_CTRLR0_FRF_MICROWIRE                        0x80U     /* 0X2 National Semiconductors Microwire Frame Format */

/* Serial Clock Polarity SCPOL | Serial Clock Phase SCPH bit[9:8] */
#define SPI_CTRLR0_SC                                   8U
#define SPI_CTRLR0_SC_MASK                              (0x3UL << SPI_CTRLR0_SC)
#define SPI_CTRLR0_SCPH_HIGH                            0x100U     /* 0x1 SPI SCPH high */
#define SPI_CTRLR0_SCPH_LOW                             0x000U     /* 0x0 SPI SCPH low */
#define SPI_CTRLR0_SCPOL_HIGH                           0x200U     /* 0x2 SPI SCPOL high */
#define SPI_CTRLR0_SCPOL_LOW                            0x000U     /* 0x0 SPI SCPOL low */

/* Transfer Mode TMOD bit[11:10] */
#define SPI_CTRLR0_TMOD                                 10U
#define SPI_CTRLR0_TMOD_MASK                            (0x3UL << SPI_CTRLR0_TMOD)
#define SPI_CTRLR0_TMOD_TRANSFER                        0x000U     /* 0x0 SPI transfer mode */
#define SPI_CTRLR0_TMOD_SEND_ONLY                       0x400U     /* 0x1 SPI send only mode */
#define SPI_CTRLR0_TMOD_RECEIVE_ONLY                    0x800U     /* 0x2 SPI receive only mode */
#define SPI_CTRLR0_TMOD_EEPROM_READ_ONLY                0xC00U     /* 0x3 SPI EEPROM read only mode */

/* Useful when SPI Controller acting as a Slave
 * When configured as a serial master, this
 * bit-field has no functionality.
 */
/*Slave Output Enable SLV_OE bit[12] */
#define SPI_CTRLR0_SLV_OE                               12U
#define SPI_CTRLR0_SLV_OE_MASK                          (0x1UL << SPI_CTRLR0_SLV_OE)
#define SPI_CTRLR0_SLV_OE_ENABLE                        0x1000U     /* 0x1 SPI slave output enable */
#define SPI_CTRLR0_SLV_OE_DISABLE                       0x0000U     /* 0x0 SPI slave output disable */

/* Slave Select toggle Enable bit[14]
 * While operating in SPI mode with clock phase (SCPH) set to
 * 0, this register controls the behavior of the slave select line
 *  between data frames.
 */
/* Slave Select toggle Enable SSTE bit[14] */
#define SPI_CTRLR0_SSTE                                 14U
#define SPI_CTRLR0_SSTE_MASK                            (1<< SPI_CTRLR0_SSTE)
#define SPI_CTRLR0_SSTE_ENABLE                          0x4000U     /* 0x1 SPI slave select toggle enable */
#define SPI_CTRLR0_SSTE_DISABLE                         0x0000U     /* 0x0 SPI slave select toggle disable */

/* Control Frame Size for the Microwire frame format CFS bit[19:16]*/
#define SPI_CTRLR0_CFS                                  16U
#define SPI_CTRLR0_CFS_MASK                             (0xF << SPI_CTRLR0_CFS)

/* SPI Frame Format SPI_FRF bit[23:22]*/
#define SPI_CTRLR0_SPI_FRF                              22U
#define SPI_CTRLR0_SPI_FRF_MASK                         (0xCU << SPI_CTRLR0_SPI_FRF)
#define SPI_CTRLR0_SPI_FRF_STANDRAD                     0x000000    /* 0x0 Standard SPI Format */
#define SPI_CTRLR0_SPI_FRF_DUAL                         0x400000    /* 0x1 Dual SPI Format */
#define SPI_CTRLR0_SPI_FRF_QUAD                         0x800000    /* 0X2 Quad SPI Format */
#define SPI_CTRLR0_SPI_FRF_OCTAL                        0xC00000    /* 0X2 Octal SPI Format */

/* SPI Hyperbus Frame format enable SPI_HYPERBUS_EN bit[24] */
#define SPI_CTRLR0_SPI_HYPERBUS_EN                      24U
#define SPI_CTRLR0_SPI_HYPERBUS_EN_SSTE_MASK            (1 << SPI_CTRLR0_SPI_HYPERBUS_EN)
#define SPI_CTRLR0_SPI_HYPERBUS_ENABLE                  0x4000U      /* 0x1 SPI Hyperbus Frame format enable */
#define SPI_CTRLR0_SPI_HYPERBUS_DISABLE                 0x0000U      /* 0x0 SPI Hyperbus Frame format disable */

/* SPI is working in Master or Slave SSI_IS_MST bit[31] */
#define SPI_CTRLR0_SSI_IS_MST                           31U
#define SPI_CTRLR0_SSI_IS_MST_MASK                      (0x1UL << SPI_CTRLR0_SSI_IS_MST)
#define SPI_CTRLR0_SSI_IS_MST_MASTER                    0x80000000U  /* 0x1 SPI master */
#define SPI_CTRLR0_SSI_IS_MST_SLAVE                     0x00000000U  /* 0x0 SPI slave  */

/* Quad SPI MODE Macros */
#define SPI_CTRLR0_SPI_QUAD_ENABLE                     (0x2UL << SPI_CTRLR0_SPI_FRF)
#define SPI_CTRLR0_SPI_QUAD_TX_MODE                     SPI_TMOD_SEND_ONLY

#define SPI_CTRLR0_SPI_QUAD_RX_MODE                     SPI_TMOD_RECEIVE_ONLY

/* Octal SPI MODE Macros */
#define SPI_CTRLR0_SPI_OCTAL_ENABLE                     (0x3UL << SPI_CTRLR0_SPI_FRF)
#define SPI_CTRLR0_SPI_OCTAL_TX_RX_MODE                 SPI_TMOD_TRANSFER
#define SPI_CTRLR0_SPI_OCTAL_TX_MODE                    SPI_TMOD_SEND_ONLY
#define SPI_CTRLR0_SPI_OCTAL_RX_MODE                    SPI_TMOD_RECEIVE_ONLY

/* LPSPI Control Register 0 (CTRLR0) bit Definition, Macros, Offsets and Masks */
/* Data Frame DFS bit[3:0] */
#define LPSPI_CTRLR0_DFS                                0U
#define LPSPI_CTRLR0_DFS_MASK                           (0xFU << LPSPI_CTRLR0_DFS)

/* Frame Format FRF bit [5:4] */
#define LPSPI_CTRLR0_FRF                                4U
#define LPSPI_CTRLR0_FRF_MASK                           (0x3UL << LPSPI_CTRLR0_FRF)
#define LPSPI_CTRLR0_FRF_MOTOROLA                       0x00U     /* 0x0 Motorola SPI Frame Format */
#define LPSPI_CTRLR0_FRF_TI                             0x10U     /* 0x1 Texas Instruments SSP Frame Format */
#define LPSPI_CTRLR0_FRF_MICROWIRE                      0x20U     /* 0X2 National Semiconductors Microwire Frame Format */

/* Serial Clock Polarity SCPOL | Serial Clock Phase SCPH bit [7:6] */
#define LPSPI_CTRLR0_SC                                 6U
#define LPSPI_CTRLR0_SC_MASK                            (0x3UL << LPSPI_CTRLR0_SC)
#define LPSPI_CTRLR0_SCPH_HIGH                          0x40U     /* 0x1 SPI SCPH high */
#define LPSPI_CTRLR0_SCPH_LOW                           0x00U     /* 0x0 SPI SCPH low */
#define LPSPI_CTRLR0_SCPOL_HIGH                         0x80U     /* 0x2 SPI SCPOL high */
#define LPSPI_CTRLR0_SCPOL_LOW                          0x00U     /* 0x0 SPI SCPOL low */

/* Transfer Mode TMOD bit[9:8] */
#define LPSPI_CTRLR0_TMOD                               8U
#define LPSPI_CTRLR0_TMOD_MASK                          (0x3UL << LPSPI_CTRLR0_TMOD)
#define LPSPI_CTRLR0_TMOD_TRANSFER                      0x000U     /* 0x0 SPI transfer mode */
#define LPSPI_CTRLR0_TMOD_SEND_ONLY                     0x100U     /* 0x1 SPI send only mode */
#define LPSPI_CTRLR0_TMOD_RECEIVE_ONLY                  0x200U     /* 0x2 SPI receive only mode */
#define LPSPI_CTRLR0_TMOD_EEPROM_READ_ONLY              0x300U     /* 0x3 SPI EEPROM read only mode */

/* Control Frame Size for the Microwire frame format CFS bit[15:12]*/
#define LPSPI_CTRLR0_CFS                                12U
#define LPSPI_CTRLR0_CFS_MASK                           (0xF << LPSPI_CTRLR0_CFS)

/* Data Frame DFS 32 bit [20:16] */
#define LPSPI_CTRLR0_DFS_32                             16U
#define LPSPI_CTRLR0_DFS32_MASK                         (0x1FU << LPSPI_CTRLR0_DFS_32)

/* Slave Select toggle Enable SSTE bit[24] */
#define LPSPI_CTRLR0_SSTE                               24U
#define LPSPI_CTRLR0_SSTE_MASK                          (1 << LPSPI_CTRLR0_SSTE)
#define LPSPI_CTRLR0_SSTE_ENABLE                        0x1000000U     /* 0x1 SPI slave select toggle enable */
#define LPSPI_CTRLR0_SSTE_DISABLE                       0x0000000U     /* 0x0 SPI slave select toggle disable */

/* Microwire Control Register (MWCR) bit Definition, Macros */
/* Microwire Transfer Mode bit[0] */
#define SPI_MWCR_MWMOD_SEQUENTIAL_MODE                  0x1U
#define SPI_MWCR_MWMOD_NON_SEQUENTIAL_MODE              0x0U

/* Microwire Direction control bit[1] */
#define SPI_MWCR_MDD_TRANSMIT                           0x2U
#define SPI_MWCR_MDD_RECEIVE                            0x0U

/* Microwire Handshake Interface bit[2] */
#define SPI_MWCR_MHS_ENABLE                             0x4U
#define SPI_MWCR_MHS_DISABLE                            0x0U

#define SPI_TXFTLR_TFT_SHIFT                            0U
#define SPI_TXFTLR_TFT_MASK                             (0xFFFFU << SPI_TXFTLR_TFT_SHIFT)
#define SPI_TXFTLR_TXFTHR_SHIFT                         16U
#define SPI_TXFTLR_TXFTHR_MASK                          (0xFFFFU << SPI_TXFTLR_TXFTHR_SHIFT)

#define SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK            0x00000001U  /* Transmit fifo empty interrupt mask*/
#define SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK        0x00000002U  /* Transmit fifo overflow interrupt mask*/
#define SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK       0x00000004U  /* Receive fifo underflow interrupt mask*/
#define SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK        0x00000008U  /* Receive fifo Overflow interrupt mask*/
#define SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK             0x00000010U  /* Receive fifo full interrupt mask*/
#define SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK  0x00000020U  /* Multi-Master contention interrupt mask.*/

#define SPI_DMACR_RX_DMA_ENABLE                         (0x1U)
#define SPI_DMACR_TX_DMA_ENABLE                         (0x2U)

#define SPI_SR_TFNF                                     (0x2U)
#define SPI_SR_RFNE                                     (0x8U)
#define SPI_SR_TX_FIFO_EMPTY                            (0x4U)
#define SPI_SR_BUSY                                     (0x1U)

/****** SPI events *****/
#define SPI_TX_FIFO_EMPTY_EVENT                         (0x01)      /* Transmit fifo empty interrupt mask*/
#define SPI_TX_FIFO_OVER_FLOW_EVENT                     (0x02)      /* Transmit fifo overflow interrupt mask*/
#define SPI_RX_FIFO_UNDER_FLOW_EVENT                    (0x04)      /* Receive fifo underflow interrupt mask*/
#define SPI_RX_FIFO_OVER_FLOW_EVENT                     (0x08)      /* Receive fifo Overflow interrupt mask*/
#define SPI_RX_FIFO_FULL_EVENT                          (0x10)      /* Receive fifo full interrupt mask*/
#define SPI_MULTI_MASTER_CONTENTION_EVENT               (0x20)      /* Multi-Master contention interrupt mask.*/

/**
 * enum SPI_MODE.
 * SPI modes with CPOL and CPHA.
 */
typedef enum _SPI_MODE
{
    SPI_MODE_0,                             /**< SPI Mode - 0 : CPHA = 0, CPOL = 0 */
    SPI_MODE_1,                             /**< SPI Mode - 1 : CPHA = 1, CPOL = 0 */
    SPI_MODE_2,                             /**< SPI Mode - 2 : CPHA = 0, CPOL = 1 */
    SPI_MODE_3                              /**< SPI Mode - 3 : CPHA = 1, CPOL = 1 */
} SPI_MODE;

/**
 * enum SPI_PROTO.
 * Serial protocols.
 */
typedef enum _SPI_PROTO
{
    SPI_PROTO_SPI,                          /**< Motorola SPI                       */
    SPI_PROTO_SSP,                          /**< Texas Instruments SSP              */
    SPI_PROTO_MICROWIRE                     /**< National Semiconductors Microwire  */
} SPI_PROTO;

/**
 * enum SPI_TMOD.
 * SPI transfer modes.
 */
typedef enum _SPI_TMOD
{
    SPI_TMOD_TX_AND_RX     = 0x00,         /**< Transmit and Receive    */
    SPI_TMOD_TX            = 0x01,         /*/mit only           */
    SPI_TMOD_RX            = 0x02,         /**< Receive only            */
    SPI_TMOD_EEPROM_READ   = 0x03          /**< EEPROM read             */
} SPI_TMOD;

/**
 * enum SPI_SS_STATE.
 * SPI Slave Select States.
 */
typedef enum _SPI_SS_STATE
{
    SPI_SS_STATE_DISABLE,                   /**< Slave select disabled  */
    SPI_SS_STATE_ENABLE,                    /**< Slave select Enabled   */
} SPI_SS_STATE;

/**
 * enum SPI_TRANSFER_STATUS.
 * Status of an ongoing SPI transfer.
 */
typedef enum _SPI_TRANSFER_STATUS
{
    SPI_TRANSFER_STATUS_NONE,               /**< Transfer status none               */
    SPI_TRANSFER_STATUS_COMPLETE,           /**< Transfer status complete           */
    SPI_TRANSFER_STATUS_OVERFLOW,           /**< Transfer status Tx/Rx overflow     */
    SPI_TRANSFER_STATUS_MASTER_CONTENTION,  /**< Transfer status master contention  */
    SPI_TRANSFER_STATUS_RX_UNDERFLOW,       /**< Transfer status Rx underflow       */
} SPI_TRANSFER_STATUS;

typedef struct _spi_transfer_t {
    volatile uint32_t               tx_current_cnt;     /**< Current Tx Transfer count        */
    volatile uint32_t               rx_current_cnt;     /**< Current Rx Transfer count        */
    uint32_t                        tx_total_cnt;       /**< Total count to Tx transfer       */
    uint32_t                        rx_total_cnt;       /**< Total count to Rx transfer       */
    const uint8_t                   *tx_buff;           /**< Pointer to TX buffer             */
    void                            *rx_buff;           /**< Pointer to Rx buffer             */
    uint32_t                        tx_default_val;     /**< Default value to Transfer        */
    bool                            tx_default_enable;  /**< Enable Tx default value transfer */
    bool                            is_master;          /**< SPI is master/slave              */
    SPI_TMOD                        mode;               /**< SPI transfer mode                */
    uint8_t                         frame_size;         /**< SPI Data frame size              */
    volatile SPI_TRANSFER_STATUS    status;             /**< transfer status                  */
} spi_transfer_t;

/**
  \fn          static inline void spi_disable(SPI_Type *spi)
  \brief       Disable the SPI instance
  \param[in]   spi     Pointer to the SPI register map
  \return      none
*/
static inline void spi_disable(SPI_Type *spi)
{
    spi->SPI_ENR = SPI_DISABLE;
}

/**
  \fn          static inline void spi_enable(SPI_Type *spi)
  \brief       Enable the SPI instance
  \param[in]   spi     Pointer to the SPI register map
  \return      none
*/
static inline void spi_enable(SPI_Type *spi)
{
    spi->SPI_ENR = SPI_ENABLE;
}

/**
  \fn          static inline void spi_mode_master(SPI_Type *spi)
  \brief       Enable master mode in the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \return      none
*/
static inline void spi_mode_master(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_CTRLR0 |= SPI_CTRLR0_SSI_IS_MST_MASTER;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_mode_slave(SPI_Type *spi)
  \brief       Enable slave mode in the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \return      none
*/
static inline void spi_mode_slave(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_CTRLR0 &= ~SPI_CTRLR0_SSI_IS_MST_MASTER;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_set_bus_speed(SPI_Type *spi, uint32_t speed, uint32_t clk)
  \brief       Set the bus speed for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   speed   The bus speed to be set
  \param[in]   clk     SPI input clk
  \return      none
*/
static inline void spi_set_bus_speed(SPI_Type *spi, uint32_t speed, uint32_t clk)
{
    spi_disable(spi);
    spi->SPI_BAUDR = (clk / speed);
    spi_enable(spi);
}

/**
  \fn          static inline uint32_t spi_get_bus_speed(SPI_Type *spi, uint32_t clk)
  \brief       Get the current bus speed of the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   clk     SPI input clk
  \return      Current bus speed
*/
static inline uint32_t spi_get_bus_speed(SPI_Type *spi, uint32_t clk)
{
    return clk / spi->SPI_BAUDR;
}

/**
  \fn          static inline void spi_mask_interrupts(SPI_Type *spi)
  \brief       Mask all the interrupts for the SPI instance
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_mask_interrupts(SPI_Type *spi)
{
    spi->SPI_IMR = 0;
}

/**
  \fn          static inline void spi_enable_rx_dma(SPI_Type *spi)
  \brief       Enable SPI RX DMA Block
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_enable_rx_dma(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_DMACR |= SPI_DMACR_RX_DMA_ENABLE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_disable_rx_dma(SPI_Type *spi)
  \brief       Enable SPI RX DMA Block
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_disable_rx_dma(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_DMACR &= ~SPI_DMACR_RX_DMA_ENABLE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_enable_tx_dma(SPI_Type *spi)
  \brief       Enable SPI TX DMA Block
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_enable_tx_dma(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_DMACR |= SPI_DMACR_TX_DMA_ENABLE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_disable_tx_dma(SPI_Type *spi)
  \brief       Enable SPI TX DMA Block
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_disable_tx_dma(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_DMACR &= ~SPI_DMACR_TX_DMA_ENABLE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_set_dma_rx_level(SPI_Type *spi, uint8_t data_level)
  \brief       Enable SPI RX DMA Block
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_set_dma_rx_level(SPI_Type *spi, uint8_t data_level)
{
    spi->SPI_DMARDLR = data_level;
}

/**
  \fn          static inline void spi_set_dma_tx_level(SPI_Type *spi, uint8_t data_level)
  \brief       Enable SPI TX DMA Block
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_set_dma_tx_level(SPI_Type *spi, uint8_t data_level)
{
    spi->SPI_DMATDLR = data_level;
}

/**
  \fn          volatile uint32_t* spi_get_data_addr(SPI_Type *spi)
  \brief       Return the Data reg Address
  \param[in]   spi   Pointer to SPI register map
  \return      \ref  Return the address
*/
static inline volatile uint32_t* spi_get_data_addr(SPI_Type *spi)
{
    return &spi->SPI_DR[0];
}

/**
  \fn          bool spi_busy(SPI_Type *spi)
  \brief       return spi busy status
  \param[in]   spi   Pointer to SPI register map
  \return      \ref  spi busy status
*/
static inline bool spi_busy(SPI_Type *spi)
{
    return (spi->SPI_SR & 1);
}

/**
  \fn          void spi_set_rx_sample_delay(SPI_Type *spi, uint8_t rx_sample_delay)
  \brief       Set Receive sample delay for the SPI instance
  \param[in]   spi              Pointer to the SPI register map
  \param[in]   rx_sample_delay  rx sample delay
  \return      none
*/
static inline void spi_set_rx_sample_delay(SPI_Type *spi, uint8_t rx_sample_delay)
{
    spi_disable(spi);
    spi->SPI_RX_SAMPLE_DELAY = rx_sample_delay;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_mw_enable_handshake(SPI_Type *spi)
  \brief       Enable microwire handshake feature
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_mw_enable_handshake(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_MWCR |= SPI_MWCR_MHS_ENABLE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_mw_disable_handshake(SPI_Type *spi)
  \brief       Disable microwire handshake feature
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_mw_disable_handshake(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_MWCR |= SPI_MWCR_MHS_DISABLE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_mw_set_sequential_mode(SPI_Type *spi)
  \brief       config microwire Sequential transfer mode
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_mw_set_sequential_mode(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_MWCR |= SPI_MWCR_MWMOD_SEQUENTIAL_MODE;
    spi_enable(spi);
}

/**
  \fn          static inline void spi_mw_set_non_sequential_mode(SPI_Type *spi)
  \brief       config microwire non-sequential transfer mode
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
static inline void spi_mw_set_non_sequential_mode(SPI_Type *spi)
{
    spi_disable(spi);
    spi->SPI_MWCR |= SPI_MWCR_MWMOD_NON_SEQUENTIAL_MODE;
    spi_enable(spi);
}

/**
  \fn          static inline void lpspi_mw_set_cfs(SPI_Type *lpspi, uint8_t cfs)
  \brief       set microwire control frame size for lpspi
  \param[in]   lpspi     Pointer to the SPI register map
  \param[in]   cfs       control frame size
  \return      none
*/
static inline void lpspi_mw_set_cfs(SPI_Type *lpspi, uint8_t cfs)
{
    spi_disable(lpspi);
    lpspi->SPI_CTRLR0 |= ((cfs - 1) << LPSPI_CTRLR0_CFS);
    spi_enable(lpspi);
}

/**
  \fn          static inline void spi_mw_set_cfs(SPI_Type *spi, uint8_t cfs)
  \brief       set microwire control frame size for spi
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   cfs       control frame size
  \return      none
*/
static inline void spi_mw_set_cfs(SPI_Type *spi, uint8_t cfs)
{
    spi_disable(spi);
    spi->SPI_CTRLR0 |= ((cfs - 1) << SPI_CTRLR0_CFS);
    spi_enable(spi);
}

/**
  \fn          void spi_set_mode(SPI_Type *spi, SPI_MODE mode)
  \brief       Set the SPI mode for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   mode    The mode to be set.
  \return      none
*/
void spi_set_mode(SPI_Type *spi, SPI_MODE mode);

/**
  \fn          void spi_set_protocol(SPI_Type *spi, SPI_PROTO format)
  \brief       Set the protocol format for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   format  The protocol to be set
  \return      none
*/
void spi_set_protocol(SPI_Type *spi, SPI_PROTO format);

/**
  \fn          void spi_set_dfs(SPI_Type *spi, uint8_t dfs)
  \brief       Set the data frame size for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   dfs     The data frame size
  \return      none
*/
void spi_set_dfs(SPI_Type *spi, uint8_t dfs);

/**
  \fn          void spi_set_tmode(SPI_Type *spi, SPI_TMOD tmod)
  \brief       Set the transfer mode for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   tmod    Transfer mode
  \return      none
*/
void spi_set_tmod(SPI_Type *spi, SPI_TMOD tmod);

/**
  \fn          SPI_TMOD spi_get_tmod(SPI_Type *spi)
  \brief       Get the transfer mode of the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \return      The current transfer mode
*/
SPI_TMOD spi_get_tmod(SPI_Type *spi);

/**
  \fn          void spi_set_sste(SPI_Type *spi, bool enable)
  \brief       Enable/Disable Slave Select Toggle for the SPI instance
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   enable    Enable/Disable control
  \return      none
*/
void spi_set_sste(SPI_Type *spi, bool enable);

/**
  \fn          void spi_set_tx_threshold(SPI_Type *spi, uint8_t threshold)
  \brief       Set Transmit FIFO interrupt threshold for the SPI instance
  \param[in]   spi        Pointer to the SPI register map
  \param[in]   threshold  Transmit FIFO threshold
  \return      none
*/
void spi_set_tx_threshold(SPI_Type *spi, uint8_t threshold);

/**
  \fn          void spi_set_rx_threshold(SPI_Type *spi, uint8_t threshold)
  \brief       Set Receive FIFO interrupt threshold for the SPI instance
  \param[in]   spi        Pointer to the SPI register map
  \param[in]   threshold  Receive FIFO threshold
  \return      none
*/
void spi_set_rx_threshold(SPI_Type *spi, uint8_t threshold);

/**
  \fn          void spi_set_tx_fifo_start_level(SPI_Type *spi, uint16_t level)
  \brief       Set Transmit FIFO start level
  \param[in]   spi    Pointer to the SPI register map
  \param[in]   level  Transmit FIFO start level
  \return      none
*/
void spi_set_tx_fifo_start_level(SPI_Type *spi, uint16_t level);

/**
  \fn          void spi_control_ss(SPI_Type *spi, uint8_t slave, SPI_SS_STATE state)
  \brief       Control the slave select line
  \param[in]   spi    Pointer to the SPI register map
  \param[in]   slave  The slave to be selected
  \param[in]   state  The state of the slave select line
  \return      none
*/
void spi_control_ss(SPI_Type *spi, uint8_t slave, SPI_SS_STATE state);

/**
  \fn          void spi_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for transmission
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_send(SPI_Type *spi);

/**
  \fn          void spi_receive(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Prepare the SPI instance for reception
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_receive(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          void spi_transfer(SPI_Type *spi)
  \brief       Prepare the SPI instance for transfer
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_transfer(SPI_Type *spi);

/**
  \fn          void spi_send_blocking(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI send described by the transfer structure.
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to the SPI transfer structure
  \return      none
*/
void spi_send_blocking(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          void spi_receive_blocking(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI receive described by the transfer structure.
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to the SPI transfer structure
  \return      none
*/
void spi_receive_blocking(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          void spi_transfer_blocking(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI transfer described by the transfer structure.
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to the SPI transfer structure
  \return      none
*/
void spi_transfer_blocking(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          void lpspi_send_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI receive described by the transfer structure.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void lpspi_send_blocking(SPI_Type *lpspi, spi_transfer_t *transfer);

/**
  \fn          void lpspi_receive_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI send described by the transfer structure.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void lpspi_receive_blocking(SPI_Type *lpspi, spi_transfer_t *transfer);

/**
  \fn          void lpspi_transfer_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI transfer described by the transfer structure.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void lpspi_transfer_blocking(SPI_Type *lpspi, spi_transfer_t *transfer);

/**
  \fn          void lpspi_set_mode(SPI_Type *spi, SPI_MODE mode)
  \brief       Set the mode for the LPSPI instance.
  \param[in]   lpspi   Pointer to the LPSPI register map
  \param[in]   mode    The mode to be set.
  \return      none
*/
void lpspi_set_mode(SPI_Type *lpspi, SPI_MODE mode);

/**
  \fn          void lpspi_set_protocol(SPI_Type *lpspi, SPI_PROTO format)
  \brief       Set the protocol format for the LPSPI instance.
  \param[in]   lpspi   Pointer to the LPSPI register map
  \param[in]   format  The protocol to be set
  \return      none
*/
void lpspi_set_protocol(SPI_Type *lpspi, SPI_PROTO format);

/**
  \fn          void lpspi_set_dfs(SPI_Type *lpspi, uint8_t dfs)
  \brief       Set the data frame size for the LPSPI instance.
  \param[in]   lpspi   Pointer to the LPSPI register map
  \param[in]   dfs     The data frame size
  \return      none
*/
void lpspi_set_dfs(SPI_Type *lpspi, uint8_t dfs);

/**
  \fn          void lpspi_set_tmod(SPI_Type *lpspi, SPI_TMOD tmod)
  \brief       Set the transfer mode for the LPSPI instance.
  \param[in]   lpspi   Pointer to the LPSPI register map
  \param[in]   tmod    Transfer mode
  \return      none
*/
void lpspi_set_tmod(SPI_Type *lpspi, SPI_TMOD tmod);

/**
  \fn          SPI_TMOD lpspi_get_tmod(SPI_Type *lpspi)
  \brief       Get the transfer mode of the LPSPI instance.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \return      The current transfer mode
*/
SPI_TMOD lpspi_get_tmod(SPI_Type *lpspi);

/**
  \fn          void lpspi_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for transmission
  \param[in]   lpspi       Pointer to the LPSPI register map
  \return      none
*/
void lpspi_send(SPI_Type *lpspi);

/**
  \fn          void lpspi_receive(SPI_Type *lpspi, uint32_t total_cnt)
  \brief       Prepare the LPSPI instance for reception
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   total_cnt total number of data count
  \return      none
*/
void lpspi_receive(SPI_Type *lpspi, uint32_t total_cnt);

/**
  \fn          void lpspi_transfer(SPI_Type *lpspi)
  \brief       Prepare the LPSPI instance for transfer
  \param[in]   lpspi      Pointer to the LPSPI register map
  \return      none
*/
void lpspi_transfer(SPI_Type *lpspi);

/**
  \fn          void lpspi_set_sste(SPI_Type *lpspi, bool enable)
  \brief       Enable/Disable Slave Select Toggle for the LPSPI instance
  \param[in]   lpspi     Pointer to the SPI register map
  \param[in]   enable    Enable/Disable control
  \return      none
*/
void lpspi_set_sste(SPI_Type *lpspi, bool enable);

/**
  \fn          void spi_mw_transmit(SPI_Type *spi, bool is_slave)
  \brief       config microwire in transmit mode
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   is_slave  whether config as master/slave
  \return      none
*/
void spi_mw_transmit(SPI_Type *spi, bool is_slave);

/**
  \fn          void spi_mw_receive(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       config microwire in receive mode
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  pointer to transfer structure
  \return      none
*/
void spi_mw_receive(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          uint32_t spi_dma_calc_rx_level(uint32_t total_cnt, uint8_t fifo_threshold)
  \brief       Calculate SPI DMA receive data level
  \param[in]   fifo_threshold  receive fifo threshold value
  \param[in]   total_cnt  total number of data count
  \return      final value after calculation
*/
uint32_t spi_dma_calc_rx_level(uint32_t total_cnt, uint8_t fifo_threshold);

/**
  \fn          void spi_dma_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for DMA send
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_dma_send(SPI_Type *spi);

/**
  \fn          void spi_dma_receive(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Prepare the SPI instance for DMA reception
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_dma_receive(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          void spi_dma_transfer(SPI_Type *spi)
  \brief       Prepare the SPI instance for DMA transfer
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_dma_transfer(SPI_Type *spi);

/**
  \fn          void lpspi_dma_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for DMA transmission
  \param[in]   lpspi       Pointer to the LPSPI register map
  \return      none
*/
void lpspi_dma_send(SPI_Type *lpspi);

/**
  \fn          void lpspi_dma_receive(SPI_Type *lpspi, uint32_t total_cnt)
  \brief       Prepare the LPSPI instance for DMA reception
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   total_cnt total number of data count
  \return      none
*/
void lpspi_dma_receive(SPI_Type *lpspi, uint32_t total_cnt);

/**
  \fn          void lpspi_dma_transfer(SPI_Type *lpspi)
  \brief       Prepare the LPSPI instance for DMA transfer
  \param[in]   lpspi      Pointer to the LPSPI register map
  \return      none
*/
void lpspi_dma_transfer(SPI_Type *lpspi);

/**
  \fn          void spi_irq_handler(SPI_Type *spi, spi_master_transfer_t *transfer)
  \brief       Handle interrupts for the SPI instance.
  \param[in]   spi         Pointer to the SPI register map
  \param[in]   transfer    The transfer structure for the SPI instance
  \return      none
*/
void spi_irq_handler(SPI_Type *spi, spi_transfer_t *transfer);

/**
  \fn          void spi_mw_irq_handler(SPI_Type *spi, spi_master_transfer_t *transfer)
  \brief       Handle interrupts for the MW frame format.
  \param[in]   spi         Pointer to the SPI register map
  \param[in]   transfer    The transfer structure for the SPI instance
  \return      none
*/
void spi_mw_irq_handler(SPI_Type *spi, spi_transfer_t *transfer);
#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */
