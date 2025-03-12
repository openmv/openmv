/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Include */
#include "Driver_ADC_Private.h"
#include "analog_config.h"

#define ARM_ADC_DRV_VERISON ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /*DRIVER VERSION*/

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion ={
    ARM_ADC_API_VERSION,
    ARM_ADC_DRV_VERISON
};

/* Driver Capabilities */
static const ARM_ADC_CAPABILITIES DriverCapabilities = {
    1,    /* Resolution 12 or 20 bits*/
    0     /* Reserved                */
};

/*
 * @func      : void Analog_config()
 * @brief     : vbat comparator value and register configuration
 * @parameter : NONE
 * @return    : NONE
 */
static void Analog_Config(void)
{
    /* Analog configuration Vbat register2 */
    analog_config_vbat_reg2();

    /* Analog configuration comparator register2 */
    analog_config_cmp_reg2();
}

/*
 * @func           : int32_t ADC_Initialize(ADC_RESOURCES *ADC, ARM_ADC_SignalEvent_t cb_event)
 * @brief          : initialize the device
 * @parameter[1]   : adc      : Pointer to /ref ADC_RESOURCES structure
 * @parameter[2]   : cb_event : Pointer to /ref ARM_ADC_Signal_Event_t cb_event
 * @return         : ARM_DRIVER_OK              : if driver initialized successfully
 *                 : ARM_DRIVER_ERROR_PARAMETER : if parameter is invalid or not
 */
static int32_t ADC_Initialize(ADC_RESOURCES *ADC, ARM_ADC_SignalEvent_t cb_event)
{
    int ret =  ARM_DRIVER_OK ;

    if(!cb_event)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* User call back Event */
    ADC->cb_event = cb_event;

    /* Setting flag to initialize */
    ADC->state |= ADC_FLAG_DRV_INIT_DONE;

    return ret;
}

/*
 * @func           : int32_t ADC_Uninitialize (ARM_ADC_SignalEvent_t cb_event)
 * @brief          : Uninitialize the device
 * @parameter[in]  : ADC    : Pointer to the structure ADC_RESOURCES
 * @return         : ARM_DRIVER_OK              : if adc is successfully initialized
 *                 : ARM_DRIVER_ERROR_PARAMETER : if adc device is invalid
 */
static int32_t ADC_Uninitialize(ADC_RESOURCES *ADC)
{
    int ret = ARM_DRIVER_OK;

    /* parameter checking */
    if(!ADC)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Checking initialized has done or not */
    if(!(ADC->state & ADC_FLAG_DRV_INIT_DONE))
        return ARM_DRIVER_OK;

    /* set call back to NULL */
    ADC->cb_event = NULL;

    /* Reset last read channel */
    ADC->conv.read_channel = 0;

    /* flags */
    ADC->state = 0;

    return ret;
}

/*
 * @func         : int32_t ADC_PowerControl(ARM_POWER_status status, ADC_RESOURCES *adc)
 * @brief        : power the driver and enable NVIC
 * @parameter[1] : ADC              : pointer to /ref ADC_RESOURCES
 * @parameter[2] : state            : power state
 * @return       : ARM_DRIVER_OK    : if power done successful
 *                 ARM_DRIVER_ERROR : if initialize is not done
*/
static int32_t ADC_PowerControl(ADC_RESOURCES *ADC, ARM_POWER_STATE state)
{
    int32_t ret = ARM_DRIVER_OK;

    switch(state)
    {
        case ARM_POWER_FULL:

            if (!(ADC->state & ADC_FLAG_DRV_INIT_DONE))
                return ARM_DRIVER_ERROR;

            if ((ADC->state & ADC_FLAG_DRV_POWER_DONE))
                return ARM_DRIVER_OK;

            /* Clear Any Pending IRQ */
            NVIC_ClearPendingIRQ (ADC->intr_done0_irq_num);
            NVIC_ClearPendingIRQ (ADC->intr_done1_irq_num);
            NVIC_ClearPendingIRQ (ADC->intr_cmpa_irq_num);
            NVIC_ClearPendingIRQ (ADC->intr_cmpb_irq_num);

            /* Set priority */
            NVIC_SetPriority (ADC->intr_done0_irq_num, ADC->intr_done0_irq_priority);
            NVIC_SetPriority (ADC->intr_done1_irq_num, ADC->intr_done1_irq_priority);
            NVIC_SetPriority (ADC->intr_cmpa_irq_num, ADC->intr_cmpa_irq_priority);
            NVIC_SetPriority (ADC->intr_cmpb_irq_num, ADC->intr_cmpb_irq_priority);

            /* Enable the NIVC */
            NVIC_EnableIRQ (ADC->intr_done0_irq_num);
            NVIC_EnableIRQ (ADC->intr_done1_irq_num);
            NVIC_EnableIRQ (ADC->intr_cmpa_irq_num);
            NVIC_EnableIRQ (ADC->intr_cmpb_irq_num);;

            /* adc clock enable */
            adc_set_clk_control(ADC->drv_instance, true);

            /* Enabling comparator clock */
            enable_cmp_periph_clk();

            /*function include vbat and comparator address and it value */
            Analog_Config();

            /* set differential control for ADC12 */
            adc_set_differential_ctrl(ADC->drv_instance,
                                      ADC->differential_enable);

            adc_set_comparator_ctrl(ADC->drv_instance,
                                    ADC->comparator_enable,
                                    ADC->comparator_bias);

            if (ADC->differential_enable == ADC_DIFFERENTIAL_ENABLE || \
               (ADC->drv_instance == ADC_INSTANCE_ADC24_0))
            {
                /* check adc instances pga enabled */
                if (ADC->pga_enable)
                {
                    /* set pga gain */
                    enable_adc_pga_gain(ADC->drv_instance, ADC->pga_value);
                }
            }

            if (ADC->drv_instance == ADC_INSTANCE_ADC24_0)
            {
                /* enable adc24 from control register */
                enable_adc24();

                /* set output rate from control register */
                set_adc24_output_rate(ADC->output_rate);

                /* Set adc24 bias from control register */
                set_adc24_bias(ADC->bias);

                /* Enabling continuous sampling */
                adc24_enable_continous_sample(ADC->regs);
            }
            else
            {
                /* set Sample width value for ADC12 */
                adc_set_sample_width(ADC->regs, ADC->sample_width);
            }

            /* set user channel input */
            adc_init_channel_select(ADC->regs, ADC->conv.user_input);

            /* set the clock divisor */
            adc_set_clk_div(ADC->regs, ADC->clock_div);

            /* set avg sample value */
            adc_set_avg_sample(ADC->regs, ADC->avg_sample_num);

            /* set number of n shift bits */
            adc_set_n_shift_bit(ADC->regs, ADC->shift_n_bit, ADC->shift_left_or_right);

            /* set sequencer control to single channel scan */
            adc_set_single_ch_scan_mode(ADC->regs, &ADC->conv);

            /* Disable the interrupt (mask the interrupt(0xF))*/
            adc_mask_interrupt(ADC->regs);

            /* Set the power flag enabled */
            ADC->state |= ADC_FLAG_DRV_POWER_DONE;

            break;

        case ARM_POWER_OFF:

            /* Disable ADC NVIC */
            NVIC_DisableIRQ (ADC->intr_done0_irq_num);
            NVIC_DisableIRQ (ADC->intr_done1_irq_num);
            NVIC_DisableIRQ (ADC->intr_cmpa_irq_num);
            NVIC_DisableIRQ (ADC->intr_cmpb_irq_num);

            /* Clear Any Pending IRQ */
            NVIC_ClearPendingIRQ (ADC->intr_done0_irq_num);
            NVIC_ClearPendingIRQ (ADC->intr_done1_irq_num);
            NVIC_ClearPendingIRQ (ADC->intr_cmpa_irq_num);
            NVIC_ClearPendingIRQ (ADC->intr_cmpb_irq_num);

            /* set the clock divisor */
            adc_set_clk_div(ADC->regs, ADC_CLOCK_DIV_MIN_VALUE);

            /* set avg sample value */
            adc_set_avg_sample(ADC->regs, ADC_AVG_SAMPLES_FOR_AVG_MIN);

            /* set Sample width value */
            adc_set_sample_width(ADC->regs, ADC_SAMPLE_WIDTH_MIN_VALUE);

            /* set number of n shift bits */
            adc_set_n_shift_bit(ADC->regs, 0, 0);

            /* Disable the interrupt (mask the interrupt(0xF)) */
            adc_mask_interrupt(ADC->regs);

            if (ADC->differential_enable == ADC_DIFFERENTIAL_ENABLE || \
               (ADC->drv_instance == ADC_INSTANCE_ADC24_0))
            {
                /* check adc instances pga enabled */
                if (ADC->pga_value)
                {
                    /* Disable pga gain */
                    disable_adc_pga_gain(ADC->drv_instance);
                }
            }

            if (ADC->drv_instance == ADC_INSTANCE_ADC24_0)
            {
                /* disable adc24 from control register */
                disable_adc24();

                /* set output rate from control register */
                set_adc24_output_rate(0U);
            }

            /* Disabling CMP clock */
            disable_cmp_periph_clk();

            /* adc clock disable */
            adc_set_clk_control(ADC->drv_instance, false);

            /* Reset the power status of ADC */
            ADC->state &= ~ADC_FLAG_DRV_POWER_DONE;

            break;

        case ARM_POWER_LOW:
        default:
             return ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
    }
        return ret;
}

