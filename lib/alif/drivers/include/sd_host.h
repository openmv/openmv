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
 * @file     sd_host.h
 * @author   Deepak Kumar
 * @email    deepak@alifsemi.com
 * @version  V0.0.1
 * @date     09-June-2023
 * @brief    SD Host Controller Register mapping.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef __SD_HOST_H__
#define __SD_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "stdint.h"

/* =========================================================================================================================== */
/* ================                                           SDMMC                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief SDMMC (SDMMC)
  */

typedef struct _SDMMC_Type {                              /*!< (SDMMC Structure                                                          */
  volatile uint32_t         SDMMC_SDMASA_R;               /*!< (@ 0x00000000) SDMA System Address Register                               */
  volatile uint16_t         SDMMC_BLOCKSIZE_R;            /*!< (@ 0x00000004) Block Size Register                                        */
  volatile uint16_t         SDMMC_BLOCKCOUNT_R;           /*!< (@ 0x00000006) 16-bit Block Count Register                                */
  volatile uint32_t         SDMMC_ARGUMENT_R;             /*!< (@ 0x00000008) Argument Register                                          */
  volatile uint16_t         SDMMC_XFER_MODE_R;            /*!< (@ 0x0000000C) Transfer Mode Register                                     */
  volatile uint16_t         SDMMC_CMD_R;                  /*!< (@ 0x0000000E) Command Register                                           */
  volatile const uint32_t   SDMMC_RESP01_R;               /*!< (@ 0x00000010) Response Register 0/1                                      */
  volatile const uint32_t   SDMMC_RESP23_R;               /*!< (@ 0x00000014) Response Register 2/3                                      */
  volatile const uint32_t   SDMMC_RESP45_R;               /*!< (@ 0x00000018) Response Register 4/5                                      */
  volatile const uint32_t   SDMMC_RESP67_R;               /*!< (@ 0x0000001C) Response Register 6/7                                      */
  volatile uint32_t         SDMMC_BUF_DATA_R;             /*!< (@ 0x00000020) Buffer Data Port Register                                  */
  volatile const uint32_t   SDMMC_PSTATE_REG;             /*!< (@ 0x00000024) Present State Register                                     */
  volatile uint8_t          SDMMC_HOST_CTRL1_R;           /*!< (@ 0x00000028) Host Control 1 Register                                    */
  volatile uint8_t          SDMMC_PWR_CTRL_R;             /*!< (@ 0x00000029) Power Control Register                                     */
  volatile uint8_t          SDMMC_BGAP_CTRL_R;            /*!< (@ 0x0000002A) Block Gap Control Register                                 */
  volatile uint8_t          SDMMC_WUP_CTRL_R;             /*!< (@ 0x0000002B) Wakeup Control Register                                    */
  volatile uint16_t         SDMMC_CLK_CTRL_R;             /*!< (@ 0x0000002C) Clock Control Register                                     */
  volatile uint8_t          SDMMC_TOUT_CTRL_R;            /*!< (@ 0x0000002E) Timeout Control Register                                   */
  volatile uint8_t          SDMMC_SW_RST_R;               /*!< (@ 0x0000002F) Software Reset Register                                    */
  volatile uint16_t         SDMMC_NORMAL_INT_STAT_R;      /*!< (@ 0x00000030) Normal Interrupt Status Register                           */
  volatile uint16_t         SDMMC_ERROR_INT_STAT_R;       /*!< (@ 0x00000032) Error Interrupt Status Register                            */
  volatile uint16_t         SDMMC_NORMAL_INT_STAT_EN_R;   /*!< (@ 0x00000034) Normal Interrupt Status Enable Register                    */
  volatile uint16_t         SDMMC_ERROR_INT_STAT_EN_R;    /*!< (@ 0x00000036) Error Interrupt Status Enable Register                     */
  volatile uint16_t         SDMMC_NORMAL_INT_SIGNAL_EN_R; /*!< (@ 0x00000038) Normal Interrupt Signal Enable Register                    */
  volatile uint16_t         SDMMC_ERROR_INT_SIGNAL_EN_R;  /*!< (@ 0x0000003A) Error Interrupt Signal Enable Register                     */
  volatile const uint16_t   SDMMC_AUTO_CMD_STAT_R;        /*!< (@ 0x0000003C) Auto CMD Status Register                                   */
  volatile uint16_t         SDMMC_HOST_CTRL2_R;           /*!< (@ 0x0000003E) Host Control 2 Register                                    */
  volatile const uint32_t   SDMMC_CAPABILITIES1_R;        /*!< (@ 0x00000040) Capabilities 1 Register (0 to 31)                          */
  volatile const uint32_t   SDMMC_CAPABILITIES2_R;        /*!< (@ 0x00000044) Capabilities 2 Register (32 to 63)                         */
  volatile const uint32_t   SDMMC_CURR_CAPABILITIES1_R;   /*!< (@ 0x00000048) Maximum Current Capabilities 1 Register (0 to
                                                                              31)                                                        */
  volatile const uint32_t   RESERVED;
  volatile uint16_t         SDMMC_FORCE_AUTO_CMD_STAT_R;  /*!< (@ 0x00000050) Force Event Register for Auto CMD Error Status             */
  volatile uint16_t         SDMMC_FORCE_ERROR_INT_STAT_R; /*!< (@ 0x00000052) Force Event Register for Error Interrupt Status            */
  volatile const uint8_t    SDMMC_ADMA_ERR_STAT_R;        /*!< (@ 0x00000054) ADMA Error Status Register                                 */
  volatile const uint8_t    RESERVED1;
  volatile const uint16_t   RESERVED2;
  volatile uint32_t         SDMMC_ADMA_SA_LOW_R;          /*!< (@ 0x00000058) ADMA System Address Register (Low)                         */
  volatile const uint32_t   RESERVED3;
  volatile const uint16_t   SDMMC_PRESET_INIT_R;          /*!< (@ 0x00000060) Register with Preset Value for Initialization              */
  volatile const uint16_t   SDMMC_PRESET_DS_R;            /*!< (@ 0x00000062) Register with Preset Value for Default Speed               */
  volatile const uint16_t   SDMMC_PRESET_HS_R;            /*!< (@ 0x00000064) Register with Preset Value for High Speed                  */
  volatile const uint16_t   SDMMC_PRESET_SDR12_R;         /*!< (@ 0x00000066) Register with Preset Value for SDR12                       */
  volatile const uint16_t   SDMMC_PRESET_SDR25_R;         /*!< (@ 0x00000068) Register with Preset Value for SDR25                       */
  volatile const uint16_t   SDMMC_PRESET_SDR50_R;         /*!< (@ 0x0000006A) Register with Preset Value for SDR50                       */
  volatile const uint32_t   RESERVED4[3];
  volatile uint32_t         SDMMC_ADMA_ID_LOW_R;          /*!< (@ 0x00000078) ADMA3 Integrated Descriptor Address Register
                                                                              (Low)                                                      */
  volatile const uint32_t   RESERVED5[27];
  volatile const uint16_t   SDMMC_P_VENDOR_SPECIFIC_AREA; /*!< (@ 0x000000E8) Pointer for Vendor Specific Area 1                         */
  volatile const uint16_t   RESERVED6;
  volatile const uint32_t   RESERVED7[4];
  volatile const uint16_t   SDMMC_SLOT_INTR_STATUS_R;     /*!< (@ 0x000000FC) Slot Interrupt Status Register                             */
  volatile const uint16_t   SDMMC_HOST_CNTRL_VERS_R;      /*!< (@ 0x000000FE) Reserved                                                   */
  volatile const uint32_t   RESERVED8[256];
  volatile const uint32_t   SDMMC_MSHC_VER_ID_R;          /*!< (@ 0x00000500) Reserved                                                   */
  volatile const uint32_t   SDMMC_MSHC_VER_TYPE_R;        /*!< (@ 0x00000504) Reserved                                                   */
  volatile uint8_t          SDMMC_MSHC_CTRL_R;            /*!< (@ 0x00000508) SDMMC Host Controller Control Register                     */
  volatile const uint8_t    RESERVED9;
  volatile const uint16_t   RESERVED10;
  volatile const uint32_t   RESERVED11;
  volatile uint8_t          SDMMC_MBIU_CTRL_R;            /*!< (@ 0x00000510) Master Bus Interface Unit Control Register                 */
  volatile const uint8_t    RESERVED12;
  volatile const uint16_t   RESERVED13;
  volatile const uint32_t   RESERVED14[6];
  volatile uint16_t         SDMMC_EMMC_CTRL_R;            /*!< (@ 0x0000052C) eMMC Control Register                                      */
  volatile uint16_t         SDMMC_BOOT_CTRL_R;            /*!< (@ 0x0000052E) eMMC Boot Control Register                                 */
  volatile const  uint32_t  RESERVED15[655];
  volatile uint32_t         SDMMC_EMBEDDED_CTRL_R;        /*!< (@ 0x00000F6C) Embedded Control Register                                  */
} SDMMC_Type;                                             /*!< Size = 3952 (0xf70)                                                       */

