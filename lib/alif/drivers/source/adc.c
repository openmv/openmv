/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Includes */
#include "adc.h"

/**
 * \fn          void adc_done0_irq_handler(ADC_Type *adc, conv_info_t *conversion)
 * \brief       Handle DONE0 (avg sample ready)interrupts for the ADC instance.
 * \param[in]   adc        : Pointer to the ADC register map
 * \param[in]   conversion : The conversion structure for the ADC instance
 * \return      none
*/
void adc_done0_irq_handler(ADC_Type *adc, conv_info_t *conversion)
{
    volatile uint32_t *sample_reg_ptr = &adc->ADC_SAMPLE_REG_[0];

    /* Clearing the done0 IRQ*/
    adc->ADC_INTERRUPT = ADC_INTR_DONE0_CLEAR;

    if (conversion->mode == ADC_CONV_MODE_CONTINUOUS)
    {
        adc->ADC_INTERRUPT = ADC_INTR_DONE0_CLEAR;
        /* read sample and store  */
        conversion->sampled_value = *(sample_reg_ptr + conversion->read_channel);
        /* Store current channel */
        conversion->curr_channel = conversion->read_channel;
        /* Next channel to be read */
        conversion->read_channel = adc->ADC_SEL;
        /* set call back */
        conversion->status |= ADC_CONV_STAT_COMPLETE;
    }

}

/**
 * @fn       : void adc_done1_irq_handler(ADC_Type *adc, conv_info_t *conversion)
 * @brief    : Handle DONE1 (all sample taken)interrupts for the ADC instance.
 * @param[1] : adc        : Pointer to the ADC register map
 * @param[2] : conversion : The conversion structure for the ADC instance
 * @return   : none
*/
void adc_done1_irq_handler(ADC_Type *adc, conv_info_t *conversion)
{
    volatile uint32_t *sample_reg_ptr = &adc->ADC_SAMPLE_REG_[0];

    /* Clearing the done IRQ*/
    adc->ADC_INTERRUPT = ADC_INTR_DONE1_CLEAR;

    if (conversion->mode == ADC_CONV_MODE_SINGLE_SHOT)
    {
        /* read sample and store to user memory */
        conversion->status |= ADC_CONV_STAT_COMPLETE;
        /* Store current channel */
        conversion->curr_channel = adc->ADC_SEL;
        /* read sample and store  */
        conversion->sampled_value = *(sample_reg_ptr + conversion->curr_channel);
    }
}

/**
 * @fn       : void adc_cmpa_irq_handler(ADC_Type *adc, conv_info_t *conversion)
 * @brief    : Handle CMPA interrupts for the ADC instance.
 * @param[1] : adc        : Pointer to the ADC register map
 * @param[2] : conversion : The conversion structure for the ADC instance
 * @return   : none
*/
void adc_cmpa_irq_handler(ADC_Type *adc, conv_info_t *conversion)
{
    /* Clearing CMPA interrupt */
    adc->ADC_INTERRUPT = ADC_INTR_COMPA_CLEAR;

    switch ((adc->ADC_CONTROL & ADC_THRSHLD_CMP_MASK_BIT) >> ADC_SHIFT_BIT)
    {
    case ADC_CMP_THRHLD_ABOVE_A:
         conversion->status |= ADC_CONV_STAT_CMP_THLD_ABOVE_A;
         break;
    case ADC_CMP_THRHLD_BELOW_A:
         conversion->status |= ADC_CONV_STAT_CMP_THLD_BELOW_A;
         break;
    case ADC_CMP_THRHLD_BETWEEN_A_B:
         conversion->status |= ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B;
         break;
    }
}

/**
 * @fn       : void adc_cmpb_irq_handler(ADC_Type *adc, conv_info_t *conversion)
 * @brief    : Handle CMPB interrupts for the ADC instance.
 * @param[1] : adc        : Pointer to the ADC register map
 * @param[2] : conversion : The conversion structure for the ADC instance
 * @return   : none
*/
void adc_cmpb_irq_handler(ADC_Type *adc, conv_info_t *conversion)
{
    adc->ADC_INTERRUPT = ADC_INTR_COMPB_CLEAR;

    switch ((adc->ADC_CONTROL & ADC_THRSHLD_CMP_MASK_BIT) >> ADC_SHIFT_BIT)
    {
    case ADC_CMP_THRHLD_ABOVE_B:
         conversion->status |= ADC_CONV_STAT_CMP_THLD_ABOVE_B;
         break;
    case ADC_CMP_THRHLD_BELOW_B:
         conversion->status |= ADC_CONV_STAT_CMP_THLD_BELOW_B;
         break;
    case ADC_CMP_THRHLD_OUTSIDE_A_B:
         conversion->status |= ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B;
         break;
    }
}
/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
