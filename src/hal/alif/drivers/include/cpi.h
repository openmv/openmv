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
 * @file     cpi.h
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     27-March-2023
 * @brief    Low level driver Specific header file.
 ******************************************************************************/

#ifndef CPI_H_
#define CPI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct {                                      /*!< LPCPI/CPI Structure                                                    */
    volatile       uint32_t  CAM_CTRL;                /*!< (@ 0x00000000) Camera Control Register                                 */
    volatile       uint32_t  CAM_INTR;                /*!< (@ 0x00000004) Camera Interrupt Status Register                        */
    volatile       uint32_t  CAM_INTR_ENA;            /*!< (@ 0x00000008) Camera Interrupt Enable Register                        */
    volatile const uint32_t  RESERVED;                /*!< (@ 0x0000000C) Camera reserved Register                                */
    volatile       uint32_t  CAM_CFG;                 /*!< (@ 0x00000010) Camera Configuration Register                           */
    volatile       uint32_t  CAM_FIFO_CTRL;           /*!< (@ 0x00000014) Camera FIFO Control Register                            */
    volatile const uint32_t  CAM_AXI_ERR_STAT;        /*!< (@ 0x00000018) Camera AXI Error Status Register                        */
    volatile const uint32_t  RESERVED1[3];            /*!< (@ 0x0000001C) Camera Reserved Register                                */
    volatile       uint32_t  CAM_VIDEO_FCFG;          /*!< (@ 0x00000028) Camera Video Frame Configuration Register               */
    volatile       uint32_t  CAM_CSI_CMCFG;           /*!< (@ 0x0000002C) Camera MIPI CSI Color Mode Configuration Register       */
    volatile       uint32_t  CAM_FRAME_ADDR;          /*!< (@ 0x00000030) Camera Video Frame A Start Address Register             */
} CPI_Type;                                           /*!< Size = 52 (0x34)                                                       */

/* Camera Control Register (CAM_CTRL) bit Definition, Macros, Offsets and Masks
 * these include CPI capture mode, capture status, software reset, start/stop capture and FIFO clock select.
 */
/* CAM_CTRL Start Capture bit[0] */
#define CAM_CTRL_START                                 (1U << 0)

/* CAM_CTRL Capture Status bit[2] */
#define CAM_CTRL_BUSY                                  (1U << 2)

/* CAM_CTRL Capture Mode bit[4] */
#define CAM_CTRL_SNAPSHOT                              (1U << 4)

/* CAM_CTRL Software Reset bit[8] */
#define CAM_CTRL_SW_RESET                              (1U << 8)

/* CAM_CTRL CAM_CTRL FIFO clock select bit[12] */
#define CAM_CTRL_FIFO_CLK_SEL                          (1U << 12)

/* CAM_INTR control  bit parameters */
/* Frame Capture Stop IRQ bit[0] */
#define CAM_INTR_STOP                                  (1U << 0)

/* Input FIFO overrun Warning IRQ bit[4] */
#define CAM_INTR_INFIFO_OVERRUN                        (1U << 4)

/* Output FIFO Overrun Warning IRQ bit[5] */
#define CAM_INTR_OUTFIFO_OVERRUN                       (1U << 5)

/* Bus error IRQ bit[6] */
#define CAM_INTR_BRESP_ERR                             (1U << 6)

/* VSYNC Detected IRQ bit[16] */
#define CAM_INTR_VSYNC                                 (1U << 16)

/* HSYNC Detected IRQ bit[20] */
#define CAM_INTR_HSYNC                                 (1U << 20)

/* Camera Configuration Register (CAM_CFG) bit Definition, Macros, Offsets and Masks
 * these include data mode, data mask etc
 */
/* CAM_CFG vsync wait bit[4] */
#define CAM_CFG_VSYNC_WAIT_Pos                         4U

/* CAM_CFG vsync mode bit[5] */
#define CAM_CFG_VSYNC_MODE_Pos                         5U

/* CAM_CFG row roundup bit[8] */
#define CAM_CFG_ROW_ROUNDUP_Pos                        8U

