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
 * @file     csi.h
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     12-April-2023
 * @brief    Low level driver Specific header file.
 ******************************************************************************/

#ifndef CSI_H_
#define CSI_H_

#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

/**
  * @brief CSI (CSI)
  */
typedef struct {                                           /*!< (@ 0x49033000) CSI Structure                                     */
    volatile       uint32_t  CSI_VERSION;                  /*!< (@ 0x00000000) Reserved                                          */
    volatile       uint32_t  CSI_N_LANES;                  /*!< (@ 0x00000004) Lane Configuration Register                       */
    volatile       uint32_t  CSI_CSI2_RESETN;              /*!< (@ 0x00000008) CSI Controller Reset Control Register             */
    volatile const uint32_t  CSI_INT_ST_MAIN;              /*!< (@ 0x0000000C) Main Interrupt Status Register                    */
    volatile       uint32_t  CSI_DATA_IDS_1;               /*!< (@ 0x00000010) DT Data ID Monitors Configuration Register        */
    volatile const uint32_t  RESERVED[7];
    volatile       uint32_t  CSI_DATA_IDS_VC_1;            /*!< (@ 0x00000030) VC Data ID Monitors Configuration Register        */
    volatile const uint32_t  RESERVED1[3];
    volatile       uint32_t  CSI_PHY_SHUTDOWNZ;            /*!< (@ 0x00000040) PHY Shutdown Control Register                     */
    volatile       uint32_t  CSI_DPHY_RSTZ;                /*!< (@ 0x00000044) PHY Reset Control Register                        */
    volatile       uint32_t  CSI_PHY_RX;                   /*!< (@ 0x00000048) PHY RX Signals Status Register                    */
    volatile const uint32_t  CSI_PHY_STOPSTATE;            /*!< (@ 0x0000004C) PHY STOPSTATE Signal Status Register              */
    volatile       uint32_t  CSI_PHY_TEST_CTRL0;           /*!< (@ 0x00000050) PHY Test Control 0 Register                       */
    volatile       uint32_t  CSI_PHY_TEST_CTRL1;           /*!< (@ 0x00000054) PHY Test Control 1 Register                       */
    volatile const uint32_t  RESERVED2[10];
    volatile       uint32_t  CSI_IPI_MODE;                 /*!< (@ 0x00000080) IPI Mode Selection Register                       */
    volatile       uint32_t  CSI_IPI_VCID;                 /*!< (@ 0x00000084) IPI VC Selection Register                         */
    volatile       uint32_t  CSI_IPI_DATA_TYPE;            /*!< (@ 0x00000088) IPI DT Selection Register                         */
    volatile       uint32_t  CSI_IPI_MEM_FLUSH;            /*!< (@ 0x0000008C) IPI Memory Flush Control Register                 */
    volatile       uint32_t  CSI_IPI_HSA_TIME;             /*!< (@ 0x00000090) IPI HSA Configuration Register                    */
    volatile       uint32_t  CSI_IPI_HBP_TIME;             /*!< (@ 0x00000094) IPI HBP Configuration Register                    */
    volatile       uint32_t  CSI_IPI_HSD_TIME;             /*!< (@ 0x00000098) IPI HSD Configuration Register                    */
    volatile       uint32_t  CSI_IPI_HLINE_TIME;           /*!< (@ 0x0000009C) IPI HLINE Configuration Register                  */
    volatile       uint32_t  CSI_IPI_SOFTRSTN;             /*!< (@ 0x000000A0) IPI Reset Control Register                        */
    volatile const uint32_t  RESERVED3[2];
    volatile       uint32_t  CSI_IPI_ADV_FEATURES;         /*!< (@ 0x000000AC) IPI Advanced Features Configuration Register      */
    volatile       uint32_t  CSI_IPI_VSA_LINES;            /*!< (@ 0x000000B0) IPI VSA Configuration Register                    */
    volatile       uint32_t  CSI_IPI_VBP_LINES;            /*!< (@ 0x000000B4) IPI VBP Configuration Register                    */
    volatile       uint32_t  CSI_IPI_VFP_LINES;            /*!< (@ 0x000000B8) IPI VFP Configuration Register                    */
    volatile       uint32_t  CSI_IPI_VACTIVE_LINES;        /*!< (@ 0x000000BC) IPI VACTIVE Configuration Register                */
    volatile const uint32_t  RESERVED4[2];
    volatile       uint32_t  CSI_VC_EXTENSION;             /*!< (@ 0x000000C8) VC Extension Configuration Register               */
    volatile       uint32_t  CSI_PHY_CAL;                  /*!< (@ 0x000000CC) PHY CALIBRATION Signal Status Register            */
    volatile const uint32_t  RESERVED5[4];
    volatile const uint32_t  CSI_INT_ST_PHY_FATAL;         /*!< (@ 0x000000E0) PHY Packet Discarded Interrupt Status Register    */
    volatile       uint32_t  CSI_INT_MSK_PHY_FATAL;        /*!< (@ 0x000000E4) PHY Packet Discarded Interrupt Mask Register      */
    volatile       uint32_t  CSI_INT_FORCE_PHY_FATAL;      /*!< (@ 0x000000E8) PHY Packet Discarded Interrupt Force Register     */
    volatile const uint32_t  RESERVED6;
    volatile const uint32_t  CSI_INT_ST_PKT_FATAL;         /*!< (@ 0x000000F0) PHY Packet Construction Interrupt Status Register */
    volatile       uint32_t  CSI_INT_MSK_PKT_FATAL;        /*!< (@ 0x000000F4) PHY Packet Construction Interrupt Mask Register   */
    volatile       uint32_t  CSI_INT_FORCE_PKT_FATAL;      /*!< (@ 0x000000F8) PHY Packet Construction Interrupt Force Register  */
    volatile const uint32_t  RESERVED7[5];
    volatile const uint32_t  CSI_INT_ST_PHY;               /*!< (@ 0x00000110) PHY Interrupt Status Register                     */
    volatile       uint32_t  CSI_INT_MSK_PHY;              /*!< (@ 0x00000114) PHY Interrupt Mask Register                       */
    volatile       uint32_t  CSI_INT_FORCE_PHY;            /*!< (@ 0x00000118) PHY Interrupt Force Register                      */
    volatile const uint32_t  RESERVED8[5];
    volatile const uint32_t  CSI_INT_ST_LINE;              /*!< (@ 0x00000130) PHY Line Construction Interrupt Status Register   */
    volatile       uint32_t  CSI_INT_MSK_LINE;             /*!< (@ 0x00000134) PHY Line Construction Interrupt Mask Register     */
    volatile       uint32_t  CSI_INT_FORCE_LINE;           /*!< (@ 0x00000138) PHY Line Construction Interrupt Force Register    */
    volatile const uint32_t  RESERVED9;
    volatile const uint32_t  CSI_INT_ST_IPI_FATAL;         /*!< (@ 0x00000140) IPI Interface Interrupt Status Register           */
    volatile       uint32_t  CSI_INT_MSK_IPI_FATAL;        /*!< (@ 0x00000144) IPI Interface Interrupt Mask Register             */
    volatile       uint32_t  CSI_INT_FORCE_IPI_FATAL;      /*!< (@ 0x00000148) IPI Interface Interrupt Force Register            */
    volatile const uint32_t  RESERVED10[77];
    volatile const uint32_t  CSI_INT_ST_BNDRY_FRAME_FATAL; /*!< (@ 0x00000280) Frame Boundary Error Interrupt Status Register    */
    volatile       uint32_t  CSI_INT_MSK_BNDRY_FRAME_FATAL;/*!< (@ 0x00000284) Frame Boundary Error Interrupt Mask Register      */
    volatile       uint32_t  CSI_INT_FORCE_BNDRY_FRAME_FATAL;/*!< (@ 0x00000288) Frame Boundary Error Interrupt Force Register   */
    volatile const uint32_t  RESERVED11;
    volatile const uint32_t  CSI_INT_ST_SEQ_FRAME_FATAL;   /*!< (@ 0x00000290) Frame Sequence Error Interrupt Status Register    */
    volatile       uint32_t  CSI_INT_MSK_SEQ_FRAME_FATAL;  /*!< (@ 0x00000294) Frame Sequence Error Interrupt Mask Register      */
    volatile       uint32_t  CSI_INT_FORCE_SEQ_FRAME_FATAL;/*!< (@ 0x00000298) Frame Sequence Error Interrupt Force Register     */
    volatile const uint32_t  RESERVED12;
    volatile const uint32_t  CSI_INT_ST_CRC_FRAME_FATAL;   /*!< (@ 0x000002A0) Frame CRC Error Interrupt Status Register         */
    volatile       uint32_t  CSI_INT_MSK_CRC_FRAME_FATAL;  /*!< (@ 0x000002A4) Frame CRC Error Interrupt Mask Register           */
    volatile       uint32_t  CSI_INT_FORCE_CRC_FRAME_FATAL;/*!< (@ 0x000002A8) Frame CRC Error Interrupt Force Register          */
    volatile const uint32_t  RESERVED13;
    volatile const uint32_t  CSI_INT_ST_PLD_CRC_FATAL;     /*!< (@ 0x000002B0) Frame Payload Error Interrupt Status Register     */
    volatile       uint32_t  CSI_INT_MSK_PLD_CRC_FATAL;    /*!< (@ 0x000002B4) Frame Payload Error Interrupt Mask Register       */
    volatile       uint32_t  CSI_INT_FORCE_PLD_CRC_FATAL;  /*!< (@ 0x000002B8) Frame Payload Error Interrupt Force Register      */
    volatile const uint32_t  RESERVED14;
    volatile const uint32_t  CSI_INT_ST_DATA_ID;           /*!< (@ 0x000002C0) DT Error Interrupt Status Register                */
    volatile       uint32_t  CSI_INT_MSK_DATA_ID;          /*!< (@ 0x000002C4) DT Error Interrupt Mask Register                  */
    volatile       uint32_t  CSI_INT_FORCE_DATA_ID;        /*!< (@ 0x000002C8) DT Error Interrupt Force Register                 */
    volatile const uint32_t  RESERVED15;
    volatile const uint32_t  CSI_INT_ST_ECC_CORRECT;       /*!< (@ 0x000002D0) ECC Interrupt Status Register                     */
    volatile       uint32_t  CSI_INT_MSK_ECC_CORRECT;      /*!< (@ 0x000002D4) ECC Interrupt Mask Register                       */
    volatile       uint32_t  CSI_INT_FORCE_ECC_CORRECT;    /*!< (@ 0x000002D8) ECC Interrupt Force Register                      */
    volatile const uint32_t  RESERVED16[9];
    volatile       uint32_t  CSI_SCRAMBLING;               /*!< (@ 0x00000300) Descrambling Control Register                     */
    volatile       uint32_t  CSI_SCRAMBLING_SEED1;         /*!< (@ 0x00000304) Descrambling Seed Configuration Lane 0 Register   */
    volatile       uint32_t  CSI_SCRAMBLING_SEED2;         /*!< (@ 0x00000308) Descrambling Seed Configuration Lane 1 Register   */
} CSI_Type;                                                /*!< Size = 780 (0x30c)                                               */

