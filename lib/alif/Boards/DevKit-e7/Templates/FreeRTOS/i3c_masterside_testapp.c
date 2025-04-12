/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/**************************************************************************//**
 * @file     i3c_masterside_testapp.c
 * @author   Prabhakar kumar
 * @email    prabhakar.kumar@alifsemi.com
 * @version  V1.0.0
 * @date     27-May-2023
 * @brief    TestApp to verify master and slave loop back test
 *
 *           Master sends n-bytes of data to Slave.
 *            Slave sends back received data to Master.
 *            then Master will compare send and received data
 *            (Master will continue sending data in loop,
 *             Master will stop if send and received data does not match).
 *
 *           Hardware Setup:
 *            Required two boards one for Master and one for Slave
 *             (as there is only one i3c instance is available on ASIC).
 *
 *           Connect SDA to SDA and SCL to SCL and GND to GND.
 *            - SDA P7_6 -> SDA P7_6
 *            - SCL P7_7 -> SCL P7_7
 *            - GND      -> GND
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include "string.h"
#include "system_utils.h"

/* Project Includes */
#include "Driver_I3C.h"

/* PINMUX Driver */
#include "pinconf.h"
#include "Driver_GPIO.h"

/* Rtos include */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/* i3c Driver instance 0 */
extern ARM_DRIVER_I3C Driver_I3C;
static ARM_DRIVER_I3C *I3Cdrv = &Driver_I3C;

/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

TaskHandle_t i3c_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
      StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
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

/* I3C slave target address */
#define I3C_SLV_TAR           (0x48)

/* transmit buffer from i3c */
uint8_t __ALIGNED(4) tx_data[4] = {0x00, 0x01, 0x02, 0x03};

/* receive buffer from i3c */
uint8_t __ALIGNED(4) rx_data[4] = {0x00};

uint32_t tx_cnt = 0;
uint32_t rx_cnt = 0;

void i3c_master_loopback_thread(void *pvParameters);

/* i3c callback events */
typedef enum _I3C_CB_EVENT{
    I3C_CB_EVENT_SUCCESS        = (1 << 0),
    I3C_CB_EVENT_ERROR          = (1 << 1)
}I3C_CB_EVENT;

/**
  \fn          int32_t hardware_init(void)
  \brief       i3c hardware pin initialization:
                - PIN-MUX configuration
                - PIN-PAD configuration
  \param[in]   void
  \return      0:success; -1:failure
*/
int32_t hardware_init(void)
{
    /* for I3C_D(PORT_7 PIN_6(SDA)/PIN_7(SCL)) instance,
     *  for I3C in I3C mode (not required for I3C in I2C mode)
     *  GPIO voltage level(flex) has to be change to 1.8-V power supply.
     *
     *  GPIO_CTRL Register field VOLT:
     *   Select voltage level for the 1.8-V/3.3-V (flex) I/O pins
     *    0x0: I/O pin will be used with a 3.3-V power supply
     *    0x1: I/O pin will be used with a 1.8-V power supply
     */

    /* Configure GPIO flex I/O pins to 1.8-V:
     *  P7_6 and P7_7 pins are part of GPIO flex I/O pins,
     *   so we can use any one of the pin to configure flex I/O.
     */
#define GPIO7_PORT          7

    extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO7_PORT);
    ARM_DRIVER_GPIO *gpioDrv = &ARM_Driver_GPIO_(GPIO7_PORT);

    int32_t  ret = 0;
    uint32_t arg = 0;

    ret = gpioDrv->Initialize(PIN_6, NULL);
    if (ret != 0)
    {
        printf("ERROR: Failed to initialize GPIO \n");
        return -1;
    }

    ret = gpioDrv->PowerControl(PIN_6, ARM_POWER_FULL);
    if (ret != 0)
    {
        printf("ERROR: Failed to powered full GPIO \n");
        return -1;
    }

    /* select control argument as flex 1.8-V */
    arg = ARM_GPIO_FLEXIO_VOLT_1V8;
    ret = gpioDrv->Control(PIN_6, ARM_GPIO_CONFIG_FLEXIO, &arg);
    if (ret != 0)
    {
        printf("ERROR: Failed to control GPIO Flex \n");
        return -1;
    }

    /* I3C_SDA_D */
    ret = pinconf_set(PORT_7, PIN_6, PINMUX_ALTERNATE_FUNCTION_6,
                PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP |
                PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA);

    /* I3C_SCL_D */
    ret = pinconf_set(PORT_7, PIN_7, PINMUX_ALTERNATE_FUNCTION_6,
                PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP |
                PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA);

    return ARM_DRIVER_OK;
}

