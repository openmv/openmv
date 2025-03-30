/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "pdm.h"

/**
  @fn          void pdm_error_detect_irq_handler(PDM_Type *pdm);
  @brief       IRQ handler for the error interrupt
  @param[in]   pdm      : Pointer to the PDM register map
  @return      none
*/
void pdm_error_detect_irq_handler(PDM_Type *pdm)
{
    pdm->PDM_IRQ_ENABLE &= ~(PDM_FIFO_OVERFLOW_IRQ);
}

/**
  @fn          void pdm_audio_detect_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer )
  @brief       IRQ handler for the audio detect interrupt
  @param[in]   pdm      : Pointer to the PDM register map
  @param[in]   transfer : The transfer structure of the PDM instance
  @return      none
*/
void pdm_audio_detect_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer )
{
    /* Check current count is greater than the buffer size */
    if(transfer->curr_cnt  >= (transfer->total_cnt))
    {
        transfer->status |= PDM_AUDIO_STATUS_DETECTION;

        /* disable irq */
        pdm->PDM_IRQ_ENABLE &= ~(PDM_AUDIO_DETECT_IRQ_STAT);
    }
    (void) pdm->PDM_AUDIO_DETECT_IRQ;
}

/**
  @fn          void pdm_warning_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer)
  @brief       IRQ handler for the PDM warning interrupt.
  @param[in]   pdm      : Pointer to the PDM register map
  @param[in]   transfer : The transfer structure of the PDM instance
  @return      none
*/
void pdm_warning_irq_handler(PDM_Type *pdm, pdm_transfer_t *transfer)
{
    uint8_t fifo_count;
    bool fifo_almost_full_irq;
    uint32_t audio_ch;
    uint32_t audio_ch_0_1;
    uint32_t audio_ch_2_3;
    uint32_t audio_ch_4_5;
    uint32_t audio_ch_6_7;

    fifo_count = pdm->PDM_FIFO_STAT;

    fifo_almost_full_irq = pdm->PDM_WARN_IRQ;

    /* User enabled channel */
    audio_ch = pdm->PDM_CTL0 & PDM_CHANNEL_ENABLE;

    if(fifo_almost_full_irq == 1)
    {
        for(uint32_t count = 0; count < fifo_count; count++)
        {
            audio_ch_0_1 = pdm->PDM_CH0_CH1_AUDIO_OUT;
            audio_ch_2_3 = pdm->PDM_CH2_CH3_AUDIO_OUT;
            audio_ch_4_5 = pdm->PDM_CH4_CH5_AUDIO_OUT;
            audio_ch_6_7 = pdm->PDM_CH6_CH7_AUDIO_OUT;

            if(transfer->curr_cnt < transfer->total_cnt)
            {
                if((audio_ch & PDM_CHANNEL_0) == PDM_CHANNEL_0)
                {
                    /* Store the ch 0 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_0_1);
                    transfer->curr_cnt ++;
                }
                if((audio_ch & PDM_CHANNEL_1) == PDM_CHANNEL_1)
                {
                    /* Store the ch 1 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_0_1 >> 16);
                    transfer->curr_cnt ++;
                }
                if((audio_ch & PDM_CHANNEL_2) == PDM_CHANNEL_2)
                {
                    /* Store the ch 2 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_2_3);
                    transfer->curr_cnt ++;
                }
                if((audio_ch & PDM_CHANNEL_3) == PDM_CHANNEL_3)
                {
                    /* Store the ch 3 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_2_3 >> 16);
                    transfer->curr_cnt ++;
                }
                if((audio_ch &  PDM_CHANNEL_4) == PDM_CHANNEL_4)
                {
                    /* Store the ch 4 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_4_5);
                    transfer->curr_cnt ++;
                }
                if((audio_ch & PDM_CHANNEL_5) == PDM_CHANNEL_5)
                {
                    /* Store the ch 5 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_4_5 >> 16);
                    transfer->curr_cnt ++;
                }
                if((audio_ch & PDM_CHANNEL_6)== PDM_CHANNEL_6)
                {
                    /* Store the ch 6 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_6_7);
                    transfer->curr_cnt ++;
                }
                if((audio_ch & PDM_CHANNEL_7) == PDM_CHANNEL_7)
                {
                    /* Store the ch 7 audio output values in the user buffer memory */
                    ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_6_7 >> 16);
                    transfer->curr_cnt ++;
                }
            }
        }
    }

    if(transfer->curr_cnt  >= (transfer->total_cnt ))
    {
        if(fifo_almost_full_irq== 1)
        {
            /* disable irq */
            pdm->PDM_IRQ_ENABLE &= ~(PDM_FIFO_ALMOST_FULL_IRQ | PDM_AUDIO_DETECT_IRQ_STAT | PDM_FIFO_OVERFLOW_IRQ);
            transfer->status |=  PDM_CAPTURE_STATUS_COMPLETE;
        }
    }

    (void) pdm->PDM_ERROR_IRQ;
}

