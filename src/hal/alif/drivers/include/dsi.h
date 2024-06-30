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
 * @file     dsi.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     17-April-2023
 * @brief    Low level driver Specific Header file.
 ******************************************************************************/

#ifndef DSI_H_
#define DSI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>


typedef struct {                                           /*!< (@ 0x49032000) DSI Structure                                              */
    volatile const uint32_t  DSI_VERSION;                  /*!< (@ 0x00000000) Reserved                                                   */
    volatile       uint32_t  DSI_PWR_UP;                   /*!< (@ 0x00000004) Power-up Control Register                                  */
    volatile       uint32_t  DSI_CLKMGR_CFG;               /*!< (@ 0x00000008) Clock Control Register                                     */
    volatile       uint32_t  DSI_DPI_VCID;                 /*!< (@ 0x0000000C) VC ID Configuration Register                               */
    volatile       uint32_t  DSI_DPI_COLOR_CODING;         /*!< (@ 0x00000010) DPI Color Coding Register                                  */
    volatile       uint32_t  DSI_DPI_CFG_POL;              /*!< (@ 0x00000014) DPI Polarity Configuration Register                        */
    volatile       uint32_t  DSI_DPI_LP_CMD_TIM;           /*!< (@ 0x00000018) DPI Low-Power Mode Configuration Register                  */
    volatile const uint32_t  RESERVED[4];
    volatile       uint32_t  DSI_PCKHDL_CFG;               /*!< (@ 0x0000002C) Protocol Configuration Register                            */
    volatile       uint32_t  DSI_GEN_VCID;                 /*!< (@ 0x00000030) Generic VC ID Configuration Register                       */
    volatile       uint32_t  DSI_MODE_CFG;                 /*!< (@ 0x00000034) Mode Configuration Register                                */
    volatile       uint32_t  DSI_VID_MODE_CFG;             /*!< (@ 0x00000038) Video Mode Configuration Register                          */
    volatile       uint32_t  DSI_VID_PKT_SIZE;             /*!< (@ 0x0000003C) Video Packet Size Register                                 */
    volatile       uint32_t  DSI_VID_NUM_CHUNKS;           /*!< (@ 0x00000040) Video Chunks Configuration Register                        */
    volatile       uint32_t  DSI_VID_NULL_SIZE;            /*!< (@ 0x00000044) Video Null Packet Configuration Register                   */
    volatile       uint32_t  DSI_VID_HSA_TIME;             /*!< (@ 0x00000048) Video HSA Configuration Register                           */
    volatile       uint32_t  DSI_VID_HBP_TIME;             /*!< (@ 0x0000004C) Video HBP Configuration Register                           */
    volatile       uint32_t  DSI_VID_HLINE_TIME;           /*!< (@ 0x00000050) Video Line Configuration Register                          */
    volatile       uint32_t  DSI_VID_VSA_LINES;            /*!< (@ 0x00000054) Video VSA Configuration Register                           */
    volatile       uint32_t  DSI_VID_VBP_LINES;            /*!< (@ 0x00000058) Video VBP Configuration Register                           */
    volatile       uint32_t  DSI_VID_VFP_LINES;            /*!< (@ 0x0000005C) Video VFP Configuration Register                           */
    volatile       uint32_t  DSI_VID_VACTIVE_LINES;        /*!< (@ 0x00000060) Video VA Configuration Register                            */
    volatile const uint32_t  RESERVED1;
    volatile       uint32_t  DSI_CMD_MODE_CFG;             /*!< (@ 0x00000068) Generic Packet Command Configuration Register              */
    volatile       uint32_t  DSI_GEN_HDR;                  /*!< (@ 0x0000006C) Generic Header Configuration Register                      */
    volatile       uint32_t  DSI_GEN_PLD_DATA;             /*!< (@ 0x00000070) Generic Payload Data Register                              */
    volatile const uint32_t  DSI_CMD_PKT_STATUS;           /*!< (@ 0x00000074) Generic Packet Status Register                             */
    volatile       uint32_t  DSI_TO_CNT_CFG;               /*!< (@ 0x00000078) Timeout Counter Configuration Register                     */
    volatile       uint32_t  DSI_HS_RD_TO_CNT;             /*!< (@ 0x0000007C) HS Read Timeout Configuration Register                     */
    volatile       uint32_t  DSI_LP_RD_TO_CNT;             /*!< (@ 0x00000080) LP Read Timeout Configuration Register                     */
    volatile       uint32_t  DSI_HS_WR_TO_CNT;             /*!< (@ 0x00000084) HS Write Timeout Configuration Register                    */
    volatile       uint32_t  DSI_LP_WR_TO_CNT;             /*!< (@ 0x00000088) LP Write Timeout Configuration Register                    */
    volatile       uint32_t  DSI_BTA_TO_CNT;               /*!< (@ 0x0000008C) BTA Timeout Configuration Register                         */
    volatile       uint32_t  DSI_SDF_3D;                   /*!< (@ 0x00000090) 3D Control Register                                        */
    volatile       uint32_t  DSI_LPCLK_CTRL;               /*!< (@ 0x00000094) Clock Lane Power Control Register                          */
    volatile       uint32_t  DSI_PHY_TMR_LPCLK_CFG;        /*!< (@ 0x00000098) Clock Lane Timer Configuration Register                    */
    volatile       uint32_t  DSI_PHY_TMR_CFG;              /*!< (@ 0x0000009C) Data Lane Timer Configuration Register                     */
    volatile       uint32_t  DSI_PHY_RSTZ;                 /*!< (@ 0x000000A0) PHY Control Register                                       */
    volatile       uint32_t  DSI_PHY_IF_CFG;               /*!< (@ 0x000000A4) PHY Configuration Register                                 */
    volatile       uint32_t  DSI_PHY_ULPS_CTRL;            /*!< (@ 0x000000A8) PHY ULPS Control Register                                  */
    volatile       uint32_t  DSI_PHY_TX_TRIGGERS;          /*!< (@ 0x000000AC) PHY TX Trigger Configuration Register                      */
    volatile const uint32_t  DSI_PHY_STATUS;               /*!< (@ 0x000000B0) PHY Status Register                                        */
    volatile       uint32_t  DSI_PHY_TST_CTRL0;            /*!< (@ 0x000000B4) PHY Test Interface Control Register 0                      */
    volatile       uint32_t  DSI_PHY_TST_CTRL1;            /*!< (@ 0x000000B8) PHY Test Interface Control Register 1                      */
    volatile const uint32_t  DSI_INT_ST0;                  /*!< (@ 0x000000BC) Interrupt Status Register 0                                */
    volatile const uint32_t  DSI_INT_ST1;                  /*!< (@ 0x000000C0) Interrupt Status Register 1                                */
    volatile       uint32_t  DSI_INT_MSK0;                 /*!< (@ 0x000000C4) Interrupt Mask Register 0                                  */
    volatile       uint32_t  DSI_INT_MSK1;                 /*!< (@ 0x000000C8) Interrupt Mask Register 1                                  */
    volatile       uint32_t  DSI_PHY_CAL;                  /*!< (@ 0x000000CC) PHY skew calibration control                               */
    volatile const uint32_t  RESERVED2[2];
    volatile       uint32_t  DSI_INT_FORCE0;               /*!< (@ 0x000000D8) Force Interrupt Register 0                                 */
    volatile       uint32_t  DSI_INT_FORCE1;               /*!< (@ 0x000000DC) Force Interrupt Register 1                                 */
    volatile const uint32_t  RESERVED3[5];
    volatile       uint32_t  DSI_PHY_TMR_RD_CFG;           /*!< (@ 0x000000F4) Data Lane Timer Read Configuration Register                */
    volatile const uint32_t  RESERVED4[2];
    volatile       uint32_t  DSI_VID_SHADOW_CTRL;          /*!< (@ 0x00000100) Video Shadow Control Register                              */
    volatile const uint32_t  RESERVED5[2];
    volatile const uint32_t  DSI_DPI_VCID_ACT;             /*!< (@ 0x0000010C) Current VC ID Register                                     */
    volatile const uint32_t  DSI_DPI_COLOR_CODING_ACT;     /*!< (@ 0x00000110) Current Color Coding Register                              */
    volatile const uint32_t  RESERVED6;
    volatile const uint32_t  DSI_DPI_LP_CMD_TIM_ACT;       /*!< (@ 0x00000118) Low-Power Mode Current Configuration Register              */
    volatile const uint32_t  RESERVED7[7];
    volatile const uint32_t  DSI_VID_MODE_CFG_ACT;         /*!< (@ 0x00000138) Video Mode Current Configuration Register                  */
    volatile const uint32_t  DSI_VID_PKT_SIZE_ACT;         /*!< (@ 0x0000013C) Video Packet Size Current Configuration Register           */
    volatile const uint32_t  DSI_VID_NUM_CHUNKS_ACT;       /*!< (@ 0x00000140) Video Chunks Current Configuration Register                */
    volatile const uint32_t  DSI_VID_NULL_SIZE_ACT;        /*!< (@ 0x00000144) Video Null Packet Size Current Configuration Register                                                   */
    volatile const uint32_t  DSI_VID_HSA_TIME_ACT;         /*!< (@ 0x00000148) Video HSA Current Configuration Register                   */
    volatile const uint32_t  DSI_VID_HBP_TIME_ACT;         /*!< (@ 0x0000014C) Video HBP Current Configuration Register                   */
    volatile const uint32_t  DSI_VID_HLINE_TIME_ACT;       /*!< (@ 0x00000150) Video Line Current Configuration Register                  */
    volatile const uint32_t  DSI_VID_VSA_LINES_ACT;        /*!< (@ 0x00000154) Video VSA Current Configuration Register                   */
    volatile const uint32_t  DSI_VID_VBP_LINES_ACT;        /*!< (@ 0x00000158) Video VBP Current Configuration Register                   */
    volatile const uint32_t  DSI_VID_VFP_LINES_ACT;        /*!< (@ 0x0000015C) Video VFP Current Configuration Register                   */
    volatile const uint32_t  DSI_VID_VACTIVE_LINES_ACT;    /*!< (@ 0x00000160) Video VA Current Configuration Register                    */
    volatile const uint32_t  RESERVED8;
    volatile const uint32_t  DSI_VID_PKT_STATUS;           /*!< (@ 0x00000168) Video Packet Status Register                               */
    volatile const uint32_t  RESERVED9[9];
    volatile const uint32_t  DSI_SDF_3D_ACT;               /*!< (@ 0x00000190) 3D Current Configuration Register                          */
} DSI_Type;                                                /*!< Size = 404 (0x194)                                                        */

