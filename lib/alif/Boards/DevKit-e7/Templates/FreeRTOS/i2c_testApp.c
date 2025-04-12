/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     i2c_testApp.c
 * @brief    FreeRtos demo application to verify I2C Master and
 *           Slave functionality with FreeRtos as an operating system
 *
 *           Code will verify below cases:
 *           1) Master transmit 30 bytes and Slave receive 30 bytes
 *           2) Slave transmit 29 bytes and Master receive 29 bytes
 *           I2C1 instance is taken as Master (PIN P7_2 and P7_3)
 *           I2C0 instance is taken as Slave  (PIN P0_2 and P0_3)
 *
 *           Hardware setup:
 *           - Connecting GPIO pins of I2C1 TO I2C0 instances
 *             SDA pin P7_2(J15) to P0_2(J11)
 *             SCL pin P7_3(J15) to P0_3(J11).
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_I2C.h"
#include "pinconf.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* Defining Address modes */
#define ADDRESS_MODE_7BIT   1                   /* I2C 7 bit addressing mode     */
#define ADDRESS_MODE_10BIT  2                   /* I2C 10 bit addressing mode    */
#define ADDRESS_MODE        ADDRESS_MODE_7BIT   /* Current Addressing mode       */

#if (ADDRESS_MODE == ADDRESS_MODE_10BIT)
    #define TAR_ADDRS       (0X2D0)  /* 10 bit Target(Slave) Address, use by Master */
    #define SAR_ADDRS       (0X2D0)  /* 10 bit Slave Own Address,     use by Slave  */
#else
    #define TAR_ADDRS       (0X40)   /* 7 bit Target(Slave) Address, use by Master  */
    #define SAR_ADDRS       (0X40)   /* 7 bit Slave Own Address,     use by Slave   */
#endif

/* Communication commands*/
#define RESTART           (0X01)
#define STOP              (0X00)

/* master transmit and slave receive */
#define MST_BYTE_TO_TRANSMIT        30

/* slave transmit and master receive */
#define SLV_BYTE_TO_TRANSMIT        29

/*Define for FreeRTOS notification objects */
#define I2C_MST_TRANSFER_DONE       0x01
#define I2C_SLV_TRANSFER_DONE       0x02
#define I2C_TWO_WAY_TRANSFER_DONE   (I2C_MST_TRANSFER_DONE | I2C_SLV_TRANSFER_DONE)

/*Define for FreeRTOS*/
#define STACK_SIZE                    1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* I2C Driver instance */
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2C_MstDrv = &Driver_I2C1;

extern ARM_DRIVER_I2C Driver_I2C0;
static ARM_DRIVER_I2C *I2C_SlvDrv = &Driver_I2C0;

/* Task handle */
TaskHandle_t i2c_xHandle;

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

/* Master parameter set */

/* Master TX Data (Any random value). */
uint8_t MST_TX_BUF[MST_BYTE_TO_TRANSMIT] =
{
    "!*!Test Message from Master!*!"
};

/* master receive buffer */
uint8_t MST_RX_BUF[SLV_BYTE_TO_TRANSMIT];

/* Master parameter set END  */


/* Slave parameter set */

/* slave receive buffer */
uint8_t SLV_RX_BUF[MST_BYTE_TO_TRANSMIT];

/* Slave TX Data (Any random value). */
uint8_t SLV_TX_BUF[SLV_BYTE_TO_TRANSMIT] =
{
    "!*!Test Message from Slave!*!"
};

/* Slave parameter set END */

static void i2c_mst_transfer_callback(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        /* Transfer or receive is finished */
        xResult = xTaskNotifyFromISR(i2c_xHandle, I2C_MST_TRANSFER_DONE,
                           eSetBits, &xHigherPriorityTaskWoken);

        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

static void i2c_slv_transfer_callback(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        /* Transfer or receive is finished */
        xResult = xTaskNotifyFromISR(i2c_xHandle, I2C_SLV_TRANSFER_DONE,
                           eSetBits, &xHigherPriorityTaskWoken);

        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

/* Pinmux */
void hardware_init()
{
    /* I2C0_SDA_A */
    pinconf_set(PORT_0, PIN_2, PINMUX_ALTERNATE_FUNCTION_3, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP));

    /* I2C0_SCL_A */
    pinconf_set(PORT_0, PIN_3, PINMUX_ALTERNATE_FUNCTION_3, \
         ( PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP));

    /* I2C1_SDA_C */
    pinconf_set(PORT_7, PIN_3, PINMUX_ALTERNATE_FUNCTION_5, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP));

    /* I2C1_SCL_C */
    pinconf_set(PORT_7, PIN_2, PINMUX_ALTERNATE_FUNCTION_5, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP));
}

