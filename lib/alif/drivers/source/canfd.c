/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     canfd.c
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     26-06-2023
 * @brief    Low Level Source File for CANFD.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "canfd.h"

/**
  \fn          static void canfd_copy_tx_buf(uint32_t* dest,
  \                                          const uint32_t* src,
  \                                          const uint8_t len)
  \brief       Copies the message from source to destination buffer
  \note        This function is only applicable for CANFD Tx buffer copy
  \param[in]   dest  : pointer to destination buffer
  \param[in]   src   : pointer to source message buffer
  \param[in]   len   : Length of message
  \return      none
*/
static void canfd_copy_tx_buf(volatile uint32_t* dest,
                              const uint32_t* src,
                              const uint8_t len)
{
    uint8_t  iter           = 0U;
    uint8_t  rem            = 0U;
    uint32_t rem_data = 0U;

    /* Copies the data from src buffer to destination buffer */
    for(iter = 0U; iter < (len/4U); iter++)
    {
        *dest++ = src[iter];
    }
    rem = (len % 4);
    iter = 0U;
    while(rem)
    {
        rem_data |= (((uint8_t*)src)[len - rem] << (8U * iter));
        rem--;
        iter++;
    }
    *dest = rem_data;
}

/**
  \fn          void canfd_enable_acpt_fltr(CANFD_Type* canfd,
  \                                        canfd_acpt_fltr_t filter_config
  \brief       Configures and enables the particular acceptance filter.
  \param[in]   canfd          : Pointer to the CANFD register map
  \param[in]   filter_config  : Filter configuration
  \return      none
*/
void canfd_enable_acpt_fltr(CANFD_Type* canfd, canfd_acpt_fltr_t filter_config)
{
    canfd->CANFD_ACFCTRL = (filter_config.filter &
                            CANFD_ACFCTRL_ACFADR_Msk);

    /* Select AMASK configuration */
    canfd->CANFD_ACFCTRL |= CANFD_ACFCTRL_SELMASK;

    /* Enable filter */
    canfd->CANFD_ACF_EN_0 |= ((1U << filter_config.filter) &
                               CANFD_ACF_EN_0_AE_X_MAX_Msk);

    if(filter_config.op_code == CANFD_ACPT_FLTR_OP_ADD_MASKABLE_ID)
    {
        /* Converting mask from CMSIS value to controller supporting mask*/
        filter_config.ac_mask = ~(filter_config.ac_mask);
    }

    /* Storing the mask */
    filter_config.ac_mask = CANFD_ACF0_3_AMASK_X_Msk(filter_config.ac_mask);

    /* 1. For all frames, the bits 29 and 30 should be zero,
     * 2. For Extended frames, the bits 29 and 30 should be one,
     * 3. For Std frames, bit 29 should be zero and bit 30 should be one */
    if(filter_config.frame_type == CANFD_ACPT_FILTER_CFG_EXT_FRAMES)
    {
        filter_config.ac_mask  |= (CANFD_ACF_3_MASK_AIDE     |
                                   CANFD_ACF_3_MASK_AIDEE);
    }
    else if(filter_config.frame_type == CANFD_ACPT_FILTER_CFG_STD_FRAMES)
    {
        filter_config.ac_mask  |= CANFD_ACF_3_MASK_AIDEE;
    }
    else
    {
        ; // Does nothing
    }

    /* Storing the mask value with the type of frame filtering */
    canfd->CANFD_ACF_0_3_MASK =  filter_config.ac_mask;

    /* Select ACODE configuration */
    canfd->CANFD_ACFCTRL &= ~CANFD_ACFCTRL_SELMASK;

    canfd->CANFD_ACF_0_3_CODE = CANFD_ACF0_3_ACODE_X(filter_config.ac_code);
}