#define SDMMC                                   ((SDMMC_Type *) SDMMC_BASE)
#define SDMMC_HC_VERSION_REG                    (SDMMC_BASE + 0xFEU)
#define SDMMC_HC_VERSION_REG_Msk                0xFFFFU
#define SDMMC_IRQ_NUM                           SDMMC_IRQ_IRQn
#define SDMMC_WAKEUP_IRQ_NUM                    SDMMC_WAKEUP_IRQ_IRQn

/**
 * @brief  Host controller driver status enum definition
 */
typedef enum _SDMMC_HC_STATUS{
    SDMMC_HC_STATUS_OK,
    SDMMC_HC_STATUS_ERR,
    SDMMC_HC_STATUS_INV_STATE
}SDMMC_HC_STATUS;

/**
 * @brief ADMA 32-Bit descriptor table
 */
typedef struct _adma2_desc_t{
    uint16_t attr;
    uint16_t len;
    uint32_t addr;
}__attribute__((__packed__))adma2_desc_t;

/* ADMA Descriptor Constant */
#define SDMMC_ADMA2_DESC_MAX_LEN                65536U
#define SDMMC_ADMA2_DESC_VALID                  (0x1U << 0U)
#define SDMMC_ADMA2_DESC_END                    (0x1U << 1U)
#define SDMMC_ADMA2_DESC_INT                    (0x1U << 2U)
#define SDMMC_ADMA2_DESC_TRAN                   (0x1U << 5U)

