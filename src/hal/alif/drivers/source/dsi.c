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
 * @file     dsi.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     17-April-2023
 * @brief    Low level driver Specific Source file.
 ******************************************************************************/

#include "dsi.h"

/**
  \fn          void dsi_dcs_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t data, uint8_t vc_id)
  \brief       Perform dsi dcs Short write.
  \param[in]   cmd is DCS command info.
  \param[in]   data to send.
  \param[in]   vc_id virtual channel ID.
  \return      none.
*/
void dsi_dcs_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t data, uint8_t vc_id)
{
    dsi->DSI_GEN_HDR = (DSI_DCS_SHORT_WRITE_DATA_TYPE << DSI_GEN_DT) | \
                         (vc_id << DSI_GEN_VC) | (cmd << DSI_GEN_WC_LSBYTE) | \
                         (data << DSI_GEN_WC_MSBYTE);
}

/**
  \fn          void dsi_dcs_cmd_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t vc_id)
  \brief       Perform dsi DCS Short write only command.
  \param[in]   cmd is DCS command info.
  \param[in]   vc_id virtual channel ID.
  \return      none.
*/
void dsi_dcs_cmd_short_write(DSI_Type *dsi, uint8_t cmd, uint8_t vc_id)
{
    dsi->DSI_GEN_HDR = (DSI_DCS_SHORT_WRITE_NODATA_TYPE << DSI_GEN_DT) | \
                       (vc_id << DSI_GEN_VC) | (cmd << DSI_GEN_WC_LSBYTE);
}

/**
  \fn          void dsi_dcs_long_write(DSI_Type *dsi, uint8_t cmd, uint32_t data, uint8_t vc_id)
  \brief       Perform dsi DCS Short write.
  \param[in]   data pointer to data buffer.
  \param[in]   len data buffer length.
  \param[in]   vc_id virtual channel ID.
  \return      none.
*/
void dsi_dcs_long_write(DSI_Type *dsi, uint8_t* data, uint32_t len, uint8_t vc_id)
{
    if (DSI_GEN_PAYLOAD_FIFO_SIZE < len)
    {
        return;
    }

    uint32_t payload_data,bytes_to_copy;
    uint32_t data_len = len;

    while (data_len)
    {
        payload_data = 0;
        bytes_to_copy = (data_len < DSI_PAYLOAD_FIFO_SLOT_DEPTH) ? data_len : DSI_PAYLOAD_FIFO_SLOT_DEPTH;

        for (uint32_t i = 0; i < bytes_to_copy; i++)
        {
            payload_data |= ((uint32_t)data[i] << (i * 8));
        }

        dsi->DSI_GEN_PLD_DATA = payload_data;

        data += bytes_to_copy;
        data_len -= bytes_to_copy;
    }

    dsi->DSI_GEN_HDR = (DSI_DCS_LONG_WRITE_DATA_TYPE << DSI_GEN_DT) |
                       ((vc_id & DSI_GEN_VC_MASK) << DSI_GEN_VC) |
                       (len << DSI_GEN_WC_LSBYTE);
}


/**
  \fn          DSI_LANE_STOPSTATE dsi_get_lane_stopstate_status(DSI_Type *dsi, DSI_LANE lane)
  \brief       Get dsi lane stopstate status.
  \param[in]   dsi     Pointer to the dsi register map.
  \param[in]   lane    dsi lane.
  \return      dsi lane stopstate status.
*/
DSI_LANE_STOPSTATE dsi_get_lane_stopstate_status(DSI_Type *dsi, DSI_LANE lane)
{
    DSI_LANE_STOPSTATE ret = 0;

    switch (lane)
    {
        case DSI_LANE_CLOCK:
            ret = (dsi->DSI_PHY_STATUS & DSI_PHY_STOPSTATECLKLANE_MASK) >> DSI_PHY_STOPSTATECLKLANE;
            break;
        case DSI_LANE_0:
            ret =  (dsi->DSI_PHY_STATUS & DSI_PHY_STOPSTATELANE_0_MASK) >> DSI_PHY_STOPSTATELANE_0;
            break;
        case DSI_LANE_1:
            ret = (dsi->DSI_PHY_STATUS & DSI_PHY_STOPSTATELANE_1_MASK) >> DSI_PHY_STOPSTATELANE_1;
            break;
        default:
            break;
    }

    return ret;
}
