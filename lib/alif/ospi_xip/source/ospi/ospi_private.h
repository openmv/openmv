/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     dwc_ospi_private.h
 * @version  V1.0.0
 * @brief    Private header file for OSPI driver to set up flash in XIP mode.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef OSPI_PRIVATE_H
#define OSPI_PRIVATE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct
{
    volatile uint32_t ctrlr0;           /* SPI Control Register 0  (0x0) */
    volatile uint32_t ctrlr1;           /* SPI Control Register 1  (0x4) */
    volatile uint32_t ssienr;           /* SPI Enable Register  (0x8) */
    volatile uint32_t mwcr;             /* SPI Microwire Control Register  (0xC) */
    volatile uint32_t ser;              /* SPI Slave Enable Register  (0x10) */
    volatile uint32_t baudr;            /* SPI Baud Rate Select Register  (0x14) */
    volatile uint32_t txftlr;           /* SPI Transmit FIFO Threshold Level Register (0x18) */
    volatile uint32_t rxftlr;           /* SPI Receive  FIFO Threshold Level Register (0x1C) */
    volatile uint32_t txflr;            /* SPI Transmit FIFO Level Register  (0x20)*/
    volatile uint32_t rxflr;            /* SPI Receive  FIFO Level Register  (0x24)*/
    volatile uint32_t sr;               /* SPI Status   Register  (0x28) */
    volatile uint32_t imr;              /* SPI Interrupt Mask Register  (0x2C) */
    volatile uint32_t isr;              /* SPI Interrupt Status Register  (0x30) */
    volatile uint32_t risr;             /* SPI Raw Interrupt Status Register (0x34)*/
    volatile uint32_t txoicr;           /* SPI Transmit FIFO Overflow Interrupt Clear Register  (0x38) */
    volatile uint32_t rxoicr;           /* SPI Receive  FIFO Overflow Interrupt Clear Register  (0x3C) */
    volatile uint32_t rxuicr;           /* SPI Receive FIFO Underflow Interrupt Clear Register  (0x40) */
    volatile uint32_t msticr;           /* SPI Multi-Master Interrupt Clear Register (0x44) */
    volatile uint32_t icr;              /* SPI Interrupt Clear Register  (0x48) */
    volatile uint32_t dmacr;            /* DMA Control Register  (0x4C) */
    volatile uint32_t dmatxdlr;         /* DMA Transmit Data Level  (0x50) */
    volatile uint32_t dmarxdlr;         /* DMA Receive Data Level  (0x54) */
    volatile uint32_t spi_idr;          /* SPI Identification Register  (0x58) */
    volatile uint32_t spi_ver_id;       /* Synopsys component version (0x5C) */
    volatile uint32_t data_reg;         /* SPI DATA Register for both Read and Write  (0x60) */
    volatile uint32_t drs[35];          /* SPI DATA Register for both Read and Write  (0x64-0xEC) */
    volatile uint32_t rx_sample_dly;    /* Rx Sample Delay Register (0xF0) */
    volatile uint32_t spi_ctrlr0;       /* SPI Control Register (0xF4) */
    volatile uint32_t txd_drive_edge;   /* Transmit Drive Edge Register (0xF8) */
    volatile uint32_t xip_mode_bits;    /* eXecute in Place - Mode bits (0xFC) */
    volatile uint32_t xip_incr_inst;
    volatile uint32_t xip_wrap_inst;
    volatile uint32_t xip_ctrl;
    volatile uint32_t xip_ser;
    volatile uint32_t xrxoicr;
    volatile uint32_t xip_cnt_time_out;
} ssi_regs_t;

typedef struct {
    volatile uint32_t  aes_control;                  /*!< (@ 0x00000000) AES Control Register                                       */
    volatile uint32_t  aes_interrupt;                /*!< (@ 0x00000004) AES Interrupt Control Register                             */
    volatile uint32_t  aes_interrupt_mask;           /*!< (@ 0x00000008) AES Interrupt Mask Register                                */
    volatile uint32_t  aes_key_0;                    /*!< (@ 0x0000000C) AES Key 0 Register                                         */
    volatile uint32_t  aes_key_1;                    /*!< (@ 0x00000010) AES Key 1 Register                                         */
    volatile uint32_t  aes_key_2;                    /*!< (@ 0x00000014) AES Key 2 Register                                         */
    volatile uint32_t  aes_key_3;                    /*!< (@ 0x00000018) AES Key 3 Register                                         */
    volatile uint32_t  aes_timeout_val;              /*!< (@ 0x0000001C) Reserved                                                   */
    volatile uint32_t  aes_rxds_delay;               /*!< (@ 0x00000020) AES RXDS Delay Register                                    */
} aes_regs_t;

/* OSPI Address Map */
#ifndef OSPI0_BASE
#define OSPI0_BASE                                      0x83000000UL
#endif
#ifndef OSPI1_BASE
#define OSPI1_BASE                                      0x83002000UL
#endif
#ifndef OSPI0_XIP_BASE
#define OSPI0_XIP_BASE                                  0xA0000000UL
#endif
#ifndef OSPI1_XIP_BASE
#define OSPI1_XIP_BASE                                  0xC0000000UL
#endif
#ifndef OSPI0_SIZE
#define OSPI0_SIZE                                      0x20000000UL
#endif
#ifndef OSPI1_SIZE
#define OSPI1_SIZE                                      0x20000000UL
#endif

/* APB-E Peripherals */
#ifndef AES0_BASE
#define AES0_BASE                                       0x83001000UL
#endif
#ifndef AES1_BASE
#define AES1_BASE                                       0x83003000UL
#endif