/* SDMMC Device ID Constnat */
#define SDMMC_DEV_ID                            1U

/* Host Controller Specific Constant */
#define SDMMC_HC_SPEC_V3                        0x0002U /**< HC spec version 3 */
#define SDMMC_HC_SPEC_V2                        0x0001U /**< HC spec version 2 */
#define SDMMC_HC_SPEC_V1                        0x0000U /**< HC spec version 1 */
#define SDMMC_HC_SPEC_VER_Msk                   0x00FFU /**< Host Specification version mask */

/* Software Reset Register */
#define SDMMC_SW_RST_ALL_Pos                    0U
#define SDMMC_SW_RST_ALL_Msk                    (1U << SDMMC_SW_RST_ALL_Pos)
#define SDMMC_SW_RST_CMD_Pos                    1U
#define SDMMC_SW_RST_CMD_Msk                    (1U << SDMMC_SW_RST_CMD_Pos)
#define SDMMC_SW_RST_DAT_Pos                    2U
#define SDMMC_SW_RST_DAT_Msk                    (1U << SDMMC_SW_RST_DAT_Pos)

/* CMD and Response register mapping */
#define SDMMC_CMD_IDX_Pos                       8U
#define SDMMC_CMD_TYPE_Pos                      6U
#define SDMMC_CMD_RSP_SEL_Pos                   0U

/* SD Clk freq */
#define SDMMC_CLK_400_KHZ                       400000U         /*!< 400KHz */
#define SDMMC_CLK_25_MHZ                        25000000U       /*!< 25MHz  */
#define SDMMC_CLK_50_MHz                        50000000U       /*!< 50MHz  */
#define SDMMC_CLK_100_MHZ                       100000000U      /*!< 100MHz */
#define SDMMC_BASE_CLK                          SD_CLK_100_MHZ

/* Supported Data Bus */
#define SDMMC_1_BIT_MODE                        0x0U
#define SDMMC_4_BIT_MODE                        0x1U
#define SDMMC_8_BIT_MODE                        0x2U
#define SDMMC_1_BIT_WIDTH_Msk                   0x1U    /*!< Bus Width 1 */
#define SDMMC_4_BIT_WIDTH_Msk                   0x2U    /*!< Bus Width 4 */
#define SDMMC_8_BIT_WIDTH_Msk                   0x20U   /*!< Bus Width 8 */