/*PWR_UP register bits parameters*/
#define DSI_SHUTDOWNZ                        0U
#define DSI_SHUTDOWNZ_MASK                   (0x1U << DSI_SHUTDOWNZ)

/*CLKMGR_CFG register bits parameters*/
#define DSI_TX_ESC_CLK_DIVISION              0U
#define DSI_TX_ESC_CLK_DIVISION_MASK         (0xFFU << DSI_TX_ESC_CLK_DIVISION)
#define DSI_TO_CLK_DIVISION                  8U
#define DSI_TO_CLK_DIVISION_MASK             (0xFFU << DSI_TO_CLK_DIVISION)

/*DPI_VCID register bits parameters*/
#define DSI_DPI_VC_ID                        0U
#define DSI_DPI_VC_ID_MASK                   (0x3U << DSI_DPI_VC_ID)

/*DPI_COLOR_CODING register bits parameters*/
#define DSI_DPI_CLR_CODING                   0U
#define DSI_DPI_CLR_CODING_MASK              (0xFU << DSI_DPI_CLR_CODING)

/*DPI_CFG_POL register bits parameters*/
#define DSI_DATAEN_ACTIVE_LOW                0U
#define DSI_DATAEN_ACTIVE_LOW_MASK           (0x1U << DSI_DATAEN_ACTIVE_LOW)
#define DSI_VSYNC_ACTIVE_LOW                 1U
#define DSI_VSYNC_ACTIVE_LOW_MASK            (0x1U << DSI_VSYNC_ACTIVE_LOW)
#define DSI_HSYNC_ACTIVE_LOW                 2U
#define DSI_HSYNC_ACTIVE_LOW_MASK            (0x1U << DSI_HSYNC_ACTIVE_LOW)
#define DSI_SHUTD_ACTIVE_LOW                 3U
#define DSI_SHUTD_ACTIVE_LOW_MASK            (0x1U << DSI_SHUTD_ACTIVE_LOW)
#define DSI_COLORM_ACTIVE_LOW                4U
#define DSI_COLORM_ACTIVE_LOW_MASK           (0x1U << DSI_COLORM_ACTIVE_LOW)

/*DSI_PCKHDL_CFG register bits parameters*/
#define DSI_EOTP_TX_EN                       0U
#define DSI_EOTP_TX_EN_MASK                  (0x1U << DSI_EOTP_TX_EN)
#define DSI_BTA_EN                           2U
#define DSI_BTA_EN_MASK                      (0x1U << DSI_BTA_EN)
#define DSI_ECC_RX_EN                        3U
#define DSI_ECC_RX_EN_MASK                   (0x1U << DSI_ECC_RX_EN)
#define DSI_CRC_RX_EN                        4U
#define DSI_CRC_RX_EN_MASK                   (0x1U << DSI_CRC_RX_EN)

/*MODE_CFG register bits parameters*/
#define DSI_CMD_VIDEO_MODE                   0U
#define DSI_CMD_VIDEO_MODE_MASK              (0x1U << DSI_CMD_VIDEO_MODE)

/*VID_MODE_CFG register bits parameters*/
#define DSI_VID_MODE_TYPE                    0U
#define DSI_VID_MODE_TYPE_MASK               (0x3U << DSI_VID_MODE_TYPE)
#define DSI_LP_CMD_EN                        8U
#define DSI_LP_CMD_EN_MASK                   (0xBFU << DSI_LP_CMD_EN)

/*VID_PKT_SIZE register bits parameters*/
#define DSI_VIDEO_PKT_SIZE                   0U
#define DSI_VIDEO_PKT_SIZE_MASK              (0x3FFFU << DSI_VIDEO_PKT_SIZE)

/*VID_NUM_CHUNKS register bits parameters*/
#define DSI_VID_NUMBER_CHUNKS                0U
#define DSI_VID_NUMBER_CHUNKS_MASK           (0x1FFFU << DSI_VID_NUMBER_CHUNKS)

/*VID_NULL_SIZE register bits parameters*/
#define DSI_VID_NULL_PKT_SIZE                0U
#define DSI_VID_NULL_PKT_SIZE_MASK           (0x1FFFU << DSI_VID_NULL_PKT_SIZE)

/*VID_HSA_TIME register bits parameters*/
#define DSI_VIDEO_HSA_TIME                   0U
#define DSI_VIDEO_HSA_TIME_MASK              (0xFFFU << DSI_VIDEO_HSA_TIME)

/*VID_HBP_TIME register bits parameters*/
#define DSI_VIDEO_HBP_TIME                   0U
#define DSI_VIDEO_HBP_TIME_MASK              (0xFFFU << DSI_VIDEO_HBP_TIME)

/*VID_HLINE_TIME register bits parameters*/
#define DSI_VIDEO_HLINE_TIME                 0U
#define DSI_VIDEO_HLINE_TIME_MASK            (0x7FFFU << DSI_VIDEO_HLINE_TIME)

/*VID_VSA_LINES register bits parameters*/
#define DSI_VSA_LINES                        0U
#define DSI_VSA_LINES_MASK                   (0x3FFU << DSI_VSA_LINES)

/*VID_VBP_LINES register bits parameters*/
#define DSI_VBP_LINES                        0U
#define DSI_VBP_LINES_MASK                   (0x3FFU << DSI_VBP_LINES)

/*VID_VFP_LINES register bits parameters*/
#define DSI_VFP_LINES                        0U
#define DSI_VFP_LINES_MASK                   (0x3FFU << DSI_VFP_LINES)

/*VID_VACTIVE_LINES register bits parameters*/
#define DSI_V_ACTIVE_LINES                   0U
#define DSI_V_ACTIVE_LINES_MASK              (0x3FFFU << DSI_V_ACTIVE_LINES)

/*GEN_HDR register bits parameters*/
#define DSI_GEN_DT                           0U
#define DSI_GEN_DT_MASK                      (0x3FU << DSI_GEN_DT)
#define DSI_GEN_VC                           6U
#define DSI_GEN_VC_MASK                      (0x3U << DSI_GEN_VC)
#define DSI_GEN_WC_LSBYTE                    8U
#define DSI_GEN_WC_LSBYTE_MASK               (0xFFU << DSI_GEN_WC_LSBYTE)
#define DSI_GEN_WC_MSBYTE                    16U
#define DSI_GEN_WC_MSBYTE_MASK               (0xFFU << DSI_GEN_WC_MSBYTE)