/* Bit fields in CTRLR0 */
#define CTRLR0_SPI_HE_OFFSET                            24U
#define CTRLR0_SPI_FRF_OFFSET                           22U
#define CTRLR0_CFS_OFFSET                               16U
#define CTRLR0_SSTE_OFFSET                              14U
#define CTRLR0_SRL_OFFSET                               13U
#define CTRLR0_SLV_OE_OFFSET                            12U
#define CTRLR0_TMOD_OFFSET                              10U
#define CTRLR0_TMOD_MASK                                (3U << CTRLR0_TMOD_OFFSET)
#define CTRLR0_SCPOL_OFFSET                             9U
#define CTRLR0_SCPH_OFFSET                              8U
#define CTRLR0_FRF_OFFSET                               6U
#define CTRLR0_DFS_OFFSET                               0U

#define CTRLR0_IS_MST                                   (1U << 31)

#define	SPI_TMOD_TR                                     0x0
#define TMOD_TO                                         0x1
#define TMOD_RO                                         0x2
#define TMOD_EPROMREAD                                  0x3

/* Bit fields for SPI FRF */
#define SINGLE                                          0x0
#define DUAL                                            0x1
#define QUAD                                            0x2
#define OCTAL                                           0x3

/* Instruction length */
#define CTRLR0_INST_L_0bit                              0x0U
#define CTRLR0_INST_L_4bit                              0x1U
#define CTRLR0_INST_L_8bit                              0x2U
#define CTRLR0_INST_L_16bit                             0x3U

/* Data frame length */
#define CTRLR0_DFS_8bit                                 0x07U
#define CTRLR0_DFS_16bit                                0x0FU
#define CTRLR0_DFS_32bit                                0x1FU

#define ADDR_L32bit                                     0x8
#define INST_L8bit                                      0x2

/* Bit fields for Frame Format FRF */
#define FRF_SPI                                         0x0
#define FRF_SSP                                         0x1
#define FRF_MICROWIRE                                   0x2

/* Bit fields in SR, 7 bits */
#define SR_MASK                                         0x7F
#define SR_BUSY                                         (1U << 0)
#define SR_TF_NOT_FULL                                  (1U << 1)
#define SR_TF_EMPTY                                     (1U << 2)
#define SR_RF_NOT_EMPT                                  (1U << 3)
#define SR_RF_FULL                                      (1U << 4)
#define SR_TX_ERR                                       (1U << 5)
#define SR_DCOL                                         (1U << 6)

/* Bit fields in CTRLR0 */
#define CTRLR0_CLK_STRETCH_EN_OFFSET                    30U
#define CTRLR0_XIP_PREFETCH_EN_OFFSET                   29U
#define CTRLR0_XIP_MBL_OFFSET                           26U
#define CTRLR0_SPI_RXDS_SIG_EN_OFFSET                   25U
#define CTRLR0_SPI_DM_EN_OFFSET                         24U
#define CTRLR0_XIP_CONT_EN_OFFSET                       21U
#define CTRLR0_XIP_INST_EN_OFFSET                       20U
#define CTRLR0_XIP_DFS_HC_OFFSET                        19U
#define CTRLR0_SPI_RXDS_EN_OFFSET                       18U
#define CTRLR0_INST_DDR_EN_OFFSET                       17U
#define CTRLR0_SPI_DDR_EN_OFFSET                        16U
#define CTRLR0_WAIT_CYCLES_OFFSET                       11U
#define CTRLR0_INST_L_OFFSET                            8U
#define CTRLR0_XIP_MD_EN_OFFSET                         7U
#define CTRLR0_ADDR_L_OFFSET                            2U
#define CTRLR0_TRANS_TYPE_OFFSET                        0U

#define XIP_CTRL_RXDS_VL_EN_OFFSET                      30U
#define XIP_PREFETCH_EN_OFFSET                          29U
#define XIP_CTRL_XIP_MBL_OFFSET                         26U
#define XIP_CTRL_RXDS_SIG_EN                            25U
#define XIP_CTRL_HYPERBUS_EN_OFFSET                     24U
#define XIP_CTRL_CONT_XFER_EN_OFFSET                    23U
#define XIP_CTRL_INST_EN_OFFSET                         22U
#define XIP_CTRL_RXDS_EN_OFFSET                         21U
#define XIP_CTRL_INST_DDR_EN_OFFSET                     20U
#define XIP_CTRL_DDR_EN_OFFSET                          19U
#define XIP_CTRL_DFC_HC_OFFSET                          18U
#define XIP_CTRL_WAIT_CYCLES_OFFSET                     13U
#define XIP_CTRL_MD_BITS_EN_OFFSET                      12U
#define XIP_CTRL_INST_L_OFFSET                          9U
#define XIP_CTRL_ADDR_L_OFFSET                          4U
#define XIP_CTRL_TRANS_TYPE_OFFSET                      2U
#define XIP_CTRL_FRF_OFFSET                             0U

#define CTRLR0_TRANS_TYPE_MASK                          3U
#define TRANS_TYPE_STANDARD                             0U
#define TRANS_TYPE_FRF_DEFINED                          2U

/* AES_CONTROL fields */
#define AES_CONTROL_LD_KEY                              (1U << 7)
#define AES_CONTROL_XIP_EN                              (1U << 4)
#define AES_CONTROL_DECRYPT_EN                          (1U << 0)

#define ospi_readl(a, r)                (a->regs->r)
#define ospi_writel(a, r, v)            a->regs->r = (v)

#define spi_enable(cfg)                 ospi_writel(cfg, ssienr, 1)
#define spi_disable(cfg)                ospi_writel(cfg, ssienr, 0)
#define spi_set_clk(cfg, div)           ospi_writel(cfg, baudr, div)

#ifdef  __cplusplus
}
#endif

#endif /* OSPI_PRIVATE_H */