/**
  \fn          CANFD_ACPT_FLTR_STATUS canfd_get_acpt_fltr_status(CANFD_Type* canfd,
  \                                                              const uint8_t filter)
  \brief       Retrieves whether the filter is free or occupied.
  \param[in]   canfd  : Pointer to the CANFD register map
  \param[in]   filter : Acceptance filter number
  \return      status of the filter (Free/Occupied)
*/
CANFD_ACPT_FLTR_STATUS canfd_get_acpt_fltr_status(CANFD_Type* canfd,
                                                  const uint8_t filter)
{
    /* Returns status of the requested filter */
    if(canfd->CANFD_ACF_EN_0 & (1U << filter))
    {
        return CANFD_ACPT_FLTR_STATUS_OCCUPIED;
    }
    return CANFD_ACPT_FLTR_STATUS_FREE;
}

/**
  \fn          void canfd_get_acpt_fltr_data(CANFD_Type* canfd,
  \                                          canfd_acpt_fltr_t *filter_config)
  \brief       Retrieves the acceptance filter data.
  \param[in]   canfd          : Pointer to the CANFD register map
  \param[in]   filter_config  : Filter configuration
  \return      none
*/
void canfd_get_acpt_fltr_data(CANFD_Type* canfd,
                              canfd_acpt_fltr_t *filter_config)
{
    /* Returns information of a requested acceptance filter */
    canfd->CANFD_ACFCTRL = (filter_config->filter &
                            CANFD_ACFCTRL_ACFADR_Msk);

    filter_config->ac_code  = (canfd->CANFD_ACF_0_3_CODE &
                               CANFD_ACF0_3_AMASK_ACODE_X_Msk);

    canfd->CANFD_ACFCTRL   |= CANFD_ACFCTRL_SELMASK;

    filter_config->ac_mask  = (canfd->CANFD_ACF_0_3_MASK &
                               CANFD_ACF0_3_AMASK_ACODE_X_Msk);

    if(filter_config->op_code == CANFD_ACPT_FLTR_OP_REMOVE_MASKABLE_ID)
    {
        /* Converts the mask to CMSIS compliance */
        filter_config->ac_mask = CANFD_ACF0_3_AMASK_X_Msk(~(filter_config->ac_mask));
    }
}

/**
  \fn          CANFD_MSG_ERROR canfd_get_last_error_code(CANFD_Type* canfd)
  \brief       Fetches the latest error occurred
  \param[in]   canfd : Pointer to the CANFD register map
  \return      last found error type
*/
CANFD_MSG_ERROR canfd_get_last_error_code(CANFD_Type* canfd)
{
    uint8_t error = 0U;

    error = ((canfd->CANFD_EALCAP & CANFD_EALCAP_KOER_Msk) >>
                                    CANFD_EALCAP_KOER_Pos);
    switch(error)
    {
        case CANFD_EALCAP_KOER_BIT:
            return CANFD_MSG_ERROR_BIT;

        case CANFD_EALCAP_KOER_FORM:
            return CANFD_MSG_ERROR_FORM;

        case CANFD_EALCAP_KOER_STUFF:
            return CANFD_MSG_ERROR_STUFF;

        case CANFD_EALCAP_KOER_ACK:
            return CANFD_MSG_ERROR_ACK;

        case CANFD_EALCAP_KOER_CRC:
            return CANFD_MSG_ERROR_CRC;

        case CANFD_EALCAP_KOER_NONE:
        default:
            return CANFD_MSG_ERROR_NONE;
    }
}

