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
 * @file     LPSPI_SPI_testapp.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     29-May-2023
 * @brief    FreeRTOS demo application for LPSPI and SPI0.
 *           - Data transfer between LPSPI(master) and SPI0(slave).
 * @bug      None.
 * @Note     Demo application will work only on M55-HE core.
 ******************************************************************************/
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
/* include for Drivers */
#include "Driver_SPI.h"
#include "pinconf.h"
#include "Driver_GPIO.h"
/*RTOS Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


#if !defined(M55_HE)
#error "This Demo application works only on M55_HE"
#endif

/* Use below macro to specify transfer type
 * 1 - Uses SPI Transfer function
 * 0 - Uses SPI Send & Receive function
 * */
#define DATA_TRANSFER_TYPE  0

#define  LPSPI          LP   /* LPSPI instance */
#define  SPI0           0    /* SPI0 instance */
#define  GPIO7          7    /* GPIO instance to config flexio Port 4 pins */

extern ARM_DRIVER_SPI ARM_Driver_SPI_(LPSPI);
ARM_DRIVER_SPI *ptrLPSPI = &ARM_Driver_SPI_(LPSPI);

extern ARM_DRIVER_SPI ARM_Driver_SPI_(SPI0);
ARM_DRIVER_SPI *ptrSPI0 = &ARM_Driver_SPI_(SPI0);

extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO7);
ARM_DRIVER_GPIO *gpioDrv7 = &ARM_Driver_GPIO_(GPIO7);

/*Define for the FreeRTOS objects*/
#define LPSPI_CALLBACK_EVENT       0x01
#define SPI0_CALLBACK_EVENT        0x02

/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t spi_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
      StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
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
    StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
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

/**
 * @fn      static int32_t pinmux_config(void)
 * @brief   LPSPI & SPI0 pinmux configuration.
 * @note    none.
 * @param   none.
 * @retval  execution status.
 */
static int32_t pinmux_config(void)
{
    int32_t ret;

    /* pinmux configurations for LPSPI pins (using A version pins) */
    ret = pinconf_set(PORT_7, PIN_4, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for LPSPI_MISO_PIN\n");
        return ARM_DRIVER_ERROR;
    }
    ret = pinconf_set(PORT_7, PIN_5, PINMUX_ALTERNATE_FUNCTION_5, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for LPSPI_MOSI_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_7, PIN_6, PINMUX_ALTERNATE_FUNCTION_5, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for LPSPI_CLK_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_7, PIN_7, PINMUX_ALTERNATE_FUNCTION_5, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for LPSPI_SS_PIN\n");
        return ret;
    }


    /* pinmux configurations for SPI0 pins (using B version pins) */
    ret = pinconf_set(PORT_5, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI0_MISO_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_5, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI0_MOSI_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_5, PIN_3, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI0_CLK_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_5, PIN_2, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI0_SS_PIN\n");
        return ret;
    }

    return ret;
}

/**
 * @fn      static void LPSPI_cb_func (uint32_t event)
 * @brief   LPSPI callback function.
 * @note    none.
 * @param   event: SPI event.
 * @retval  none.
 */
static void LPSPI_cb_func (uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event == ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        xTaskNotifyFromISR(spi_xHandle, LPSPI_CALLBACK_EVENT, eSetBits, &xHigherPriorityTaskWoken);

        if (xResult == pdTRUE)        {    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );    }
    }
}

/**
 * @fn      static void SPI0_cb_func (uint32_t event)
 * @brief   SPI0 callback function.
 * @note    none.
 * @param   event: SPI event.
 * @retval  none.
 */
static void SPI0_cb_func (uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event == ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        xTaskNotifyFromISR(spi_xHandle, SPI0_CALLBACK_EVENT, eSetBits, &xHigherPriorityTaskWoken);

        if (xResult == pdTRUE)        {    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );    }
    }
}

/**
 * @fn      static void lpspi_spi0_transfer(void *pvParameters)
 * @brief   demo application function for data transfer.
 * @note    none.
 * @param   pvParameters.
 * @retval  none.
 */