/*GEN_PLD_DATA register bits parameters*/
#define DSI_GEN_PLD_B1                       0U
#define DSI_GEN_PLD_B1_MASK                  (0xFFU << DSI_GEN_PLD_B1)
#define DSI_GEN_PLD_B2                       8U
#define DSI_GEN_PLD_B2_MASK                  (0xFFU << DSI_GEN_PLD_B2)
#define DSI_GEN_PLD_B3                       16U
#define DSI_GEN_PLD_B3_MASK                  (0xFFU << DSI_GEN_PLD_B3)
#define DSI_GEN_PLD_B4                       24U
#define DSI_GEN_PLD_B4_MASK                  (0xFFU << DSI_GEN_PLD_B4)

/*LPCLK_CTRL register bits parameters*/
#define DSI_PHY_TXREQUESTCLKHS               0U
#define DSI_PHY_TXREQUESTCLKHS_MASK          (0x1U << DSI_PHY_TXREQUESTCLKHS)
#define DSI_AUTO_CLKLANE_CTRL                1U
#define DSI_AUTO_CLKLANE_CTRL_MASK           (0x1U << DSI_AUTO_CLKLANE_CTRL)

/*PHY_TMR_LPCLK_CFG register bits parameters*/
#define DSI_PHY_CLKLP2HS_TIME                0U
#define DSI_PHY_CLKLP2HS_TIME_MASK           (0x3FFU << DSI_PHY_CLKLP2HS_TIME)
#define DSI_PHY_CLKHS2LP_TIME                16U
#define DSI_PHY_CLKHS2LP_TIME_MASK           (0x3FFU << DSI_PHY_CLKHS2LP_TIME)

/*PHY_TMR_CFG register bits parameters*/
#define DSI_PHY_LP2HS_TIME                   0U
#define DSI_PHY_LP2HS_TIME_MASK              (0x3FFU << DSI_PHY_LP2HS_TIME)
#define DSI_PHY_HS2LP_TIME                   16U
#define DSI_PHY_HS2LP_TIME_MASK              (0x3FFU << DSI_PHY_HS2LP_TIME)

/*PHY_RSTZ register bits parameters*/
#define DSI_PHY_SHUTDOWNZ                    0U
#define DSI_PHY_SHUTDOWNZ_MASK               (0x1U << DSI_PHY_SHUTDOWNZ)
#define DSI_PHY_RST                          1U
#define DSI_PHY_RST_MASK                     (0x1U << DSI_PHY_RST)
#define DSI_PHY_ENABLECLK                    2U
#define DSI_PHY_ENABLECLK_MASK               (0x1U << DSI_PHY_ENABLECLK)
#define DSI_PHY_FORCEPLL                     3U
#define DSI_PHY_FORCEPLL_MASK                (0x1U << DSI_PHY_FORCEPLL)

/*PHY_IF_CFG register bits parameters*/
#define DSI_PHY_N_LANE                       0U
#define DSI_PHY_N_LANE_MASK                  (0x3U << DSI_PHY_N_LANE)
#define DSI_PHY_STOP_WAIT_TIME               8U
#define DSI_PHY_STOP_WAIT_TIME_MASK          (0xFFU << DSI_PHY_STOP_WAIT_TIME)

/*PHY_STATUS register bits parameters*/
#define DSI_PHY_LOCK                         0U
#define DSI_PHY_LOCK_MASK                    (0x1U << DSI_PHY_LOCK)
#define DSI_PHY_STOPSTATECLKLANE             2U
#define DSI_PHY_STOPSTATECLKLANE_MASK        (0x1U << DSI_PHY_STOPSTATECLKLANE)
#define DSI_PHY_STOPSTATELANE_0              4U
#define DSI_PHY_STOPSTATELANE_0_MASK         (0x1U << DSI_PHY_STOPSTATELANE_0)
#define DSI_PHY_STOPSTATELANE_1              7U
#define DSI_PHY_STOPSTATELANE_1_MASK         (0x1U << DSI_PHY_STOPSTATELANE_1)

/*PHY_TEST_CTRL0 register bits parameters*/
#define DSI_PHY_TESTCLR                       0U
#define DSI_PHY_TESTCLR_MASK                  (0x1U << DSI_PHY_TESTCLR)
#define DSI_PHY_TESTCLK                       1U
#define DSI_PHY_TESTCLK_MASK                  (0x1U << DSI_PHY_TESTCLK)

/*PHY_TEST_CTRL1 register bits parameters*/
#define DSI_PHY_TESTDIN                       0U
#define DSI_PHY_TESTDIN_MASK                  (0xFFU << DSI_PHY_TESTDIN)
#define DSI_PHY_TESTDOUT                      8U
#define DSI_PHY_TESTDOUT_MASK                 (0xFFU << DSI_PHY_TESTDOUT)
#define DSI_PHY_TESTEN                        16U
#define DSI_PHY_TESTEN_MASK                   (0x1U << DSI_PHY_TESTEN)

/*DCS Data types and Command*/
#define DSI_DCS_SHORT_WRITE_NODATA_TYPE      0x05
#define DSI_DCS_SHORT_WRITE_DATA_TYPE        0x15
#define DSI_DCS_LONG_WRITE_DATA_TYPE         0x39

/*DSI Features macro*/
#define DSI_GEN_PAYLOAD_FIFO_SIZE            512
#define DSI_PAYLOAD_FIFO_SLOT_DEPTH          4
#define DSI_DPHY_LONG_PKT_OVERHEAD           6
#define DSI_PAYLOAD_FIFO_SIZE                4096
#define DSI_BTA_TIMEOUT_COUNT                0xD00

/* DSI_IRQ0 register bits parameters */
#define DSI_IRQ0_ACK_WITH_ERR_0     (1U << 0)        /**< SoT error from the Acknowledge error report                                       */
#define DSI_IRQ0_ACK_WITH_ERR_1     (1U << 1)        /**< SoT Sync error from the Acknowledge error report                                  */
#define DSI_IRQ0_ACK_WITH_ERR_2     (1U << 2)        /**< EoT Sync error from the Acknowledge error report                                  */
#define DSI_IRQ0_ACK_WITH_ERR_3     (1U << 3)        /**< Escape Mode Entry Command error from the Acknowledge error report                 */
#define DSI_IRQ0_ACK_WITH_ERR_4     (1U << 4)        /**< Low-power Transmit Sync error from the Acknowledge error report                   */
#define DSI_IRQ0_ACK_WITH_ERR_5     (1U << 5)        /**< Peripheral Timeout error from the Acknowledge error report.                       */
#define DSI_IRQ0_ACK_WITH_ERR_6     (1U << 6)        /**< False Control error from the Acknowledge error report                             */
#define DSI_IRQ0_ACK_WITH_ERR_7     (1U << 7)        /**< Reserved (specific to device) from the acknowledge error report                   */
#define DSI_IRQ0_ACK_WITH_ERR_8     (1U << 8)        /**< ECC error single-bit (detected and corrected) from the Acknowledge error report  */
#define DSI_IRQ0_ACK_WITH_ERR_9     (1U << 9)        /**< ECC error multi-bit (detected not corrected) from the Acknowledge error report  */
#define DSI_IRQ0_ACK_WITH_ERR_10    (1U << 10)       /**< Checksum error (long packet only) from the Acknowledge error report               */
#define DSI_IRQ0_ACK_WITH_ERR_11    (1U << 11)       /**< Not recognized DSI data type from the Acknowledge error report                    */
#define DSI_IRQ0_ACK_WITH_ERR_12    (1U << 12)       /**< DSI VC ID Invalid from the Acknowledge error report                               */
#define DSI_IRQ0_ACK_WITH_ERR_13    (1U << 13)       /**< Invalid transmission length from the Acknowledge error report                     */
#define DSI_IRQ0_ACK_WITH_ERR_14    (1U << 14)       /**< Reserved (specific to device) from the Acknowledge error report                   */
#define DSI_IRQ0_ACK_WITH_ERR_15    (1U << 15)       /**< DSI protocol violation from the Acknowledge error report                          */
#define DSI_IRQ0_DPHY_ERRORS_0      (1U << 16)       /**< ErrEsc escape entry error from Lane 0                                             */
#define DSI_IRQ0_DPHY_ERRORS_1      (1U << 17)       /**< ErrSyncEsc low-power data transmission synchronization error from Lane 0          */
#define DSI_IRQ0_DPHY_ERRORS_2      (1U << 18)       /**< Control error ErrControl from Lane 0                                              */
#define DSI_IRQ0_DPHY_ERRORS_3      (1U << 19)       /**< LP0 contention error ErrContentionLP0 from Lane 0                                 */
#define DSI_IRQ0_DPHY_ERRORS_4      (1U << 20)        /**< LP1 contention error ErrContentionLP1 from Lane 0                                 */