/**
  \fn          void canfd_set_nominal_bit_time(CANFD_Type* canfd,
  \                                            const uint32_t bitrate_seg,
  \                                            const uint8_t prescaler)
  \brief       Sets the slow speed bit-timing of CANFD instance.
  \param[in]   canfd       : Pointer to the CANFD register map
  \param[in]   bitrate_seg : Segments - Propagation, Sampling
  \param[in]   prescaler   : Prescaler value
  \return      none
*/
void canfd_set_nominal_bit_time(CANFD_Type* canfd,
                                const uint32_t bitrate_seg,
                                const uint8_t prescaler)
{
    /* Configures Nominal bit rate registers */
    canfd->CANFD_S_PRESC = CANFD_DECREMENT(prescaler, 1U);

    canfd->CANFD_S_SEG_1 = CANFD_DECREMENT((((bitrate_seg >>
                                              CANFD_BIT_PROP_SEG_Pos) &
                                              0xFFU) +
                                            ((bitrate_seg >>
                                              CANFD_BIT_PHASE_SEG1_Pos) &
                                              0xFFU)), 2U);

    canfd->CANFD_S_SEG_2 = CANFD_DECREMENT(((bitrate_seg >>
                                             CANFD_BIT_PHASE_SEG2_Pos) &
                                             0xFFU), 1U);
    canfd->CANFD_S_SJW   = CANFD_DECREMENT(((bitrate_seg >>
                                             CANFD_BIT_SJW_Pos) &
                                             0xFFU), 1U);
}

/**
  \fn          void canfd_set_fd_bit_time(CANFD_Type* canfd,
  \                                       const uint32_t bitrate_seg,
  \                                       const uint8_t prescaler)
  \brief       Sets the fast speed bit-timing of CANFD instance.
  \param[in]   canfd       : Pointer to the CANFD register map
  \param[in]   bitrate_seg : Segments - Propagation, Sampling
  \param[in]   prescaler   : Prescaler value
  \return      none
*/
void canfd_set_fd_bit_time(CANFD_Type* canfd,
                           const uint32_t bitrate_seg,
                           const uint8_t prescaler)
{
    /* Configures Fast bit rate registers */
    canfd->CANFD_F_PRESC = CANFD_DECREMENT(prescaler, 1U);

    canfd->CANFD_F_SEG_1 = CANFD_DECREMENT((((bitrate_seg >>
                                              CANFD_BIT_PROP_SEG_Pos) & 0xFFU) +
                                            ((bitrate_seg >>
                                              CANFD_BIT_PHASE_SEG1_Pos) &
                                              0xFFU)), 2U);

    canfd->CANFD_F_SEG_2 = CANFD_DECREMENT(((bitrate_seg >>
                                             CANFD_BIT_PHASE_SEG2_Pos) &
                                             0xFFU), 1U);
    canfd->CANFD_F_SJW   = CANFD_DECREMENT(((bitrate_seg >>
                                             CANFD_BIT_SJW_Pos) &
                                             0xFFU), 1U);
}

/**
  \fn          void canfd_setup_tx_retrans(CANFD_Type* canfd,
  \                                        const uint8_t buf_type,
  \                                        const bool enable)
  \brief       Enables/Disables the Tx msg retransmission
  \param[in]   canfd    : Pointer to the CANFD register map
  \param[in]   buf_type : Bufer type
  \param[in]   enable   : Command to enable/disable msg retransmission
  \return      none
*/
void canfd_setup_tx_retrans(CANFD_Type* canfd,
                            const uint8_t buf_type,
                            const bool enable)
{
    if(enable)
    {
        if(buf_type == CANFD_BUF_TYPE_PRIMARY)
        {
            /* Enables message retransmission of PTB */
            canfd->CANFD_CFG_STAT &= (~CANFD_CFG_STAT_TPSS);
        }
        else
        {
            /* Enables message retransmission of STB */
            canfd->CANFD_CFG_STAT &= (~CANFD_CFG_STAT_TSSS);
        }
    }
    else
    {
        if(buf_type == CANFD_BUF_TYPE_PRIMARY)
        {
            /* Disables message retransmission PTB*/
            canfd->CANFD_CFG_STAT |= CANFD_CFG_STAT_TPSS;
        }
        else
        {
            /* Disables message retransmission of STB*/
            canfd->CANFD_CFG_STAT |= CANFD_CFG_STAT_TSSS;
        }
    }
}