/* CSI2 Parameters */
#define CSI_IPI_FIFO_DEPTH                           1024
#define CSI2_HOST_IPI_DWIDTH                         64
#define CSI2_HSD_MIN                                 1
#define CSI2_HSA_MIN                                 2
#define CSI2_HBP_MIN                                 10
#define CSI2_SHORT_PKT_BYTES                         4
#define CSI2_LONG_PKT_BYTES                          6
#define CSI2_BYTES_PER_HS_CLK                        1

/* CSI N LANES register (CSI_N_LANES) bit[2:0] */
#define CSI_N_LANES_Pos                              0U
#define CSI_N_LANES_Msk                              (0x7U << CSI_N_LANES_Pos)

/* CSI software reset register (CSI_CSI2_RESETN) bit[0] */
#define CSI_RESETN                                   (1U << 0)

/* CSI PHY SHUTDOWN register (CSI_PHY_SHUTDOWNZ) bit[0] */
#define CSI_PHY_SHUTDOWN                             (1U << 0)

/* CSI DPHY reset register (CSI_DPHY_RSTZ) bit[0] */
#define CSI_PHY_RST                                  (1U << 0)

/* CSI PHY test control register (CSI_PHY_STOPSTATE) bit[0] */
#define CSI_PHY_TESTCLR                              (1U << 0)

