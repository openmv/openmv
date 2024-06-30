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
 * @file     i2c_using_i3c_testApp.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     31-May-2023
 * @brief    FreeRtos TestApp to verify i2c communication with
 *           multiple i2c slave devices using i3c IP
 *
 *           hardware setup
 *           BMI Slave is internally connected with the I3C_D
 *           No hardware setup required.
 *           Pins used:
 *           P7_6 (SDA)
 *           P7_7 (SCL)
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>

/* Project Includes */
#include "Driver_I3C.h"
#include "system_utils.h"

/* PINMUX Driver */
#include "pinconf.h"

/* Rtos include */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


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

/* i3c Driver */
extern ARM_DRIVER_I3C Driver_I3C;
static ARM_DRIVER_I3C *I3Cdrv = &Driver_I3C;

void i2c_using_i3c_demo_thread(void *pvParameters);

/* i3c callback events */
typedef enum _I3C_CB_EVENT{
    I3C_CB_EVENT_SUCCESS        = (1 << 0),
    I3C_CB_EVENT_ERROR          = (1 << 1)
}I3C_CB_EVENT;

volatile int32_t cb_event_flag = 0;

/**
  \fn          INT hardware_init(void)
  \brief       i3c hardware pin initialization:
                - PIN-MUX configuration
                - PIN-PAD configuration
  \param[in]   void
  \return      ARM_DRIVER_OK : success; ARM_DRIVER_ERROR : failure
*/
int32_t hardware_init(void)
{

  /* I3C_SDA_D */
  pinconf_set(PORT_7, PIN_6, PINMUX_ALTERNATE_FUNCTION_6,
          PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | \
          PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA);

  /* I3C_SCL_D */
  pinconf_set( PORT_7, PIN_7, PINMUX_ALTERNATE_FUNCTION_6,
          PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | \
          PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA);

    return ARM_DRIVER_OK;
}

