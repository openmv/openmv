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
 * @file     : cdc_testApp.c
 * @author   : Chandra Bhushan Singh
 * @email    : chandrabhushan.singh@alifsemi.com
 * @version  : V1.0.0
 * @date     : 16-Oct-2023
 * @brief    : FreeRTOS demo application code for CDC200 driver
 *              - Fills memory buffer with colors to test LCD panels
 *
 *              Hardware setup:
 *              - Must use either Focus LCDs E50RA-I-MW550-N or E43GB-I-MW405-C
 *              - GPIO port P6_1 is used to enable/disable boost converter for LED backlight
 *              - GPIO port P15_5 is used to drive LCD panel reset
 * @bug      : None.
 * @Note     : None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include <string.h>

#include "RTE_Device.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/* include the CDC200 driver */
#include "Driver_CDC200.h"

/* SE Services */
#include "se_services_port.h"

/*RTOS Includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

extern ARM_DRIVER_CDC200 Driver_CDC200;
static ARM_DRIVER_CDC200 *CDCdrv = &Driver_CDC200;

/*Define for FreeRTOS*/
#define STACK_SIZE                      1024
#define TIMER_SERVICE_TASK_STACK_SIZE   configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE            configMINIMAL_STACK_SIZE

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t display_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &IdleTcb;
    *ppxIdleTaskStackBuffer = IdleStack;
    *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pxTask;

    for (;;);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &TimerTcb;
    *ppxTimerTaskStackBuffer = TimerStack;
    *pulTimerTaskStackSize = TIMER_SERVICE_TASK_STACK_SIZE;
}

void vApplicationIdleHook(void)
{
    for (;;);
}

/*****************Only for FreeRTOS use *************************/

#define DIMAGE_X       (RTE_PANEL_HACTIVE_TIME)
#define DIMAGE_Y       (RTE_PANEL_VACTIVE_LINE)

#if (RTE_CDC200_PIXEL_FORMAT == 0)
/* ARGB8888 32-bit Format (4-bytes) */
#define PIXEL_BYTES    (4)
#elif (RTE_CDC200_PIXEL_FORMAT == 1)
/* RGB888 24-bit Format (3-bytes) */
#define PIXEL_BYTES    (3)
#elif (RTE_CDC200_PIXEL_FORMAT == 2)
/* RGB565  16-bit Format (2-bytes) */
#define PIXEL_BYTES    (2)
#elif (RTE_CDC200_PIXEL_FORMAT == 3)
/* RGBA8888 32-bit Format (3-bytes) */
#define PIXEL_BYTES    (4)
#endif

static uint8_t lcd_image[DIMAGE_Y][DIMAGE_X][PIXEL_BYTES] __attribute__((section(".bss.lcd_frame_buf")));

/**
 *    @func         : void display_callback(uint32_t event)
 *    @brief        : CDC demo callback
 *                  - normally is not called
 *    \param[in]    : event: Display Event
 *    @return       : NONE
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
 *    @func         : void cdc_demo_thread_entry(void *pvParameters)
 *    @brief        : CDC demo using FreeRTOS operating system.
 *                  - initialize the CDC200 controller
 *                  - write various colors to the memory address.
 *    @param        : pvParameters.
 *    @return       : NONE
*/
void cdc_demo_thread_entry(void *pvParameters)
{
    int32_t ret            = 0;
    uint32_t service_error_code;
    uint32_t error_code;
    run_profile_t runp = {0};

    ARM_DRIVER_VERSION version;

    printf("\r\n >>> CDC demo with FreeRTOS is starting up!!! <<< \r\n");

    /* Initialize the SE services */
    se_services_port_init();

    /* Enable MIPI Clocks */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, true, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 100MHz clock enable = %d\n", error_code);
        return;
    }

    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, true, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 38.4Mhz(HFOSC) clock enable = %d\n", error_code);
        goto error_disable_100mhz_clk;
    }

    /* Get the current run configuration from SE */
    error_code = SERVICES_get_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        printf("\r\nSE: get_run_cfg error = %d\n", error_code);
        goto error_disable_hfosc_clk;
    }

    /*
     * Note:
     * This demo uses a specific profile setting that only enables the
     * items it needs. For example, it only requests the RAM regions and
     * peripheral power that are relevant for this demo. If you want to adapt
     * this example for your own use case, you should adjust the profile setting
     * accordingly. You can either add any additional items that you need, or
     * remove the request altogether to use the default setting that turns on
     * almost everything.
     */

    runp.memory_blocks = MRAM_MASK | SRAM0_MASK;

    runp.phy_pwr_gating = MIPI_PLL_DPHY_MASK | MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK | LDO_PHY_MASK;

    /* Set the new run configuration */
    error_code = SERVICES_set_run_cfg(se_services_s_handle,
                                      &runp,
                                      &service_error_code);
    if(error_code)
    {
        printf("\r\nSE: set_run_cfg error = %d\n", error_code);
        goto error_disable_hfosc_clk;
    }

    version = CDCdrv->GetVersion();
    printf("\r\n CDC version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize CDC driver */
    ret = CDCdrv->Initialize(display_callback);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC init failed\n");
        goto error_disable_hfosc_clk;
    }

    /* Power control CDC */
    ret = CDCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Power up failed\n");
        goto error_uninitialize;
    }

    /* configure CDC controller */
    ret = CDCdrv->Control(CDC200_CONFIGURE_DISPLAY, (uint32_t)lcd_image);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC controller configuration failed\n");
        goto error_poweroff;
    }

    printf(">>> Allocated memory buffer Address is 0x%X <<<\n",(uint32_t)lcd_image);

    /* Start CDC */
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
            sys_busy_loop_us(100 * 1000);

        memset(lcd_image, 0xFF, sizeof(lcd_image));
        for(uint32_t count = 0; count < 20; count++)
            sys_busy_loop_us(100 * 1000);
    }

    /* Stop CDC controller */
    ret = CDCdrv->Stop();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Stop failed\n");
        goto error_poweroff;
    }

error_poweroff:
    /* Power off CDC */
    ret = CDCdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Power OFF failed.\r\n");
    }
error_uninitialize:
    /* Un-initialize CDC driver */
    ret = CDCdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: CDC Uninitialize failed.\r\n");
    }
error_disable_hfosc_clk:
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, false, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 38.4Mhz(HFOSC)  clock disable = %d\n", error_code);
    }
error_disable_100mhz_clk:
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, false, &service_error_code);
    if(error_code != SERVICES_REQ_SUCCESS)
    {
        printf("SE: MIPI 100MHz clock disable = %d\n", error_code);
    }

    printf("\r\n XXX CDC demo exiting XXX...\r\n");
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
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

    /* System Initialization */
    SystemCoreClockUpdate();

    /* Create application main thread */
    BaseType_t xReturned = xTaskCreate(cdc_demo_thread_entry, "cdc_demo_thread_entry",
                                       216, NULL,configMAX_PRIORITIES-1, &display_xHandle);
    if(xReturned != pdPASS)
    {

        vTaskDelete(display_xHandle);
        return -1;
     }

    /* Start thread execution */
    vTaskStartScheduler();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
