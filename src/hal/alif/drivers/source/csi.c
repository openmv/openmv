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
 * @file     csi.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     12-April-2023
 * @brief    Low level driver Specific source file.
 ******************************************************************************/

#include "csi.h"

/**
  \fn          CSI_LANE_STOPSTATE csi_get_lane_stopstate_status(CSI_Type *csi, CSI_LANE lane)
  \brief       Get CSI lane stopstate status.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   lane    CSI lane.
  \return      CSI lane stopstate status.
*/
CSI_LANE_STOPSTATE csi_get_lane_stopstate_status(CSI_Type *csi, CSI_LANE lane)
{
    CSI_LANE_STOPSTATE ret = 0;

    switch (lane)
    {
        case CSI_LANE_CLOCK:
        {
            ret = (csi->CSI_PHY_STOPSTATE & CSI_PHY_STOPSTATECLK_Msk) >> CSI_PHY_STOPSTATECLK_Pos;
            break;
        }

        case CSI_LANE_0:
        {
            ret = (csi->CSI_PHY_STOPSTATE & CSI_PHY_STOPSTATEDATA_0_Msk) >> CSI_PHY_STOPSTATEDATA_0_Pos;
            break;
        }

        case CSI_LANE_1:
        {
            ret = (csi->CSI_PHY_STOPSTATE & CSI_PHY_STOPSTATEDATA_1_Msk) >> CSI_PHY_STOPSTATEDATA_1_Pos;
            break;
        }

        default:
        {
            break;
        }
    }

    return ret;
}

/**
  \fn          void csi_set_ipi_mode(CSI_Type *csi, CSI_IPI_MODE mode)
  \brief       Set CSI IPI mode.
  \param[in]   csi     Pointer to the CSI register map.
  \param[in]   mode    0: Camera timing
                       1: Controller timing
  \return      none.
*/
void csi_set_ipi_mode(CSI_Type *csi, CSI_IPI_MODE mode)
{
    if(mode == CSI_IPI_MODE_CAM_TIMIMG)
    {
        csi->CSI_IPI_MODE &= ~CSI_IPI_MOD;
    }
    else
    {
        csi->CSI_IPI_MODE |= CSI_IPI_MOD;
    }
}

/**
  \fn          void csi_set_ipi_color_cop(CSI_Type *csi, CSI_IPI_COLOR_COM_TYPE color_component)
  \brief       Set CSI IPI color component.
  \param[in]   csi             Pointer to the CSI register map.
  \param[in]   color_component 0: 48-bit interface
                               1: 16-bit interface
  \return      none.
*/
void csi_set_ipi_color_cop(CSI_Type *csi, CSI_IPI_COLOR_COM_TYPE color_component)
{
    if(color_component == CSI_IPI_COLOR_COM_TYPE_COLOR16)
    {
        csi->CSI_IPI_MODE |= CSI_IPI_COLOR_COM;
    }
    else
    {
        csi->CSI_IPI_MODE &= ~CSI_IPI_COLOR_COM;
    }
}

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
                                              uint16_t hsd_time, uint16_t hline_time)
{
    csi->CSI_IPI_HSA_TIME &= ~CSI_IPI_HSA_TIME_Msk;
    csi->CSI_IPI_HSA_TIME = hsa_time;

    csi->CSI_IPI_HBP_TIME &= ~CSI_IPI_HBP_TIME_Msk;
    csi->CSI_IPI_HBP_TIME = hbp_time;

    csi->CSI_IPI_HSD_TIME &= ~CSI_IPI_HSD_TIME_Msk;
    csi->CSI_IPI_HSD_TIME = hsd_time;

    csi->CSI_IPI_HLINE_TIME &= ~CSI_IPI_HLINE_TIME_Msk;
    csi->CSI_IPI_HLINE_TIME = hline_time;
}

/**
  \fn          void csi_set_ipi_line_event_selection(CSI_Type *csi, CSI_IPI_LINE_EVENT_SELECT line_event)
  \brief       Set CSI IPI line event.
  \param[in]   csi        Pointer to the CSI register map.
  \param[in]   line_event 0: Controller selects it automatically.
                          1: Select packets from list programmed in bits [21-17] of this
                             register.
  \return      none.
*/
void csi_set_ipi_line_event_selection(CSI_Type *csi, CSI_IPI_LINE_EVENT_SELECT line_event)
{
    if(line_event == CSI_IPI_LINE_EVENT_SELECT_AUTO)
    {
        csi->CSI_IPI_ADV_FEATURES &= ~CSI_IPI_LINE_EVENT_SELECTION;
    }
    else
    {
        csi->CSI_IPI_ADV_FEATURES |= CSI_IPI_LINE_EVENT_SELECTION;
    }
}

/**
  \fn          void csi_set_ipi_sync_event_type(CSI_Type *csi, CSI_IPI_SYNC_EVENT sync_event)
  \brief       Set CSI IPI sync event type.
  \param[in]   csi        Pointer to the CSI register map.
  \param[in]   sync_event 0: Frame Start do not trigger any sync event.
                          1: Legacy mode. Frame Start triggers a sync event
  \return      none.
*/
void csi_set_ipi_sync_event_type(CSI_Type *csi, CSI_IPI_SYNC_EVENT sync_event_type)
{
    if(sync_event_type == CSI_IPI_SYNC_EVENT_FSN)
    {
        csi->CSI_IPI_ADV_FEATURES &= ~CSI_IPI_SYNC_EVENT_MODE;
    }
    else
    {
        csi->CSI_IPI_ADV_FEATURES |= CSI_IPI_SYNC_EVENT_MODE;
    }
}

/**
  \fn          void csi_set_vertical_timing(CSI_Type *csi, uint16_t vsa_line, uint16_t vbp_line,
                                                             uint16_t vfp_line, uint16_t vactive_line)
  \brief       Set CSI IPI VSA, VBP, VFP and VACTIVE lines.
  \param[in]   csi           Pointer to the CSI register map.
  \param[in]   vsa_lines     VSA period measured in number of horizontal lines.
  \param[in]   vbp_lines     VBP period measured in number of horizontal lines.
  \param[in]   vfp_lines     VFP period measured in number of horizontal lines.
  \param[in]   vactive_lines Vertical Active period measured in number of horizontal lines.
  \return      none.
*/
void csi_set_vertical_timing(CSI_Type *csi, uint16_t vsa_lines, uint16_t vbp_lines,
                                            uint16_t vfp_lines, uint16_t vactive_lines)
{
    csi->CSI_IPI_VSA_LINES &= ~CSI_IPI_VSA_LINE_Msk;
    csi->CSI_IPI_VSA_LINES = vsa_lines;

    csi->CSI_IPI_VBP_LINES &= ~CSI_IPI_VBP_LINE_Msk;
    csi->CSI_IPI_VBP_LINES = vbp_lines;

    csi->CSI_IPI_VFP_LINES &= ~CSI_IPI_VFP_LINE_Msk;
    csi->CSI_IPI_VFP_LINES = vfp_lines;

    csi->CSI_IPI_VACTIVE_LINES &= ~CSI_IPI_VACTIVE_LINE_Msk;
    csi->CSI_IPI_VACTIVE_LINES = vactive_lines;
}