/* DSI_IRQ1 register bits parameters */
#define DSI_IRQ1_TO_HS_TX             (1U << 0)       /**< High-speed transmission timeout counter reached the end and contention has been detected                                        */
#define DSI_IRQ1_TO_LP_RX             (1U << 1)       /**< Low-power reception timeout counter reached the end and contention has been detected                                            */
#define DSI_IRQ1_ECC_SINGLE_ERR       (1U << 2)       /**< ECC single error has been detected and corrected in a received packet                                                           */
#define DSI_IRQ1_ECC_MILTI_ERR        (1U << 3)       /**< ECC multiple error has been detected in a received packet                                                                       */
#define DSI_IRQ1_CRC_ERR              (1U << 4)       /**< CRC error has been detected in the received packet payload                                                                      */
#define DSI_IRQ1_PKT_SIZE_ERR         (1U << 5)       /**< Packet size error has been detected during the packet reception                                                                 */
#define DSI_IRQ1_EOPT_ERR             (1U << 6)       /**< EoTp packet has not been received at the end of the incoming peripheral transmission                                            */
#define DSI_IRQ1_DPI_PLD_WR_ERR       (1U << 7)       /**< During a DPI pixel line storage the payload FIFO becomes full and the data stored is corrupted                                 */
#define DSI_IRQ1_GEN_CMD_WR_ERR       (1U << 8)       /**< System tried to write a command through the Generic interface and the FIFO is full. Therefore the command is not written       */
#define DSI_IRQ1_GEN_PLD_WR_ERR       (1U << 9)       /**< System tried to write a payload data through the Generic interface and the FIFO is full. Therefore the payload is not written  */
#define DSI_IRQ1_GEN_PLD_SEND_ERR     (1U << 10)      /**< During a Generic interface packet build the payload FIFO becomes empty and corrupt data is sent                                */
#define DSI_IRQ1_GEN_PLD_RD_ERR       (1U << 11)      /**< During a DCS read data the payload FIFO becomes empty and the data sent to the interface is corrupted                          */
#define DSI_IRQ1_GEN_PLD_RECEV_ERR    (1U << 12)      /**< During a generic interface packet read back the payload FIFO becomes full and the received data is corrupted                   */
#define DSI_IRQ1_DPI_BUFF_PLD_UNDER   (1U << 19)      /**< Underflow has occurred when reading payload to build DSI packet for video mode                                                  */

/**
 * enum  DSI_N_LANES
 * DSI Number of data lanes.
 */
typedef enum _DSI_N_LANES{
    DSI_N_LANES_ONE,              /**< Number of lanes is one */
    DSI_N_LANES_TWO               /**< Number of lanes is two */
}DSI_N_LANES;

/**
 * enum  DSI_LANE
 * DSI lanes.
 */
typedef enum _DSI_LANE{
    DSI_LANE_CLOCK,              /**< DSI clock lane */
    DSI_LANE_0,                  /**< DSI lane 0 */
    DSI_LANE_1                   /**< DSI lane 1 */
}DSI_LANE;

/**
 * enum  DSI_VC_ID
 * DSI virtual channel ID.
 */
typedef enum _DSI_VC_ID{
    DSI_VC_ID_0,             /**< Virtual channel ID 0 */
    DSI_VC_ID_1,             /**< Virtual channel ID 1 */
    DSI_VC_ID_2,             /**< Virtual channel ID 2 */
    DSI_VC_ID_3              /**< Virtual channel ID 3 */
}DSI_VC_ID;

/**
 * enum  DSI_COLOR_CODING
 * DSI color coding.
 */
typedef enum _DSI_COLOR_CODING{
    DSI_COLOR_CODING_16_BIT  = 0x1,   /**<  16-bit configuration 2 */
    DSI_COLOR_CODING_18_BIT  = 0x4,   /**<  18-bit configuration 2 */
    DSI_COLOR_CODING_24_BIT  = 0x5    /**<  24-bit                 */
}DSI_COLOR_CODING;

/**
 * enum  DSI_POLARITY
 * DSI DPI polarity.
 */
typedef enum _DSI_POLARITY{
    DSI_POLARITY_ACTIVE_HIGH,      /**< DSi polarity high */
    DSI_POLARITY_ACTIVE_LOW        /**< DSI polarity low  */
}DSI_POLARITY;

/**
 * enum  DSI_VIDEO_MODE
 * DSI Video mode.
 */
typedef enum _DSI_VIDEO_MODE{
    DSI_VIDEO_MODE_NON_BURST_SYNC_PULSES,    /**< Non burst sync pulses  */
    DSI_VIDEO_MODE_NON_BURST_SYNC_EVENTS,    /**< Non burst sync events  */
    DSI_VIDEO_MODE_BURST_MODE,               /**< Burst mode             */
}DSI_VIDEO_MODE;

/**
 * enum  DSI_CMD_MODE_CFG
 * DSI command mode configuration.
 */
typedef enum _DSI_CMD_MODE_CFG {
    DSI_CMD_MODE_CFG_GEN_SW_0P_TX      = (1U << 8),    /* Configures the Generic short write packet with zero parameter command transmission      */
    DSI_CMD_MODE_CFG_GEN_SW_1P_TX      = (1U << 9),    /* Configures the Generic short write packet with one parameter command transmission       */
    DSI_CMD_MODE_CFG_GEN_SW_2P_TX      = (1U << 10),   /* Configures the Generic short write packet with two parameters command transmission type */
    DSI_CMD_MODE_CFG_GEN_SR_0P_TX      = (1U << 11),   /* Configures the Generic short read packet with zero parameter command transmission       */
    DSI_CMD_MODE_CFG_GEN_SR_1P_TX      = (1U << 12),   /* Configures the Generic short read packet with one parameter command transmission        */
    DSI_CMD_MODE_CFG_GEN_SR_2P_TX      = (1U << 13),   /* Configures the Generic short read packet with two parameters command transmission       */
    DSI_CMD_MODE_CFG_GEN_LW_TX         = (1U << 14),   /* Configures the Generic long write packet command transmission                           */
    DSI_CMD_MODE_CFG_DCS_SW_0P_TX      = (1U << 16),   /* Configures the DCS short write packet with zero parameter command transmission          */
    DSI_CMD_MODE_CFG_DCS_SW_1P_TX      = (1U << 17),   /* Configures the DCS short write packet with one parameter command transmission           */
    DSI_CMD_MODE_CFG_DCS_SR_0P_TX      = (1U << 18),   /* Configures the DCS short read packet with zero parameter command transmission           */
    DSI_CMD_MODE_CFG_DCS_LW_TX         = (1U << 19),   /* Configures the DCS long write packet command transmission                               */
    DSI_CMD_MODE_CFG_MAX_RD_PKT_SIZE   = (1U << 24)    /* Configures the maximum read packet size command transmission                            */
}DSI_CMD_MODE_CFG;

/**
 * enum  DSI_PLL_STATUS
 * DSI phy lock status.
 */
typedef enum _DSI_PLL_STATUS{
    DSI_PLL_STATUS_NOT_LOCKED,         /**< Status of the Not PHY locked */
    DSI_PLL_STATUS_LOCKED              /**< Status of the PHY locked */
}DSI_PLL_STATUS;

/**
 * enum  DSI_LANE_STOPSTATE
 * DSI lane stopstate status.
 */
typedef enum _DSI_LANE_STOPSTATE{
    DSI_LANE_STOPSTATE_OFF,         /**< Status of the lane stopstate off*/
    DSI_LANE_STOPSTATE_ON           /**< Status of the lane stopstate on*/
}DSI_LANE_STOPSTATE;

/**
  \fn          static inline void dsi_power_up_enable(DSI_Type *dsi)
  \brief       Enable dsi power up.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_power_up_enable(DSI_Type *dsi)
{
    dsi->DSI_PWR_UP |= DSI_SHUTDOWNZ_MASK;
}

/**
  \fn          static inline void dsi_power_up_disable(DSI_Type *dsi)
  \brief       Disable dsi power up.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_power_up_disable(DSI_Type *dsi)
{
    dsi->DSI_PWR_UP &= ~DSI_SHUTDOWNZ_MASK;
}

/**
  \fn          static inline void dsi_set_tx_escap_clock_divider(DSI_Type *dsi, uint32_t div)
  \brief       Set tx escape clock divider.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   div    tx escape clock divider to set.
               bit[7..0] This field indicates the division factor for the TX Escape clock source (LANEBYTECLK). The values 0 and 1 stop the TXCLKESC generation.
  \return      none.
*/
static inline void dsi_set_tx_escap_clock_divider(DSI_Type *dsi, uint32_t div)
{
    dsi->DSI_CLKMGR_CFG &= ~DSI_TX_ESC_CLK_DIVISION_MASK;
    dsi->DSI_CLKMGR_CFG |= (div << DSI_TX_ESC_CLK_DIVISION);
}