/**
  \fn          void canfd_setup_tx_delay_comp(CANFD_Type* canfd,
  \                                           const uint8_t offset,
  \                                           const bool enable)
  \brief       Enables/Disables the Tx delay compensation
  \param[in]   canfd  : Pointer to the CANFD register map
  \param[in]   offset : Secondary sampling point offest value
  \param[in]   enable : Command to enable/disable TDC
  \return      none
*/
void canfd_setup_tx_delay_comp(CANFD_Type* canfd,
                               const uint8_t offset,
                               const bool enable)
{
    if(enable)
    {
        /* Enables transeiver delay compensation and
         * cofigures offset point value */
        canfd->CANFD_TDC  = CANFD_TDC_TDCEN;
        canfd->CANFD_TDC |= (offset & CANFD_TDC_SSPOFF_Msk);
    }
    else
    {
        /* Disables transeiver delay compensation */
        canfd->CANFD_TDC &= ~CANFD_TDC_TDCEN;
    }
}

/**
  \fn          void canfd_set_err_warn_limit(CANFD_Type* canfd,
  \                                          const uint8_t ewl)
  \brief       Configures Warning limits for Rbuf storage and errors
  \note        If ewl value is greater than CANFD_MAX_ERROR_WARN_LIMIT
  \            the limit will be set to CANFD_MAX_ERROR_WARN_LIMIT
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   ewl   : Limit value for Error warning
  \return      none
*/
void canfd_set_err_warn_limit(CANFD_Type* canfd, const uint8_t ewl)
{

    if(ewl <= CANFD_MAX_ERROR_WARN_LIMIT)
    {
        /* Sets the in range error warning value */
        canfd->CANFD_LIMIT |= ((((ewl / 8U) - 1U) <<
                                  CANFD_LIMIT_EWL_Pos) &
                                  CANFD_LIMIT_EWL_Msk);
    }
    else
    {
        /* Sets error warning to Max */
        canfd->CANFD_LIMIT |= ((((CANFD_MAX_ERROR_WARN_LIMIT / 8U) - 1U) <<
                                  CANFD_LIMIT_EWL_Pos) &
                                  CANFD_LIMIT_EWL_Msk);
    }
}

/**
  \fn          void canfd_send(CANFD_Type* canfd,
  \                            const canfd_tx_info_t tx_header,
  \                            const uint8_t *data,
  \                            const uint8_t size)
  \brief       Prepares and transmits the message
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   tx_header  : Header of tx message
  \param[in]   data       : Message payload
  \param[in]   size       : payload size
  \return      none
*/
void canfd_send(CANFD_Type* canfd, const canfd_tx_info_t tx_header,
                const uint8_t *data, const uint8_t size)
{
    volatile tbuf_regs_t* tx_msg = (volatile tbuf_regs_t*)canfd->CANFD_TBUF;

    /* Copies ID and control fields */
    tx_msg->can_id = (tx_header.id | CANFD_MSG_TTSEN);

    tx_msg->control = (CANFD_MSG_IDE(tx_header.frame_type)    |
                       CANFD_MSG_RTR(tx_header.rtr)           |
                       CANFD_MSG_FDF(tx_header.edl)           |
                       CANFD_MSG_BRS(tx_header.brs)           |
                       CANFD_MSG_DLC(tx_header.dlc));

    /* Copies tx data if it is a data frame*/
    if(tx_header.rtr == 0U)
    {
        canfd_copy_tx_buf((volatile uint32_t*)tx_msg->data,
                          (uint32_t*)data, size);
    }

    if(tx_header.buf_type != CANFD_BUF_TYPE_PRIMARY)
    {
        /* Moves the pointer to next buf slot and
         * enables the tx of all frames in sec buf*/
        canfd->CANFD_TCTRL |= CANFD_TCTRL_TSNEXT;
        canfd->CANFD_TCMD  |= CANFD_TCMD_TSALL;
    }
    else
    {
        /* Enables primary buffer transmission */
        canfd->CANFD_TCMD |= CANFD_TCMD_TPE;
    }
}