/* Card Types */
#define SDMMC_CARD_SDSC                         1U      /*!< SDSC, Ver 1 Cards  */
#define SDMMC_CARD_SDHC                         2U      /*!< SDHC, SDXC Ver 2    */
#define SDMMC_CARD_SDXC                         3U      /*!< SDXC               */
#define SDMMC_CARD_MMC                          4U      /*!< MMC Cards          */
#define SDMMC_CARD_SDIO                         5U      /*!< SDIO Cards         */
#define SDMMC_CARD_COMBO                        6U      /*!< SD Combo Cards     */

/* Command index */
#define APP_CMD_PREFIX                          0x80U    /* App CMD prefix       */
#define CMD0                                    0x00U
#define CMD1                                    0x01U
#define CMD2                                    0x02U
#define CMD3                                    0x03U
#define CMD4                                    0x04U
#define CMD5                                    0x05U
#define CMD6                                    0x06U
#define ACMD6                                   (APP_CMD_PREFIX + 0x06U)
#define CMD7                                    0x07U
#define CMD8                                    0x08U
#define CMD9                                    0x09U
#define CMD10                                   0x0AU
#define CMD11                                   0x0BU
#define CMD12                                   0x0CU
#define CMD13                                   0x0DU
#define ACMD13                                  (APP_CMD_PREFIX + 0x0DU)
#define CMD16                                   0x10U
#define CMD17                                   0x11U
#define CMD18                                   0x12U
#define CMD19                                   0x13U
#define CMD21                                   0x15U
#define CMD23                                   0x17U
#define ACMD23                                  (APP_CMD_PREFIX + 0x17U)
#define CMD24                                   0x18U
#define CMD25                                   0x19U
#define CMD32                                   0x20U
#define CMD33                                   0x21U
#define CMD35                                   0x23U
#define CMD36                                   0x24U
#define CMD38                                   0x26U
#define CMD41                                   0x29U
#define ACMD41                                  (APP_CMD_PREFIX + 0x29U)
#define ACMD42                                  (APP_CMD_PREFIX + 0x2AU)
#define ACMD51                                  (APP_CMD_PREFIX + 0x33U)
#define CMD51                                   0x33U
#define CMD52                                   0x34U
#define CMD53                                   0x35U
#define CMD55                                   0x37U
#define CMD58                                   0x3AU

/* CMD_R */
#define SDMMC_CMD_R_CRC_CHK_EN_Msk              8U
#define SDMMC_CMD_R_CMD_IDX_CHK_EN_Msk          0x10U
#define SDMMC_CMD_R_DATA_PRES_SEL_Pos           5U
#define SDMMC_CMD_R_DATA_PRES_SEL_Msk           (1U << SDMMC_CMD_R_DATA_PRES_SEL_Pos)

/* Response type */
#define SDMMC_RESP_NONE                         0U   /*!< No response expected     */
#define SDMMC_RESP_R136                         1U   /*!< 128Bit response expected */
#define SDMMC_RESP_R48                          2U   /*!< Single response expected */
#define SDMMC_RESP_R48B                         3U   /*!< check busy after resp    */

#define SDMMC_RESP_R1                           (SDMMC_RESP_R48 | SDMMC_CMD_R_CRC_CHK_EN_Msk | SDMMC_CMD_R_CMD_IDX_CHK_EN_Msk)

/* Present state */
#define SDMMC_CMD_INHIBIT_Msk                   1U
#define SDMMC_DAT_INHIBIT_Msk                   2U
#define SDMMC_CARD_INSRT_Msk                    0x00010000U
#define SDMMC_CARD_INSRT_Pos                    0x10U
#define RD_XFER_ACTIVE_Msk                      0x200U
#define WR_XFER_ACTIVE_Msk                      0x100U
#define XFER_ACTIVE_Msk                         0x300U
#define RD_XFER_ACTIVE_Pos                      0x9U
#define WR_XFER_ACTIVE_Pos                      0x8U
#define DMA_IRQ_Pos                             0x3U
#define DMA_IRQ_Msk                             (1U << DMA_IRQ_Pos)
#define NORMAL_INT_STAT_XFER_COMPLETE_Pos       1U
#define NORMAL_INT_STAT_XFER_COMPLETE_Msk       (1U << NORMAL_INT_STAT_XFER_COMPLETE_Pos)