/**
  \fn          static inline uint32_t dsi_get_tx_escap_clock_divider(DSI_Type *dsi)
  \brief       Set tx escape clock divider.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      tx escape clock divider to set.
               bit[7..0] This field indicates the division factor for the TX Escape clock source (LANEBYTECLK). The values 0 and 1 stop the TXCLKESC generation.
*/
static inline uint32_t dsi_get_tx_escap_clock_divider(DSI_Type *dsi)
{
    return (dsi->DSI_CLKMGR_CFG & DSI_TX_ESC_CLK_DIVISION_MASK) >> DSI_TX_ESC_CLK_DIVISION;
}

/**
  \fn          static inline void dsi_set_dpi_vcid(DSI_Type *dsi, DSI_VC_ID id)
  \brief       Set dsi dpi virtual channel id.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   id     virtual channel id to set.
               bit[1..0] This field configures the DPI virtual channel id that is indexed to the Video mode packets
  \return      none.
*/
static inline void dsi_set_dpi_vcid(DSI_Type *dsi, DSI_VC_ID id)
{
    dsi->DSI_DPI_VCID &= ~DSI_DPI_VC_ID_MASK;
    dsi->DSI_DPI_VCID |= (id << DSI_DPI_VC_ID);
}

/**
  \fn          static inline DSI_VC_ID dsi_get_dpi_vcid(DSI_Type *dsi)
  \brief       Get dsi dpi virtual channel id.
  \param[in]   dsi    Pointer to the dsi register map.
  \return      virtual channel id.
               bit[1..0] This field configures the DPI virtual channel id that is indexed to the Video mode packets

*/
static inline DSI_VC_ID dsi_get_dpi_vcid(DSI_Type *dsi)
{
    return (dsi->DSI_DPI_VCID & DSI_DPI_VC_ID_MASK) >> DSI_DPI_VC_ID;
}

/**
  \fn          static inline void dsi_dpi_set_color_coding(DSI_Type *dsi, DSI_COLOR_CODING color)
  \brief       Set dsi dpi color coding.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   color   dsi color coding to set.
  \return      none.
*/
static inline void dsi_dpi_set_color_coding(DSI_Type *dsi, DSI_COLOR_CODING color)
{
    dsi->DSI_DPI_COLOR_CODING &= ~DSI_DPI_CLR_CODING_MASK;
    dsi->DSI_DPI_COLOR_CODING |= (color << DSI_DPI_CLR_CODING);
}

/**
  \fn          static inline DSI_COLOR_CODING dsi_dpi_get_color_coding(DSI_Type *dsi)
  \brief       Get dsi dpi color coding.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi color coding value.
*/
static inline DSI_COLOR_CODING dsi_dpi_get_color_coding(DSI_Type *dsi)
{
    return ((dsi->DSI_DPI_COLOR_CODING & DSI_DPI_CLR_CODING_MASK) >> DSI_DPI_CLR_CODING);
}

/**
  \fn          static inline DSI_POLARITY dsi_dpi_get_dataen_polarity(DSI_Type *dsi)
  \brief       Get dsi dpi dataen line polarity.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi dataen polarity to get.
*/
static inline DSI_POLARITY dsi_dpi_get_dataen_polarity(DSI_Type *dsi)
{
    return (dsi->DSI_DPI_CFG_POL & DSI_DATAEN_ACTIVE_LOW_MASK) >> DSI_DATAEN_ACTIVE_LOW;
}

/**
  \fn          static inline DSI_POLARITY dsi_dpi_get_vsync_polarity(DSI_Type *dsi)
  \brief       Get dsi dpi vsync line polarity.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi vsync polarity to get.
*/
static inline DSI_POLARITY dsi_dpi_get_vsync_polarity(DSI_Type *dsi)
{
    return (dsi->DSI_DPI_CFG_POL & DSI_VSYNC_ACTIVE_LOW_MASK) >> DSI_VSYNC_ACTIVE_LOW;
}

/**
  \fn          static inline DSI_POLARITY dsi_dpi_get_hsync_polarity(DSI_Type *dsi)
  \brief       Get dsi dpi hsync line polarity.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi hsync polarity to get.
*/
static inline DSI_POLARITY dsi_dpi_get_hsync_polarity(DSI_Type *dsi)
{
    return (dsi->DSI_DPI_CFG_POL & DSI_HSYNC_ACTIVE_LOW_MASK) >> DSI_HSYNC_ACTIVE_LOW;
}

/**
  \fn          static inline DSI_POLARITY dsi_dpi_get_shutd_polarity(DSI_Type *dsi)
  \brief       Get dsi dpi shutd line polarity.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi shutd polarity to get.
*/
static inline DSI_POLARITY dsi_dpi_get_shutd_polarity(DSI_Type *dsi)
{
    return (dsi->DSI_DPI_CFG_POL & DSI_SHUTD_ACTIVE_LOW_MASK) >> DSI_SHUTD_ACTIVE_LOW;
}

/**
  \fn          static inline DSI_POLARITY dsi_dpi_get_colorm_polarity(DSI_Type *dsi)
  \brief       Get dsi dpi colorm line polarity.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi colorm polarity to get.
*/
static inline DSI_POLARITY dsi_dpi_get_colorm_polarity(DSI_Type *dsi)
{
    return (dsi->DSI_DPI_CFG_POL & DSI_COLORM_ACTIVE_LOW_MASK) >> DSI_COLORM_ACTIVE_LOW;
}

/**
  \fn          static inline void dsi_video_mode_enable(DSI_Type *dsi)
  \brief       Enable dsi video mode.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_video_mode_enable(DSI_Type *dsi)
{
    dsi->DSI_MODE_CFG &= ~DSI_CMD_VIDEO_MODE_MASK;
}

/**
  \fn          static inline void dsi_command_mode_enable(DSI_Type *dsi)
  \brief       Enable dsi command mode.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_command_mode_enable(DSI_Type *dsi)
{
    dsi->DSI_MODE_CFG |= DSI_CMD_VIDEO_MODE_MASK;
}
/**
  \fn          static inline void dsi_set_video_mode_type(DSI_Type *dsi, uint32_t type)
  \brief       Set dsi video mode type.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   type   dsi video mode type to set.
  \return      none.
*/
static inline void dsi_set_video_mode_type(DSI_Type *dsi, DSI_VIDEO_MODE type)
{
    dsi->DSI_VID_MODE_CFG &= ~DSI_VID_MODE_TYPE_MASK;
    dsi->DSI_VID_MODE_CFG |= (type << DSI_VID_MODE_TYPE);
}

/**
  \fn          static inline DSI_VIDEO_MODE dsi_get_video_mode_type(DSI_Type *dsi)
  \brief       Get dsi video mode type.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video mode type.
*/
static inline DSI_VIDEO_MODE dsi_get_video_mode_type(DSI_Type *dsi)
{
    return (dsi->DSI_VID_MODE_CFG & DSI_VID_MODE_TYPE_MASK) >> DSI_VID_MODE_TYPE;
}

/**
  \fn          static inline void dsi_command_transmission_enable(DSI_Type *dsi)
  \brief       Enable dsi command transmission.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_command_transmission_enable(DSI_Type *dsi)
{
    dsi->DSI_VID_MODE_CFG |= DSI_LP_CMD_EN_MASK;
}

/**
  \fn          static inline void dsi_command_transmission_disable(DSI_Type *dsi)
  \brief       Disable dsi command transmission.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_command_transmission_disable(DSI_Type *dsi)
{
    dsi->DSI_VID_MODE_CFG &= ~DSI_LP_CMD_EN_MASK;
}

/**
  \fn          static inline void dsi_set_video_packet_size(DSI_Type *dsi, uint32_t pkt_size)
  \brief       Set dsi video packet size.
  \param[in]   dsi       Pointer to the dsi register map.
  \param[in]   pkt_size  dsi video packet size to set.
               bit[13..0] This field configures the number of pixels in a single video packet.
  \return      none.
*/
static inline void dsi_set_video_packet_size(DSI_Type *dsi, uint32_t pkt_size)
{
    dsi->DSI_VID_PKT_SIZE &= ~DSI_VIDEO_PKT_SIZE_MASK;
    dsi->DSI_VID_PKT_SIZE |= (pkt_size << DSI_VIDEO_PKT_SIZE);
}

/**
  \fn          static inline uint32_t dsi_get_video_packet_size(DSI_Type *dsi)
  \brief       Get dsi video packet size.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video packet size.
               bit[13..0] This field configures the number of pixels in a single video packet.
*/
static inline uint32_t dsi_get_video_packet_size(DSI_Type *dsi)
{
    return dsi->DSI_VID_PKT_SIZE;
}

