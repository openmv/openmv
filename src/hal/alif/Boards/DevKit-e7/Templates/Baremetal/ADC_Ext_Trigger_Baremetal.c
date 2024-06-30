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
 * @file     : ADC_Ext_Trigger_Baremetal.c
 * @author   : Prabhakar kumar
 * @email    : prabhakar.kumar@alifsemi.com
 * @version  : V1.0.0
 * @date     : 04-Sept-2023
 * @brief    : Baremetal demo application code for testing the external trigger
 *             feature for the ADC12 & ADC24
 *              - Generating the pulse from the Utimer channel 0 Driver A for starting
 *                the ADC conversion.
 *              - Input in analog signal corresponding output is digital value.
 *              - Converted digital value are stored in user provided memory
 *                address.
 *              ADC configurations for Demo testApp:
 *              Single channel scan(Default scan ADC12)
 *              - GPIO pin P1_4 are connected to Regulated DC Power supply.
 *                DC Power supply:
 *                - +ve connected to P1_4 (ADC2 channel 0) at 1.0V
 *                - -ve connect to GND.
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
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <stdio.h>
/* Project include */
#include "Driver_UTIMER.h"
#include "Driver_ADC.h"
#include "pinconf.h"

#include "se_services_port.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

static volatile uint32_t cb_compare_a_status = 0;

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

/* Macro for ADC12 and ADC24 */
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

#define NUM_CHANNELS          8
#define NUM_TEST_SAMPLES      3
#define NUM_PULSE_GENERATE    3

/* Demo purpose adc_sample*/
uint32_t adc_sample[NUM_CHANNELS];

volatile uint32_t num_samples = 0;

/**
 * @fn      static int32_t pinmux_config(void)
 * @brief   ADC external trigger pinmux configuration
 * @retval  execution status.
 */
static int32_t pinmux_config(void)
{
    int32_t ret = 0U;

    if (ADC_INSTANCE == ADC12)
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

    if (ADC_INSTANCE == ADC24)
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
}

/**
 * @function    void utimer_compare_mode_cb_func(event)
 * @brief       utimer compare mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_compare_mode_cb_func(uint8_t event)
{
    if (event == ARM_UTIMER_EVENT_COMPARE_A) {
        cb_compare_a_status++;
    }
}

/**
 * @function    void utimer_compare_mode_app(void)
 * @brief       utimer compare mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_compare_mode_app(void)
{
    int32_t ret;
    uint8_t channel = 0;
    uint32_t count_array[3];

    /*
     * utimer channel 0 is configured for utimer compare mode (driver A).
     */
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 1 sec = 1/(0.0025*(10^-6)) = 400000000
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 500ms = (500*(10^-3)/(0.0025*(10^-6)) = 200000000
     * DEC = 200000000
     * HEX = 0xBEBC200
     */
    count_array[0] =  0x000000000;       /*< initial counter value >*/
    count_array[1] =  0x17D78400;        /*< over flow count value >*/
    count_array[2] =  0xBEBC200;         /*< compare a/b value>*/

    ret = ptrUTIMER->Initialize (channel, utimer_compare_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_compare_mode_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter (channel, ARM_UTIMER_MODE_COMPARING, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount (channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount (channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount (channel, ARM_UTIMER_COMPARE_A, count_array[2]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->Start(channel);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_compare_mode_poweroff;
    }

    for (int index = 0; index < NUM_PULSE_GENERATE; index++)
    {
        while(1)
        {
            if (cb_compare_a_status) {
                cb_compare_a_status = 0;
                break;
            }
        }
    }

    ret = ptrUTIMER->Stop (channel, ARM_UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    }

error_compare_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_compare_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }
}

void adc_ext_trigger_demo()
{
    int32_t ret                = 0;
    uint32_t error_code        = SERVICES_REQ_SUCCESS;
    uint32_t service_error_code;
    ARM_DRIVER_VERSION version;

    /* Initialize the SE services */
    se_services_port_init();

    /* enable the 160 MHz clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_CLK_160M,
                           /*bool enable   */ true,
                                              &service_error_code);
    if(error_code)
    {
        printf("SE Error: 160 MHz clk enable = %d\n", error_code);
        return;
    }

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
    if (ret != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC init failed\n");
        return;
    }

    /* Power control ADC */
    ret = ADCdrv->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC Power up failed\n");
        goto error_uninitialize;
    }

    /* set initial channel */
    ret = ADCdrv->Control(ARM_ADC_CHANNEL_INIT_VAL, ARM_ADC_CHANNEL_0);
    if (ret != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC channel failed\n");
        goto error_poweroff;
    }

    printf(">>> Allocated memory buffer Address is 0x%X <<<\n",(uint32_t)adc_sample);

    /* Enable ADC from External trigger pulse */
    ret = ADCdrv->Control(ARM_ADC_EXTERNAL_TRIGGER_ENABLE, ARM_ADC_EXTERNAL_TRIGGER_SRC_0);
    if (ret != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC External trigger enable failed\n");
        goto error_poweroff;
    }

    /* Start ADC */
    ret = ADCdrv->Start();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Start failed\n");
        goto error_poweroff;
    }

    /* Generating pulse from Utimer */
    utimer_compare_mode_app();

    while (num_samples != NUM_TEST_SAMPLES);

    /* Disable ADC external trigger conversion */
    ret = ADCdrv->Control(ARM_ADC_EXTERNAL_TRIGGER_DISABLE, ARM_ADC_EXTERNAL_TRIGGER_SRC_0);
    if (ret != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC External trigger disable failed\n");
        goto error_poweroff;
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
    if (ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: ADC Power OFF failed.\r\n");
    }

error_uninitialize:
    /* Un-initialize ADC driver */
    ret = ADCdrv->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: ADC Uninitialize failed.\r\n");
    }
    /* disable the 160MHz clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_CLK_160M,
                           /*bool enable   */ false,
                                              &service_error_code);
    if(error_code)
    {
        printf("SE Error: 160 MHz clk disable = %d\n", error_code);
        return;
    }

    printf("\r\n ADC demo exiting...\r\n");
}

/* Define main entry point.  */
int main()
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if (ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    /* Enter the demo Application.  */
    adc_ext_trigger_demo();
    return 0;
}
