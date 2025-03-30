/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     : ADC_Baremetal.c
 * @author   : Prabhakar kumar
 * @email    : prabhakar.kumar@alifsemi.com
 * @version  : V1.0.0
 * @date     : 22-Feb-2022
 * @brief    : Baremetal demo application code for ADC driver
 *              - Input in analog signal corresponding output is digital value.
 *              - Converted digital value are stored in user provided memory
 *                address.
 *                ADC has two conversion mode
 *                1) Single shot conversion
 *                  - single channel scan
 *                2) Continuous conversion
 *                  - single channel scan
 *                  - Multiple channel scan
 *
 *              Hardware setup:
 *              - GPIO port are reserved for ADC12 instance 0 input:
 *                P0_0(channel0)
 *                P0_1(channel1)
 *                P0_2(channel2)
 *                P0_3(channel3)
 *                P0_4(channel4)
 *                P0_5(channel5)
 *              - GPIO port are reserved for ADC12 instance 1 input:
 *                P0_6(channel0)
 *                P0_7(channel1)
 *                P1_0(channel2)
 *                P1_1(channel3)
 *                P1_2(channel4)
 *                P1_3(channel5)
 *              - GPIO port are reserved for ADC12 instance 2 input:
 *                P1_4(channel0)
 *                P1_5(channel1)
 *                P1_6(channel2)
 *                P1_7(channel3)
 *                P2_0(channel4)
 *                P2_1(channel5)
 *
 *              - GPIO port are reserved for ADC24 instance input:
 *                P0_0 (+ve input ) and P0_4 (-ve input) channel 0
 *                P0_1 (+ve input ) and P0_5 (-ve input) channel 1
 *                P0_2 (+ve input ) and P0_6 (-ve input) channel 2
 *                P0_3 (+ve input ) and P0_7 (-ve input) channel 2
 *
 *              Single shot Scan (selective channel scan)
 *              - User can select the particular channel using
 *                 ARM_ADC_CHANNEL_#, where # denotes the channel number.
 *
 *              Continuous Scan
 *              - Single shot scan
 *              - User can select the particular channel using
 *                ARM_ADC_CHANNEL_#, where # denotes the channel number.
 *
 *              - Continuous channel scan
 *                - it rotate through all the ADC channels and
 *                  continuously stores the value in given memory buffer.
 *                  User can skip channels using ARM_ADC_MASK_CHANNEL_# macro.
 *
 *              Comparator
 *              - comparing channels for both the scan for below threshold
 *                " Above / below threshold A
 *                " Above / below threshold B
 *                " Between/outside threshold A and B
 *
 *              ADC configurations for Demo testApp:
 *                Single channel scan(Default scan)
 *                 - GPIO pin P1_4 are connected to Regulated DC Power supply.
 *                    DC Power supply:
 *                     - +ve connected to P1_4 (ADC2 channel 0) at 1.0V
 *                     - -ve connect to GND.
 *
 *                Continuous Channel scan
 *                - Used ADC instance 2,all channels(0-5) are connected to dc supply
 *                - channel 2 and 4 are masked using MASK_CHANNEL macro.
 *                - GND both dc supply channel -ve
 *              Differential input
 *              -ADC12
 *                GPIO pin P1_4 and P1_5 are connected to Regulated DC Power supply.
 *                2 channel DC Power supply:
 *                - +ve connected to P1_4 (ADC122 channel 0) at 1.0V and
 *                  +ve connected to P1_5 (ADC122 channel 1) at 0.4V
 *                - -ve connect to GND.
 *              -ADC24
                  GPIO pin P1_4 and P1_5 are connected to Regulated DC Power supply.
 *                2 channel DC Power supply:
 *                - +ve connected to P0_0 (ADC122 channel 0) at 1.0V and
 *                  +ve connected to P0_4 (ADC122 channel 1) at 0.5V
 *                - -ve connect to GND.
 * @bug      : None.
 * @Note     : None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include "system_utils.h"

/* include for ADC Driver */
#include "Driver_ADC.h"