/* Card CSD */
#define CSD_SPEC_VER_Msk                        0x003C0000U
#define READ_BLK_LEN_Msk                        0x00000F00U
#define C_SIZE_MULT_Msk                         0x00000380U
#define C_SIZE_LOWER_Msk                        0xFFC00000U
#define C_SIZE_UPPER_Msk                        0x00000003U
#define CSD_STRUCT_Msk                          0x00C00000U
#define CSD_V2_C_SIZE_Msk                       0x3FFFFF00U
#define CSD_CCC_Msk                             0xFFF00000U
#define CSD_CCC_SHIFT                           20U
#define CSD_CCC_CLASS5_Msk                      0x20U

/* Blk Size */
#define SDMMC_BLK_SIZE_512_Msk                  0x0200U     /*!< Blk Size 512             */
#define SDMMC_NORM_INTR_ALL_Msk                 0x0000FFFFU /*!< Mask for normal irq bits */
#define SDMMC_ERROR_INTR_ALL_Msk                0x0000F3FFU /*!< Mask for error irq bits  */

/* Power Control */
#define SDMMC_PC_BUS_PWR_VDD1_Msk               0x00000001U /**< Bus Power Control  */
#define SDMMC_PC_BUS_VSEL_Msk                   0x0000000EU /**< Bus Voltage Select */
#define SDMMC_PC_BUS_VSEL_3V3_Msk               0x0000000EU /**< Bus Voltage 3.3V   */
#define SDMMC_PC_BUS_VSEL_3V0_Msk               0x0000000CU /**< Bus Voltage 3.0V   */
#define SDMMC_PC_BUS_VSEL_1V8_Msk               0x0000000AU /**< Bus Voltage 1.8V   */
#define SDMMC_PC_EMMC_HW_RST_Msk                0x00000010U /**< HW reset for eMMC  */

/* Clock Control */
#define SDMMC_INTERNAL_CLK_EN_Msk               0x1U
#define SDMMC_INTERNAL_CLK_STABLE_Msk           0x2U
#define SDMMC_CLK_EN_Msk                        0x4U
#define SDMMC_PLL_EN_Msk                        0x8U
#define SDMMC_CLK_GEN_SEL_Pos                   0x5U
#define SDMMC_DIV_CLK_MODE                      0x0U
#define SDMMC_PROG_CLK_MODE                     0x1U
#define SDMMC_CLK_GEN_SEL_Msk                   (SDMMC_DIV_CLK_MODE << SDMMC_CLK_GEN_SEL_Pos)
#define SDMMC_UPPER_FREQ_SEL_Pos                6U
#define SDMMC_FREQ_SEL_Pos                      8U
#define SDMMC_CLK_200KHz_DIV                    (2U << SDMMC_UPPER_FREQ_SEL_Pos)
#define SDMMC_CLK_400KHz_DIV                    (1U << SDMMC_UPPER_FREQ_SEL_Pos)
#define SDMMC_CLK_UPPR_FREQ_SEL                 SDMMC_CLK_400KHz_DIV
#define SDMMC_CLK_800KHz_DIV                    0x80U
#define SDMMC_CLK_1_5MHz_DIV                    0x40U
#define SDMMC_CLK_3MHz_DIV                      0x20U
#define SDMMC_CLK_6MHz_DIV                      0x10U
#define SDMMC_CLK_12_5MHz_DIV                   0x4U
#define SDMMC_CLK_25MHz_DIV                     0x2U
#define SDMMC_CLK_50MHz_DIV                     0x1U
#define SDMMC_CLK_100MHz_DIV                    0x0U
#define SDMMC_INIT_FREQ                         SDMMC_CLK_400KHz_DIV
#define SDMMC_INIT_CLK_DIVSOR_Msk               (SDMMC_INIT_FREQ)

/* Host Capabilities */
#define SDMMC_HOST_SD_CAP_VOLT_3V3_Msk          0x01000000U /*!< 3.3V support */
#define SDMMC_HOST_SD_CAP_VOLT_3V0_Msk          0x02000000U /*!< 3.0V support */
#define SDMMC_HOST_SD_CAP_VOLT_1V8_Msk          0x04000000U /*!< 1.8V support */