/**
  \fn          void canfd_receive(CANFD_Type* canfd,
  \                               canfd_data_transfer_t *dest_data))
  \brief       Fetches the data from Rx buffer
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   dest_data  : Destination Data pointer
  \return      none
*/
void canfd_receive(CANFD_Type* canfd, canfd_transfer_t *dest_data)
{
    uint8_t iter        = 0U;
    rbuf_regs_t* rx_msg = (rbuf_regs_t*)canfd->CANFD_RBUF;

    dest_data->rx_header.id         = (rx_msg->can_id & (~CANFD_MSG_ESI_Msk));
    dest_data->rx_header.esi        = ((rx_msg->can_id >> CANFD_MSG_ESI_Pos) & 1U);
    dest_data->rx_header.frame_type = ((rx_msg->control >> CANFD_MSG_IDE_Pos) & 1U);
    dest_data->rx_header.rtr        = ((rx_msg->control >> CANFD_MSG_RTR_Pos) & 1U);
    dest_data->rx_header.edl        = ((rx_msg->control >> CANFD_MSG_FDF_Pos) & 1U);
    dest_data->rx_header.brs        = ((rx_msg->control >> CANFD_MSG_BRS_Pos) & 1U);
    dest_data->rx_header.status     = rx_msg->status;
    dest_data->rx_header.dlc        = ((rx_msg->control >> CANFD_MSG_DLC_Pos) & 0xFU);

    /* Copy the data*/
    for(iter = 0U; iter < dest_data->rx_count; iter++)
    {
        dest_data->rx_ptr[iter] = rx_msg->data[iter];
    }

    dest_data->rx_header.timestamp[0U] = rx_msg->rx_timestamp[0U];

    /* Release the buffer */
    canfd->CANFD_RCTRL   |= CANFD_RCTRL_RREL;
}

/**
  \fn          void canfd_send_blocking(CANFD_Type* canfd,
  \                                     const canfd_tx_info_t tx_header,
  \                                     const uint8_t *data,
  \                                     const uint8_t size)
  \brief       Prepares and transmits the message in blocking mode
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   tx_header  : Header of tx message
  \param[in]   data       : Message payload
  \param[in]   size       : payload size
  \return      none
*/
void canfd_send_blocking(CANFD_Type* canfd, const canfd_tx_info_t tx_header,
                         const uint8_t *data, const uint8_t size)
{
    volatile tbuf_regs_t* tx_msg = (volatile tbuf_regs_t*)canfd->CANFD_TBUF;

    /* Copies ID and control fields */
    tx_msg->can_id = (tx_header.id | CANFD_MSG_TTSEN);

    tx_msg->control = (CANFD_MSG_IDE(tx_header.frame_type)    |
                       CANFD_MSG_RTR(tx_header.rtr)           |
                       CANFD_MSG_FDF(tx_header.edl)           |
                       CANFD_MSG_BRS(tx_header.brs)           |
                       CANFD_MSG_DLC(tx_header.dlc));

    /* Copies tx data if it is a data frame*/
    if(tx_header.rtr == 0U)
    {
        canfd_copy_tx_buf((volatile uint32_t*)tx_msg->data,
                          (uint32_t*)data, size);
    }

    if(tx_header.buf_type != CANFD_BUF_TYPE_PRIMARY)
    {
        while(!canfd_stb_empty(canfd))
        {
            ;
        }

        /* Moves the pointer to next buf slot and
         * enables the tx of all frames in sec buf*/
        canfd->CANFD_TCTRL |= CANFD_TCTRL_TSNEXT;
        canfd->CANFD_TCMD  |= CANFD_TCMD_TSALL;

        while(!canfd_stb_empty(canfd))
        {
            ;
        }
    }
    else
    {
        while(canfd_ptb_tx_active(canfd))
        {
            ;
        }

        /* Enables primary buffer transmission */
        canfd->CANFD_TCMD |= CANFD_TCMD_TPE;

        while(canfd_ptb_tx_active(canfd))
        {
            ;
        }
    }
}