/* PINMUX include */
#include "pinconf.h"

#include "se_services_port.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* single shot conversion scan use ARM_ADC_SINGLE_SHOT_CH_CONV*/
/* continuous conversion scan use ARM_ADC_CONTINOUS_CH_CONV */

#define ADC_CONVERSION    ARM_ADC_SINGLE_SHOT_CH_CONV
//#define ADC_CONVERSION    ARM_ADC_CONTINOUS_CH_CONV

/* For rotating through one channel use ARM_ADC_FIXED_CHANNEL_SCAN */
/* For continuous rotating through all channel use ARM_ADC_MULTIPLE_CHANNEL_SCAN*/

/* @note : When conversion is selected ARM_ADC_CH_SINGLE_SHOT_SCAN
 *         ADC_SCAN should be in ARM_ADC_SINGLE_CH_SCAN
 * */
#define ADC_SCAN       ARM_ADC_SINGLE_CH_SCAN
//#define ADC_SCAN       ARM_ADC_MULTIPLE_CH_SCAN

/* Macro */
#define ADC12    1
#define ADC24    0

/* For ADC12 use ADC_INSTANCE ADC12  */
/* For ADC24 use ADC_INSTANCE ADC24  */
#define ADC_INSTANCE         ADC12
//#define ADC_INSTANCE         ADC24

#if (ADC_INSTANCE == ADC12)
/* Instance for ADC12 */
extern ARM_DRIVER_ADC Driver_ADC122;
static ARM_DRIVER_ADC *ADCdrv = &Driver_ADC122;
#else
/* Instance for ADC24 */
extern ARM_DRIVER_ADC Driver_ADC24;
static ARM_DRIVER_ADC *ADCdrv = &Driver_ADC24;
#endif

#define COMP_A_THLD_VALUE                   (0X00)                                                /* Comparator A threshold value */
#define COMP_B_THLD_VALUE                   (0x00)                                                /* Comparator B threshold value */
#define MASK_CHANNEL                        (ARM_ADC_MASK_CHANNEL_2 |ARM_ADC_MASK_CHANNEL_4)     /* Masking particular channels  */

#define MAX_NUM_THRESHOLD        (6)
#define NUM_CHANNELS             (8)

/* store comparator result */
uint32_t comp_value[MAX_NUM_THRESHOLD] = {0};

/* Demo purpose adc_sample*/
uint32_t adc_sample[NUM_CHANNELS];

volatile uint32_t num_samples = 0;

/**
 * @fn      static int32_t pinmux_config(void)
 * @brief   ADC pinmux configuration
 * @retval  execution status.
 */