/*
 * @func        : int32_t ADC_Start( ADC_RESOURCES *ADC)
 * @brief       : Start the adc and initialize interrupt
 * @parameter   : ADC  : pointer to ADC_RESOURCES structure
 * @return      : ARM_DRIVER_OK              : if the function are return successful
 *                ARM_DRIVER_ERROR_PARAMETER : if parameter are invalid
 */
static int32_t ADC_Start( ADC_RESOURCES *ADC)
{
    /* Check Power done or not */
    if (!(ADC->state & ADC_FLAG_DRV_POWER_DONE))
        return ARM_DRIVER_ERROR;

    if(ADC->busy == 1U)
        return ARM_DRIVER_ERROR_BUSY;

    /* setup conversion status */
    ADC->conv.status    = ADC_CONV_STAT_NONE;

    /* active the conv busy flag */
    ADC->busy = 1U;

    /* enable the interrupt(unmask the interrupt 0x0)*/
    adc_unmask_interrupt(ADC->regs);

    if (ADC->ext_trig_val)
    {
        /* Enable the trigger */
        adc_enable_external_trigger(ADC->regs, ADC->ext_trig_val);
    }
    else
    {
        /* Start the ADC conversion mode */
        if (ADC->conv.mode == ADC_CONV_MODE_SINGLE_SHOT)
        {
            /* Enable single shot conversion */
            adc_enable_single_shot_conv(ADC->regs);
        }
        else
        {
            /* Enable continuous conversion */
            adc_enable_continuous_conv(ADC->regs);
        }
    }

    return ARM_DRIVER_OK;
}

/*
 * @func      : int32_t ADC_Stop( ADC_RESOURCES *adc)
 * @brief     : Disable the adc
 * @parameter : ADC  : pointer to ADC_RESOURCES structure
 * @return    : ARM_DRIVER_OK : if function return successfully
 */
static int32_t ADC_Stop(ADC_RESOURCES *ADC)
{
    /* Check Power done or not */
    if (!(ADC->state & ADC_FLAG_DRV_POWER_DONE))
        return ARM_DRIVER_ERROR;


    /* Disable the interrupt(mask the interrupt 0xF)*/
    adc_mask_interrupt(ADC->regs);

    if (ADC->ext_trig_val)
    {
        /* Disable the trigger */
        adc_disable_external_trigger(ADC->regs, ADC->ext_trig_val);
    }
    else
    {
        /* Disable the adc */
        if (ADC->conv.mode == ADC_CONV_MODE_SINGLE_SHOT)
        {
            adc_disable_single_shot_conv(ADC->regs);
        }
        else
        {
            adc_disable_continuous_conv(ADC->regs);
        }
    }

    return ARM_DRIVER_OK;
}