/**
  \fn          static inline void dsi_set_video_number_chunks(DSI_Type *dsi, uint32_t pkt_size)
  \brief       Set dsi video number chunks.
  \param[in]   dsi         Pointer to the dsi register map.
  \param[in]   num_chunks  dsi video number chunks to set.
               bit[12..0] This register configures the number of chunks to be transmitted during a Line period.
  \return      none.
*/
static inline void dsi_set_video_number_chunks(DSI_Type *dsi, uint32_t num_chunks)
{
    dsi->DSI_VID_NUM_CHUNKS &= ~DSI_VID_NUMBER_CHUNKS_MASK;
    dsi->DSI_VID_NUM_CHUNKS |= (num_chunks << DSI_VID_NUMBER_CHUNKS);
}

/**
  \fn          static inline uint32_t dsi_get_video_number_chunks(DSI_Type *dsi)
  \brief       Get dsi video number chunks.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video number chunks.
               bit[12..0] This register configures the number of chunks to be transmitted during a Line period.
*/
static inline uint32_t dsi_get_video_number_chunks(DSI_Type *dsi)
{
    return dsi->DSI_VID_NUM_CHUNKS;
}

/**
  \fn          static inline void dsi_set_video_null_packet_size(DSI_Type *dsi, uint32_t pkt_size)
  \brief       Set dsi video null packet size.
  \param[in]   dsi         Pointer to the dsi register map.
  \param[in]   null_size   dsi video null packet size.
               bit[12..0] This register configures the number of bytes inside a null packet.
  \return      none.
*/
static inline void dsi_set_video_null_packet_size(DSI_Type *dsi, uint32_t null_size)
{
    dsi->DSI_VID_NULL_SIZE &= ~DSI_VID_NULL_PKT_SIZE_MASK;
    dsi->DSI_VID_NULL_SIZE |= (null_size << DSI_VID_NULL_PKT_SIZE);
}

/**
  \fn          static inline uint32_t dsi_get_video_null_packet_size(DSI_Type *dsi)
  \brief       Get dsi video null packet size.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video null packet size.
               bit[12..0] This register configures the number of chunks to be transmitted during a Line period.
*/
static inline uint32_t dsi_get_video_null_packet_size(DSI_Type *dsi)
{
    return dsi->DSI_VID_NULL_SIZE;
}

/**
  \fn          static inline void dsi_set_video_hsa_time(DSI_Type *dsi, uint32_t hsa)
  \brief       Set dsi video hsa time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   hsa   dsi video hsa time.
               bit[11..0] This field configures the Horizontal Synchronism Active period in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_video_hsa_time(DSI_Type *dsi, uint32_t hsa)
{
    dsi->DSI_VID_HSA_TIME &= ~DSI_VIDEO_HSA_TIME_MASK;
    dsi->DSI_VID_HSA_TIME |= (hsa << DSI_VIDEO_HSA_TIME);
}

/**
  \fn          static inline uint32_t dsi_get_video_hsa_time(DSI_Type *dsi)
  \brief       Get dsi video hsa time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video hsa time.
               bit[11..0] This field configures the Horizontal Synchronism Active period in LANEBYTECLK cycles.
*/
static inline uint32_t dsi_get_video_hsa_time(DSI_Type *dsi)
{
    return dsi->DSI_VID_HSA_TIME;
}

/**
  \fn          static inline void dsi_set_video_hbp_time(DSI_Type *dsi, uint32_t hbp)
  \brief       Set dsi video hbp time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   hbp   dsi video hbp time.
               bit[11..0] This field configures the HBP period in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_video_hbp_time(DSI_Type *dsi, uint32_t hbp)
{
    dsi->DSI_VID_HBP_TIME &= ~DSI_VIDEO_HBP_TIME_MASK;
    dsi->DSI_VID_HBP_TIME |= (hbp << DSI_VIDEO_HBP_TIME);
}

/**
  \fn          static inline uint32_t dsi_get_video_hbp_time(DSI_Type *dsi)
  \brief       Get dsi video hbp time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video hbp time.
               bit[11..0] This field configures the HBP period in LANEBYTECLK cycles.
*/
static inline uint32_t dsi_get_video_hbp_time(DSI_Type *dsi)
{
    return dsi->DSI_VID_HBP_TIME;
}

/**
  \fn          static inline void dsi_set_video_hline_time(DSI_Type *dsi, uint32_t hline)
  \brief       Set dsi video hline time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   hline   dsi video hline time.
               bit[14..0] This field configures the size of the total line time (HSA + HBP + HACT + HFP) counted in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_video_hline_time(DSI_Type *dsi, uint32_t hline)
{
    dsi->DSI_VID_HLINE_TIME &= ~DSI_VIDEO_HLINE_TIME_MASK;
    dsi->DSI_VID_HLINE_TIME |= (hline << DSI_VIDEO_HLINE_TIME);
}

/**
  \fn          static inline uint32_t dsi_get_video_hline_time(DSI_Type *dsi)
  \brief       Get dsi video hline time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video hline time.
               bit[14..0] This field configures the size of the total line time (HSA + HBP + HACT + HFP) counted in LANEBYTECLK cycles.
*/
static inline uint32_t dsi_get_video_hline_time(DSI_Type *dsi)
{
    return dsi->DSI_VID_HLINE_TIME;
}

/**
  \fn          static inline void dsi_set_video_vsa_lines(DSI_Type *dsi, uint32_t vsa)
  \brief       Set dsi video vsa lines.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   vsa    dsi video vsa lines.
               bit[9..0] This field configures the VSA period measured in number of horizontal lines.
  \return      none.
*/
static inline void dsi_set_video_vsa_lines(DSI_Type *dsi, uint32_t vsa)
{
    dsi->DSI_VID_VSA_LINES &= ~DSI_VSA_LINES_MASK;
    dsi->DSI_VID_VSA_LINES |= (vsa << DSI_VSA_LINES);
}

/**
  \fn          static inline uint32_t dsi_get_video_vsa_lines(DSI_Type *dsi)
  \brief       Get dsi video vsa lines.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video vsa time.
               bit[9..0] This field configures the VSA period measured in number of horizontal lines.
*/
static inline uint32_t dsi_get_video_vsa_lines(DSI_Type *dsi)
{
    return dsi->DSI_VID_VSA_LINES;
}

/**
  \fn          static inline void dsi_set_video_vbp_lines(DSI_Type *dsi, uint32_t vbp)
  \brief       Set dsi video vbp lines.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   vbp    dsi video vbp lines.
               bit[9..0] This field configures the VBP period measured in number of horizontal lines.
  \return      none.
*/
static inline void dsi_set_video_vbp_lines(DSI_Type *dsi, uint32_t vbp)
{
    dsi->DSI_VID_VBP_LINES &= ~DSI_VBP_LINES_MASK;
    dsi->DSI_VID_VBP_LINES |= (vbp << DSI_VBP_LINES);
}

/**
  \fn          static inline uint32_t dsi_get_video_vbp_lines(DSI_Type *dsi)
  \brief       Get dsi video vbp lines.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video vbp time.
               bit[9..0] This field configures the VBP period measured in number of horizontal lines.
*/
static inline uint32_t dsi_get_video_vbp_lines(DSI_Type *dsi)
{
    return dsi->DSI_VID_VBP_LINES;
}

/**
  \fn          static inline void dsi_set_video_vfp_lines(DSI_Type *dsi, uint32_t vfp)
  \brief       Set dsi video vfp lines.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   vfp    dsi video vfp lines.
               bit[9..0] This field configures the VFP period measured in number of horizontal lines.
  \return      none.
*/
static inline void dsi_set_video_vfp_lines(DSI_Type *dsi, uint32_t vfp)
{
    dsi->DSI_VID_VFP_LINES &= ~DSI_VFP_LINES_MASK;
    dsi->DSI_VID_VFP_LINES |= (vfp << DSI_VFP_LINES);
}

/**
  \fn          static inline uint32_t dsi_get_video_vfp_lines(DSI_Type *dsi)
  \brief       Get dsi video vfp lines.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video vfp time.
               bit[9..0] This field configures the VFP period measured in number of horizontal lines.
*/
static inline uint32_t dsi_get_video_vfp_lines(DSI_Type *dsi)
{
    return dsi->DSI_VID_VFP_LINES;
}