/* Xfer Mode Control */
#define SDMMC_XFER_MODE_DMA_EN_Pos              0U
#define SDMMC_XFER_MODE_DMA_EN_Msk              (1U << SDMMC_XFER_MODE_DMA_EN_Pos)
#define SDMMC_XFER_MODE_BLK_CNT_EN_Pos          1U
#define SDMMC_XFER_MODE_BLK_CNT_Msk             (1U << SDMMC_XFER_MODE_BLK_CNT_EN_Pos)
#define SDMMC_XFER_MODE_AUTO_CMD_EN_Pos         2U
#define SDMMC_XFER_MODE_AUTO_CMD_DISABLE        0U
#define SDMMC_XFER_MODE_AUTO_CMD12              1U
#define SDMMC_XFER_MODE_AUTO_CMD23              2U
#define SDMMC_XFER_MODE_AUTO_CMD_AUTO_SEL       3U
#define SDMMC_XFER_MODE_AUTO_CMD_EN_Msk         (SDMMC_XFER_MODE_AUTO_CMD12 << SDMMC_XFER_MODE_AUTO_CMD_EN_Pos)
#define SDMMC_XFER_MODE_DATA_XFER_DIR_Pos       4U
#define SDMMC_XFER_MODE_DATA_XFER_RD_Msk        (1U << SDMMC_XFER_MODE_DATA_XFER_DIR_Pos)
#define SDMMC_XFER_MODE_DATA_XFER_WR_Msk        (0U << SDMMC_XFER_MODE_DATA_XFER_DIR_Pos)
#define SDMMC_XFER_MODE_MULTI_BLK_SEL_Pos       5U
#define SDMMC_XFER_MODE_MULTI_BLK_SEL_Msk       (1U << SDMMC_XFER_MODE_MULTI_BLK_SEL_Pos)
#define SDMMC_XFER_MODE_RESP_ERR_CHK_Pos        7U
#define SDMMC_XFER_MODE_RESP_ERR_CHK_EN_Msk     (1U << SDMMC_XFER_MODE_RESP_ERR_CHK_Pos)

/* DMA */
#define SDMMC_HC_DMA_ADMA2_32_Msk               0x00000010U /**< ADMA2 Mode - 32 bit */

/* Wakeup Control Register Mask */
#define SDMMC_WKUP_CARD_IRQ_Msk                 0x00000001U /*!< Card Interrupt           */
#define SDMMC_WKUP_CARD_INSRT_Msk               0x00000002U /*!< Card Insertion           */
#define SDMMC_WKUP_CARD_REM_Msk                 0x00000004U /*!< Card Removal             */

/* Normal IRQs Mask */
#define SDMMC_INTR_CC_Msk                       0x00000001U /*!< Command Complete         */
#define SDMMC_INTR_TC_Msk                       0x00000002U /*!< Transfer Complete        */
#define SDMMC_INTR_BGE_Msk                      0x00000004U /*!< Block Gap Event          */
#define SDMMC_INTR_DMA_Msk                      0x00000008U /*!< DMA Interrupt            */
#define SDMMC_INTR_BWR_Msk                      0x00000010U /*!< Buffer Write Ready       */
#define SDMMC_INTR_BRR_Msk                      0x00000020U /*!< Buffer Read Ready        */
#define SDMMC_INTR_CARD_INSRT_Msk               0x00000040U /*!< Card Insert              */
#define SDMMC_INTR_CARD_REM_Msk                 0x00000080U /*!< Card Remove              */
#define SDMMC_INTR_CARD_Msk                     0x00000100U /*!< Card Interrupt           */
#define SDMMC_INTR_INT_A_Msk                    0x00000200U /*!< INT A Interrupt          */
#define SDMMC_INTR_INT_B_Msk                    0x00000400U /*!< INT B Interrupt          */
#define SDMMC_INTR_INT_C_Msk                    0x00000800U /*!< INT C Interrupt          */
#define SDMMC_INTR_RE_TUNING_Msk                0x00001000U /*!< Re-Tuning Interrupt      */
#define SDMMC_INTR_BOOT_ACK_RECV_Msk            0x00002000U /*!< Boot Ack Recv Irq        */
#define SDMMC_INTR_BOOT_TERM_Msk                0x00004000U /*!< Boot Terminate Interrupt */
#define SDMMC_INTR_ERR_Msk                      0x00008000U /*!< Error Interrupt          */
#define SDMMC_NORM_INTR_ALL_Msk                 0x0000FFFFU