/* CAM_CFG pixel clock polarity bit[12] */
#define CAM_CFG_PIXELCLK_POL_Pos                       12U

/* CAM_CFG hsync polarity bit[13] */
#define CAM_CFG_HSYNC_POL_Pos                          13U

/* CAM_CFG vsync polarity bit[14] */
#define CAM_CFG_VSYNC_POL_Pos                          14U

/* CAM_CFG Data Mode bit[18-16] */
#define CAM_CFG_DATA_MODE_Pos                          16U
#define CAM_CFG_DATA_MODE_Msk                          (0x7U << CAM_CFG_DATA_MODE_Pos)

/* CAM_CFG Data endianness bit[20] */
#define CAM_CFG_DATA_ENDIANNESS_Pos                    20U

/* CAM_CFG code10on8 bit[24] */
#define CAM_CFG_CODE10ON8_Pos                          24U

/* CAM_CFG Data Mask bit[29-28] */
#define CAM_CFG_DATA_MASK_Pos                          28U
#define CAM_CFG_DATA_MASK_Msk                          (0x3U << CAM_CFG_DATA_MASK_Pos)

/* Camera FIFO Control Register (CAM_FIFO_CTRL) bit Definition, Macros, Offsets and Masks
 * these include FIFO read and write watermark.
 */
/* CAM_FIFO_CTRL FIFO read watermark bit[4:0] */
#define CAM_FIFO_CTRL_RD_WMARK_Pos                     0U
#define CAM_FIFO_CTRL_RD_WMARK_Msk                     (0X1FU << CAM_FIFO_CTRL_RD_WMARK_Pos)

/* CAM_FIFO_CTRL FIFO write watermark bit[12:8] */
#define CAM_FIFO_CTRL_WR_WMARK_Pos                     8U
#define CAM_FIFO_CTRL_WR_WMARK_Msk                     (0X1FU << CAM_FIFO_CTRL_WR_WMARK_Pos)

/* Camera AXI Error Status Register (CAM_AXI_ERR_STAT) bit Definition, Macros, Offsets and Masks
 * these include CPI BRESP error counter and code.
 */
/* CAM_AXI_ERR_STAT frame width bit[1:0] */
#define CAM_AXI_ERR_STAT_BRESP_Pos                     0U
#define CAM_AXI_ERR_STAT_BRESP_Msk                     (0x3U << CAM_AXI_ERR_STAT_BRESP_Pos)

/* CAM_AXI_ERR_STAT frame height bit[15:8] */
#define CAM_AXI_ERR_STAT_CNT_Pos                       8U
#define CAM_AXI_ERR_STAT_CNT_Msk                       (0xFF00U << CAM_AXI_ERR_STAT_CNT_Pos)

/* Camera Video Frame Configuration Register (CAM_VIDEO_FCFG) bit Definition, Macros, Offsets and Masks
 * these include CPI frame width and frame height.
 */
/* CAM_VIDEO_FCFG frame width bit[13:0] */
#define CAM_VIDEO_FCFG_DATA_Pos                        0U
#define CAM_VIDEO_FCFG_DATA_Msk                        (0x3FFFU << CAM_VIDEO_FCFG_DATA_Pos)

/* CAM_VIDEO_FCFG frame height bit[27:16] */
#define CAM_VIDEO_FCFG_ROW_Pos                         16U
#define CAM_VIDEO_FCFG_ROW_Msk                         (0xFFFU << CAM_VIDEO_FCFG_ROW_Pos)

/* Camera MIPI CSI Color Mode Configuration Register (CAM_CSI_CMCFG) bit Definition, Macros, Offsets and Masks
 * these include color encode mode.
 */
/* Camera MIPI CSI color mode 16-bit encode bit[3:0] */
#define CAM_CSI_CMCFG_MODE_Pos                         0U
#define CAM_CSI_CMCFG_MODE_Msk                         (0XFU << CAM_CSI_CMCFG_MODE_Pos)

/* Camera Video Frame Start Address Register (CAM_FRAME_ADDR) bit Definition, Macros, Offsets and Masks
 * these include CPI framebuffer start address.
 */
