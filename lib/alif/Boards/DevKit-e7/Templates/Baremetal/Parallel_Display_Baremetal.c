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
 * @file     : Parallel_Display_Test_Baremetal.c
 * @author   : Chandra Bhushan Singh
 * @email    : chandrabhushan.singh@alifsemi.com
 * @version  : V1.0.0
 * @date     : 19-June-2023
 * @brief    : Baremetal demo application code for parallel display
 * @bug      : None.
 * @Note     : None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include <string.h>

#include <RTE_Components.h>
#include CMSIS_device_header

/* include the CDC200 driver */
#include "Driver_CDC200.h"

/* PINMUX Driver */
#include "pinconf.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


#define DIMAGE_X       (RTE_PANEL_HACTIVE_TIME)
#define DIMAGE_Y       (RTE_PANEL_VACTIVE_LINE)

#if RTE_CDC200_PIXEL_FORMAT == 0
/* ARGB8888 32-bit Format (4-bytes) */
#define PIXEL_BYTES    (4)

#elif RTE_CDC200_PIXEL_FORMAT == 1
/* RGB888 24-bit Format (3-bytes) */
#define PIXEL_BYTES    (3)

#elif RTE_CDC200_PIXEL_FORMAT == 2
/* RGB565  16-bit Format (2-bytes) */
#define PIXEL_BYTES    (2)

#elif RTE_CDC200_PIXEL_FORMAT == 3
/* RGBA8888 32-bit Format (3-bytes) */
#define PIXEL_BYTES    (4)

#elif (RTE_CDC200_PIXEL_FORMAT == 6)
/* ARGB1555 16-bit Format (2-bytes) */
#define PIXEL_BYTES    (2)

#elif (RTE_CDC200_PIXEL_FORMAT == 7)
/* ARGB4444 16-bit Format (2-bytes) */
#define PIXEL_BYTES    (2)
#endif

static uint8_t lcd_image[DIMAGE_Y][DIMAGE_X][PIXEL_BYTES] __attribute__((section(".bss.lcd_frame_buf")));

/* CDC200 driver instance */
extern ARM_DRIVER_CDC200 Driver_CDC200;
static ARM_DRIVER_CDC200 *CDCdrv = &Driver_CDC200;

/**
  *    @func    : void display_callback()
  *    @brief   : Parallel display demo callback
  *                 - normally is not called
  *    @return  : NONE
  */
static void display_callback(uint32_t event)
{
    if(event & ARM_CDC_DSI_ERROR_EVENT)
    {
        /* Transfer Error: Received Hardware error */
        while(1);
    }
}

/**
  \fn          int cdc200_pinmux(void)
  \brief       cdc hardware pin initialization:
                   - PIN-MUX configuration
  \param[in]   none
  \return      0:success; -1:failure
  */