/* Error IRQs Mask */
#define SDMMC_INTR_ERR_CT_Msk                   0x00000001U /*!< Command Timeout Error    */
#define SDMMC_INTR_ERR_CCRC_Msk                 0x00000002U /*!< Command CRC Error        */
#define SDMMC_INTR_ERR_CEB_Msk                  0x00000004U /*!< Command End Bit Error    */
#define SDMMC_INTR_ERR_CI_Msk                   0x00000008U /*!< Command Index Error      */
#define SDMMC_INTR_ERR_DT_Msk                   0x00000010U /*!< Data Timeout Error       */
#define SDMMC_INTR_ERR_DCRC_Msk                 0x00000020U /*!< Data CRC Error           */
#define SDMMC_INTR_ERR_DEB_Msk                  0x00000040U /*!< Data End Bit Error       */
#define SDMMC_INTR_ERR_CUR_LMT_Msk              0x00000080U /*!< Current Limit Error      */
#define SDMMC_INTR_ERR_AUTO_CMD12_Msk           0x00000100U /*!< Auto CMD12 Error         */
#define SDMMC_INTR_ERR_ADMA_Msk                 0x00000200U /*!< ADMA Error               */
#define SDMMC_INTR_ERR_TR_Msk                   0x00001000U /*!< Tuning Error             */
#define SDMMC_INTR_VEND_SPF_ERR_Msk             0x0000E000U /*!< Vendor Specific Error    */
#define SDMMC_ERROR_INTR_ALL_Msk                0x0000F3FFU /*!< Mask for error bits      */

/* SD Status */
#define SDMMC_STATUS_Msk                        0x00001E00U
#define SDMMC_STATUS_Pos                        9U

/* Host Control 1 */
#define SDMMC_HOST_CTRL1_LED_ON                 0x1U
#define SDMMC_HOST_CTRL1_4_BIT_WIDTH            0x2U /*!< Host control 1 4bit mode */
#define SDMMC_HOST_CTRL1_HIGH_SPEED_MODE_EN     0x4U /*!< Host control 1 High speed enable */
#define SDMMC_HOST_CTRL1_SDMA_MODE              0x0U
#define SDMMC_HOST_CTRL1_ADMA2_MODE             0x1U
#define SDMMC_HOST_CTRL1_ADMA3_MODE             0x2U
#define SDMMC_HOST_CTRL1_ADMA32_MODE_Msk        (0x2U << 3U)
#define SDMMC_HOST_CTRL1_ADMA64MODE             0x3U
#define SDMMC_HOST_CTRL1_DMA_SEL                SD_SEL_SDMA
#define SDMMC_HOST_CTRL1_DMA_SEL_1BIT_MODE      0x0U

/* Host Control 2*/
#define SDMMC_HOST_CTRL2_ASYNC_INT_EN_Msk       (1U << 14U)
#define SDMMC_HOST_CTRL2_VER4_EN_Msk            (1U << 12U)
#define SDMMC_HOST_CTRL2_CMD23_EN_Msk           (1U << 11U)
#define SDMMC_HOST_CTRL2_SIGNALING_EN_Msk       (1U << 3U)

/* Card Interface Conditions constants */
#define SDMMC_CMD8_VOL_PATTERN                  0x1AAU   /*!< CMD8 Voltage Pattern */

/* Card Operating Conditions constants */
#define SDMMC_OCR_READY                         0x80000000U  /*!< OCR Ready */
#define SDMMC_OCR_S18R                          0x1000000U
#define SDMMC_ROCR_S18A                         SDMMC_OCR_S18R
#define SDMMC_CMD41_HCS                         0x40000000U  /*!< ACMD41 High Capacity support */
#define SDMMC_CMD41_3V3                         0x00300000U  /*!< ACMD41 3.3v support */
#define SDMMC_CMD41_S18A                        SDMMC_OCR_S18R /*!< ACMD41 switch to 1.8v Accepted */

/* Time out constant */
#define SDMMC_MAX_TIMEOUT_32                    0xFFFFFFFFU
#define SDMMC_MAX_TIMEOUT_16                    0xFFFFU

/* SD Relative Card Address Constnat */
#define SDMMC_RCA_Pos                           0x10U
#define SDMMC_RCA_Msk                           0xFFFF0000U

#ifdef __cplusplus
}

#endif

#endif /* __SD_HOST_H__ */