static int32_t pinmux_config(void)
{
    int32_t ret = 0U;

    if (ADC_INSTANCE == ADC12)
    {
        if (ADC_SCAN == ARM_ADC_SINGLE_CH_SCAN)
        {
            /* ADC122 channel 0 */
            ret = pinconf_set(PORT_1, PIN_4, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
        }

        if (ADC_SCAN == ARM_ADC_MULTIPLE_CH_SCAN)
        {
            /* ADC122 channel 0 */
            ret = pinconf_set(PORT_1, PIN_4, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }

            /* ADC122 channel 1 */
            ret = pinconf_set(PORT_1, PIN_5, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC122 channel 2 */
            ret = pinconf_set(PORT_1, PIN_6, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC122 channel 3 */
            ret = pinconf_set(PORT_1, PIN_7, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE );
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC122 channel 4 */
            ret = pinconf_set(PORT_2, PIN_0, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC122 channel 5 */
            ret = pinconf_set(PORT_2, PIN_1, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
        }
    }

    if (ADC_INSTANCE == ADC24)
    {

        if (ADC_SCAN == ARM_ADC_SINGLE_CH_SCAN)
        {
            /* ADC24 channel 0 */
            ret = pinconf_set(PORT_0, PIN_0, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC24 channel 0 */
            ret = pinconf_set(PORT_0, PIN_4, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
        }

        if (ADC_SCAN == ARM_ADC_MULTIPLE_CH_SCAN)
        {
            /* ADC24 channel 0 */
            ret = pinconf_set(PORT_0, PIN_0, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC24 channel 0 */
            ret = pinconf_set(PORT_0, PIN_4, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }

            /* ADC24 channel 1 */
            ret = pinconf_set(PORT_0, PIN_1, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC24 channel 1 */
            ret = pinconf_set(PORT_0, PIN_5, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }

            /* ADC24 channel 2 */
            ret = pinconf_set(PORT_0, PIN_2, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC24 channel 2 */
            ret = pinconf_set(PORT_0, PIN_6, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC24 channel 1 */
            ret = pinconf_set(PORT_0, PIN_3, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
            /* ADC24 channel 1 */
            ret = pinconf_set(PORT_0, PIN_7, PINMUX_ALTERNATE_FUNCTION_7,
                              PADCTRL_READ_ENABLE);
            if(ret)
            {
                printf("ERROR: Failed to configure PINMUX \r\n");
                return ret;
            }
        }
    }

    return ret;
}

/*
 * @func   : void adc_conversion_callback(uint32_t event, uint8_t channel, uint32_t sample_output)
 * @brief  : adc conversion isr callback
 * @return : NONE
*/
static void adc_conversion_callback(uint32_t event, uint8_t channel, uint32_t sample_output)
{
    if (event & ARM_ADC_EVENT_CONVERSION_COMPLETE)
    {
        num_samples += 1;

        /* Store the value for the respected channels */
        adc_sample[channel] = sample_output;
    }
    if (event & ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_A)
    {
        comp_value[0] += 1;
    }
    if (event & ARM_ADC_COMPARATOR_THRESHOLD_ABOVE_B)
    {
        comp_value[1] += 1;
    }
    if (event & ARM_ADC_COMPARATOR_THRESHOLD_BELOW_A)
    {
        comp_value[2] += 1;
    }
    if (event & ARM_ADC_COMPARATOR_THRESHOLD_BELOW_B)
    {
        comp_value[3] += 1;
    }
    if(event & ARM_ADC_COMPARATOR_THRESHOLD_BETWEEN_A_B)
    {
        comp_value[4] += 1;
    }
    if(event & ARM_ADC_COMPARATOR_THRESHOLD_OUTSIDE_A_B)
    {
        comp_value[5] += 1;
    }
}

/**
 *    @func         : void ADC_demo()
 *    @brief        : ADC demo
 *                  - test to verify the adc.
 *                  - input analog signal corresponding convert into digital value
 *                  - converted value is the allocated user memory address.
 *    @return       : NONE
*/
void ADC_demo()
{
    int32_t  ret               = 0;
    uint32_t error_code        = SERVICES_REQ_SUCCESS;
    uint32_t service_error_code;
    ARM_DRIVER_VERSION    version;

    /* Initialize the SE services */
    se_services_port_init();

/* enable the HFOSC clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_CLK_160M,
                           /*bool enable   */ true,
                                              &service_error_code);
    if(error_code)
        printf("SE: clk enable = %d\n", error_code);

    printf("\r\n >>> ADC demo starting up!!! <<< \r\n");

    version = ADCdrv->GetVersion();
    printf("\r\n ADC version api:%X driver:%X...\r\n",version.api, version.drv);

    /* PINMUX */
    ret = pinmux_config();
    if(ret != 0)
    {
        printf("Error in pin-mux configuration\n");
        return;
    }

    /* Initialize ADC driver */
    ret = ADCdrv->Initialize(adc_conversion_callback);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC init failed\n");
        return;
    }

    /* Power control ADC */
    ret = ADCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Power up failed\n");
        goto error_uninitialize;
    }

#if (ADC_CONVERSION == ARM_ADC_SINGLE_SHOT_CH_CONV)

    /* set conversion mode */
    ret = ADCdrv->Control(ARM_ADC_CONVERSION_MODE_CTRL, ADC_CONVERSION);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Comparator failed\n");
        goto error_poweroff;
    }

    /* set initial channel */
    ret = ADCdrv->Control(ARM_ADC_CHANNEL_INIT_VAL, ARM_ADC_CHANNEL_0);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC channel failed\n");
        goto error_poweroff;
    }
#endif

#if (ADC_CONVERSION == ARM_ADC_CONTINOUS_CH_CONV)

    /* set conversion mode */
    ret = ADCdrv->Control(ARM_ADC_CONVERSION_MODE_CTRL, ADC_CONVERSION);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC setting conversion mode failed\n");
        goto error_poweroff;
    }

    if (ADC_SCAN == ARM_ADC_SINGLE_CH_SCAN)
    {
        /* set channel */
        ret = ADCdrv->Control(ARM_ADC_CHANNEL_INIT_VAL, ARM_ADC_CHANNEL_0);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: ADC setting channel failed\n");
            goto error_poweroff;
        }
    }
    else /* Multiple channel scan */
    {
        /* set sequencer controller */
        ret = ADCdrv->Control(ARM_ADC_SEQUENCER_CTRL, ARM_ADC_MULTIPLE_CH_SCAN);
        if(ret != ARM_DRIVER_OK){
           printf("\r\n Error: ADC sequencer controller failed\n");
           goto error_poweroff;
        }

        /* set channel */
        ret = ADCdrv->Control(ARM_ADC_CHANNEL_INIT_VAL, ARM_ADC_CHANNEL_0);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: ADC setting channel failed\n");
            goto error_poweroff;
        }

        /* Masking the channel */
        ret = ADCdrv->Control(ARM_ADC_SEQUENCER_MSK_CH_CTRL, MASK_CHANNEL);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: ADC sequencer masking channel failed\n");
            goto error_poweroff;
        }
    }
#endif

    /* set comparator a value */
    ret = ADCdrv->Control(ARM_ADC_COMPARATOR_A, COMP_A_THLD_VALUE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC set Comparator A threshold failed\n");
        goto error_poweroff;
    }

    /* set comparator b value */
    ret = ADCdrv->Control(ARM_ADC_COMPARATOR_B, COMP_B_THLD_VALUE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC set comparator B  threshold failed\n");
        goto error_poweroff;
    }

    /* select the threshold comparison */
    ret = ADCdrv->Control(ARM_ADC_THRESHOLD_COMPARISON, ARM_ADC_ABOVE_A_AND_ABOVE_B);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Threshold comparison failed\n");
        goto error_poweroff;
    }

    printf(">>> Allocated memory buffer Address is 0x%X <<<\n",(uint32_t)adc_sample);
    /* Start ADC */
    ret = ADCdrv->Start();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Start failed\n");
        goto error_poweroff;
    }

    /* wait for timeout */
    if (ADC_CONVERSION == ARM_ADC_CONTINOUS_CH_CONV)
    {
        while(num_samples < 1000);
    }
    else
    {
        /* single shot conversion */
        while(num_samples < 1);
    }

    /* Stop ADC */
    ret = ADCdrv->Stop();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Stop failed\n");
        goto error_poweroff;
    }

    printf("\n >>> ADC conversion completed \n");
    printf(" Converted value are stored in user allocated memory address.\n");
    printf("\n ---END--- \r\n wait forever >>> \n");
    while(1);

error_poweroff:

    /* Power off ADC peripheral */
    ret = ADCdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: ADC Power OFF failed.\r\n");
    }
    /* disable the HFOSC clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_CLK_160M,
                           /*bool enable   */ false,
                                              &service_error_code);
    if(error_code)
        printf("SE: clk enable = %d\n", error_code);

error_uninitialize:

    /* Un-initialize ADC driver */
    ret = ADCdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: ADC Uninitialize failed.\r\n");
    }

    printf("\r\n ADC demo exiting...\r\n");
}

/* Define main entry point.  */
int main()
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif
    /* Enter the demo Application.  */
    ADC_demo();
    return 0;
}