/* CAM_FRAME_ADDR framebuffer start address bit[31:3] */
#define CAM_FRAME_ADDR_ADDR_Pos                        0U
#define CAM_FRAME_ADDR_ADDR_Msk                        (0xFFFFFFF8U << CAM_FRAME_ADDR_ADDR_Pos)

/**
 * enum  CPI_VIDEO_CAPTURE_STATUS
 * CPI Video Capture Status.
 */
typedef enum _CPI_VIDEO_CAPTURE_STATUS
{
    CPI_VIDEO_CAPTURE_STATUS_NOT_CAPTURING,         /**< not capturing video                                             */
    CPI_VIDEO_CAPTURE_STATUS_CAPTURING              /**< capturing video                                                 */
} CPI_VIDEO_CAPTURE_STATUS;

/**
 * enum CPI_MODE_SELECT.
 * CPI capture mode.
*/
typedef enum _CPI_MODE_SELECT
{
    CPI_MODE_SELECT_VIDEO,                          /**< Capture video data frames continuously                          */
    CPI_MODE_SELECT_SNAPSHOT                        /**< Capture one frame then stop (snapshot mode)                     */
} CPI_MODE_SELECT;

/**
 * enum  CPI_INTERFACE
 * CPI interface.
 */
typedef enum _CPI_INTERFACE
{
    CPI_INTERFACE_PARALLEL,                         /**< select video data from parallel camera interface                */
    CPI_INTERFACE_MIPI_CSI                          /**< select video data from MIPI_CSI interface (Not valid for LPCPI) */
} CPI_INTERFACE;

/**
 * enum  CPI_WAIT_VSYNC
 * CPI wait for vsync to start capture frame.
 */
typedef enum _CPI_WAIT_VSYNC
{
    CPI_WAIT_VSYNC_DISABLE,                         /**< CPI Start video capture without waiting for VSYNC               */
    CPI_WAIT_VSYNC_ENABLE                           /**< CPI Start video capture on rising edge of VSYNC                 */
} CPI_WAIT_VSYNC;

/**
 * enum  CPI_CAPTURE_DATA_ENABLE
 * CPI vsync signal mode for capturing data.
 */
typedef enum _CPI_CAPTURE_DATA_ENABLE
{
    CPI_CAPTURE_DATA_ENABLE_IF_HSYNC_HIGH,          /**< Capture data when HSYNC is high                                 */
    CPI_CAPTURE_DATA_ENABLE_IF_VSYNC_AND_HSYNC_HIGH /**< Capture data when both VSYNC and HSYNC are high                 */
} CPI_CAPTURE_DATA_ENABLE;

/**
 * enum CPI_ROW_ROUNDUP.
 * CPI round up pixel data to 64 bit at end of each row.
*/
typedef enum _CPI_ROW_ROUNDUP
{
    CPI_ROW_ROUNDUP_DISABLE,                        /**< CPI round up pixel data to 64 bit at end of each row disable    */
    CPI_ROW_ROUNDUP_ENABLE                          /**< CPI round up pixel data to 64 bit at end of each row            */
} CPI_ROW_ROUNDUP;

/**
 * enum  CPI_SIG_POLARITY
 * CPI signal polarity.
 */
typedef enum _CPI_SIG_POLARITY
{
    CPI_SIG_POLARITY_INVERT_DISABLE,                /**< Disable invert signal polarity                                  */
    CPI_SIG_POLARITY_INVERT_ENABLE                  /**< Enable invert signal polarity                                   */
} CPI_SIG_POLARITY;

/**
 * enum  CPI_DATA_MODE
 * CPI data mode selection.
 */