static void lpspi_spi0_transfer(void *pvParameters)
{
    uint32_t lpspi_tx_buff, spi0_rx_buff = 0;
    int32_t ret = ARM_DRIVER_OK;
    uint32_t lpspi_control, spi0_control;
    uint32_t arg = ARM_GPIO_FLEXIO_VOLT_1V8;
    BaseType_t xReturned;
#if DATA_TRANSFER_TYPE
    uint32_t spi0_tx_buff, lpspi_rx_buff = 0;
#endif

    /*
     * H/W connections on devkit:
     * short LPSPI MISO (P7_4 -> J12-27 pin) and SPI0 MISO (P5_0 -> J12-13 pin).
     * short LPSPI MOSI (P7_5 -> J15-9 pin) and SPI0 MOSI (P5_1 -> J12-15 pin).
     * short LPSPI SCLK (P7_6 -> J15-8 pin) and SPI0 SCLK (P5_3 -> J14-5 pin).
     * short LPSPI SS (P7_7 -> J15-10 pin) and SPI0 SS (P5_2 -> J12-17 pin).
     * */

    printf("*** Demo FreeRTOS app using SPI0 & LPSPI is starting ***\n");

    ret = pinmux_config();
    if (ret)
    {
        printf("Error in pinmux configuration\n");
        return;
    }

    /* config any of the LPSPI pins (flexio) to 1.8V */
    ret = gpioDrv7->Initialize(PIN_4, NULL);
    if ((ret != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    }
    ret = gpioDrv7->PowerControl(PIN_4, ARM_POWER_FULL);
    if ((ret != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    }
    ret = gpioDrv7->Control(PIN_4, ARM_GPIO_CONFIG_FLEXIO, &arg);
    if ((ret != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    }

    /* LPSPI Configuration as master */
    ret = ptrLPSPI->Initialize(LPSPI_cb_func);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize the LPSPI\n");
        return;
    }

    ret = ptrLPSPI->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to power LPSPI\n");
        goto error_lpspi_uninitialize;
    }

    lpspi_control = (ARM_SPI_MODE_MASTER | ARM_SPI_SS_MASTER_HW_OUTPUT | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(32));

    /* Baudrate is 1MHz */
    ret = ptrLPSPI->Control(lpspi_control, 1000000);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure LPSPI\n");
        goto error_lpspi_power_off;
    }

    /* SPI0 Configuration as slave */
    ret = ptrSPI0->Initialize(SPI0_cb_func);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize the SPI0\n");
        goto error_lpspi_power_off;
    }

    ret = ptrSPI0->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to power SPI0\n");
        goto error_spi0_uninitialize;
    }

    spi0_control = (ARM_SPI_MODE_SLAVE | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(32));

    ret = ptrSPI0->Control(spi0_control, NULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure SPI0\n");
        goto error_spi0_power_off;
    }

    ret = ptrLPSPI->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to enable the slave select of LPSPI\n");
        goto error_spi0_power_off;
    }

#if DATA_TRANSFER_TYPE
    lpspi_tx_buff = 0xAAAAAAAA;
    spi0_tx_buff = 0x55555555;

    ret = ptrSPI0->Transfer(&spi0_tx_buff, &spi0_rx_buff, 1);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed SPI0 to configure as tx_rx\n");
        goto error_spi0_power_off;
    }

    ret = ptrLPSPI->Transfer(&lpspi_tx_buff, &lpspi_rx_buff, 1);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: LPSPI Failed to configure as tx_rx\n");
        goto error_spi0_power_off;
    }

#else
    lpspi_tx_buff = 0x12345678;

    ret = ptrSPI0->Receive(&spi0_rx_buff, 1);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: SPI0 Failed to configure as receive only\n");
        goto error_spi0_power_off;
    }

    ret = ptrLPSPI->Send(&lpspi_tx_buff, 1);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: LPSPI Failed to configure as send only\n");
        goto error_spi0_power_off;
    }
#endif

    xReturned = xTaskNotifyWait(NULL,LPSPI_CALLBACK_EVENT|SPI0_CALLBACK_EVENT,NULL, portMAX_DELAY);
    if (xReturned != pdTRUE )
    {
        printf("\n\r Task Wait Time out expired \n\r");
        goto error_spi0_power_off;
    }

    while (!((ptrLPSPI->GetStatus().busy == 0) && (ptrSPI0->GetStatus().busy == 0)));
    printf("Data Transfer completed\n");

    printf("SPI0 received value 0x%x\n", spi0_rx_buff);
#if DATA_TRANSFER_TYPE
    printf("LPSPI received value 0x%x\n", lpspi_rx_buff);
#endif

error_spi0_power_off :
    ret = ptrSPI0->PowerControl(ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("Failed to Power off SPI0\n");
    }

error_spi0_uninitialize :
    ret = ptrSPI0->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
        printf("Failed to Un-Initialized SPI0\n");
    }

error_lpspi_power_off :
    ret = ptrLPSPI->PowerControl(ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("Failed to Power off LPSPI\n");
    }

error_lpspi_uninitialize :
    ret = ptrLPSPI->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
        printf("Failed to Un-Initialized LPSPI\n");
    }

    printf("*** Demo FreeRTOS app using SPI0 & LPSPI is ended ***\n");

    /* thread delete */
    vTaskDelete(NULL);
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
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
   BaseType_t xReturned = xTaskCreate(lpspi_spi0_transfer, "SPI_Thread", 216, NULL,configMAX_PRIORITIES-1, &spi_xHandle);
   if (xReturned != pdPASS)
   {

       vTaskDelete(spi_xHandle);
       return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

}