/*PHY_STOPSTATE register bits parameters*/
#define CSI_PHY_STOPSTATEDATA_0_Pos                  0U
#define CSI_PHY_STOPSTATEDATA_0_Msk                  (0x1U << CSI_PHY_STOPSTATEDATA_0_Pos)
#define CSI_PHY_STOPSTATEDATA_1_Pos                  1U
#define CSI_PHY_STOPSTATEDATA_1_Msk                  (0x1U << CSI_PHY_STOPSTATEDATA_1_Pos)
#define CSI_PHY_STOPSTATECLK_Pos                     16U
#define CSI_PHY_STOPSTATECLK_Msk                     (0x1U << CSI_PHY_STOPSTATECLK_Pos)

/* CSI Register (CSI_IPI_MODE) bit Definition, Macros, Offsets and Masks
 * these include IPI mode, IPI enable, IPI color component.
 */
/* IPI mode bit[0] */
#define CSI_IPI_MOD                                  (1U << 0)

/* IPI color component bit[8] */
#define CSI_IPI_COLOR_COM                            (1U << 8)

/* IPI enable bit[24] */
#define CSI_IPI_ENABLE                               (1U << 24)

/* CSI IPI virtual channel ID register (CSI_IPI_VCID) bits[1:0] */
#define CSI_IPI_VC_ID_Pos                            0U
#define CSI_IPI_VC_ID_Msk                            (0x3U << CSI_IPI_VC_ID_Pos)

/* CSI IPI data type register (CSI_IPI_DATA_TYPE) bits[5:0} */
#define CSI_IPI_DT_TYPE_Pos                          0U
#define CSI_IPI_DT_TYPE_Msk                          (0x3FU << CSI_IPI_DT_TYPE_Pos)

/* CSI Register (CSI_IPI_MEM_FLUSH) bit Definition, Macros, Offsets and Masks
 * these include auto and manual memory flush bits.
 */
/* CSI IPI manual memory flush bit[0] */
#define CSI_IPI_FLUSH                                (1U << 0)

/* CSI IPI auto memory flush bit[8] */
#define CSI_IPI_AUTO_FLUSH                           (1U << 8)

/* CSI IPI HSA time register (CSI_IPI_HSA_TIME) bits[11:0] */
#define CSI_IPI_HSA_TIME_Pos                         0U
#define CSI_IPI_HSA_TIME_Msk                         (0xFFFU << CSI_IPI_HSA_TIME_Pos)

/* CSI IPI HBP time register (CSI_IPI_HBP_TIME) bitS[11:0] */
#define CSI_IPI_HBP_TIME_Pos                         0U
#define CSI_IPI_HBP_TIME_Msk                         (0xFFFU << CSI_IPI_HBP_TIME_Pos)

/* CSI IPI HSD time register (CSI_IPI_HSD_TIME) bits[11:0] */
#define CSI_IPI_HSD_TIME_Pos                         0U
#define CSI_IPI_HSD_TIME_Msk                         (0xFFFU << CSI_IPI_HSD_TIME_Pos)

/* CSI IPI HLINE time register (CSI_IPI_HLINE_TIME) bits[14:0] */
#define CSI_IPI_HLINE_TIME_Pos                       0U
#define CSI_IPI_HLINE_TIME_Msk                       (0x7FFFU << CSI_IPI_HLINE_TIME_Pos)

/* CSI IPI soft reset register (CSI_IPI_SOFTRSTN) bit[0] */
#define CSI_IPI_SOFTRST                              (1U << 0)

/* CSI Register (CSI_IPI_ADV_FEATURES) bit Definition, Macros, Offsets and Masks
 * these include data overwrite enable, data overwrite, video enable, blanking enable etc.
 */
/* CSI IPI data overwrite enable bit[0] */
#define CSI_IPI_DT_OVERWRITE                         (1U << 0)

/* CSI IPI data overwrite bits[13:8] */
#define CSI_IPI_DT_Pos                               8U
#define CSI_IPI_DT_Msk                               (0x3FU << CSI_IPI_DT_Pos)

/* CSI Event selection bit parameters */
/* CSI IPI line event select enable bit[16] */
#define CSI_IPI_LINE_EVENT_SELECTION                 (1U << 16)

/* CSI IPI video enable bit[17] */
#define CSI_IPI_EVENT_SELECTION_EN_VIDEO             (1U << 17)

/* CSI IPI line start enable bit[18] */
#define CSI_IPI_EVENT_SELECTION_EN_LINE_START        (1U << 18)

/* CSI IPI null enable bit[19] */
#define CSI_IPI_EVENT_SELECTION_EN_NULL              (1U << 19)

/* CSI IPI blanking enable bit[20] */
#define CSI_IPI_EVENT_SELECTION_EN_BLANKING          (1U << 20)

/* CSI IPI embedded enable bit[21] */
#define CSI_IPI_EVENT_SELECTION_EN_EMBEDDED          (1U << 21)

/* CSI IPI sync event type bit[24] */
#define CSI_IPI_SYNC_EVENT_MODE                      (1U << 24)

/* CSI IPI VSA lines register (CSI_IPI_VSA_LINES) bits[9:0] */
#define CSI_IPI_VSA_LINE_Pos                         0U
#define CSI_IPI_VSA_LINE_Msk                         (0x3FFU << CSI_IPI_VSA_LINE_Pos)

/* CSI IPI VBP lines register (CSI_IPI_VBP_LINES) bits[9:0] */
#define CSI_IPI_VBP_LINE_Pos                         0U
#define CSI_IPI_VBP_LINE_Msk                         (0x3FFU << CSI_IPI_VBP_LINE_Pos)

/* CSI IPI VFP lines register (CSI_IPI_VFP_LINES) bits[9:0] */
#define CSI_IPI_VFP_LINE_Pos                         0U
#define CSI_IPI_VFP_LINE_Msk                         (0x3FFU << CSI_IPI_VFP_LINE_Pos)

/* CSI IPI VACTIVE lines register (CSI_IPI_VACTIVE_LINES) bits[13:0] */
#define CSI_IPI_VACTIVE_LINE_Pos                     0U
#define CSI_IPI_VACTIVE_LINE_Msk                     (0x3FFFU << CSI_IPI_VACTIVE_LINE_Pos)