/*
 * @func         : in32_t ADC_Control(uint32_t control , uint32_t arg, ADC_RESOURCES adc)
 * @brief        : control the following
 *                 - ARM_SET_SHIFT_CONTROL             : to control shift control of bits
 *                 - ARM_SET_SEQUENCER_CTRL            : selecting sample individual or rotate through
 *                                                       each unmasked sample
 *                 - ARM_ADC_SEQUENCER_MSK_CTRL        : to control masking of the channel
 *                 - ARM_ADC_CHANNEL_INIT_VAL          : to select initial channel for storing
 *                 - ARM_SET_ADC_COMPARATOR_A          : to set comparator a value
 *                 - ARM_SET_ADC_COMPARATOR_B          : to set comparator b value
 *                 - ARM_SET_ADC_THRESHOLD_COMPARISON  : to set the threshold comparison
 *                 - ARM_ADC_SET_CONVERSION_MODE       : to set conversion mode
 * @parameter[1] : ADC  : pointer to ADC_RESOURCES structure
 * @parameter[2] : Control : Selecting the operation
 * @parameter[3] : arg     : values for the the operation
 * @return[1]    : ARM_DRIVER_OK              : if function return successfully
 * @return[2]    : ARM_DRIVER_ERROR_PARAMETER : if adc parameter are invalid
 */