typedef enum _CPI_DATA_MODE
{
    CPI_DATA_MODE_BIT_1,                            /**< Select 1 bit data mode                                          */
    CPI_DATA_MODE_BIT_2,                            /**< Select 2 bit data mode                                          */
    CPI_DATA_MODE_BIT_4,                            /**< Select 4 bit data mode                                          */
    CPI_DATA_MODE_BIT_8,                            /**< Select 8 bit data mode                                          */
    CPI_DATA_MODE_BIT_16,                           /**< Select 16 bit data mode (Not valid for LPCPI)                   */
    CPI_DATA_MODE_BIT_32,                           /**< Select 32 bit data mode (Valid for MIPI CSI interface)          */
    CPI_DATA_MODE_BIT_64                            /**< Select 64 bit data mode (Valid for MIPI CSI interface)          */
} CPI_DATA_MODE;

/**
 * enum  CPI_DATA_FIELD
 * CPI MSB/LSB to be stored first in memory.
 */
typedef enum _CPI_DATA_ENDIANNESS {
    CPI_DATA_ENDIANNESS_LSB_FIRST,                  /**< Select LSB first to be stored in memory                         */
    CPI_DATA_ENDIANNESS_MSB_FIRST                   /**< Select MSB first to be stored in memory                         */
} CPI_DATA_ENDIANNESS;

/**
 * enum CPI_CODE10ON8_CODING
 * CPI transfer 10 bit code on 8 bit data bus.
 */
typedef enum _CPI_CODE10ON8_CODING {
    CPI_CODE10ON8_CODING_DISABLE,                   /**< Disable special 10-bit coding                                   */
    CPI_CODE10ON8_CODING_ENABLE                     /**< Enable special 10-bit coding                                    */
} CPI_CODE10ON8_CODING;

/**
 * enum  CPI_DATA_MASK
 * CPI data mask.
 * Valid for 16 bit data mode only and not valid for LPCPI.
 */
typedef enum _CPI_DATA_MASK
{
    CPI_DATA_MASK_BIT_16,                           /**< Select 16 bit data mask                                         */
    CPI_DATA_MASK_BIT_10,                           /**< Select 10 bit data mask                                         */
    CPI_DATA_MASK_BIT_12,                           /**< Select 12 bit data mask                                         */
    CPI_DATA_MASK_BIT_14                            /**< Select 14 bit data mask                                         */
} CPI_DATA_MASK;

/**
 * @struct  _cpi_sensor_info_t
 * @brief    CPI camera sensor information.
 */
typedef struct _cpi_sensor_info_t
{
    CPI_INTERFACE            interface;             /**< CPI Interface                                                  */
    CPI_WAIT_VSYNC           vsync_wait;            /**< CPI VSYNC Wait                                                 */
    CPI_CAPTURE_DATA_ENABLE  vsync_mode;            /**< CPI VSYNC Mode                                                 */
    CPI_SIG_POLARITY         pixelclk_pol;          /**< CPI Pixel Clock Polarity                                       */
    CPI_SIG_POLARITY         hsync_pol;             /**< CPI HSYNC Polarity                                             */
    CPI_SIG_POLARITY         vsync_pol;             /**< CPI VSYNC Polarity                                             */
    CPI_DATA_MODE            data_mode;             /**< CPI Data Mode                                                  */
    CPI_DATA_ENDIANNESS      data_endianness;       /**< Select MSB/LSB                                                 */
    CPI_CODE10ON8_CODING     code10on8;             /**< code10on8 enable/disable                                       */
    CPI_DATA_MASK            data_mask;             /**< CPI Data Mask                                                  */
} cpi_sensor_info_t;

/**
 * @struct  cpi_fifo_ctrl_cfg_t
 * @brief    CPI fifo control configuration.
 */
typedef struct _cpi_fifo_ctrl_cfg_t
{
    uint8_t rd_wmark;                               /**< FIFO read watermark                                            */
    uint8_t wr_wmark;                               /**< FIFO write watermark                                           */
} cpi_fifo_ctrl_cfg_t;

/**
 * @struct  _cpi_frame_cfg_t
 * @brief    CPI frame configuration.
 */
typedef struct _cpi_frame_cfg_t
{
    uint16_t data;                               /**< Valid data in a row                                              */
    uint16_t row;                               /**< Valid data rows in a frame                                        */
} cpi_frame_cfg_t;

/**
 * enum  CPI_COLOR_MODE_CONFIG
 * CPI CSI IPI color mode configuration.
 * Not valid for LPCPI.
 */