/* CSI interrupt mask registers */
/* CSI PHY packet discard interrupt mask bits[1:0] */
#define CSI_INT_PHY_FATAL_MASK                       0x3U

/* CSI PHY packet construction interrupt mask bits[0] */
#define CSI_INT_PKT_FATAL_MASK                       0x1U

/* CSI frame boundary error interrupt mask bits[15:0] */
#define CSI_INT_BNDRY_FRAME_FATAL_MASK               0xFFFFU

/* CSI frame sequence error interrupt mask bits[15:0] */
#define CSI_INT_SEQ_FRAME_FATAL_MASK                 0xFFFFU

/* CSI frame CRC error interrupt mask bits[15:0] */
#define CSI_INT_CRC_FRAME_FATAL_MASK                 0xFFFFU

/* CSI frame payload error interrupt mask bits[15:0] */
#define CSI_INT_PLD_CRC_FATAL_MASK                   0xFFFFU

/* CSI data ID error interrupt mask bits[15:0] */
#define CSI_INT_DATA_ID_MASK                         0xFFFFU

/* CSI frame ECC error interrupt mask bits[15:0] */
#define CSI_INT_ECC_CORRECT_MASK                     0xFFFFU

/* CSI PHY interrupt mask bit[17:16] and bit[1] */
#define CSI_INT_PHY_MASK                             0x30003U

/* CSI line interrupt mask bits[19:16] and bits[3:0] */
#define CSI_INT_LINE_MASK                            0xF000FU

/* CSI IPI interrupt mask bits[5:0] */
#define CSI_INT_IPI_FATAL_MASK                       0x3FU

/* CSI_IRQ control  bit parameters */
#define CSI_IRQ_PHY_FATAL                            (1U << 0)       /**< PHY packet discard IRQ */

#define CSI_IRQ_PKT_FATAL                            (1U << 1)       /**< PHY packet construction IRQ */

#define CSI_IRQ_BNDRY_FRAME_FATAL                    (1U << 2)       /**< Frame boundary error IRQ */

#define CSI_IRQ_SEQ_FRAME_FATAL                      (1U << 3)       /**< Frame sequence error IRQ */

#define CSI_IRQ_CRC_FRAME_FATAL                      (1U << 4)       /**< Frame CRC error IRQ */

#define CSI_IRQ_PLD_CRC_FATAL                        (1U << 5)       /**< Frame payload error IRQ */

#define CSI_IRQ_DATA_ID                              (1U << 6)       /**< Data ID IRQ */

#define CSI_IRQ_ECC_CORRECT                          (1U << 7)       /**< ECC IRQ */

#define CSI_IRQ_PHY                                  (1U << 16)      /**< PHY IRQ */

#define CSI_IRQ_LINE                                 (1U << 17)      /**< PHY line construction IRQ */

#define CSI_IRQ_IPI_FATAL                            (1U << 18)      /**< IPI IRQ */

/**
 * enum _CSI_N_LANES
 * CSI N lanes
*/
typedef enum _CSI_N_LANES
{
    CSI_N_LANES_1,                                 /**< Select 1 lane                                        */
    CSI_N_LANES_2                                  /**< Select 2 lanes                                       */
} CSI_N_LANES;

/**
 * enum  CSI_VC_ID
 * CSI virtual channel ID.
 */
typedef enum _CSI_VC_ID{
    CSI_VC_ID_0,             /**< Virtual channel ID 0 */
    CSI_VC_ID_1,             /**< Virtual channel ID 1 */
    CSI_VC_ID_2,             /**< Virtual channel ID 2 */
    CSI_VC_ID_3              /**< Virtual channel ID 3 */
}CSI_VC_ID;

/**
 * enum  CSI_LANE
 * CSI lane select.
 */
typedef enum _CSI_LANE
{
    CSI_LANE_CLOCK,                                /**< CSI clock lane                                       */
    CSI_LANE_0,                                    /**< CSI lane 0                                           */
    CSI_LANE_1                                     /**< CSI lane 1                                           */
}CSI_LANE;

/**
 * enum  CSI_LANE_STOPSTATE
 * CSI lane stopstate status.
 */
typedef enum _CSI_LANE_STOPSTATE
{
    CSI_LANE_STOPSTATE_OFF,                        /**< Status of the lane stopstate off                     */
    CSI_LANE_STOPSTATE_ON                          /**< Status of the lane stopstate on                      */
} CSI_LANE_STOPSTATE;
/**
 * enum  CSI_IPI_MODE
 * CSI IPI mode.
 */
typedef enum _CSI_IPI_MODE
{
    CSI_IPI_MODE_CAM_TIMIMG,                      /**< Camera timing                                        */
    CSI_IPI_MODE_CTRL_TIMING                      /**< Controller timing                                    */
} CSI_IPI_MODE;

/**
 * enum  CSI_IPI_COLOR_COM_TYPE
 * CSI IPI color component.
 */
typedef enum _CSI_IPI_COLOR_COM_TYPE
{
    CSI_IPI_COLOR_COM_TYPE_COLOR48,                /***< 48 bit interface                                    */
    CSI_IPI_COLOR_COM_TYPE_COLOR16                 /***< 16 bit interface                                    */
} CSI_IPI_COLOR_COM_TYPE;

/**
 * enum  CSI_IPI_VIDEO
 * CSI IPI video packets for sync events.
 */
typedef enum _CSI_IPI_VIDEO
{
    CSI_IPI_VIDEO_DISABLE,                         /***< Enable use of video packets for IPI sync events.     */
    CSI_IPI_VIDEO_ENABLE                           /***< Disable use of video packets for IPI sync even       */
} CSI_IPI_VIDEO;

/**
 * enum  CSI_IPI_LINE_START
 * CSI IPI line start packets for sync events.
 */
typedef enum _CSI_IPI_LINE_START
{
    CSI_IPI_LINE_START_DISABLE,                   /***< Enable use of line start packets for IPI sync events */
    CSI_IPI_LINE_START_ENABLE                     /***< Disable use of line start packets for IPI sync events*/
} CSI_IPI_LINE_START;

/**
 * enum  CSI_IPI_NULL
 * CSI IPI null packets for sync events.
 */