static int32_t ADC_Control(ADC_RESOURCES *ADC, uint32_t Control, uint32_t arg)
{
    int ret = ARM_DRIVER_OK;

    /* Check Power done or not */
    if (!(ADC->state & ADC_FLAG_DRV_POWER_DONE))
        return ARM_DRIVER_ERROR;

    switch(Control)
    {
        case ARM_ADC_SHIFT_CTRL:

            /*selecting the mode for the shifting bit left(0) or right(1) */
            if(arg)
            {
                adc_output_right_shift(ADC->regs);
            }
            else
            {
                adc_output_left_shift(ADC->regs);
            }

        break;

        case ARM_ADC_SEQUENCER_CTRL:

            if(!(arg == 0 || arg == 1))
                return ARM_DRIVER_ERROR_PARAMETER;

            /*selecting the mode of control for taking single scan(1) or multiple channel scan(0)*/
            if(arg == ADC_SCAN_MODE_SINGLE_CH)
            {
                adc_set_single_ch_scan_mode(ADC->regs, &ADC->conv);
            }
            else
            {
                adc_set_multi_ch_scan_mode(ADC->regs, &ADC->conv);
            }

        break;

        case ARM_ADC_SEQUENCER_MSK_CH_CTRL:

            if(!(arg < ADC_MSK_ALL_CHANNELS))
                 return ARM_DRIVER_ERROR_PARAMETER;

            /* set channel to be masked */
            adc_sequencer_msk_ch_control(ADC->regs, arg);

        break;

        case ARM_ADC_CHANNEL_INIT_VAL:

            if(!(arg < ADC_MAX_INIT_CHANNEL))
                 return ARM_DRIVER_ERROR_PARAMETER;

            if(ADC->differential_enable == ADC_DIFFERENTIAL_ENABLE)
            {
                /* check for differential input channels
                 * 3 input channels are used in differential mode
                 * which are channel 0,1 and 2
                 */
                if (arg > ADC_MAX_DIFFERENTIAL_CHANNEL)
                    return ARM_DRIVER_ERROR_PARAMETER;
            }

            if(ADC->drv_instance == ADC_INSTANCE_ADC24_0)
            {
                /* 4 Differential input channels  are there in ADC24 */
                if (arg > ADC24_MAX_DIFFERENTIAL_CHANNEL)
                    return ARM_DRIVER_ERROR_PARAMETER;
            }

            /* select the initial value */
            adc_init_channel_select(ADC->regs, arg);

            /* Store first channel to start conversion */
            ADC->conv.read_channel = arg;

        break;

        case ARM_ADC_COMPARATOR_A:
            /* set comparator A */
            adc_set_comparator_A(ADC->regs, arg);
        break;

        case ARM_ADC_COMPARATOR_B:
            /* set comparator B */
            adc_set_comparator_B(ADC->regs, arg);
        break;

        case ARM_ADC_THRESHOLD_COMPARISON:

            if(!(arg < 3))
                return ARM_DRIVER_ERROR_PARAMETER;
            /* set comparison control bit */
            adc_set_comparator_ctrl_bit(ADC->regs, arg);

        break;

        case ARM_ADC_CONVERSION_MODE_CTRL:

            if(!(arg == 0 || arg == 1))
                return ARM_DRIVER_ERROR_PARAMETER;

            /* set conversion mode */
            if (arg)
            {
                ADC->conv.mode = ADC_CONV_MODE_SINGLE_SHOT;
            }
            else
            {
                ADC->conv.mode = ADC_CONV_MODE_CONTINUOUS;
            }
        break;

        case ARM_ADC_EXTERNAL_TRIGGER_ENABLE:

            if(arg > ADC_EXTERNAL_TRIGGER_MAX_VAL)
                return ARM_DRIVER_ERROR_PARAMETER;

            ADC->ext_trig_val = arg;
        break;

        case ARM_ADC_EXTERNAL_TRIGGER_DISABLE:

            if(arg > ADC_EXTERNAL_TRIGGER_MAX_VAL)
                return ARM_DRIVER_ERROR_PARAMETER;

            ADC->ext_trig_val = arg;
        break;

        case ARM_ADC_HARDWARE_AVERAGING_CTRL:

            /* argument is power of 2 */
            if ((arg & (arg - 1)) == 0)
            {
                /* Check if the value is between 2 to 256 */
                if (arg < ADC_AVG_SAMPLES_FOR_AVG_MIN || arg > ADC_AVG_SAMPLES_FOR_AVG_MAX)
                    return ARM_DRIVER_ERROR;
            }

            /* set average sample number */
            adc_set_avg_sample(ADC->regs, arg);
        break;

        case ARM_ADC_INPUT_CLOCK_DIV_CTRL:

            /* check for CLOCK INPUT */
             if (arg > ADC_CLOCK_DIV_MIN_VALUE || arg < ADC_CLOCK_DIV_MAX_VALUE)
                 return ARM_DRIVER_ERROR;

            /* set the clock divisor */
            adc_set_clk_div(ADC->regs, arg);
        break;

        case ARM_ADC_SAMPLE_WIDTH_CTRL:

            /* check for sample width input */
            if (ADC->drv_instance != ADC_INSTANCE_ADC24_0)
            {
                if ((arg < ADC_SAMPLE_WIDTH_MIN_VALUE || arg > ADC_SAMPLE_WIDTH_MAX_VALUE))
                     return ARM_DRIVER_ERROR_PARAMETER;
            }

            /* set Sample width value for ADC12 and ADC24*/
            adc_set_sample_width(ADC->regs, arg);
        break;

        case ARM_ADC_DIFFERENTIAL_MODE_CTRL:

            if (arg)
            {
                if (ADC->drv_instance != ADC_INSTANCE_ADC24_0)
                {
                    adc_set_differential_ctrl(ADC->drv_instance, ENABLE);
                }

                /* set pga gain */
                enable_adc_pga_gain(ADC->drv_instance, ADC->pga_value);
            }
            else
            {
                /* Disable differential */
                if (ADC->drv_instance != ADC_INSTANCE_ADC24_0)
                {
                    adc_set_differential_ctrl(ADC->drv_instance, DISABLE);
                }
                /* Disable pga gain */
                disable_adc_pga_gain(ADC->drv_instance);
            }
        break;

        case ARM_ADC_SET_PGA_GAIN_CTRL:

            /* check for pga gain input */
            if(arg > ADC_PGA_GAIN_MAX_VALUE)
                return ARM_DRIVER_ERROR_PARAMETER;

            /* set pga gain */
            enable_adc_pga_gain(ADC->drv_instance, arg);
        break;

        case ARM_ADC_24_BIAS_CTRL:

            /* check for bias control input */
            if(arg > ADC_24_BIAS_MAX_VALUE)
                return ARM_DRIVER_ERROR_PARAMETER;

            set_adc24_bias(arg);
        break;

        case ARM_ADC_24_OUTPUT_RATE_CTRL:

            /* check for the arg input */
            if(arg < ADC_24_OUPUT_RATE_MAX_VALUE)
                return ARM_DRIVER_ERROR_PARAMETER;

            /* set output rate from control register */
            set_adc24_output_rate(arg);
        break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ret;
}

/* RTE_ADC120 */
#if (RTE_ADC120)

static ADC_RESOURCES ADC120_RES = {
  .cb_event                = NULL,                                    /* ARM_ADC_SignalEvent_t        */
  .regs                    = (ADC_Type *)ADC120_BASE,                 /* ADC register base address    */
  .conv.user_input         = RTE_ADC120_INPUT_NUM,                    /* user input                   */
  .drv_instance            = ADC_INSTANCE_ADC12_0,                    /* Driver instances             */
  .intr_done0_irq_num      = (IRQn_Type) ADC120_DONE0_IRQ_IRQn,       /* ADC DONE0 IRQ number         */
  .intr_done1_irq_num      = (IRQn_Type) ADC120_DONE1_IRQ_IRQn,       /* ADC DONE1 IRQ number         */
  .intr_cmpa_irq_num       = (IRQn_Type) ADC120_CMPA_IRQ_IRQn,        /* ADC CMPA IRQ number          */
  .intr_cmpb_irq_num       = (IRQn_Type) ADC120_CMPB_IRQ_IRQn,        /* ADC CMPB IRQ number          */
  .busy                    = 0,                                       /* ADC busy                     */
  .intr_done0_irq_priority = RTE_ADC120_DONE0_IRQ_PRIORITY,           /* ADC done0 irq priority       */
  .intr_done1_irq_priority = RTE_ADC120_DONE1_IRQ_PRIORITY,           /* ADC done1 irq priority       */
  .intr_cmpa_irq_priority  = RTE_ADC120_CMPA_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .intr_cmpb_irq_priority  = RTE_ADC120_CMPB_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .clock_div               = RTE_ADC120_CLOCK_DIV,                    /* clock divisor                */
  .avg_sample_num          = RTE_ADC120_AVG_SAMPLE_NUM,               /* average sample number        */
  .sample_width            = RTE_ADC120_SAMPLE_WIDTH,                 /* sample width                 */
  .shift_n_bit             = RTE_ADC120_SHIFT_N_BIT,                  /* number of shift bit          */
  .shift_left_or_right     = RTE_ADC120_SHIFT_LEFT_OR_RIGHT,          /* shifting left to right       */
  .differential_enable     = RTE_ADC120_DIFFERENTIAL_EN,
  .comparator_enable       = RTE_ADC120_COMPARATOR_EN,
  .comparator_bias         = RTE_ADC120_COMPARATOR_BIAS,
  .pga_enable              = RTE_ADC120_PGA_EN,
  .pga_value               = RTE_ADC120_PGA_GAIN
};

/**
 @fn        : void ADC120_DONE0_IRQHandler(void)
 @brief     : DONE0 (AVG SAMPLE RDY) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC120_DONE0_IRQHandler(void)
{
  conv_info_t *conv = &(ADC120_RES.conv);

  adc_done0_irq_handler(ADC120_RES.regs, conv);

  if (conv->status & ADC_CONV_STAT_COMPLETE)
  {
      /* set busy flag to 0U */
      ADC120_RES.busy = 0U;

      /* clearing conversion complete status */
      conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

      ADC120_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
  }
}

/**
 @fn        : void ADC120_DONE1_IRQHandler (void)
 @brief     : DONE1 (All sample taken) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC120_DONE1_IRQHandler (void)
{
    conv_info_t *conv = &(ADC120_RES.conv);

    adc_done1_irq_handler(ADC120_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_COMPLETE)
    {
        /* set busy flag to 0U */
        ADC120_RES.busy = 0U;

        /* clearing conversion complete status */
        conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

        ADC120_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
    }
}