void I2C_Thread(void *pvParameters)
{
    int ret                      = 0;
    ARM_DRIVER_VERSION version;
    ARM_I2C_CAPABILITIES capabilities;
    uint32_t task_notified_value = 0;

    printf("\r\n >>> I2C demo Thread starting up!!! <<< \r\n");

    /* Pinmux */
    hardware_init();

    version = I2C_MstDrv->GetVersion();
    printf("\r\n I2C version api:0x%X driver:0x%X...\r\n",version.api, version.drv);

    /* Initialize Master I2C driver */
    ret = I2C_MstDrv->Initialize(i2c_mst_transfer_callback);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C master init failed\n");
        return;
    }

    /* Initialize Slave I2C driver */
    ret = I2C_SlvDrv->Initialize(i2c_slv_transfer_callback);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C slave init failed\n");
        return;
    }

    /* I2C Master Power control  */
    ret = I2C_MstDrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C Master Power up failed\n");
        goto error_uninitialize;
    }

    /* I2C Slave Power control */
    ret = I2C_SlvDrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C Slave Power up failed\n");
        goto error_poweroff;
    }

    /* I2C Master Control */
    ret = I2C_MstDrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C Master Control failed\n");
        goto error_uninitialize;
    }

    /* I2C Slave Control */
#if (ADDRESS_MODE == ADDRESS_MODE_10BIT)
    ret = I2C_SlvDrv->Control(ARM_I2C_OWN_ADDRESS,
                             (SAR_ADDRS | ARM_I2C_ADDRESS_10BIT));
#else
    ret = I2C_SlvDrv->Control(ARM_I2C_OWN_ADDRESS, SAR_ADDRS);
#endif
     if(ret != ARM_DRIVER_OK){
         printf("\r\n Error: I2C Slave Control failed\n");
         goto error_uninitialize;
     }

     printf("\n----------------Master transmit/slave receive-----------------------\n");

     /* I2C Slave Receive */
     ret = I2C_SlvDrv->SlaveReceive(SLV_RX_BUF, MST_BYTE_TO_TRANSMIT);
     if (ret != ARM_DRIVER_OK)
     {
         printf("\r\n Error: I2C Slave Receive failed\n");
         goto error_uninitialize;
     }
     /* delay */
     sys_busy_loop_us(500);

     /* I2C Master Transmit */
#if (ADDRESS_MODE == ADDRESS_MODE_10BIT)
    I2C_MstDrv->MasterTransmit((TAR_ADDRS | ARM_I2C_ADDRESS_10BIT),
                               MST_TX_BUF, MST_BYTE_TO_TRANSMIT, STOP);
#else
    I2C_MstDrv->MasterTransmit(TAR_ADDRS, MST_TX_BUF,
                               MST_BYTE_TO_TRANSMIT, STOP);
#endif
     if (ret != ARM_DRIVER_OK)
     {
         printf("\r\n Error: I2C Master Transmit failed\n");
         goto error_uninitialize;
     }

     while(pdTRUE)
     {
         /* wait for master/slave callback. */
         if(xTaskNotifyWait(NULL, NULL, &task_notified_value, portMAX_DELAY) != pdFALSE)
         {
             /* Checks if both callbacks are successful */
             if(task_notified_value != I2C_TWO_WAY_TRANSFER_DONE)
             {
                 xTaskNotifyStateClear(NULL);
             }
             else
             {
                 task_notified_value = 0;
                 break;
             }
         }
     }

     /* 3ms delay */
     vTaskDelay(3);

     /* Compare received data. */
     if(memcmp(&SLV_RX_BUF, &MST_TX_BUF, MST_BYTE_TO_TRANSMIT))
     {
         printf("\n Error: Master transmit/slave receive failed \n");
         printf("\n ---Stop--- \r\n wait forever >>> \n");
         while(1);
     }

     printf("\n----------------Master receive/slave transmit-----------------------\n");

     /* I2C Master Receive */