/**
  @fn          void pdm_receive_blocking(PDM_Type *pdm, pdm_transfer_t *transfer)
  @brief       Performs a blocking PDM receive operation based on the provided transfer structure.
  @param[in]   pdm      : Pointer to the PDM register map
  @param[in]   transfer : The transfer structure of the PDM instance
  @return      none
*/
void pdm_receive_blocking(PDM_Type *pdm, pdm_transfer_t *transfer)
{
    uint32_t audio_ch;
    uint32_t audio_ch_0_1;
    uint32_t audio_ch_2_3;
    uint32_t audio_ch_4_5;
    uint32_t audio_ch_6_7;

    /* User enabled channel */
    audio_ch = pdm->PDM_CTL0 & PDM_CHANNEL_ENABLE;

    (void) pdm->PDM_ERROR_IRQ;

    while(transfer->curr_cnt < transfer->total_cnt)
    {
        while(pdm->PDM_FIFO_STAT < 1)
        {
        }

        for(uint32_t count = 0; count < pdm->PDM_FIFO_STAT; count++)
        {
            audio_ch_0_1 = pdm->PDM_CH0_CH1_AUDIO_OUT;
            audio_ch_2_3 = pdm->PDM_CH2_CH3_AUDIO_OUT;
            audio_ch_4_5 = pdm->PDM_CH4_CH5_AUDIO_OUT;
            audio_ch_6_7 = pdm->PDM_CH6_CH7_AUDIO_OUT;

            if((audio_ch & PDM_CHANNEL_0) == PDM_CHANNEL_0)
            {
                /* Store the ch 0 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_0_1);
                transfer->curr_cnt ++;
            }
            if((audio_ch & PDM_CHANNEL_1) == PDM_CHANNEL_1)
            {
                /* Store the ch 1 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_0_1 >> 16);
                transfer->curr_cnt ++;
            }
            if((audio_ch & PDM_CHANNEL_2) == PDM_CHANNEL_2)
            {
                /* Store the ch 2 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_2_3);
                transfer->curr_cnt ++;
            }
            if((audio_ch & PDM_CHANNEL_3) == PDM_CHANNEL_3)
            {
                /* Store the ch 3 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_2_3 >> 16);
                transfer->curr_cnt ++;
            }
            if((audio_ch &  PDM_CHANNEL_4) == PDM_CHANNEL_4)
            {
                /* Store the ch 4 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_4_5);
                transfer->curr_cnt ++;
            }
            if((audio_ch & PDM_CHANNEL_5) == PDM_CHANNEL_5)
            {
                /* Store the ch 5 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_4_5 >> 16);
                transfer->curr_cnt ++;
            }
            if((audio_ch & PDM_CHANNEL_6)== PDM_CHANNEL_6)
            {
                /* Store the ch 6 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_6_7);
                transfer->curr_cnt ++;
            }
            if((audio_ch & PDM_CHANNEL_7) == PDM_CHANNEL_7)
            {
                /* Store the ch 7 audio output values in the user buffer memory */
                ((uint16_t *)transfer->buf)[transfer->curr_cnt] = (uint16_t)(audio_ch_6_7 >> 16);
                transfer->curr_cnt ++;
            }
        }
    }

    /* Check for PDM error status register */
    if(pdm->PDM_ERROR_IRQ == PDM_INTERRUPT_STATUS_VALUE)
    {
         transfer->status |= PDM_ERROR_DETECT;
    }

    /* Set the capture complete event in the transfer status. */
    transfer->status |=  PDM_CAPTURE_STATUS_COMPLETE;
}