/**
 @fn        : void ADC120_CMPA_IRQHandler (void)
 @brief     : CMPA Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC120_CMPA_IRQHandler (void)
{
    conv_info_t *conv = &(ADC120_RES.conv);

    adc_cmpa_irq_handler(ADC120_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_A);

        ADC120_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_A, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_A);

        ADC120_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_A, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B);

        ADC120_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B, 0, 0);
    }
}

/**
 @fn        : void ADC120_CMPB_IRQHandler (void)
 @brief     : CMPB Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC120_CMPB_IRQHandler (void)
{
    conv_info_t *conv = &(ADC120_RES.conv);

    adc_cmpb_irq_handler(ADC120_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_B);

        ADC120_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_B, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_B);

        ADC120_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_B, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B);

        ADC120_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B, 0, 0);
    }
}

/**
 @fn       ARM_DRIVER_VERSION ADC120_GetVersion(void)
 @brief    Get ADC120 VERSION
 @return   DriverVersion
**/
static ARM_DRIVER_VERSION ADC120_GetVersion(void)
{
  return DriverVersion;
}
/**
 @fn       ARM_ADC120_CAPABILITIES ADC120_GetCapabilities(void)
 @brief    Get ADC120 CAPABILITIES
 @return   DriverCapabilities
**/
static ARM_ADC_CAPABILITIES ADC120_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
 @fn           : int32_t ADC120_Initialize(ARM_ADC_SignalEvent_t cb_event)
 @brief        : Initialize the ADC Interface
 @parameter[1] : cb_event : Pointer to \ref ARM_ADC_SignalEvent_t
 @return       : execution_status
**/
static int32_t ADC120_Initialize(ARM_ADC_SignalEvent_t cb_event)
{
  return (ADC_Initialize(&ADC120_RES, cb_event));
}

/**
 @fn           : int32_t ADC120_Uninitialize(void)
 @brief        : Un-Initialize the ADC Interface
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC120_Uninitialize(void)
{
  return (ADC_Uninitialize(&ADC120_RES));
}

/**
 @fn           : int32_t ADC120_Start(void)
 @brief        : start ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC120_Start(void)
{
  return (ADC_Start(&ADC120_RES));
}

/**
 @fn           : int32_t ADC120_Stop(void)
 @brief        : stop ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC120_Stop(void)
{
  return (ADC_Stop(&ADC120_RES));
}

/**
 @fn           : int32_t ADC120_PowerControl(ARM_POWER_STATE status)
 @brief        : Control ADC Interface power
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC120_PowerControl(ARM_POWER_STATE status)
{
  return(ADC_PowerControl(&ADC120_RES, status));
}

/**
 @fn           : int32_t ADC121_Control(uint32_t Control, uint32_t arg)
 @brief        : Control ADC Interface
 @parameter[1] : Control : control operation
 @parameter[2] : arg     : Argument for operation
 @return       : execution_status
**/
static int32_t ADC120_Control(uint32_t Control, uint32_t arg)
{
  return (ADC_Control(&ADC120_RES, Control, arg));
}

extern ARM_DRIVER_ADC Driver_ADC120;
ARM_DRIVER_ADC Driver_ADC120 ={
    ADC120_GetVersion,
    ADC120_GetCapabilities,
    ADC120_Initialize,
    ADC120_Uninitialize,
    ADC120_Start,
    ADC120_Stop,
    ADC120_PowerControl,
    ADC120_Control
};
#endif /* RTE_ADC120 */

/* RTE_ADC121 */
#if (RTE_ADC121)

static ADC_RESOURCES ADC121_RES = {
  .cb_event                = NULL,                                    /* ARM_ADC_SignalEvent_t        */
  .regs                    = (ADC_Type *)ADC121_BASE,                 /* ADC register base address    */
  .conv.user_input         = RTE_ADC121_INPUT_NUM,                    /* user input                   */
  .drv_instance            = ADC_INSTANCE_ADC12_1,                    /* Driver instances             */
  .intr_done0_irq_num      = (IRQn_Type) ADC121_DONE0_IRQ_IRQn,       /* ADC DONE0 number             */
  .intr_done1_irq_num      = (IRQn_Type) ADC121_DONE1_IRQ_IRQn,       /* ADC DONE1 IRQ number         */
  .intr_cmpa_irq_num       = (IRQn_Type) ADC121_CMPA_IRQ_IRQn,        /* ADC CMPA IRQ number          */
  .intr_cmpb_irq_num       = (IRQn_Type) ADC121_CMPB_IRQ_IRQn,        /* ADC CMPB IRQ number          */
  .busy                    = 0,                                       /* ADC busy                     */
  .intr_done0_irq_priority = RTE_ADC121_DONE0_IRQ_PRIORITY,           /* ADC done0 irq priority       */
  .intr_done1_irq_priority = RTE_ADC121_DONE1_IRQ_PRIORITY,           /* ADC done1 irq priority       */
  .intr_cmpa_irq_priority  = RTE_ADC121_CMPA_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .intr_cmpb_irq_priority  = RTE_ADC121_CMPB_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .clock_div               = RTE_ADC121_CLOCK_DIV,                    /* clock divisor                */
  .avg_sample_num          = RTE_ADC121_AVG_SAMPLE_NUM,               /* average sample number        */
  .sample_width            = RTE_ADC121_SAMPLE_WIDTH,                 /* sample width                 */
  .shift_n_bit             = RTE_ADC121_SHIFT_N_BIT,                  /* number of shift bit          */
  .shift_left_or_right     = RTE_ADC121_SHIFT_LEFT_OR_RIGHT,          /* shifting left to right       */
  .differential_enable     = RTE_ADC121_DIFFERENTIAL_EN,
  .comparator_enable       = RTE_ADC121_COMPARATOR_EN,
  .comparator_bias         = RTE_ADC121_COMPARATOR_BIAS,
  .pga_enable              = RTE_ADC121_PGA_EN,
  .pga_value               = RTE_ADC121_PGA_GAIN
};

/**
 @fn        : void ADC121_DONE0_IRQHandler(void)
 @brief     : DONE0 (AVG SAMPLE RDY) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC121_DONE0_IRQHandler(void)
{
  conv_info_t *conv = &(ADC121_RES.conv);

  adc_done0_irq_handler(ADC121_RES.regs, conv);

  if (conv->status & ADC_CONV_STAT_COMPLETE)
  {
      /* set busy flag to 0U */
      ADC121_RES.busy = 0U;

      /* clearing conversion complete status */
      conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

      ADC121_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
  }
}