#if (ADDRESS_MODE == ADDRESS_MODE_10BIT)
     ret = I2C_MstDrv->MasterReceive((TAR_ADDRS | ARM_I2C_ADDRESS_10BIT),
                                     MST_RX_BUF, SLV_BYTE_TO_TRANSMIT, STOP);
#else
     ret = I2C_MstDrv->MasterReceive(TAR_ADDRS, MST_RX_BUF,
                                     SLV_BYTE_TO_TRANSMIT, STOP);
#endif
     if (ret != ARM_DRIVER_OK)
     {
         printf("\r\n Error: I2C Master Receive failed\n");
         goto error_uninitialize;
     }

     /* I2C Slave Transmit */
     I2C_SlvDrv->SlaveTransmit(SLV_TX_BUF, SLV_BYTE_TO_TRANSMIT);
     if (ret != ARM_DRIVER_OK)
     {
         printf("\r\n Error: I2C Slave Transmit failed\n");
         goto error_uninitialize;
     }

     while(pdTRUE)
     {
          /* wait for master/slave callback. */
          if(xTaskNotifyWait(NULL, NULL, &task_notified_value, portMAX_DELAY) != pdFALSE)
          {
              /* Checks if both callbacks are successful */
              if(task_notified_value != I2C_TWO_WAY_TRANSFER_DONE)
              {
                  xTaskNotifyStateClear(NULL);
              }
              else
              {
                  task_notified_value = 0;
                  break;
              }
          }
     }
     /* 3ms delay */
     vTaskDelay(3);

     /* Compare received data. */
     if(memcmp(&SLV_TX_BUF, &MST_RX_BUF, SLV_BYTE_TO_TRANSMIT))
     {
         printf("\n Error: Master receive/slave transmit failed\n");
         printf("\n ---Stop--- \r\n wait forever >>> \n");
         while(1);
     }

     ret =I2C_MstDrv->Uninitialize();
     if (ret == ARM_DRIVER_OK)
     {
         printf("\r\n I2C Master Uninitialized\n");
         goto error_uninitialize;
     }
     ret =I2C_SlvDrv->Uninitialize();
     if (ret == ARM_DRIVER_OK)
     {
         printf("\r\n I2C Slave Uninitialized\n");
         goto error_uninitialize;
     }

     printf("\n >>> I2C Communication completed without any error <<< \n");
     printf("\n ---END--- \r\n wait forever >>> \n");
     while(1);

  error_poweroff:
      /* Power off I2C peripheral */
      ret = I2C_MstDrv->PowerControl(ARM_POWER_OFF);
      if(ret != ARM_DRIVER_OK)
      {
         printf("\r\n Error: I2C Power OFF failed.\r\n");
      }
      ret = I2C_SlvDrv->PowerControl(ARM_POWER_OFF);
      if(ret != ARM_DRIVER_OK)
      {
         printf("\r\n Error: I2C Power OFF failed.\r\n");
      }

  error_uninitialize:
      /* Un-initialize I2C driver */
      ret = I2C_MstDrv->Uninitialize();
      if(ret == ARM_DRIVER_OK)
      {
        printf("\r\n I2C Master Uninitialized\r\n");
      }
      ret = I2C_SlvDrv->Uninitialize();
      if(ret == ARM_DRIVER_OK)
      {
         printf("\r\n I2C Slave Uninitialized\r\n");
      }
          printf("\r\n >>> I2C demo thread exiting <<<\r\n");

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
   BaseType_t xReturned = xTaskCreate(I2C_Thread, "I2C_Thread", 256, NULL,
                                      configMAX_PRIORITIES-1, &i2c_xHandle);
   if (xReturned != pdPASS)
   {
      vTaskDelete(i2c_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();
}