/**
  \fn          static inline void dsi_set_video_vactive_lines(DSI_Type *dsi, uint32_t vactive)
  \brief       Set dsi video vactive lines.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   vactive    dsi video vactive lines.
               bit[9..0] This field configures the VACTIVE period measured in number of horizontal lines.
  \return      none.
*/
static inline void dsi_set_video_vactive_lines(DSI_Type *dsi, uint32_t vactive)
{
    dsi->DSI_VID_VACTIVE_LINES &= ~DSI_V_ACTIVE_LINES_MASK;
    dsi->DSI_VID_VACTIVE_LINES |= (vactive << DSI_V_ACTIVE_LINES);
}

/**
  \fn          static inline uint32_t dsi_get_video_vactive_lines(DSI_Type *dsi)
  \brief       Get dsi video vactive lines.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi video vactive time.
               bit[9..0] This field configures the VACTIVE period measured in number of horizontal lines.
*/
static inline uint32_t dsi_get_video_vactive_lines(DSI_Type *dsi)
{
    return dsi->DSI_VID_VACTIVE_LINES;
}

/**
  \fn          static inline void dsi_set_command_mode_config(DSI_Type *dsi, uint32_t cfg)
  \brief       Set dsi command mode configuration to low power transition.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   cfg   dsi command mode configuration to set.
  \return      none.
*/
static inline void dsi_set_command_mode_config(DSI_Type *dsi, DSI_CMD_MODE_CFG cfg)
{
    dsi->DSI_CMD_MODE_CFG |= cfg;
}

/**
  \fn          static inline void dsi_set_command_mode_config(DSI_Type *dsi)
  \brief       Get dsi command mode configuration to  low power transition.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi command mode configuration.
*/
static inline DSI_CMD_MODE_CFG dsi_get_command_mode_config(DSI_Type *dsi)
{
    return dsi->DSI_CMD_MODE_CFG;
}

/**
  \fn          static inline void dsi_reception_enable(DSI_Type *dsi)
  \brief       Enable dsi reception using bus turnaround.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_reception_enable(DSI_Type *dsi)
{
    dsi->DSI_BTA_TO_CNT = DSI_BTA_TIMEOUT_COUNT;
    dsi->DSI_PCKHDL_CFG |= DSI_BTA_EN_MASK | DSI_ECC_RX_EN_MASK | DSI_CRC_RX_EN_MASK;
}

/**
  \fn          static inline void dsi_hs_eotp_enable(DSI_Type *dsi)
  \brief       Enable dsi end of transmission packet in hs mode.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_hs_eotp_enable(DSI_Type *dsi)
{
    dsi->DSI_PCKHDL_CFG |= DSI_EOTP_TX_EN_MASK;
}

/**
  \fn          static inline void dsi_auto_clklane_enable(DSI_Type *dsi)
  \brief       Enable dsi automatic mechanism on clock lane.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_auto_clklane_enable(DSI_Type *dsi)
{
    dsi->DSI_LPCLK_CTRL |= DSI_AUTO_CLKLANE_CTRL_MASK;
}

/**
  \fn          static inline void dsi_phy_txrequestclkhs_disable(DSI_Type *dsi)
  \brief       Disable dsi txrequestclkhs.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_txrequestclkhs_disable(DSI_Type *dsi)
{
    dsi->DSI_LPCLK_CTRL &= ~DSI_PHY_TXREQUESTCLKHS_MASK;
}

/**
  \fn          static inline void dsi_phy_txrequestclkhs_enable(DSI_Type *dsi)
  \brief       Enable dsi txrequestclkhs.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_txrequestclkhs_enable(DSI_Type *dsi)
{
    dsi->DSI_LPCLK_CTRL |= DSI_PHY_TXREQUESTCLKHS_MASK;
}

/**
  \fn          static inline void dsi_auto_clklane_disable(DSI_Type *dsi)
  \brief       Disable dsi automatic mechanism on clock lane.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_auto_clklane_disable(DSI_Type *dsi)
{
    dsi->DSI_LPCLK_CTRL &= ~DSI_AUTO_CLKLANE_CTRL_MASK;
}

/**
  \fn          static inline void dsi_set_phy_clklp2hs_time(DSI_Type *dsi, uint32_t time)
  \brief       Set dsi phy clklp2hs time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   time   dsi phy clklp2hs time to set.
               bit[9..0] This field configures the maximum time that the D-PHY clock lane takes to go from low-power to high-speed transmission measured in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_phy_clklp2hs_time(DSI_Type *dsi, uint32_t time)
{
    dsi->DSI_PHY_TMR_LPCLK_CFG &= ~DSI_PHY_CLKLP2HS_TIME_MASK;
    dsi->DSI_PHY_TMR_LPCLK_CFG |= (time << DSI_PHY_CLKLP2HS_TIME);
}

/**
  \fn          sstatic inline uint32_t dsi_get_phy_clklp2hs_time(DSI_Type *dsi)
  \brief       Get dsi phy clklp2hs time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi phy clklp2hs time.
               bit[9..0] This field configures the maximum time that the D-PHY clock lane takes to go from low-power to high-speed transmission measured in LANEBYTECLK cycles.
*/
static inline uint32_t dsi_get_phy_clklp2hs_time(DSI_Type *dsi)
{
    return (dsi->DSI_PHY_TMR_LPCLK_CFG & DSI_PHY_CLKLP2HS_TIME_MASK) >> DSI_PHY_CLKLP2HS_TIME;
}

/**
  \fn          static inline void dsi_set_phy_clkhs2lp_time(DSI_Type *dsi, uint32_t time)
  \brief       Set dsi phy clkhs2lp time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   time   dsi phy clkhs2lp time to set.
               bit[9..0] This field configures the maximum time that the D-PHY clock lane takes to go from high-speed to low-power transmission measured in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_phy_clkhs2lp_time(DSI_Type *dsi, uint32_t time)
{
    dsi->DSI_PHY_TMR_LPCLK_CFG &= ~DSI_PHY_CLKHS2LP_TIME_MASK;
    dsi->DSI_PHY_TMR_LPCLK_CFG |= (time << DSI_PHY_CLKHS2LP_TIME);
}

/**
  \fn          static inline uint32_t dsi_get_phy_clkhs2lp_time(DSI_Type *dsi)
  \brief       Get dsi phy clkhs2lp time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi phy clkhs2lp time.
               bit[9..0] This field configures the maximum time that the D-PHY clock lane takes to go from low-power to high-speed transmission measured in LANEBYTECLK cycles
*/
static inline uint32_t dsi_get_phy_clkhs2lp_time(DSI_Type *dsi)
{
    return (dsi->DSI_PHY_TMR_LPCLK_CFG & DSI_PHY_CLKHS2LP_TIME_MASK) >> DSI_PHY_CLKHS2LP_TIME;
}

/**
  \fn          static inline void dsi_set_phy_lp2hs_time(DSI_Type *dsi, uint32_t time)
  \brief       Set dsi phy data lane lp2hs time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   time   dsi phy data lane lp2hs time to set.
               bit[9..0] This field configures the maximum time that the D-PHY data lanes take to go from low-power to high-speed transmission measured in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_phy_lp2hs_time(DSI_Type *dsi, uint32_t time)
{
    dsi->DSI_PHY_TMR_CFG &= ~DSI_PHY_LP2HS_TIME_MASK;
    dsi->DSI_PHY_TMR_CFG |= (time << DSI_PHY_LP2HS_TIME);
}

/**
  \fn          static inline uint32_t dsi_get_phy_lp2hs_time(DSI_Type *dsi)
  \brief       Get dsi phy data lane lp2hs time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi phy data lane lp2hs time.
               bit[9..0] This field configures the maximum time that the D-PHY data lanes take to go from low-power to high-speed transmission measured in LANEBYTECLK cycles.
*/
static inline uint32_t dsi_get_phy_lp2hs_time(DSI_Type *dsi)
{
    return (dsi->DSI_PHY_TMR_CFG & DSI_PHY_LP2HS_TIME_MASK) >> DSI_PHY_LP2HS_TIME;
}

/**
  \fn          static inline void dsi_set_phy_hs2lp_time(DSI_Type *dsi, uint32_t time)
  \brief       Set dsi phy data lane hs2lp time.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   time   dsi phy data lane hs2lp time to set.
               bit[9..0] This field configures the maximum time that the D-PHY data lanes take to go from high-speed to low-power transmission measured in LANEBYTECLK cycles.
  \return      none.
*/
static inline void dsi_set_phy_hs2lp_time(DSI_Type *dsi, uint32_t time)
{
    dsi->DSI_PHY_TMR_CFG &= ~DSI_PHY_HS2LP_TIME_MASK;
    dsi->DSI_PHY_TMR_CFG |= (time << DSI_PHY_HS2LP_TIME);
}