/**
 @fn        : void ADC121_DONE1_IRQHandler (void)
 @brief     : DONE1 (All sample taken) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC121_DONE1_IRQHandler (void)
{
    conv_info_t *conv = &(ADC121_RES.conv);

    adc_done1_irq_handler(ADC121_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_COMPLETE)
    {
        /* set busy flag to 0U */
        ADC121_RES.busy = 0U;

        /* clearing conversion complete status */
        conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

        ADC121_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
    }
}

/**
 @fn        : void ADC121_CMPA_IRQHandler (void)
 @brief     : CMPA Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC121_CMPA_IRQHandler (void)
{
    conv_info_t *conv = &(ADC121_RES.conv);

    adc_cmpa_irq_handler(ADC121_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_A);

        ADC121_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_A, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_A);

        ADC121_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_A, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B);

        ADC121_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B, 0, 0);
    }
}

/**
 @fn        : void ADC122_CMPB_IRQHandler (void)
 @brief     : CMPB Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC121_CMPB_IRQHandler (void)
{
    conv_info_t *conv = &(ADC121_RES.conv);

    adc_cmpb_irq_handler(ADC121_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_B);

        ADC121_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_B, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_B);

        ADC121_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_B, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B);

        ADC121_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B, 0, 0);
    }
}

/**
 @fn       ARM_DRIVER_VERSION ADC121_GetVersion(void)
 @brief    Get ADC121 VERSION
 @return   DriverVersion
**/
static ARM_DRIVER_VERSION ADC121_GetVersion(void)
{
  return DriverVersion;
}
/**
 @fn       ARM_ADC121_CAPABILITIES ADC121_GetCapabilities(void)
 @brief    Get ADC121 CAPABILITIES
 @return   DriverCapabilities
**/
static ARM_ADC_CAPABILITIES ADC121_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
 @fn           : int32_t ADC121_Initialize(ARM_ADC_SignalEvent_t cb_event)
 @brief        : Initialize the ADC Interface
 @parameter[1] : cb_event : Pointer to \ref ARM_ADC_SignalEvent_t
 @return       : execution_status
**/
static int32_t ADC121_Initialize(ARM_ADC_SignalEvent_t cb_event)
{
  return (ADC_Initialize(&ADC121_RES, cb_event));
}

/**
 @fn           : int32_t ADC121_Uninitialize(void)
 @brief        : Un-Initialize the ADC Interface
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC121_Uninitialize(void)
{
  return (ADC_Uninitialize(&ADC121_RES));
}

/**
 @fn           : int32_t ADC121_Start(void)
 @brief        : start ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC121_Start(void)
{
  return (ADC_Start(&ADC121_RES));
}

/**
 @fn           : int32_t ADC121_Stop(void)
 @brief        : stop ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC121_Stop(void)
{
  return (ADC_Stop(&ADC121_RES));
}

/**
 @fn           : int32_t ADC121_PowerControl(ARM_POWER_STATE status)
 @brief        : Control ADC Interface power
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC121_PowerControl(ARM_POWER_STATE status)
{
  return(ADC_PowerControl(&ADC121_RES, status));
}

/**
 @fn           : int32_t ADC121_Control(uint32_t Control, uint32_t arg)
 @brief        : Control ADC Interface
 @parameter[1] : Control : control operation
 @parameter[2] : arg     : Argument for operation
 @return       : execution_status
**/
static int32_t ADC121_Control(uint32_t Control, uint32_t arg)
{
  return (ADC_Control(&ADC121_RES, Control, arg));
}

extern ARM_DRIVER_ADC Driver_ADC121;
ARM_DRIVER_ADC Driver_ADC121 ={
    ADC121_GetVersion,
    ADC121_GetCapabilities,
    ADC121_Initialize,
    ADC121_Uninitialize,
    ADC121_Start,
    ADC121_Stop,
    ADC121_PowerControl,
    ADC121_Control
};
#endif /* RTE_ADC121 */

/* RTE_ADC122 */
#if (RTE_ADC122)

static ADC_RESOURCES ADC122_RES = {
  .cb_event                = NULL,                                    /* ARM_ADC_SignalEvent_t        */
  .regs                    = (ADC_Type *)ADC122_BASE,                 /* ADC register base address    */
  .conv.user_input         = RTE_ADC122_INPUT_NUM,                    /* user input                   */
  .drv_instance            = ADC_INSTANCE_ADC12_2,                    /* Driver instances             */
  .intr_done0_irq_num      = (IRQn_Type) ADC122_DONE0_IRQ_IRQn,       /* ADC DONE0 IRQ number         */
  .intr_done1_irq_num      = (IRQn_Type) ADC122_DONE1_IRQ_IRQn,       /* ADC DONE1 IRQ number         */
  .intr_cmpa_irq_num       = (IRQn_Type) ADC122_CMPA_IRQ_IRQn,        /* ADC CMPA IRQ number          */
  .intr_cmpb_irq_num       = (IRQn_Type) ADC122_CMPB_IRQ_IRQn,        /* ADC CMPB IRQ number          */
  .busy                    = 0,                                       /* ADC busy                     */
  .intr_done0_irq_priority = RTE_ADC122_DONE0_IRQ_PRIORITY,           /* ADC done0 irq priority       */
  .intr_done1_irq_priority = RTE_ADC122_DONE1_IRQ_PRIORITY,           /* ADC done1 irq priority       */
  .intr_cmpa_irq_priority  = RTE_ADC122_CMPA_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .intr_cmpb_irq_priority  = RTE_ADC122_CMPB_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .clock_div               = RTE_ADC122_CLOCK_DIV,                    /* clock divisor                */
  .avg_sample_num          = RTE_ADC122_AVG_SAMPLE_NUM,               /* average sample number        */
  .sample_width            = RTE_ADC122_SAMPLE_WIDTH,                 /* sample width                 */
  .shift_n_bit             = RTE_ADC122_SHIFT_N_BIT,                  /* number of shift bit          */
  .shift_left_or_right     = RTE_ADC122_SHIFT_LEFT_OR_RIGHT,          /* shifting left to right       */
  .differential_enable     = RTE_ADC122_DIFFERENTIAL_EN,
  .comparator_enable       = RTE_ADC122_COMPARATOR_EN,
  .comparator_bias         = RTE_ADC122_COMPARATOR_BIAS,
  .pga_enable              = RTE_ADC122_PGA_EN,
  .pga_value               = RTE_ADC122_PGA_GAIN
};

