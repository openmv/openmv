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
 * @file     cpi.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     27-March-2023
 * @brief    Low level driver Specific source file.
 ******************************************************************************/

#include "cpi.h"

/**
  \fn          void cpi_software_reset(CPI_Type *cpi)
  \brief       CPI software reset
  \param[in]   cpi      Pointer to the CPI register map.
  \param[in]   mode     Soft reset the CPI
  \return      none.
*/
void cpi_software_reset(CPI_Type *cpi)
{
    cpi->CAM_CTRL = 0;
    cpi->CAM_CTRL |= CAM_CTRL_SW_RESET;
    cpi->CAM_CTRL = 0;
    cpi->CAM_CTRL = (CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
}

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
void cpi_start_capture(CPI_Type *cpi, CPI_MODE_SELECT mode)
{
    cpi->CAM_CTRL = 0;
    cpi->CAM_CTRL |= CAM_CTRL_SW_RESET;
    cpi->CAM_CTRL = 0;

    if(mode == CPI_MODE_SELECT_SNAPSHOT)
    {
        cpi->CAM_CTRL = (CAM_CTRL_SNAPSHOT | CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
    }

    else
    {
        cpi->CAM_CTRL = (CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
    }
}

/**
  \fn           void cpi_set_config(CPI_Type *cpi, cpi_cfg_info_t *info)
  \brief        Configure the cpi with given information.
  \param[in]    cpi    Pointer to CPI register map
  \param[in]    info   Pointer to cpi configuration structure.
  \return       none
*/
void cpi_set_config(CPI_Type *cpi, cpi_cfg_info_t *info)
{
    cpi->CAM_CFG = (info->sensor_info.interface | (info->sensor_info.vsync_wait << CAM_CFG_VSYNC_WAIT_Pos) |
                    (info->sensor_info.vsync_mode << CAM_CFG_VSYNC_MODE_Pos) |
                    (info->rw_roundup << CAM_CFG_ROW_ROUNDUP_Pos) |
                    (info->sensor_info.pixelclk_pol << CAM_CFG_PIXELCLK_POL_Pos) |
                    (info->sensor_info.hsync_pol << CAM_CFG_HSYNC_POL_Pos) |
                    (info->sensor_info.vsync_pol << CAM_CFG_VSYNC_POL_Pos) |
                    (info->sensor_info.data_mode << CAM_CFG_DATA_MODE_Pos));

    if(info->sensor_info.data_mode <= CPI_DATA_MODE_BIT_8)
    {
        cpi->CAM_CFG |= (info->sensor_info.data_endianness << CAM_CFG_DATA_ENDIANNESS_Pos);
    }

    if(info->sensor_info.data_mode == CPI_DATA_MODE_BIT_8)
    {
        cpi->CAM_CFG |= (info->sensor_info.code10on8 << CAM_CFG_CODE10ON8_Pos);
    }

    if(info->sensor_info.data_mode == CPI_DATA_MODE_BIT_16)
    {
        cpi->CAM_CFG &= ~CAM_CFG_DATA_MASK_Msk;
        cpi->CAM_CFG |= (info->sensor_info.data_mask << CAM_CFG_DATA_MASK_Pos);
    }

    cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_RD_WMARK_Msk;
    cpi->CAM_FIFO_CTRL = info->fifo_ctrl.rd_wmark;
    cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_WR_WMARK_Msk;
    cpi->CAM_FIFO_CTRL |= (info->fifo_ctrl.wr_wmark << CAM_FIFO_CTRL_WR_WMARK_Pos);

    cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_DATA_Msk;
    cpi->CAM_VIDEO_FCFG = info->frame_cfg.data;
    cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_ROW_Msk;
    cpi->CAM_VIDEO_FCFG |= (info->frame_cfg.row << CAM_VIDEO_FCFG_ROW_Pos);

    if(info->sensor_info.interface == CPI_INTERFACE_MIPI_CSI)
    {
        cpi->CAM_CSI_CMCFG |= info->csi_ipi_color_mode;
    }
}