/**
  \fn          void I3C_callback(UINT event)
  \brief       i3c isr callback
  \param[in]   event: i3c Event
  \return      none
*/
static void I3C_callback(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_I3C_EVENT_TRANSFER_DONE)
    {
        /* Transfer Success */
        xResult = xTaskNotifyFromISR(i3c_xHandle, I3C_CB_EVENT_SUCCESS,
                                     eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
    if (event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        /* Transfer Error */
        xResult = xTaskNotifyFromISR(i3c_xHandle, I3C_CB_EVENT_ERROR,
                                     eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

/**
  \fn          void i3c_master_loopback_demo(void *pvParameters)
  \brief       TestApp to verify i3c master mode loopback
               This demo does:
                 - initialize i3c driver;
                 - 1 byte of data transmitted from master and slave receive 1 byte and
                   same 1 byte of data received by slave is transmitted through slave
                   transmit and master receive 1byte.
  \return      none
*/
void i3c_master_loopback_thread(void *pvParameters)
{
    uint32_t   ret           = 0;
    uint32_t   len           = 0;
    int32_t    cmp           = 0;
    uint8_t    slave_addr    = 0;
    uint32_t   actual_events = 0;

    ARM_I3C_CMD i3c_cmd;

    ARM_DRIVER_VERSION version;

    printf("\r\n \t\t >>> Master loop back demo starting up!!! <<< \r\n");

    /* Get i3c driver version. */
    version = I3Cdrv->GetVersion();
    printf("\r\n i3c version api:0x%X driver:0x%X \r\n",  \
                           version.api, version.drv);

    if((version.api < ARM_DRIVER_VERSION_MAJOR_MINOR(7U, 0U))        ||
       (version.drv < ARM_DRIVER_VERSION_MAJOR_MINOR(7U, 0U)))
    {
        printf("\r\n Error: >>>Old driver<<< Please use new one \r\n");
        return;
    }

    /* Initialize i3c hardware pins using PinMux Driver. */
    ret = hardware_init();
    if(ret != 0)
    {
        printf("\r\n Error: i3c hardware_init failed.\r\n");
        return;
    }

    /* Initialize I3C driver */
    ret = I3Cdrv->Initialize(I3C_callback);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Initialize failed.\r\n");
        return;
    }

    /* Power up I3C peripheral */
    ret = I3Cdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Power Up failed.\r\n");
        goto error_uninitialize;
    }

    /* Initialize I3C master */
    ret = I3Cdrv->Control(I3C_MASTER_INIT, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Master Init control failed.\r\n");
        goto error_uninitialize;
    }

    /* i3c Speed Mode Configuration: Bus mode slow  */
    ret = I3Cdrv->Control(I3C_MASTER_SET_BUS_MODE,
                          I3C_BUS_SLOW_MODE);

    /* Reject Hot-Join request */
    ret = I3Cdrv->Control(I3C_MASTER_SETUP_HOT_JOIN_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Hot Join control failed.\r\n");
        goto error_uninitialize;
    }

    /* Reject Master request */
    ret = I3Cdrv->Control(I3C_MASTER_SETUP_MR_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Master Request control failed.\r\n");
        goto error_uninitialize;
    }

    /* Reject Slave Interrupt request */
    ret = I3Cdrv->Control(I3C_MASTER_SETUP_SIR_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Slave Interrupt Request control failed.\r\n");
        goto error_uninitialize;
    }

    sys_busy_loop_us(1000);

    /* Assign Dynamic Address to i3c slave */
    printf("\r\n >> i3c: Get dynamic addr for static addr:0x%X.\r\n",I3C_SLV_TAR);

    i3c_cmd.rw            = 0U;
    i3c_cmd.cmd_id        = I3C_CCC_SETDASA;
    i3c_cmd.len           = 1U;
    /* Assign Slave's Static address */
    i3c_cmd.addr          = I3C_SLV_TAR;
    i3c_cmd.data          = NULL;
    i3c_cmd.def_byte      = 0U;

    ret = I3Cdrv->MasterAssignDA(&i3c_cmd);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C MasterAssignDA failed.\r\n");
        goto error_poweroff;
    }

    /* Waiting for the callback */
    xTaskNotifyWait(NULL, I3C_CB_EVENT_SUCCESS | I3C_CB_EVENT_ERROR,
                    &actual_events, portMAX_DELAY);

    if(actual_events == I3C_CB_EVENT_ERROR)
    {
        printf("\r\n Error: First attempt failed. retrying \r\n");
        /* Delay */
        sys_busy_loop_us(1000);

        /* Observation:
         *  Master needs to send "MasterAssignDA" two times,
         *  First time slave is not giving ACK.
         */

        /* Assign Dynamic Address to i3c slave */
        ret = I3Cdrv->MasterAssignDA(&i3c_cmd);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error: I3C MasterAssignDA failed.\r\n");
            goto error_poweroff;
        }

        /* Waiting for the callback */
        xTaskNotifyWait(NULL, I3C_CB_EVENT_SUCCESS | I3C_CB_EVENT_ERROR,
                        &actual_events, portMAX_DELAY);

        if(actual_events == I3C_CB_EVENT_ERROR)
        {
            printf("\r\nError: I3C MasterAssignDA failed.\r\n");
            goto error_poweroff;
        }
    }

    /* Get assigned dynamic address for the static address */
    ret = I3Cdrv->GetSlaveDynAddr(I3C_SLV_TAR, &slave_addr);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Failed to get Dynamic Address.\r\n");
        goto error_poweroff;
    }
    else
    {
        printf("\r\n >> i3c: Rcvd dyn_addr:0x%X for static addr:0x%X\r\n",
                slave_addr,I3C_SLV_TAR);
    }

    /* i3c Speed Mode Configuration: Normal I3C mode */
    ret = I3Cdrv->Control(I3C_MASTER_SET_BUS_MODE,
                          I3C_BUS_NORMAL_MODE);

    while(1)
    {
        len = 4;

        /* Delay */
        sys_busy_loop_us(1000);

        /* fill any random TX data. */
        tx_data[0] += 1;
        tx_data[1] += 1;
        tx_data[2] += 1;
        tx_data[3] += 1;

        /* Master transmit */
        ret = I3Cdrv->MasterTransmit(slave_addr, tx_data, len);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error: I3C Master Transmit failed. \r\n");
            goto error_poweroff;
        }

        xTaskNotifyWait(NULL, I3C_CB_EVENT_SUCCESS | I3C_CB_EVENT_ERROR,
                        &actual_events, portMAX_DELAY);

        if(actual_events & I3C_CB_EVENT_ERROR)
        {
            printf("\nError: I3C Master transmit Failed\n");
            while(1);
        }

        tx_cnt += 1;

        /* Delay */
        sys_busy_loop_us(1000);

        /* clear rx_data buffer */
        rx_data[0] = 0x00;
        rx_data[1] = 0x00;
        rx_data[2] = 0x00;
        rx_data[3] = 0x00;

        /* Master receive */
        ret = I3Cdrv->MasterReceive(slave_addr, rx_data, len);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error: I3C Master Receive failed. \r\n");
            goto error_poweroff;
        }

        /* wait for callback event. */
        xTaskNotifyWait(NULL, I3C_CB_EVENT_SUCCESS | I3C_CB_EVENT_ERROR,
                        &actual_events, portMAX_DELAY);

        if(actual_events & I3C_CB_EVENT_ERROR)
        {
            printf("\nError: I3C Master Receive failed.\n");
            while(1);
        }

        rx_cnt += 1;

        /* compare tx and rx data, stop if data does not match */
        cmp = memcmp(tx_data, rx_data, len);
        if(cmp != 0)
        {
           printf("\nError: TX and RX data mismatch.\n");
           while(1);
        }
}
error_poweroff:

    /* Power off I3C peripheral */
    ret = I3Cdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
         printf("\r\n Error: I3C Power OFF failed.\r\n");
    }

error_uninitialize:

    /* Un-initialize I3C driver */
    ret = I3Cdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Uninitialize failed.\r\n");
    }

    printf("\r\n I3C demo exiting...\r\n");

    /* thread delete */
    vTaskDelete( NULL );
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main(void)
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
   BaseType_t xReturned = xTaskCreate(i3c_master_loopback_thread,
                                      "i3c_master_loopback_thread",
                                      STACK_SIZE, NULL,
                                      configMAX_PRIORITIES-1, &i3c_xHandle);
   if (xReturned != pdPASS)
   {
      vTaskDelete(i3c_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