typedef enum _CPI_COLOR_MODE_CONFIG {
    CPI_COLOR_MODE_CONFIG_IPI16_RAW6,               /**< Select color encode mode  IPI-16 RAW 6                          */
    CPI_COLOR_MODE_CONFIG_IPI16_RAW7,               /**< Select color encode mode  IPI-16 RAW 7                          */
    CPI_COLOR_MODE_CONFIG_IPI16_RAW8,               /**< Select color encode mode  IPI-16 RAW 8                          */
    CPI_COLOR_MODE_CONFIG_IPI16_RAW10,              /**< Select color encode mode  IPI-16 RAW 10                         */
    CPI_COLOR_MODE_CONFIG_IPI16_RAW12,              /**< Select color encode mode  IPI-16 RAW 12                         */
    CPI_COLOR_MODE_CONFIG_IPI16_RAW14,              /**< Select color encode mode  IPI-16 RAW 14                         */
    CPI_COLOR_MODE_CONFIG_IPI16_RAW16,              /**< Select color encode mode  IPI-16 RAW 16                         */
    CPI_COLOR_MODE_CONFIG_IPI48_RGB444,             /**< Select color encode mode  IPI-48 RGB444                         */
    CPI_COLOR_MODE_CONFIG_IPI48_RGB555,             /**< Select color encode mode  IPI-48 RGB555                         */
    CPI_COLOR_MODE_CONFIG_IPI48_RGB565,             /**< Select color encode mode  IPI-48 RGB565                         */
    CPI_COLOR_MODE_CONFIG_IPI48_RGB666,             /**< Select color encode mode  IPI-48 RGB666                         */
    CPI_COLOR_MODE_CONFIG_IPI48_XRGB888,            /**< Select color encode mode  IPI-48 XRGB888                        */
    CPI_COLOR_MODE_CONFIG_IPI48_RGBX888,            /**< Select color encode mode  IPI-48 RGBX888                        */
    CPI_COLOR_MODE_CONFIG_IPI48_RAW32,              /**< Select color encode mode  IPI-48 RAW 32                         */
    CPI_COLOR_MODE_CONFIG_IPI48_RAW48,              /**< Select color encode mode  IPI-48 RAW 48                         */
} CPI_COLOR_MODE_CONFIG;

/**
 * @struct  _cpi_cfg_info_t
 * @brief    CPI configuration information.
 */
typedef struct _cpi_cfg_info_t
{
    cpi_sensor_info_t      sensor_info;             /**< CPI sensor configuration                                        */
    CPI_ROW_ROUNDUP        rw_roundup;              /**< CPI row roundup                                                 */
    cpi_fifo_ctrl_cfg_t    fifo_ctrl;               /**< CPI FIFO control                                                */
    cpi_frame_cfg_t        frame_cfg;               /**< CPI Frame                                                       */
    CPI_COLOR_MODE_CONFIG  csi_ipi_color_mode;      /**< CPI CSI IPI color mode                                          */
} cpi_cfg_info_t;


/**
  \fn          CPI_VIDEO_CAPTURE_STATUS cpi_get_capture_status(CPI_Type *cpi)
  \brief       Get the capture status of the CPI.
  \param[in]   cpi      Pointer to the CPI register map.
  \return      Capture status of the CPI.
*/
static inline CPI_VIDEO_CAPTURE_STATUS cpi_get_capture_status(CPI_Type *cpi)
{
    return (cpi->CAM_CTRL & CAM_CTRL_BUSY) ? \
            CPI_VIDEO_CAPTURE_STATUS_CAPTURING : CPI_VIDEO_CAPTURE_STATUS_NOT_CAPTURING;
}

/**
  \fn          uint32_t cpi_get_interrupt_status(CPI_Type *cpi)
  \brief       Get the interrupt status from the CPI.
  \param[in]   cpi      Pointer to the CPI register map.
  \return      interrupt status from the CPI.
*/
static inline uint32_t cpi_get_interrupt_status(CPI_Type *cpi)
{
    return (cpi->CAM_INTR & cpi->CAM_INTR_ENA);
}