/**
 @fn        : void ADC122_DONE0_IRQHandler(void)
 @brief     : DONE0 (AVG SAMPLE RDY) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC122_DONE0_IRQHandler(void)
{
  conv_info_t *conv = &(ADC122_RES.conv);

  adc_done0_irq_handler(ADC122_RES.regs, conv);

  if (conv->status & ADC_CONV_STAT_COMPLETE)
  {
      /* set busy flag to 0U */
      ADC122_RES.busy = 0U;

      /* clearing conversion complete status */
      conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

      ADC122_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
  }
}

/**
 @fn        : void ADC122_DONE1_IRQHandler (void)
 @brief     : DONE1 (All sample taken) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC122_DONE1_IRQHandler (void)
{
    conv_info_t *conv = &(ADC122_RES.conv);

    adc_done1_irq_handler(ADC122_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_COMPLETE)
    {
        /* set busy flag to 0U */
        ADC122_RES.busy = 0U;

        /* clearing conversion complete status */
        conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

        ADC122_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
    }
}

/**
 @fn        : void ADC122_CMPA_IRQHandler (void)
 @brief     : CMPA Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC122_CMPA_IRQHandler (void)
{
    conv_info_t *conv = &(ADC122_RES.conv);

    adc_cmpa_irq_handler(ADC122_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_A);

        ADC122_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_A,0,0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_A);

        ADC122_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_A,0,0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B);

        ADC122_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B,0,0);
    }
}

/**
 @fn        : void ADC122_CMPB_IRQHandler (void)
 @brief     : CMPB Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC122_CMPB_IRQHandler (void)
{
    conv_info_t *conv = &(ADC122_RES.conv);

    adc_cmpb_irq_handler(ADC122_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_B);

        ADC122_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_B,0,0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_B);

        ADC122_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_B,0,0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B);

        ADC122_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B,0,0);
    }
}

/**
 @fn       ARM_DRIVER_VERSION ADC1_GetVersion(void)
 @brief    Get ADC1 VERSION
 @return   DriverVersion
**/
static ARM_DRIVER_VERSION ADC122_GetVersion(void)
{
  return DriverVersion;
}
/**
 @fn       ARM_ADC122_CAPABILITIES ADC122_GetCapabilities(void)
 @brief    Get ADC122 CAPABILITIES
 @return   DriverCapabilities
**/
static ARM_ADC_CAPABILITIES ADC122_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
 @fn           : int32_t ADC122_Initialize(ARM_ADC_SignalEvent_t cb_event)
 @brief        : Initialize the ADC Interface
 @parameter[1] : cb_event : Pointer to \ref ARM_ADC_SignalEvent_t
 @return       : execution_status
**/
static int32_t ADC122_Initialize(ARM_ADC_SignalEvent_t cb_event)
{
  return (ADC_Initialize(&ADC122_RES, cb_event));
}

/**
 @fn           : int32_t ADC122_Uninitialize(void)
 @brief        : Un-Initialize the ADC Interface
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC122_Uninitialize(void)
{
  return (ADC_Uninitialize(&ADC122_RES));
}

/**
 @fn           : int32_t ADC122_Start(void)
 @brief        : start ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC122_Start(void)
{
    return (ADC_Start(&ADC122_RES));
}

/**
 @fn           : int32_t ADC122_Stop(void)
 @brief        : stop ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC122_Stop(void)
{
  return (ADC_Stop(&ADC122_RES));
}

/**
 @fn           : int32_t ADC122_PowerControl(ARM_POWER_STATE status)
 @brief        : Control ADC Interface power
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC122_PowerControl(ARM_POWER_STATE status)
{
  return(ADC_PowerControl( &ADC122_RES, status));
}

/**
 @fn           : int32_t ADC122_Control(uint32_t Control, uint32_t arg)
 @brief        : Control ADC Interface
 @parameter[1] : Control : control operation
 @parameter[2] : arg     : Argument for operation
 @return       : execution_status
**/
static int32_t ADC122_Control(uint32_t Control, uint32_t arg)
{
  return (ADC_Control(&ADC122_RES, Control, arg));
}

extern ARM_DRIVER_ADC Driver_ADC122;
ARM_DRIVER_ADC Driver_ADC122 ={
    ADC122_GetVersion,
    ADC122_GetCapabilities,
    ADC122_Initialize,
    ADC122_Uninitialize,
    ADC122_Start,
    ADC122_Stop,
    ADC122_PowerControl,
    ADC122_Control
};
#endif /* RTE_ADC122 */

/* RTE_ADC24 */
#if (RTE_ADC24)

static ADC_RESOURCES ADC24_RES = {
  .cb_event                = NULL,                                   /* ARM_ADC_SignalEvent_t        */
  .regs                    = (ADC_Type *)ADC24_BASE,                 /* ADC register base address    */
  .conv.user_input         = RTE_ADC24_INPUT_NUM,                    /* user input                   */
  .drv_instance            = ADC_INSTANCE_ADC24_0,                   /* Driver instances             */
  .intr_done0_irq_num      = (IRQn_Type) ADC24_DONE0_IRQ_IRQn,       /* ADC DONE0 IRQ number         */
  .intr_done1_irq_num      = (IRQn_Type) ADC24_DONE1_IRQ_IRQn,       /* ADC DONE1 IRQ number         */
  .intr_cmpa_irq_num       = (IRQn_Type) ADC24_CMPA_IRQ_IRQn,        /* ADC CMPA IRQ number          */
  .intr_cmpb_irq_num       = (IRQn_Type) ADC24_CMPB_IRQ_IRQn,        /* ADC CMPB IRQ number          */
  .busy                    = 0,                                      /* ADC busy                     */
  .intr_done0_irq_priority = RTE_ADC24_DONE0_IRQ_PRIORITY,           /* ADC done0 irq priority       */
  .intr_done1_irq_priority = RTE_ADC24_DONE1_IRQ_PRIORITY,           /* ADC done1 irq priority       */
  .intr_cmpa_irq_priority  = RTE_ADC24_CMPA_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .intr_cmpb_irq_priority  = RTE_ADC24_CMPB_IRQ_PRIORITY,            /* ADC cmpa irq priority        */
  .clock_div               = RTE_ADC24_CLOCK_DIV,                    /* clock divisor                */
  .avg_sample_num          = RTE_ADC24_AVG_SAMPLE_NUM,               /* average sample number        */
  .shift_n_bit             = RTE_ADC24_SHIFT_N_BIT,                  /* number of shift bit          */
  .shift_left_or_right     = RTE_ADC24_SHIFT_LEFT_OR_RIGHT,          /* shifting left to right       */
  .pga_enable              = RTE_ADC24_PGA_EN,
  .pga_value               = RTE_ADC24_PGA_GAIN,
  .bias                    = RTE_ADC24_BIAS,
  .output_rate             = RTE_ADC24_OUTPUT_RATE
};