typedef enum _CSI_IPI_NULL
{
    CSI_IPI_NULL_DISABLE,                         /***< Enable use of null packets for IPI sync events.      */
    CSI_IPI_NULL_ENABLE                           /***< Disable use of null packets for IPI sync even        */
} CSI_IPI_NULL;

/**
 * enum  CSI_IPI_BLANKING
 * CSI IPI blanking packets for sync events.
 */
typedef enum _CSI_IPI_BLANKING
{
    CSI_IPI_BLANKING_DISABLE,                     /***< Enable use of blanking packets for IPI sync events.  */
    CSI_IPI_BLANKING_ENABLE                       /***< Disable use of blanking packets for IPI sync even    */
} CSI_IPI_BLANKING;

/**
 * enum  CSI_IPI_EMBEDDED
 * CSI IPI embedded packets for sync events.
 */
typedef enum _CSI_IPI_EMBEDDED
{
    CSI_IPI_EMBEDDED_DISABLE,                     /***< Enable use of embedded packets for IPI sync events.  */
    CSI_IPI_EMBEDDED_ENABLE                       /***< Disable use of embedded packets for IPI sync even    */
} CSI_IPI_EMBEDDED;

/**
 * enum  _CSI_IPI_SYNC_EVENT_SEL
 * CSI line event select.
 */
typedef enum _CSI_IPI_LINE_EVENT_SELECT
{
    CSI_IPI_LINE_EVENT_SELECT_AUTO,                /**< Controller selects it automatically                  */
    CSI_IPI_LINE_EVENT_SELECT_PROG                 /**< Select packets from list programmed in bits [21-17]  */
} CSI_IPI_LINE_EVENT_SELECT;

/**
 * enum  CSI_IPI_SYNC_EVENT
 * CSI sync event.
 */
typedef enum _CSI_IPI_SYNC_EVENT
{
    CSI_IPI_SYNC_EVENT_FSN,                        /**< Frame Start do not trigger any sync event.           */
    CSI_IPI_SYNC_EVENT_FS                          /**< Frame Start triggers a sync event.                   */
} CSI_IPI_SYNC_EVENT;

/**
 * enum CSI_DATA_TYPE
 * CSI data types supported
 */
typedef enum _CSI_DATA_TYPE
{
    CSI_DT_RGB444 = 0x20,                       /**< Data type RGB444 */
    CSI_DT_RGB555 = 0x21,                       /**< Data type RGB555 */
    CSI_DT_RGB565 = 0x22,                       /**< Data type RGB565 */
    CSI_DT_RGB666 = 0x23,                       /**< Data type RGB666 */
    CSI_DT_RGB888 = 0x24,                       /**< Data type RGB888 */
    CSI_DT_RAW6   = 0x28,                       /**< Data type RAW6   */
    CSI_DT_RAW7   = 0x29,                       /**< Data type RAW7   */
    CSI_DT_RAW8   = 0x2A,                       /**< Data type RAW8   */
    CSI_DT_RAW10  = 0x2B,                       /**< Data type RAW10  */
    CSI_DT_RAW12  = 0x2C,                       /**< Data type RAW12  */
    CSI_DT_RAW14  = 0x2D,                       /**< Data type RAW14  */
    CSI_DT_RAW16  = 0x2E                        /**< Data type RAW16  */
} CSI_DATA_TYPE;

/**
  \fn          void csi_set_n_active_lanes(CSI_Type *csi, CSI_N_LANES n_lanes)
  \brief       Set CSI number of active lanes.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   n_lanes 0: 1 Data Lane
                       1: 2 Data Lanes
  \return      none.
*/
static inline void csi_set_n_active_lanes(CSI_Type *csi, CSI_N_LANES n_lanes)
{
    csi->CSI_N_LANES &= ~CSI_N_LANES_Msk;
    csi->CSI_N_LANES |= n_lanes;
}

/**
  \fn          void csi_enable_software_reset_state(CSI_Type *csi)
  \brief       Enable the CSI software reset state.
  \param[in]   csi   Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_software_reset_state(CSI_Type *csi)
{
    csi->CSI_CSI2_RESETN &= ~CSI_RESETN;
}

/**
  \fn          void csi_disable_software_reset_state(CSI_Type *csi)
  \brief       Disable the CSI software reset state.
  \param[in]   csi   Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_software_reset_state(CSI_Type *csi)
{
    csi->CSI_CSI2_RESETN |= CSI_RESETN;
}

/**
  \fn          uint32_t csi_get_interrupt_status(CSI_Type *csi)
  \brief       Get CSI interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI interrupt status.
*/
static inline uint32_t csi_get_interrupt_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_MAIN;
}

/**
  \fn          void csi_enable_dphy_shutdown_line(CSI_Type *csi)
  \brief       Enable CSI DPHY shutdown line.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_dphy_shutdown_line(CSI_Type *csi)
{
    csi->CSI_PHY_SHUTDOWNZ |= CSI_PHY_SHUTDOWN;
}

/**
  \fn          void csi_disable_dphy_shutdown_line_disable(CSI_Type *csi)
  \brief       Disable CSI DPHY shutdown line.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_dphy_shutdown_line(CSI_Type *csi)
{
    csi->CSI_PHY_SHUTDOWNZ &= ~CSI_PHY_SHUTDOWN;
}

/**
  \fn          void csi_enable_dphy_reset_line(CSI_Type *csi)
  \brief       Enable CSI DPHY reset line.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_dphy_reset_line(CSI_Type *csi)
{
    csi->CSI_DPHY_RSTZ |= CSI_PHY_RST;
}

/**
  \fn          void csi_disable_dphy_reset_line(CSI_Type *csi)
  \brief       Disable CSI DPHY reset line.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_dphy_reset_line(CSI_Type *csi)
{
    csi->CSI_DPHY_RSTZ &= ~CSI_PHY_RST;
}

/**
  \fn          void csi_enable_dphy_testclr_line(CSI_Type *csi)
  \brief       Enable CSI DPHY test clear line.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_dphy_testclr_line(CSI_Type *csi)
{
    csi->CSI_PHY_TEST_CTRL0 |= CSI_PHY_TESTCLR;
}

/**
  \fn          void csi_disable_dphy_testclr_line(CSI_Type *csi)
  \brief       Disable CSI DPHY test clear line.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_dphy_testclr_line(CSI_Type *csi)
{
    csi->CSI_PHY_TEST_CTRL0 &= ~CSI_PHY_TESTCLR;
}

/**
  \fn          void csi_enable_ipi_mode(CSI_Type *csi)
  \brief       Enable CSI IPI mode.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_ipi_mode(CSI_Type *csi)
{
    csi->CSI_IPI_MODE |= CSI_IPI_ENABLE;
}

/**
  \fn          void csi_disable_ipi_mode(CSI_Type *csi)
  \brief       Disable CSI IPI mode.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_ipi_mode(CSI_Type *csi)
{
    csi->CSI_IPI_MODE &= ~CSI_IPI_ENABLE;
}

/**
  \fn          void csi_set_ipi_vc_id(CSI_Type *csi, uint32_t vc_id)
  \brief       Set CSI IPI virtual channel ID.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   vc_id   CSI IPI virtual channel ID to be set.
  \return      none.
*/
static inline void csi_set_ipi_vc_id(CSI_Type *csi, uint8_t vc_id)
{
    csi->CSI_IPI_VCID &= ~CSI_IPI_VC_ID_Msk;
    csi->CSI_IPI_VCID = vc_id;
}

