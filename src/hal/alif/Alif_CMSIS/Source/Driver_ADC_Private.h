/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef DRIVER_ADC_PRIVATE_H_
#define DRIVER_ADC_PRIVATE_H_

/*---System include ----*/
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_ADC.h"
#include "adc.h"
#include "sys_ctrl_adc.h"

typedef enum {
    ADC_FLAG_DRV_INIT_DONE    = (1U << 0),  /* ADC Driver is Initialized */
    ADC_FLAG_DRV_POWER_DONE   = (1U << 1),  /* ADC Driver is Powered     */
} ADC_FLAG_Type;

/* Access structure for the saving the ADC Setting and status*/
typedef struct _ADC_RESOURCES
{
    ARM_ADC_SignalEvent_t   cb_event;                  /* ADC APPLICATION CALLBACK EVENT                       */
    ADC_Type                *regs;                     /* ADC register base address                            */
    conv_info_t             conv;                      /* ADC conversion information                           */
    ADC_INSTANCE            drv_instance;              /* ADC Driver instances                                 */
    IRQn_Type               intr_done0_irq_num;        /* ADC avg sample ready interrupt number                */
    IRQn_Type               intr_done1_irq_num;        /* ADC all sample taken interrupt number                */
    IRQn_Type               intr_cmpa_irq_num;         /* ADC comparator A interrupt number                    */
    IRQn_Type               intr_cmpb_irq_num;         /* ADC comparator B interrupt number                    */
    uint8_t                 ext_trig_val;              /* ADC external trigger enable value                    */
    uint8_t                 busy;                      /* ADC conversion busy flag                             */
    uint32_t                intr_done0_irq_priority;   /* ADC done0 Irq Priority                               */
    uint32_t                intr_done1_irq_priority;   /* ADC done1 Irq Priority                               */
    uint32_t                intr_cmpa_irq_priority;    /* ADC cmpa Irq Priority                                */
    uint32_t                intr_cmpb_irq_priority;    /* ADC cmpb Irq Priority                                */
    uint32_t                state;                     /* ADC state                                            */
    uint32_t                clock_div;                 /* ADC clock divisor                                    */
    uint32_t                avg_sample_num;            /* ADC average sample number                            */
    uint32_t                sample_width;              /* ADC sample width                                     */
    uint32_t                shift_n_bit;               /* ADC number of bits to shift                          */
    uint32_t                shift_left_or_right;       /* ADC shift bit left or right                          */
    bool                    differential_enable;       /* ADC12 differential enable                            */
    bool                    comparator_enable;         /* ADC12 comparator enable                              */
    uint8_t                 comparator_bias;           /* ADC12 comparator bias                                */
    uint32_t                pga_enable;                /* ADC Programmable gain amplifier(PGA) enable          */
    uint32_t                pga_value;                 /* ADC Programmable gain amplifier(PGA)                 */
    uint32_t                bias;                      /* ADC24 bias control value                             */
    uint32_t                output_rate;               /* ADC24 output rate                                    */
}ADC_RESOURCES;

#endif /* DRIVER_ADC_PRIVATE_H_ */