int cdc200_pinmux(void)
{
    int ret;

    /* Configure Pin : P5_3 as cdc_pclk_a */
    ret = pinconf_set (PORT_5, PIN_3, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P5_4 as cdc_de_a */
    ret = pinconf_set (PORT_5, PIN_4, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P4_1 as cdc_hsync_b */
    ret = pinconf_set (PORT_4, PIN_1, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P4_0 as cdc_vsync_b */
    ret = pinconf_set (PORT_4, PIN_0, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_0 as cdc_d0_a */
    ret = pinconf_set (PORT_8, PIN_0, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_1 as cdc_d1_a */
    ret = pinconf_set (PORT_8, PIN_1, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_2 as cdc_d2_a */
    ret = pinconf_set (PORT_8, PIN_2, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_3 as cdc_d3_a */
    ret = pinconf_set (PORT_8, PIN_3, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_4 as cdc_d4_a */
    ret = pinconf_set (PORT_8, PIN_4, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_5 as cdc_d5_a */
    ret = pinconf_set (PORT_8, PIN_5, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_6 as cdc_d6_a */
    ret = pinconf_set (PORT_8, PIN_6, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P8_7 as cdc_d7_a */
    ret = pinconf_set (PORT_8, PIN_7, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
         printf("\r\n Error: cdc200 PINMUX failed.\r\n");
         return -1;
    }

    /* Configure Pin : P9_0 as cdc_d8_a */
    ret = pinconf_set (PORT_9, PIN_0, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P9_1 as cdc_d9_a */
    ret = pinconf_set (PORT_9, PIN_1, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P9_2 as cdc_d10_a */
    ret = pinconf_set (PORT_9, PIN_2, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P9_3 as cdc_d11_a */
    ret = pinconf_set (PORT_9, PIN_3, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P9_4 as cdc_d12_a */
    ret = pinconf_set (PORT_9, PIN_4, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P12_5 as cdc_d13_b */
    ret = pinconf_set (PORT_9, PIN_5, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P12_6 as cdc_d14_b */
    ret = pinconf_set (PORT_9, PIN_6, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P12_7 as cdc_d15_b */
    ret = pinconf_set (PORT_9, PIN_7, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_0 as cdc_d16_b */
    ret = pinconf_set (PORT_13, PIN_0, PINMUX_ALTERNATE_FUNCTION_6, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_1 as cdc_d17_b */
    ret = pinconf_set (PORT_13, PIN_1, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_2 as cdc_d18_b */
    ret = pinconf_set (PORT_13, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_3 as cdc_d19_b */
    ret = pinconf_set (PORT_13, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_4 as cdc_d20_b */
    ret = pinconf_set (PORT_13, PIN_4, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_5 as cdc_d21_b */
    ret = pinconf_set (PORT_13, PIN_5, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_6 as cdc_d22_b */
    ret = pinconf_set (PORT_13, PIN_6, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    /* Configure Pin : P13_7 as cdc_d23_b */
    ret = pinconf_set (PORT_13, PIN_7, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: cdc200 PINMUX failed.\r\n");
        return -1;
    }

    return 0;
}

/**
  *    @func         : void Paralle_Display_Demo()
  *    @brief        : Parallel display demo
  *                  - initialize the CDC200 controller
  *                  - initialize the LCD panel
  *                  - write black and white to the memory address.
  *    @return       : NONE
  */
static void Parallel_Display_Demo()
{
    int32_t ret;
    ARM_DRIVER_VERSION version;

    /* Hardware initialization for CDC */
    ret = cdc200_pinmux();
    if(ret != 0)
    {
        printf("\r\n Error: CDC200 Hardware Initialize failed.\r\n");
        return;
    }

    printf("\r\n >>> CDC demo starting up!!! <<< \r\n");

    version = CDCdrv->GetVersion();
    printf("\r\n CDC version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize CDC controller */
    ret = CDCdrv->Initialize(display_callback);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC init failed\n");
        return;
    }

    /* Power ON CDC controller */
    ret = CDCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: CDC Power up failed\n");
        goto error_uninitialize;
    }

    /* configure CDC controller */
    ret = CDCdrv->Control(CDC200_CONFIGURE_DISPLAY, (uint32_t)lcd_image);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC controller configuration failed\n");
        goto error_uninitialize;
    }

    printf(">>> Allocated memory buffer Address is 0x%X <<<\n",(uint32_t)lcd_image);

    /* Start CDC controller */
    ret = CDCdrv->Start();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Start failed\n");
        goto error_poweroff;
    }

    while(1)
    {
        memset(lcd_image, 0x00, sizeof(lcd_image));
        for(uint32_t count = 0; count < 20; count++)
            sys_busy_loop_us(100000);

        memset(lcd_image, 0xFF, sizeof(lcd_image));
        for(uint32_t count = 0; count < 20; count++)
            sys_busy_loop_us(100000);

    }

    /* Stop CDC controller */
    ret = CDCdrv->Stop();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Stop failed\n");
        goto error_poweroff;
    }

error_poweroff:
    /* Power off CDC controller */
    ret = CDCdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Power OFF failed.\r\n");
    }

error_uninitialize:
    /* Un-initialize CDC controller */
    ret = CDCdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Uninitialize failed.\r\n");
    }

    printf("\r\n XXX CDC demo exiting XXX...\r\n");
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
    Parallel_Display_Demo();
    return 0;
}