/**
  \fn          void csi_set_ipi_data_type(CSI_Type *csi, uint32_t data_type)
  \brief       Set CSI IPI data type.
  \param[in]   csi       Pointer to the CSI register map.
  \param[in]   data_type CSI IPI data type to be set.
  \return      none.
*/
static inline void csi_set_ipi_data_type(CSI_Type *csi, uint8_t data_type)
{
    csi->CSI_IPI_DATA_TYPE &= ~CSI_IPI_DT_TYPE_Msk;
    csi->CSI_IPI_DATA_TYPE = data_type;
}

/**
  \fn          void csi_set_ipi_mem_flush_manual(CSI_Type *csi)
  \brief       Set CSI IPI manual memory flush method.
  \param[in]   csi       Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_ipi_mem_flush_manual(CSI_Type *csi)
{
    csi->CSI_IPI_MEM_FLUSH |= CSI_IPI_FLUSH;
}

/**
  \fn          void csi_enable_ipi_mem_flush_auto(CSI_Type *csi)
  \brief       Enable CSI IPI auto memory flush method.
  \param[in]   csi       Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_ipi_mem_flush_auto(CSI_Type *csi)
{
    csi->CSI_IPI_MEM_FLUSH |= CSI_IPI_AUTO_FLUSH;
}

/**
  \fn          void csi_disable_ipi_mem_flush_auto(CSI_Type *csi)
  \brief       Disable CSI IPI auto memory flush method.
  \param[in]   csi       Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_ipi_mem_flush_auto(CSI_Type *csi)
{
    csi->CSI_IPI_MEM_FLUSH &= ~CSI_IPI_AUTO_FLUSH;
}

/**
  \fn          void csi_set_packet_configuration(CSI_Type *csi, uint32_t packet_config)
  \brief       CSI IPI packet configuration.
  \param[in]   csi           Pointer to the CSI register map.
  \param[in]   packet_config IPI packet configuration (refer CSI_IPI_EVENT_SELECTION_* macros Bitmask).
  \return      none.
*/
static inline void csi_set_packet_configuration(CSI_Type *csi, uint32_t packet_config)
{
    csi->CSI_IPI_ADV_FEATURES |= packet_config;
}

/**
  \fn          void csi_enable_ipi_dt_overwrite(CSI_Type *csi)
  \brief       Enable CSI IPI data overwrite.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_enable_ipi_dt_overwrite(CSI_Type *csi)
{
    csi->CSI_IPI_ADV_FEATURES |= CSI_IPI_DT_OVERWRITE;
}

/**
  \fn          void csi_disable_ipi_dt_overwrite(CSI_Type *csi)
  \brief       Disable CSI IPI data overwrite.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_disable_ipi_dt_overwrite(CSI_Type *csi)
{
    csi->CSI_IPI_ADV_FEATURES &= ~CSI_IPI_DT_OVERWRITE;
}

/**
  \fn          void csi_set_ipi_dt_overwrite(CSI_Type *csi, uint8_t data)
  \brief       Set CSI IPI data overwrite.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   data    CSI IPI data to be overwrite
  \return      none.
*/
static inline void csi_set_ipi_dt_overwrite(CSI_Type *csi, uint8_t data)
{
    csi->CSI_IPI_ADV_FEATURES &= ~CSI_IPI_DT_Msk;
    csi->CSI_IPI_ADV_FEATURES |= data << CSI_IPI_DT_Pos;
}

/**
  \fn          uint32_t csi_get_phy_pkt_discard_intr_status(CSI_Type *csi)
  \brief       Get CSI PHY packet discarded interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI PHY packet discarded interrupt status.
*/
static inline uint32_t csi_get_phy_pkt_discard_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_PHY_FATAL;
}

/**
  \fn          void csi_set_phy_pkt_discard_intr_mask(CSI_Type *csi)
  \brief       Set CSI PHY packet discard interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_phy_pkt_discard_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PHY_FATAL |= CSI_INT_PHY_FATAL_MASK;
}

/**
  \fn          void csi_clear_phy_pkt_discard_intr_mask(CSI_Type *csi)
  \brief       Clear CSI PHY packet discard interrupt_mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_phy_pkt_discard_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PHY_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_phy_pkt_construction_intr_status(CSI_Type *csi)
  \brief       Get CSI PHY packet construction interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI PHY packet construction interrupt status.
*/
static inline uint32_t csi_get_phy_pkt_construction_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_PKT_FATAL;
}

/**
  \fn          void csi_set_phy_pkt_construction_intr_mask(CSI_Type *csi)
  \brief       Set CSI PHY packet construction interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_phy_pkt_construction_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PKT_FATAL |= CSI_INT_PKT_FATAL_MASK;
}

/**
  \fn          void csi_clear_phy_pkt_construction_intr_mask(CSI_Type *csi)
  \brief       Clear CSI PHY packet construction interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_phy_pkt_construction_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PKT_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_phy_intr_status(CSI_Type *csi)
  \brief       Get CSI PHY interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI PHY interrupt status.
*/
static inline uint32_t csi_get_phy_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_PHY;
}