/**
  \fn          static inline uint32_t dsi_get_phy_clkhs2lp_time(DSI_Type *dsi)
  \brief       Get dsi phy hs2lp time.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi phy data lanehs2lp time.
               bit[9..0] This field configures the maximum time that the D-PHY data lanes take to go from high-speed to low-power transmission measured in LANEBYTECLK cycles.
*/
static inline uint32_t dsi_get_phy_hs2lp_time(DSI_Type *dsi)
{
    return (dsi->DSI_PHY_TMR_CFG & DSI_PHY_HS2LP_TIME_MASK) >> DSI_PHY_HS2LP_TIME;
}

/**
  \fn          static inline void dsi_dphy_shutdown_enable(DSI_Type *dsi)
  \brief       Enable dsi phy shutdown.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_shutdown_enable(DSI_Type *dsi)
{
    dsi->DSI_PHY_RSTZ |= DSI_PHY_SHUTDOWNZ_MASK;
}

/**
  \fn          static inline void dsi_dphy_shutdown_disable(DSI_Type *dsi)
  \brief       Disable dsi phy shutdown.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_shutdown_disable(DSI_Type *dsi)
{
    dsi->DSI_PHY_RSTZ &= ~DSI_PHY_SHUTDOWNZ_MASK;
}

/**
  \fn          static inline void dsi_dphy_reset_enable(DSI_Type *dsi)
  \brief       Enable dsi phy reset.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_reset_enable(DSI_Type *dsi)
{
    dsi->DSI_PHY_RSTZ |= DSI_PHY_RST_MASK;
}

/**
  \fn          static inline void dsi_dphy_reset_disable(DSI_Type *dsi)
  \brief       Disable dsi phy reset.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_reset_disable(DSI_Type *dsi)
{
    dsi->DSI_PHY_RSTZ &= ~DSI_PHY_RST_MASK;
}

/**
  \fn          static inline void dsi_set_active_lanes(DSI_Type *dsi, DSI_N_LANES lanes)
  \brief       Set dsi active data lanes.
  \param[in]   dsi    Pointer to the dsi register map.
  \param[in]   lanes  Active lanes to set.
               bit[1..0] This field configures the number of active data lanes.
  \return      none.
*/
static inline void dsi_set_active_lanes(DSI_Type *dsi, DSI_N_LANES lanes)
{
    dsi->DSI_PHY_IF_CFG &= ~DSI_PHY_N_LANE_MASK;
    dsi->DSI_PHY_IF_CFG |= (lanes << DSI_PHY_N_LANE);
}

/**
  \fn          static inline DSI_N_LANES dsi_get_active_lanes(DSI_Type *dsi)
  \brief       Get dsi active data lanes.
  \param[in]   dsi    Pointer to the dsi register map.
  \return      lanes  Active lanes to set.
               bit[1..0] This field configures the number of active data lanes.
*/
static inline DSI_N_LANES dsi_get_active_lanes(DSI_Type *dsi)
{
    return (dsi->DSI_PHY_IF_CFG & DSI_PHY_N_LANE_MASK) >> DSI_PHY_N_LANE;
}

/**
  \fn          static inline void dsi_phy_enable_clock(DSI_Type *dsi)
  \brief       Enable dsi phy clock.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_enable_clock(DSI_Type *dsi)
{
    dsi->DSI_PHY_RSTZ |= DSI_PHY_ENABLECLK_MASK;
}

/**
  \fn          static inline void dsi_phy_disable_clock(DSI_Type *dsi)
  \brief       Disable dsi phy clock.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_disable_clock(DSI_Type *dsi)
{
    dsi->DSI_PHY_RSTZ &= ~DSI_PHY_ENABLECLK_MASK;
}

/**
  \fn          static inline DSI_PLL_STATUS dsi_get_phy_lock_status(DSI_Type *dsi)
  \brief       Get dsi phy pll lock status.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi phy pll lock status.
*/
static inline DSI_PLL_STATUS dsi_get_phy_lock_status(DSI_Type *dsi)
{
    return (dsi->DSI_PHY_STATUS & DSI_PHY_LOCK_MASK) >> DSI_PHY_LOCK;
}

/**
  \fn          static inline void dsi_phy_testclr_enable(DSI_Type *dsi)
  \brief       Enable dsi phy test clear.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_testclr_enable(DSI_Type *dsi)
{
    dsi->DSI_PHY_TST_CTRL0 |= DSI_PHY_TESTCLR_MASK;
}

/**
  \fn          static inline void dsi_phy_testclr_disable(DSI_Type *dsi)
  \brief       Disable dsi phy test clear.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      none
*/
static inline void dsi_phy_testclr_disable(DSI_Type *dsi)
{
    dsi->DSI_PHY_TST_CTRL0 &= ~DSI_PHY_TESTCLR_MASK;
}

/**
  \fn          static inline void dsi_irq0_enable(DSI_Type *dsi, uint32_t irqs)
  \brief       Enable dsi interrupt0.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   irqs    dsi interrupt0 to enable refer DSI_IRQ0_* macros.
  \return      none.
*/
static inline void dsi_irq0_enable(DSI_Type *dsi, uint32_t irqs)
{
    dsi->DSI_INT_MSK0 |= irqs;
}

/**
  \fn          static inline void dsi_irq1_enable(DSI_Type *dsi, uint32_t irqs)
  \brief       Enable dsi interrupt1.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   irqs    dsi interrupt1 to enable refer DSI_IRQ1_* macros.
  \return      none.
*/
static inline void dsi_irq1_enable(DSI_Type *dsi, uint32_t irqs)
{
    dsi->DSI_INT_MSK1 |= irqs;
}

/**
  \fn          static inline uint32_t dsi_irq0_status(DSI_Type *dsi)
  \brief       Get dsi interrupt0 status.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi interrupt0 status refer DSI_IRQ0_* macros.
*/
static inline uint32_t dsi_irq0_status(DSI_Type *dsi)
{
    return dsi->DSI_INT_ST0;
}

/**
  \fn          static inline uint32_t dsi_irq1_status(DSI_Type *dsi)
  \brief       Get dsi interrupt1 status.
  \param[in]   dsi     Pointer to the dsi register map.
  \return      dsi interrupt1 status refer DSI_IRQ1_* macros.
*/
static inline uint32_t dsi_irq1_status(DSI_Type *dsi)
{
    return dsi->DSI_INT_ST1;
}

/**
  \fn          static inline void dsi_irq0_force(DSI_Type *dsi, uint32_t irqs)
  \brief       Force the dsi interrupt0 to occur.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   irqs    dsi interrupt0 to force refer DSI_IRQ0_* macros.
  \return      none.
*/
static inline void dsi_irq0_force(DSI_Type *dsi, uint32_t irqs)
{
    dsi->DSI_INT_FORCE0 |= irqs;
}

/**
  \fn          static inline void dsi_irq1_force(DSI_Type *dsi, uint32_t irqs)
  \brief       Force the dsi interrupt1 to occur.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   irqs    dsi interrupt1 to force refer DSI_IRQ1_* macros.
  \return      none.
*/
static inline void dsi_irq1_force(DSI_Type *dsi, uint32_t irqs)
{
    dsi->DSI_INT_FORCE0 |= irqs;
}

/**
  \fn          void dsi_dcs_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t data, uint8_t vc_id)
  \brief       Perform dsi dcs Short write.
  \param[in]   cmd is DCS command info.
  \param[in]   data to send.
  \param[in]   vc_id virtual channel ID.
  \return      none.
*/
void dsi_dcs_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t data, uint8_t vc_id);

/**
  \fn          void dsi_dcs_cmd_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t vc_id)
  \brief       Perform dsi DCS Short write only command.
  \param[in]   cmd is DCS command info.
  \param[in]   vc_id virtual channel ID.
  \return      none.
*/
void dsi_dcs_cmd_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t vc_id);

/**
  \fn          void dsi_dcs_long_write(DSI_Type *dsi, uint8_t cmd, uint32_t data, uint8_t vc_id)
  \brief       Perform dsi DCS Short write.
  \param[in]   data pointer to data buffer.
  \param[in]   len data buffer length.
  \param[in]   vc_id virtual channel ID.
  \return      none.
*/
void dsi_dcs_long_write(DSI_Type *dsi, uint8_t* data, uint32_t len, uint8_t vc_id);

/**
  \fn          DSI_LANE_STOPSTATE dsi_get_lane_stopstate_status(DSI_Type *dsi, DSI_LANE lane)
  \brief       Get dsi lane stopstate status.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   lane    dsi lane.
  \return      dsi lane stopstate status.
*/
DSI_LANE_STOPSTATE dsi_get_lane_stopstate_status(DSI_Type *dsi, DSI_LANE lane);

#ifdef __cplusplus
}
#endif
#endif /* DSI_H_ */