/**
  \fn          void canfd_receive_blocking(CANFD_Type* canfd,
  \                                        canfd_data_transfer_t *dest_data))
  \brief       Fetches the data from Rx buffer in blocking mode
  \param[in]   canfd      : Pointer to the CANFD register map
  \param[in]   dest_data  : Destination Data pointer
  \return      none
*/
void canfd_receive_blocking(CANFD_Type* canfd, canfd_transfer_t *dest_data)
{
    uint8_t iter        = 0U;
    rbuf_regs_t* rx_msg = (rbuf_regs_t*)canfd->CANFD_RBUF;

    while(!canfd_rx_msg_available(canfd))
    {
        ;
    }

    dest_data->rx_header.id         = (rx_msg->can_id & (~CANFD_MSG_ESI_Msk));
    dest_data->rx_header.esi        = ((rx_msg->can_id >> CANFD_MSG_ESI_Pos) & 1U);
    dest_data->rx_header.frame_type = ((rx_msg->control >> CANFD_MSG_IDE_Pos) & 1U);
    dest_data->rx_header.rtr        = ((rx_msg->control >> CANFD_MSG_RTR_Pos) & 1U);
    dest_data->rx_header.edl        = ((rx_msg->control >> CANFD_MSG_FDF_Pos) & 1U);
    dest_data->rx_header.brs        = ((rx_msg->control >> CANFD_MSG_BRS_Pos) & 1U);
    dest_data->rx_header.status     = rx_msg->status;
    dest_data->rx_header.dlc        = ((rx_msg->control >> CANFD_MSG_DLC_Pos) & 0xFU);

    /* Copy the data*/
    for(iter = 0U; iter < dest_data->rx_count; iter++)
    {
        dest_data->rx_ptr[iter] = rx_msg->data[iter];
    }

    dest_data->rx_header.timestamp[0U] = rx_msg->rx_timestamp[0U];

    /* Release the buffer */
    canfd->CANFD_RCTRL   |= CANFD_RCTRL_RREL;
}

/**
  \fn          void canfd_clear_interrupt(CANFD_Type* canfd,
                                          const uint32_t event)
  \brief       Clears the interrupt
  \param[in]   canfd : Pointer to the CANFD register map
  \param[in]   event : Interrupt event
  \return      none
*/
void canfd_clear_interrupt(CANFD_Type* canfd, const uint32_t event)
{
    uint8_t temp = (uint8_t)event;

    if(event & CANFD_RTIF_REG_Msk)
    {
        if(event & CANFD_RTIF_RIF)
        {
            if(canfd_rx_msg_available(canfd))
            {
                /* If Rx data is still available
                 * then this interrrupt won't be cleared*/
                temp &= ~(CANFD_RTIF_RIF);
            }
        }
        /* Clears Data interrupt */
        canfd->CANFD_RTIF = (temp & CANFD_RTIF_REG_Msk);
        (void)canfd->CANFD_RTIF;
    }
    else if((event >> 8U) & CANFD_ERRINT_REG_Msk)
    {
        /* Clears Error interrupt */
        temp = (canfd->CANFD_ERRINT & CANFD_ERRINT_EN_Msk);
        temp |= ((event >> 8U) & CANFD_ERRINT_REG_Msk);
        canfd->CANFD_ERRINT = temp;
        (void)canfd->CANFD_ERRINT;
    }
}

/**
  \fn          uint32_t canfd_irq_handler(CANFD_Type* canfd)
  \brief       Returns the interrupt event
  \param[in]   canfd  : Pointer to the CANFD register map
  \return      CANFD interrupt event
*/
uint32_t canfd_irq_handler(CANFD_Type* canfd)
{
    uint32_t event = 0U;

    event = (canfd->CANFD_RTIF & CANFD_RTIF_REG_Msk);
    if(!(event))
    {
        event = ((canfd->CANFD_ERRINT & CANFD_ERRINT_REG_Msk) << 8U);
    }
    return event;
}