/**
  \fn          void csi_set_phy_intr_mask(CSI_Type *csi)
  \brief       Set CSI PHY interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_phy_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PHY |= CSI_INT_PHY_MASK;
}

/**
  \fn          void csi_clear_phy_intr_mask(CSI_Type *csi)
  \brief       Clear CSI PHY interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_phy_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PHY = 0;
}

/**
  \fn          uint32_t csi_get_phy_line_construction_intr_status(CSI_Type *csi)
  \brief       Get CSI PHY line construction interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI PHY line construction interrupt status.
*/
static inline uint32_t csi_get_phy_line_construction_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_LINE;
}

/**
  \fn          uint32_t csi_set_phy_line_construction_intr_mask(CSI_Type *csi)
  \brief       Set CSI PHY line construction interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_phy_line_construction_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_LINE |= CSI_INT_LINE_MASK;
}

/**
  \fn          uint32_t csi_clear_phy_line_construction_intr_mask(CSI_Type *csi)
  \brief       Clear CSI PHY line construction interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_phy_line_construction_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_LINE = 0;
}

/**
  \fn          uint32_t csi_get_ipi_intr_status(CSI_Type *csi)
  \brief       Get CSI IPI interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI IPI interrupt status.
*/
static inline uint32_t csi_get_ipi_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_IPI_FATAL;
}

/**
  \fn          void csi_set_ipi_intr_mask(CSI_Type *csi)
  \brief       Set CSI IPI interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_ipi_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_IPI_FATAL |= CSI_INT_IPI_FATAL_MASK;
}

/**
  \fn          void csi_clear_ipi_intr_mask(CSI_Type *csi)
  \brief       Clear CSI IPI interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_ipi_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_IPI_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_frame_bndry_err_intr_status(CSI_Type *csi)
  \brief       Get CSI frame boundary error interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI frame boundary error interrupt status.
*/
static inline uint32_t csi_get_frame_bndry_err_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_BNDRY_FRAME_FATAL;
}

/**
  \fn          uint32_t csi_set_frame_bndry_err_intr_mask(CSI_Type *csi)
  \brief       Set CSI frame boundary error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_frame_bndry_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_BNDRY_FRAME_FATAL |= CSI_INT_BNDRY_FRAME_FATAL_MASK;
}

/**
  \fn          uint32_t csi_clear_frame_bndry_err_intr_mask(CSI_Type *csi)
  \brief       Clear CSI frame boundary error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_frame_bndry_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_BNDRY_FRAME_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_frame_seq_err_intr_status(CSI_Type *csi)
  \brief       Get CSI frame sequence error interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI frame sequence error interrupt status.
*/
static inline uint32_t csi_get_frame_seq_err_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_SEQ_FRAME_FATAL;
}

/**
  \fn          uint32_t csi_set_frame_seq_err_intr_mask(CSI_Type *csi)
  \brief       Set CSI frame sequence error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_frame_seq_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_SEQ_FRAME_FATAL |= CSI_INT_SEQ_FRAME_FATAL_MASK;
}

/**
  \fn          uint32_t csi_clear_frame_seq_err_intr_mask(CSI_Type *csi)
  \brief       Clear CSI frame sequence error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_frame_seq_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_SEQ_FRAME_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_frame_crc_err_intr_status(CSI_Type *csi)
  \brief       Get CSI frame crc error interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI frame crc error interrupt status.
*/
static inline uint32_t csi_get_frame_crc_err_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_CRC_FRAME_FATAL;
}

/**
  \fn          uint32_t csi_set_frame_crc_err_intr_mask(CSI_Type *csi)
  \brief       Set CSI frame crc error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_frame_crc_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_CRC_FRAME_FATAL |= CSI_INT_CRC_FRAME_FATAL_MASK;
}

/**
  \fn          uint32_t csi_clear_frame_crc_err_intr_mask(CSI_Type *csi)
  \brief       Clear CSI frame crc error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_frame_crc_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_CRC_FRAME_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_frame_payload_err_intr_status(CSI_Type *csi)
  \brief       Get CSI frame payload error interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI frame payload error interrupt status.
*/
static inline uint32_t csi_get_frame_payload_err_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_PLD_CRC_FATAL;
}

/**
  \fn          uint32_t csi_set_frame_payload_err_intr_mask(CSI_Type *csi)
  \brief       Set CSI frame payload error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_frame_payload_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PLD_CRC_FATAL |= CSI_INT_PLD_CRC_FATAL_MASK;
}

/**
  \fn          uint32_t csi_clear_frame_payload_err_intr_mask(CSI_Type *csi)
  \brief       Clear CSI frame payload error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_frame_payload_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_PLD_CRC_FATAL = 0;
}

/**
  \fn          uint32_t csi_get_dt_err_intr_status(CSI_Type *csi)
  \brief       Get CSI data error interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI data error interrupt status.
*/
static inline uint32_t csi_get_dt_err_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_DATA_ID;
}

/**
  \fn          uint32_t csi_set_dt_err_intr_mask(CSI_Type *csi)
  \brief       Set CSI data error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_dt_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_DATA_ID |= CSI_INT_DATA_ID_MASK;
}

/**
  \fn          uint32_t csi_clear_dt_err_intr_mask(CSI_Type *csi)
  \brief       Clear CSI data error interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_dt_err_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_DATA_ID = 0;
}

/**
  \fn          uint32_t csi_get_ecc_intr_status(CSI_Type *csi)
  \brief       Get CSI ECC interrupt status.
  \param[in]   csi     Pointer to the CSI register map.
  \return      CSI ECC interrupt status.
*/
static inline uint32_t csi_get_ecc_intr_status(CSI_Type *csi)
{
    return csi->CSI_INT_ST_ECC_CORRECT ;
}

/**
  \fn          uint32_t csi_set_ecc_intr_mask(CSI_Type *csi)
  \brief       Set CSI ECC interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_set_ecc_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_ECC_CORRECT |= CSI_INT_ECC_CORRECT_MASK;
}

/**
  \fn          uint32_t csi_clear_ecc_intr_mask(CSI_Type *csi)
  \brief       Clear CSI ECC interrupt mask.
  \param[in]   csi     Pointer to the CSI register map.
  \return      none.
*/
static inline void csi_clear_ecc_intr_mask(CSI_Type *csi)
{
    csi->CSI_INT_MSK_ECC_CORRECT = 0;
}