/**
 @fn        : void ADC24_CMPB_IRQHandler (void)
 @brief     : DONE0 (AVG SAMPLE RDY) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC24_DONE0_IRQHandler(void)
{
  conv_info_t *conv = &(ADC24_RES.conv);

  adc_done0_irq_handler(ADC24_RES.regs, conv);

  if (conv->status & ADC_CONV_STAT_COMPLETE)
  {
      /* set busy flag to 0U */
      ADC24_RES.busy = 0U;

      /* clearing conversion complete status */
      conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

      ADC24_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
  }
}

/**
 @fn        : void ADC24_DONE1_IRQHandler (void)
 @brief     : DONE1 (All sample taken) Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC24_DONE1_IRQHandler (void)
{
    conv_info_t *conv = &(ADC24_RES.conv);

    adc_done1_irq_handler(ADC24_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_COMPLETE)
    {
        /* set busy flag to 0U */
        ADC24_RES.busy = 0U;

        /* clearing conversion complete status */
        conv->status = (conv->status & ~ADC_CONV_STAT_COMPLETE);

        ADC24_RES.cb_event(ARM_ADC_EVENT_CONVERSION_COMPLETE, conv->curr_channel, conv->sampled_value);
    }
}

/**
 @fn        : void ADC24_CMPA_IRQHandler (void)
 @brief     : CMPA Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC24_CMPA_IRQHandler (void)
{
    conv_info_t *conv = &(ADC24_RES.conv);

    adc_cmpa_irq_handler(ADC24_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_A);

        ADC24_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_A, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_A)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_A);

        ADC24_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_A, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BETWEEN_A_B);

        ADC24_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B, 0, 0);
    }
}

/**
 @fn        : void ADC24_CMPB_IRQHandler (void)
 @brief     : CMPB Interrupt Handler
 @parameter : NONE
 @return    : NONE
**/
void ADC24_CMPB_IRQHandler (void)
{
    conv_info_t *conv = &(ADC24_RES.conv);

    adc_cmpb_irq_handler(ADC24_RES.regs, conv);

    if (conv->status & ADC_CONV_STAT_CMP_THLD_ABOVE_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_ABOVE_B);

        ADC24_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_B, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_BELOW_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_BELOW_B);

        ADC24_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_BELOW_B, 0, 0);
    }

    if (conv->status & ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B)
    {
        /* clearing comparator status */
        conv->status = (conv->status & ~ADC_CONV_STAT_CMP_THLD_OUTSIDE_A_B);

        ADC24_RES.cb_event(ARM_ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B, 0, 0);
    }
}

/**
 @fn       ARM_DRIVER_VERSION ADC24_GetVersion(void)
 @brief    Get ADC24 VERSION
 @return   DriverVersion
**/
static ARM_DRIVER_VERSION ADC24_GetVersion(void)
{
  return DriverVersion;
}
/**
 @fn       ARM_ADC24_CAPABILITIES ADC24_GetCapabilities(void)
 @brief    Get ADC24 CAPABILITIES
 @return   DriverCapabilities
**/
static ARM_ADC_CAPABILITIES ADC24_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
 @fn           : int32_t ADC24_Initialize(ARM_ADC_SignalEvent_t cb_event)
 @brief        : Initialize the ADC Interface
 @parameter[1] : cb_event : Pointer to \ref ARM_ADC_SignalEvent_t
 @return       : execution_status
**/
static int32_t ADC24_Initialize(ARM_ADC_SignalEvent_t cb_event)
{
  return (ADC_Initialize(&ADC24_RES, cb_event));
}

/**
 @fn           : int32_t ADC24_Uninitialize(void)
 @brief        : Un-Initialize the ADC Interface
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC24_Uninitialize(void)
{
  return (ADC_Uninitialize(&ADC24_RES));
}

/**
 @fn           : int32_t ADC24_Start(void)
 @brief        : start ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC24_Start(void)
{
  return (ADC_Start(&ADC24_RES));
}

/**
 @fn           : int32_t ADC24_Stop(void)
 @brief        : stop ADC driver
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC24_Stop(void)
{
  return (ADC_Stop(&ADC24_RES));
}

/**
 @fn           : int32_t ADC24_PowerControl(ARM_POWER_STATE status)
 @brief        : Control ADC Interface power
 @parameter    : NONE
 @return       : execution_status
**/
static int32_t ADC24_PowerControl(ARM_POWER_STATE status)
{
  return(ADC_PowerControl( &ADC24_RES, status));
}

/**
 @fn           : int32_t ADC24_Control(uint32_t Control, uint32_t arg)
 @brief        : Control ADC Interface
 @parameter[1] : Control : control operation
 @parameter[2] : arg     : Argument for operation
 @return       : execution_status
**/
static int32_t ADC24_Control(uint32_t Control, uint32_t arg)
{
  return (ADC_Control(&ADC24_RES, Control, arg));
}

extern ARM_DRIVER_ADC Driver_ADC24;
ARM_DRIVER_ADC Driver_ADC24 ={
    ADC24_GetVersion,
    ADC24_GetCapabilities,
    ADC24_Initialize,
    ADC24_Uninitialize,
    ADC24_Start,
    ADC24_Stop,
    ADC24_PowerControl,
    ADC24_Control
};
#endif /* RTE_ADC24 */