/**
  \fn           void cpi_enable_interrupt(CPI_Type *cpi, uint32_t irq_bitmask)
  \brief        Enable CPI interrupt.
  \param[in]    cpi         Pointer to CPI register map.
  \param[in]    irq_bitmask Possible camera events (refer CPI_INTR_* macros Bitmask).
  \return       none
*/
static inline void cpi_enable_interrupt(CPI_Type *cpi, uint32_t irq_bitmask)
{
    cpi->CAM_INTR_ENA |= irq_bitmask;
}

/**
  \fn           void cpi_disable_interrupt(CPI_Type *cpi, uint32_t irq_bitmask)
  \brief        Disable CPI interrupt.
  \param[in]    cpi         Pointer to CPI register map.
  \param[in]    irq_bitmask Possible camera events (refer CPI_INTR_* macros Bitmask).
  \return       none
*/
static inline void cpi_disable_interrupt(CPI_Type *cpi, uint32_t irq_bitmask)
{
    cpi->CAM_INTR_ENA &= ~irq_bitmask;
}

/**
  \fn           void cpi_irq_handler_clear_intr_status(CPI_Type *cpi, uint32_t irq_bitmask)
  \brief        Clear CPI interrupt.
  \param[in]    cpi         Pointer to CPI register map
  \param[in]    irq_bitmask CPI interrupt status (refer CPI_INTR_* macros Bitmask)
  \return       none.
*/
static inline void cpi_irq_handler_clear_intr_status(CPI_Type *cpi, uint32_t irq_bitmask)
{
    cpi->CAM_INTR |= irq_bitmask;
    (void)cpi->CAM_INTR;
}

/**
  \fn          void cpi_set_framebuff_start_addr(CPI_Type *cpi, uint32_t addr)
  \brief       Set the Video frame start address.
  \param[in]   cpi   Pointer to the CPI register map.
  \param[in]   addr  Video frame start address.
  \return      none.
*/
static inline void cpi_set_framebuff_start_addr(CPI_Type *cpi, uint32_t addr)
{
    cpi->CAM_FRAME_ADDR = addr;
}

/**
  \fn          void cpi_stop_capture(CPI_Type *cpi)
  \brief       CPI Stop capturing frame.
  \param[in]   cpi   Pointer to the CPI register map.
  \return      none.
*/
static inline void cpi_stop_capture(CPI_Type *cpi)
{
    cpi->CAM_CTRL = 0;
}

/**
\fn          void cpi_software_reset(CPI_Type *cpi)
\brief       CPI software reset
\param[in]   cpi      Pointer to the CPI register map.
\param[in]   mode     Soft reset the CPI
\return      none.
*/
void cpi_software_reset(CPI_Type *cpi);

/**
  \fn          void cpi_start_capture(CPI_Type *cpi, CPI_MODE_SELECT mode)
  \brief       Capture frame in snapshot/ continuous capture mode.
                   -Set CAM_CTRL = 0—prepare for soft reset
                   -Set CAM_CTRL = 0x100—activate soft reset
                   -Set CAM_CTRL = 0—stop soft reset
                   -Set CAM_CTRL = 0x1001 or 0x1011—start the CPI controller,
                    with bit [SNAPSHOT] defining the operation mode:
                        -[SNAPSHOT] = 0—Capture video frames continuously
                        -[SNAPSHOT] = 1—Capture one frame then stop
  \param[in]   cpi      Pointer to the CPI register map.
  \param[in]   mode     Select to capture one frame and stop/ or to capture video frames continuously
  \return      none.
*/
void cpi_start_capture(CPI_Type *cpi, CPI_MODE_SELECT mode);

/**
  \fn           void cpi_set_config(CPI_Type *cpi, cpi_cfg_info_t *info)
  \brief        Configure the cpi with given information.
  \param[in]    cpi    Pointer to CPI register map
  \param[in]    info   Pointer to cpi configuration structure.
  \return       none
*/
void cpi_set_config(CPI_Type *cpi, cpi_cfg_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* CPI_H_ */