/**
  \fn          CSI_LANE_STOPSTATE csi_get_lane_stopstate_status(CSI_Type *csi, CSI_LANE lane)
  \brief       Get CSI lane stopstate status.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   lane    CSI lane.
  \return      CSI lane stopstate status.
*/
CSI_LANE_STOPSTATE csi_get_lane_stopstate_status(CSI_Type *csi, CSI_LANE lane);

/**
  \fn          void csi_set_ipi_mode(CSI_Type *csi, CSI_IPI_MODE mode)
  \brief       Set CSI IPI mode.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   mode    0: Camera timing
                       1: Controller timing
  \return      none.
*/
void csi_set_ipi_mode(CSI_Type *csi, CSI_IPI_MODE mode);

/**
  \fn          void csi_set_ipi_color_cop(CSI_Type *csi, CSI_IPI_COLOR_COM_TYPE color_cop)
  \brief       Set CSI IPI color component.
  \param[in]   csi       Pointer to the CSI register map.
  \param[in]   color_cop 0: 48-bit interface
                         1: 16-bit interface
  \return      none.
*/
void csi_set_ipi_color_cop(CSI_Type *csi, CSI_IPI_COLOR_COM_TYPE color_cop);

/**
  \fn          void csi_set_horizontal_timing(CSI_Type *csi, uint16_t hsa_time, uint16_t hbp_time,
                                                             uint16_t hsd_time, uint16_t hline_time)
  \brief       Set CSI IPI HSA, HBP, HSD and HLINE timings.
  \param[in]   csi        Pointer to the CSI register map.
  \param[in]   hsa_time   video Horizontal Synchronism Active (HSA) time in PIXCLK cycles.
  \param[in]   hbp_time   video HBP period in PIXCLK cycles.
  \param[in]   hsd_time   video HSP delay period in PIXCLK cycles.
  \param[in]   hline_time overall time for each video line.
  \return      none.
*/
void csi_set_horizontal_timing(CSI_Type *csi, uint16_t hsa_time, uint16_t hbp_time,
                                              uint16_t hsd_time, uint16_t hline_time);

/**
  \fn          void csi_set_ipi_video(CSI_Type *csi, CSI_IPI_VIDEO select)
  \brief       Allows the use of video packets for IPI synchronization
               events.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   select  0: Disable CSI IPI video packets.
                       1: Enable CSI IPI video packets.
  \return      none.
*/
void csi_set_ipi_video(CSI_Type *csi, CSI_IPI_VIDEO select);

/**
  \fn          void csi_set_ipi_line_start(CSI_Type *csi, CSI_IPI_LINE_START select)
  \brief       Allows the use of line select packets for IPI synchronization
               events.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   select  0: Disable CSI IPI line select packets.
                       1: Enable CSI IPI line select packets.
  \return      none.
*/
void csi_set_ipi_line_start(CSI_Type *csi, CSI_IPI_LINE_START select);

/**
  \fn          void csi_set_ipi_null(CSI_Type *csi, CSI_IPI_NULL select)
  \brief       Allows the use of null packets for IPI synchronization
               events.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   select  0: Disable CSI IPI null packets.
                       1: Enable CSI IPI null packets.
  \return      none.
*/
void csi_set_ipi_null(CSI_Type *csi, CSI_IPI_NULL select);

/**
  \fn          void csi_set_ipi_blanking(CSI_Type *csi, CSI_IPI_BLANKING select)
  \brief       Allows the use of blanking packets for IPI synchronization
               events.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   select  0: Disable CSI IPI blanking packets.
                       1: Enable CSI IPI blanking packets.
  \return      none.
*/
void csi_set_ipi_blanking(CSI_Type *csi, CSI_IPI_BLANKING select);

/**
  \fn          void csi_set_ipi_embedded(CSI_Type *csi, CSI_IPI_EMBEDDED select)
  \brief       Allows the use of embedded packets for IPI synchronization
               events.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   select  0: Disable CSI IPI embedded packets.
                       1: Enable CSI IPI embedded packets.
  \return      none.
*/
void csi_set_ipi_embedded(CSI_Type *csi, CSI_IPI_EMBEDDED select);

/**
  \fn          void csi_set_ipi_line_event_selection(CSI_Type *csi, CSI_IPI_LINE_EVENT_SELECT line_event)
  \brief       Set CSI IPI line event.
  \param[in]   csi        Pointer to the CSI register map.
  \param[in]   line_event 0: Controller selects it automatically.
                          1: Select packets from list programmed in bits [21-17] of this
                             register.
  \return      none.
*/
void csi_set_ipi_line_event_selection(CSI_Type *csi, CSI_IPI_LINE_EVENT_SELECT line_event);

/**
  \fn          void csi_set_ipi_sync_event_type(CSI_Type *csi, CSI_IPI_SYNC_EVENT sync_event)
  \brief       Set CSI IPI sync event type.
  \param[in]   csi        Pointer to the CSI register map.
  \param[in]   sync_event 0: Frame Start do not trigger any sync event.
                          1: Legacy mode. Frame Start triggers a sync event
  \return      none.
*/
void csi_set_ipi_sync_event_type(CSI_Type *csi, CSI_IPI_SYNC_EVENT sync_event);

/**
  \fn          void csi_set_vertical_timing(CSI_Type *csi, uint16_t vsa_line, uint16_t vbp_line,
                                                             uint16_t vfp_line, uint16_t vactive_line)
  \brief       Set CSI IPI VSA, VBP, VFP and VACTIVE timings.
  \param[in]   csi           Pointer to the CSI register map.
  \param[in]   vsa_lines     VSA period measured in number of horizontal lines.
  \param[in]   vbp_lines     VBP period measured in number of horizontal lines.
  \param[in]   vfp_lines     VFP period measured in number of horizontal lines.
  \param[in]   vactive_lines Vertical Active period measured in number of horizontal lines.
  \return      none.
*/
void csi_set_vertical_timing(CSI_Type *csi, uint16_t vsa_lines, uint16_t vbp_lines,
                                            uint16_t vfp_lines, uint16_t vactive_lines);

#ifdef __cplusplus
}
#endif
#endif /* CSI_H_ */