/**
  \fn          void I3C_callback(UINT event)
  \brief       i3c isr callback
  \param[in]   event: i3c Event
  \return      none
*/
void I3C_callback(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_I3C_EVENT_TRANSFER_DONE)
    {
        /* Transfer Success */
        xResult = xTaskNotifyFromISR(i3c_xHandle,I3C_CB_EVENT_SUCCESS,eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }

    if (event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        /* Transfer Error */
        xResult = xTaskNotifyFromISR(i3c_xHandle,I3C_CB_EVENT_ERROR,eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

/**
  \fn          void i2c_using_i3c_demo_thread(void *pvParameters)
  \brief       TestApp to verify i2c communication with
                multiple i2c slave devices using i3c IP.


               This demo thread does:
                 - initialize i3c driver;
                 - set i2c speed mode to Standard mode 100 KBPS;
                 - attach all i2c slave devices to i3c;
                 - continuously read from register address 0x0000(chip-id)
                    for all the attached slaves;
                 - display result depending on whether
                   slave has given ACK or NACK.
  \return      none
*/
void i2c_using_i3c_demo_thread(void *pvParameters)
{

/* Maximum 8 Slave Devices are supported */
#define MAX_SLAVE_SUPPORTED   3

/* BMI slave address (chip-id 0x43) */
#define BMI_ADDR              0x69

/* Dummy Slave */
#define DUMMY_SLAVE1          0x50
#define DUMMY_SLAVE2          0x55

    int32_t   i      = 0;
    int32_t   ret    = 0;
    int32_t   len    = 0;
    uint32_t  actual_events = 0;

    /* @NOTE:
     *  I3C expects data to be aligned in 4-bytes (multiple of 4) for DMA.
     */
    uint8_t __ALIGNED(4) tx_data[4] = {0};   /* transmit data to   i3c */
    uint8_t __ALIGNED(4) rx_data[4] = {0};   /* receive  data from i3c */

    ARM_DRIVER_VERSION version;

    /* array of i2c slave address(static) */
    uint8_t slave_addr[MAX_SLAVE_SUPPORTED] =
    {
        BMI_ADDR,
        DUMMY_SLAVE1,
        DUMMY_SLAVE2
    };

    printf("\r\n \t\t >>> i2c using i3c demo starting up!!! <<< \r\n");

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
    if(ret != ARM_DRIVER_OK)
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
        goto error_poweroff;
    }

    /* Initialize I3C master */
    ret = I3Cdrv->Control(I3C_MASTER_INIT, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Master Init control failed.\r\n");
        goto error_uninitialize;
    }

    /* i2c Speed Mode Configuration:
     *  I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS  : Fast Mode Plus   1 MBPS
     *  I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS : Fast Mode      400 KBPS
     *  I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS : Standard Mode  100 KBPS
     */
    ret = I3Cdrv->Control(I3C_MASTER_SET_BUS_MODE,
                          I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Control failed.\r\n");
        goto error_poweroff;
    }

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

    /* Attach all the slave address */
    printf("\r\n Start attaching all i2c slave addr to i3c.\r\n");
    for(i=0; i<MAX_SLAVE_SUPPORTED; i++)
    {
        printf("\r\n  >> i=%d attaching i2c slave addr:0x%X to i3c...\r\n",  \
                           i, slave_addr[i]);

        ret = I3Cdrv->AttachSlvDev(ARM_I3C_DEVICE_TYPE_I2C, slave_addr[i]);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error: I3C Attach I2C device failed.\r\n");
            goto error_poweroff;
        }
    }

    /* Let's Continuously read from register address 0x0000(chip-id) for
     * all the attached slaves and display received data depending on
     * whether slave has given ACK or NACK.
    */
    while(1)
    {
        for(i=0; i<MAX_SLAVE_SUPPORTED; i++)
        {
            /* To Read from register address 0x0000(chip-id):
             *  First write 0x0000 using MasterTransmit and
             *   then Read  data   using MasterReceive
             */
            tx_data[0] = 0x00; /* Register Address[15:8] goes first  */
            tx_data[1] = 0x00; /* Register Address[07:0] goes second */

            /* TX/RX length is 2 Byte
             * (assume slave requires 16-bit data for TX/RX).
             */
            len = 2;

            printf("\r\n ------------------------------------------------------------ \r\n");
            printf("\r\n >> i=%d TX slave addr:0x%X reg_addr:[0]0x%X [1]0x%X \r\n",  \
                               i, slave_addr[i], tx_data[0],tx_data[1]);

            /* For TX, User has to pass
             * Slave Address + TX data + length of the TX data.
             */
            ret = I3Cdrv->MasterTransmit(slave_addr[i], tx_data, len);
            if(ret != ARM_DRIVER_OK)
            {
                printf("\r\n Error: i2c Master Transmit failed. \r\n");
                goto error_detach;
            }

            /* wait till any event to comes in isr callback */
            xTaskNotifyWait(NULL,I3C_CB_EVENT_SUCCESS|I3C_CB_EVENT_ERROR,&actual_events, portMAX_DELAY);

            if(actual_events & I3C_CB_EVENT_SUCCESS)
            {
                /* TX Success: Got ACK from slave */
                printf("\r\n \t\t >> i=%d TX Success: Got ACK from slave addr:0x%X.\r\n",  \
                               i, slave_addr[i]);
            }

            if(actual_events & I3C_CB_EVENT_ERROR)
            {
                /* TX Error: Got NACK from slave */
                printf("\r\n \t\t >> i=%d TX Error: Got NACK from slave addr:0x%X \r\n",  \
                               i, slave_addr[i]);
            }

            printf("\r\n\r\n >> i=%d RX slave addr:0x%X \r\n",i, slave_addr[i]);

            /* clear rx data buffer. */
            rx_data[0] = 0;
            rx_data[1] = 0;
            rx_data[2] = 0;

            len = 3;
            /* For RX, User has to pass
             * Slave Address + Pointer to RX data + length of the RX data.
             */
            ret = I3Cdrv->MasterReceive(slave_addr[i], rx_data, len);
            if(ret != ARM_DRIVER_OK)
            {
                printf("\r\n Error: i2c Master Receive failed. \r\n");
                goto error_detach;;
            }

            /* wait till any event to comes in isr callback */
            xTaskNotifyWait(NULL,I3C_CB_EVENT_SUCCESS | I3C_CB_EVENT_ERROR,&actual_events, portMAX_DELAY);

            if(actual_events & I3C_CB_EVENT_SUCCESS)
            {
                /* RX Success: Got ACK from slave */
                printf("\r\n \t\t >> i=%d RX Success: Got ACK from slave addr:0x%X.\r\n",  \
                               i, slave_addr[i]);
                printf("\r\n \t\t >> i=%d RX Received Data from slave:[0]0x%X [1]0x%X [2]0x%X.\r\n",  \
                               i,rx_data[0],rx_data[1],rx_data[2]);
            }

            if(actual_events & I3C_CB_EVENT_ERROR)
            {
                /* RX Error: Got NACK from slave */
                printf("\r\n \t\t >> i=%d RX Error: Got NACK from slave addr:0x%X \r\n",  \
                               i, slave_addr[i]);
            }

            printf("\r\n ---------------------------XXX------------------------------ \r\n");

            /* delay for 1 milli sec */
            sys_busy_loop_us (1000);
        }
    }

error_detach:

    /* Detach all attached slave address */
    for(i=0; i<MAX_SLAVE_SUPPORTED; i++)
    {
        printf("\r\n i=%d detaching i2c slave addr:0x%X from i3c.\r\n",i, slave_addr[i]);
        ret = I3Cdrv->Detachdev(slave_addr[i]);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error: I3C Detach I2C device failed.\r\n");
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

    printf("\r\n XXX I3C demo thread exiting XXX...\r\n");
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
   BaseType_t xReturned = xTaskCreate(i2c_using_i3c_demo_thread,
                                      "i2c_using_i3c_demo_thread",
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
