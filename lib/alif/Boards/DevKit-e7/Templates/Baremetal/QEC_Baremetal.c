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
 * @file     QEC_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     21-July-2023
 * @brief    Baremetal demo application for QEC0 channel.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include "Driver_UTIMER.h"
#include "Driver_GPIO.h"
#include "pinconf.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* GPIO related definitions */
#define GPIO1                          1
#define GPIO1_PIN0                     0
#define GPIO1_PIN1                     1
#define GPIO1_PIN2                     2

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

/* GPIO1 Driver instance */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO1);
ARM_DRIVER_GPIO *ptrGPIO = &ARM_Driver_GPIO_(GPIO1);

/**
 * @function    int pinmux_config(void)
 * @brief       QEC h/w pin init using pinmux driver
 * @note        none
 * @param       none
 * @retval      execution status
 */
static int32_t pinmux_config(void)
{
    int32_t ret;

    ret = pinconf_set (PORT_8, PIN_4, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = pinconf_set (PORT_8, PIN_5, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = pinconf_set (PORT_8, PIN_6, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    return 0;
}

/**
 * @function    int gpio_init(void)
 * @brief       GPIO initialization using gpio driver
 * @note        none
 * @param       none
 * @retval      execution status
 */
static int32_t gpio_init(void)
{
    int32_t ret = ARM_DRIVER_OK;

    /* init P1_0 as GPIO */
    ret = pinconf_set (PORT_1, PIN_0, PINMUX_ALTERNATE_FUNCTION_0, 0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = ptrGPIO->Initialize (GPIO1_PIN0, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to initialize GPIO1_PIN0 as GPIO\n");
        return -1;
    }

    ret = ptrGPIO->PowerControl (GPIO1_PIN0, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to Power up GPIO1_PIN0\n");
        return -1;
    }

    ret = ptrGPIO->SetDirection (GPIO1_PIN0, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set direction for GPIO1_PIN0\n");
        return -1;
    }

    ret = ptrGPIO->SetValue (GPIO1_PIN0, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN0\n");
        return -1;
    }

    /* init P1_1 as GPIO */
    ret = pinconf_set (PORT_1, PIN_1, PINMUX_ALTERNATE_FUNCTION_0, 0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = ptrGPIO->Initialize (GPIO1_PIN1, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to initialize GPIO1_PIN1 as GPIO\n");
        return -1;
    }

    ret = ptrGPIO->PowerControl (GPIO1_PIN1, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to Power up GPIO1_PIN1\n");
        return -1;
    }

    ret = ptrGPIO->SetDirection (GPIO1_PIN1, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set direction for GPIO1_PIN1\n");
        return -1;
    }

    ret = ptrGPIO->SetValue (GPIO1_PIN1, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN1\n");
        return -1;
    }

    /* init P1_2 as GPIO */
    ret = pinconf_set (PORT_1, PIN_2, PINMUX_ALTERNATE_FUNCTION_0, 0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = ptrGPIO->Initialize (GPIO1_PIN2, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to initialize GPIO1_PIN2 as GPIO\n");
        return -1;
    }

    ret = ptrGPIO->PowerControl (GPIO1_PIN2, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to Power up GPIO1_PIN2\n");
        return -1;
    }

    ret = ptrGPIO->SetDirection (GPIO1_PIN2, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set direction for GPIO1_PIN2\n");
        return -1;
    }

    ret = ptrGPIO->SetValue (GPIO1_PIN2, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN2\n");
        return -1;
    }

    return 0;
}

/**
 * @function    void qec0_app(void)
 * @brief       QEC0 demo application
 * @note        none
 * @param       none
 * @retval      none
 */
static void qec0_app(void)
{
    int32_t ret;
    uint8_t channel = 12;
    uint32_t init_count = 0;

    ARM_UTIMER_TRIGGER_CONFIG upcount_trig = {
        .triggerTarget = ARM_UTIMER_TRIGGER_UPCOUNT,
        .triggerSrc = ARM_UTIMER_SRC_0,
        .trigger = ARM_UTIMER_SRC0_TRIG0_RISING
    };

    ARM_UTIMER_TRIGGER_CONFIG downcount_trig = {
        .triggerTarget = ARM_UTIMER_TRIGGER_DOWNCOUNT,
        .triggerSrc = ARM_UTIMER_SRC_0,
        .trigger = ARM_UTIMER_SRC0_TRIG1_RISING
    };

    ARM_UTIMER_TRIGGER_CONFIG clear_trig = {
        .triggerTarget = ARM_UTIMER_TRIGGER_CAPTURE_A,
        .triggerSrc = ARM_UTIMER_SRC_0,
        .trigger = ARM_UTIMER_SRC0_TRIG2_RISING
    };

    /*
     * utimer channel 12 is configured as QEC0.
     * For testing purpose, QEC inputs are connected to gpio's. And GPIO's are toggled accordingly.
     *
     * H/W connection : short P1_0 and P8_4, short P1_1 and P8_5, short P1_2 and P8_6.
     **/

    printf("*** QEC demo application started ***\n");

    ret = pinmux_config();
    if (ret) {
        printf("pinmux failed\n");
    }

    ret = gpio_init();
    if (ret) {
        printf("gpio init failed\n");
    }

    ret = ptrUTIMER->Initialize (channel, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_qec_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter (channel, ARM_UTIMER_MODE_TRIGGERING, ARM_UTIMER_COUNTER_TRIANGLE);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_qec_poweroff;
    }

    ret = ptrUTIMER->SetCount (channel, ARM_UTIMER_CNTR, init_count);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_qec_poweroff;
    }

    ret = ptrUTIMER->ConfigTrigger (channel, &upcount_trig);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("utimer channel %d triggered for up count using Trig0\n", channel);
    }

    ret = ptrUTIMER->ConfigTrigger (channel, &downcount_trig);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("utimer channel %d triggered for down count using Trig1\n", channel);
    }

    ret = ptrUTIMER->ConfigTrigger (channel, &clear_trig);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("utimer channel %d triggered for counter clear using Trig3\n", channel);
    }

    /* Toggling gpio's connected to x for 20 times to increment cnt value for 10 times */
    for(int i=0; i<20; i++)
    {
        ret = ptrGPIO->SetValue (GPIO1_PIN0, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO1_PIN0\n");
        }
    }

    printf("counter value after counter increment : %d\n", ptrUTIMER->GetCount (channel, ARM_UTIMER_CNTR));

    /* Toggling gpio's connected to x for 10 times to increment cnt value for 5 times */
    for(int i=0; i<10; i++)
    {
        ret = ptrGPIO->SetValue (GPIO1_PIN1, GPIO_PIN_OUTPUT_STATE_TOGGLE);
       if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO1_PIN1\n");
        }
    }

    printf("counter value after counter decrement: %d\n", ptrUTIMER->GetCount (channel, ARM_UTIMER_CNTR));

    /* Making z event as high to clear count value */
    ret = ptrGPIO->SetValue (GPIO1_PIN2, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN2\n");
    }

    printf("counter value after counter clear: %d\n", ptrUTIMER->GetCount (channel, ARM_UTIMER_CNTR));

    ret = ptrUTIMER->Stop (channel, ARM_UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_qec_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_qec_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: QEC completed *** \r\n\n");
}

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
    qec0_app();
}
