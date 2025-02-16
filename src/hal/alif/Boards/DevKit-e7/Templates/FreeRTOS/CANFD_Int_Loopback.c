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
 * @file     CANFD_Int_Loopback.c
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     05-July-2023
 * @brief    FreeRTOS demo application for CANFD.
 *           - Performs Internal loopback test.
 * @bug      None
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <RTE_Components.h>
#include CMSIS_device_header
#include "pinconf.h"
#include "Driver_CAN.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#include "se_services_port.h"

/* It is recommended to use the bit rate and bit segments
 * as specified in the Hardware reference manual for proper communication.
 *
 * Nominal bit rate 500kbps, Fast bit rate 2Mbps, 20MHz CANFD clock are set
 * for this example
*/
#define CANFD_NOMINAL_BITRATE               500000U
#define CANFD_BIT_TIME_PROP_SEG             2U
#define CANFD_BIT_TIME_SEG1                 30U
#define CANFD_BIT_TIME_SEG2                 8U
#define CANFD_BIT_TIME_SJW                  8U
#define CANFD_FAST_BITRATE                  2000000U
#define CANFD_FAST_BIT_TIME_PROP_SEG        1U
#define CANFD_FAST_BIT_TIME_SEG1            7U
#define CANFD_FAST_BIT_TIME_SEG2            2U
#define CANFD_FAST_BIT_TIME_SJW             2U
#define CANFD_TRANSCEIVER_TX_DELAY_COMP     8U

#define CANFD_NOMINAL_BITTIME_SEGMENTS      ((CANFD_BIT_TIME_PROP_SEG << 0U)      | \
                                            (CANFD_BIT_TIME_SEG1 << 8U)           | \
                                            (CANFD_BIT_TIME_SEG2 << 16U)          | \
                                            (CANFD_BIT_TIME_SJW << 24U))

#define CANFD_FAST_BITTIME_SEGMENTS         ((CANFD_FAST_BIT_TIME_PROP_SEG << 0U) | \
                                            (CANFD_FAST_BIT_TIME_SEG1 << 8U)      | \
                                            (CANFD_FAST_BIT_TIME_SEG2 << 16U)     | \
                                            (CANFD_FAST_BIT_TIME_SJW << 24U))

/* Object filter settings */
#define CANFD_OBJECT_FILTER_CODE_1          0x5A5U
#define CANFD_OBJECT_FILTER_CODE_2          0x01FF5A5AU
#define CANFD_OBJECT_FILTER_MASK            0U

#define CANFD_MAX_MSG_SIZE                  64U

/* Application Message Frame types */
typedef enum _CANFD_FRAME
{
    CANFD_FRAME_STD_ID_CLASSIC_DATA,
    CANFD_FRAME_STD_ID_RTR,
    CANFD_FRAME_STD_ID_FD_DATA,
    CANFD_FRAME_EXT_ID_RTR,
    CANFD_FRAME_EXT_ID_CLASSIC_DATA,
    CANFD_FRAME_EXT_ID_FD_DATA,
    CANFD_FRAME_OVER
}CANFD_FRAME;

/* Define for FreeRTOS notification objects */
#define CANFD_RX_SUCCESS                    0x01U
#define CANFD_ERROR                         0x02U
#define CANFD_ALL_NOTIFICATIONS             (CANFD_RX_SUCCESS | CANFD_ERROR)

/* Define for FreeRTOS */
#define CANFD_TASK_STACK_SIZE               512U      /* 512 Words */
#define TIMER_SERVICE_TASK_STACK_SIZE       configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE                configMINIMAL_STACK_SIZE

static StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
static StaticTask_t IdleTcb;
static StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
static StaticTask_t TimerTcb;

/* Task handle */
static TaskHandle_t canfd_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
      StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
   *ppxIdleTaskTCBBuffer = &IdleTcb;
   *ppxIdleTaskStackBuffer = IdleStack;
   *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
   ARG_UNUSED(pxTask);
   ARG_UNUSED(pcTaskName);

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

/* CANFD instance object */
extern ARM_DRIVER_CAN  Driver_CANFD;
static ARM_DRIVER_CAN* CANFD_instance           = &Driver_CANFD;

/* File Global variables */
static volatile bool error_occurred             = false;
static volatile bool rx_buf_overrun             = false;
static bool          stop_execution             = false;
static volatile bool rx_msg_error               = false;
static uint8_t       tx_obj_id                  = 255U;
static uint8_t       rx_obj_id                  = 255U;
static ARM_CAN_MSG_INFO tx_msg_header;
static ARM_CAN_MSG_INFO rx_msg_header;
static uint8_t          tx_msg_size             = 0U;
static volatile uint8_t rx_msg_size             = 0U;
static uint8_t tx_data[CANFD_MAX_MSG_SIZE + 1U] =
                       "!!!!!!***** CANFD TESTAPP Message Communication Test *****!!!!!!";
static uint8_t rx_data[CANFD_MAX_MSG_SIZE + 1U];

/* A map between Data length code to the payload size */
static const uint8_t canfd_len_dlc_map[0x10U] =
                     {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
                      12U, 16U, 20U, 24U, 32U, 48U, 64U};

/* Support functions */
static bool canfd_process_rx_message(void);
static bool canfd_transmit_message(const CANFD_FRAME msg_type);
static void canfd_check_error(void);

/**
 * @fn      static void cb_unit_event(uint32_t event)
 * @brief   CANFD Callback function for events
 * @note    none
 * @param   event: CANFD event
 * @retval  none
 */
static void cb_unit_event(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;
    if(event == ARM_CAN_EVENT_UNIT_WARNING)
    {
        /* Communication error occurred */
        xResult = xTaskNotifyFromISR(canfd_xHandle, CANFD_ERROR,
                                     eSetBits, &xHigherPriorityTaskWoken);

        if(xResult == pdTRUE)
        {
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }
    }
}

/**
 * @fn      static void cb_object_event(uint32_t obj_idx, uint32_t event)
 * @brief   CANFD Callback function for particular object events
 * @note    none
 * @param   obj_idx : Object ID
 * @param   event   : CANFD event
 * @retval  none
 */
static void cb_object_event(uint32_t obj_idx, uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if((event & ARM_CAN_EVENT_RECEIVE) ||
       (event & ARM_CAN_EVENT_RECEIVE_OVERRUN))
    {
        /* Invokes Message Read function if the Receive Object matches */
        if(obj_idx == rx_obj_id)
        {
            /*  Reading arrived CAN Message */
            if(CANFD_instance->MessageRead(obj_idx, &rx_msg_header,
                                           rx_data,
                                           rx_msg_size) != ARM_DRIVER_OK)
            {
                rx_msg_error = true;
            }

            if(event & ARM_CAN_EVENT_RECEIVE_OVERRUN)
            {
                rx_buf_overrun = true;
            }

            /* Rx Success - Notify the task*/
            xResult = xTaskNotifyFromISR(canfd_xHandle, CANFD_RX_SUCCESS,
                                         eSetBits, &xHigherPriorityTaskWoken);

            if(xResult == pdTRUE)
            {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        }
    }
}

/**
 * @fn      static void canfd_lbi_demo_task(void *pvParameters)
 * @brief   CANFD Internal Loopback Demo
 * @note    none
 * @param   pvParameters : Task parameter
 * @retval  none
 */
static void canfd_lbi_demo_task(void *pvParameters)
{
    CANFD_FRAME msg_type            = CANFD_FRAME_STD_ID_CLASSIC_DATA;
    int32_t ret_val                 = ARM_DRIVER_OK;
    ARM_CAN_CAPABILITIES              can_capabilities;
    ARM_CAN_OBJ_CAPABILITIES          can_obj_capabilities;
    uint8_t iter                    = 0U;
    bool cur_sts                    = true;
    uint32_t task_notified_value    = 0U;
    uint32_t error_code             = 0U;
    uint32_t service_error_code     = 0U;

    ARG_UNUSED(pvParameters);

    /* Initialize the SE services */
    se_services_port_init();

    /* Enables the HFOSC clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                                              CLKEN_HFOSC,
                                              true,
                                              &service_error_code);
    if(error_code)
    {
        printf("SE Error: HFOSC clk enable = %d\n", (int)error_code);
        return;
    }

    /* Enables the 160MHz clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                                              CLKEN_CLK_160M,
                                              true,
                                              &service_error_code);
    if(error_code)
    {
        printf("SE Error: 160 MHz clk enable = %d\n", (int)error_code);
        return;
    }

    printf("*** CANFD Internal Loopback Demo app is starting ***\n");

    if(ret_val != ARM_DRIVER_OK)
    {
        printf("Error in pin-mux configuration\n");
        return;
    }

    /* Get CANFD capabilities */
    can_capabilities = CANFD_instance->GetCapabilities();
    printf("Num of objects supported: %d\r\n", can_capabilities.num_objects);

    /* Initializing CANFD Access struct */
    ret_val = CANFD_instance->Initialize(cb_unit_event, cb_object_event);
    if(ret_val != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize the CANFD \n");
        return;
    }

    /* Powering up CANFD */
    ret_val = CANFD_instance->PowerControl(ARM_POWER_FULL);
    if(ret_val != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to Power up the CANFD \n");
        goto uninitialise_canfd;
    }

    /* Setting CANFD to FD mode */
    if(can_capabilities.fd_mode == 1U)
    {
        CANFD_instance->Control(ARM_CAN_SET_FD_MODE, ENABLE);
        if(ret_val != ARM_DRIVER_OK)
        {
           printf("ERROR: CANFD Enabling FD mode failed\r\n");
           goto power_off_canfd;
        }
    }

    /* Initializing up CANFD module */
    ret_val = CANFD_instance->SetMode(ARM_CAN_MODE_INITIALIZATION);
    if(ret_val != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to set CANFD to INIT mode \r\n");
        goto power_off_canfd;
    }
    /* Setting bit rate for CANFD */
    ret_val = CANFD_instance->SetBitrate(ARM_CAN_BITRATE_NOMINAL,
                                         CANFD_NOMINAL_BITRATE,
                                         CANFD_NOMINAL_BITTIME_SEGMENTS);
    if(ret_val != ARM_DRIVER_OK)
    {
       printf("ERROR: Failed to set CANFD Nominal Bitrate\r\n");
       goto power_off_canfd;
    }
    /* Setting bit rate for CANFD */
    if(can_capabilities.fd_mode == 1U)
    {
        ret_val = CANFD_instance->SetBitrate(ARM_CAN_BITRATE_FD_DATA,
                                             CANFD_FAST_BITRATE,
                                             CANFD_FAST_BITTIME_SEGMENTS);
        if(ret_val != ARM_DRIVER_OK)
        {
           printf("ERROR: Failed to set CANFD Fast Bitrate\r\n");
           goto power_off_canfd;
        }

        /* Sets below Transceiver's Transmitter Delay Compensation value
         * for Fast bit rate of 2Mbps when CANFD clock is 20MHz */
        ret_val = CANFD_instance->Control(ARM_CAN_SET_TRANSCEIVER_DELAY,
                                          CANFD_TRANSCEIVER_TX_DELAY_COMP);
        if(ret_val != ARM_DRIVER_OK)
        {
           printf("ERROR: Failed to set CANFD TDC \r\n");
           goto power_off_canfd;
        }
    }
    /* Assign IDs to Tx and Rx objects */
    for(iter = 0U; iter < can_capabilities.num_objects; iter++)
    {
        can_obj_capabilities = CANFD_instance->ObjectGetCapabilities(iter);
        if((can_obj_capabilities.tx == 1U) && (tx_obj_id == 255U))
        {
            tx_obj_id = iter;
        }
        else if((can_obj_capabilities.rx == 1U) && (rx_obj_id == 255U))
        {
            rx_obj_id = iter;
        }
    }

    ret_val = CANFD_instance->ObjectConfigure(tx_obj_id, ARM_CAN_OBJ_TX);
    if(ret_val != ARM_DRIVER_OK)
    {
       printf("ERROR: Object Tx configuration failed\r\n");
       goto power_off_canfd;
    }

    ret_val = CANFD_instance->ObjectConfigure(rx_obj_id, ARM_CAN_OBJ_RX);
    if(ret_val != ARM_DRIVER_OK)
    {
       printf("ERROR: Object Rx configuration failed\r\n");
       goto power_off_canfd;
    }
    /* Setting Object filter of CANFD */
    ret_val = CANFD_instance->ObjectSetFilter(rx_obj_id,
                                              ARM_CAN_FILTER_ID_EXACT_ADD,
                                              CANFD_OBJECT_FILTER_CODE_1,
                                              CANFD_OBJECT_FILTER_MASK);
    if(ret_val == ARM_DRIVER_ERROR_SPECIFIC)
    {
       printf("ERROR: No free Filter available\r\n");
    }
    else if(ret_val != ARM_DRIVER_OK)
    {
       printf("ERROR: Failed to set CANFD Object filter\r\n");
       goto power_off_canfd;
    }

    /* Setting Object filter of CANFD */
    ret_val = CANFD_instance->ObjectSetFilter(rx_obj_id,
                                              ARM_CAN_FILTER_ID_EXACT_ADD,
                                              CANFD_OBJECT_FILTER_CODE_2,
                                              CANFD_OBJECT_FILTER_MASK);
    if(ret_val == ARM_DRIVER_ERROR_SPECIFIC)
    {
       printf("ERROR: No free Filter available\r\n");
    }
    else if(ret_val != ARM_DRIVER_OK)
    {
       printf("ERROR: Failed to set CANFD Object filter\r\n");
       goto power_off_canfd;
    }

    /* Setting CANFD to Normal mode */
    ret_val = CANFD_instance->SetMode(ARM_CAN_MODE_LOOPBACK_INTERNAL);
    if(ret_val != ARM_DRIVER_OK)
    {
       printf("ERROR: Failed to set CANFD to Internal Loopback mode\r\n");
       goto power_off_canfd;
    }

    while(!(stop_execution))
    {
        /* Checks current status is Ok*/
        if(cur_sts)
        {
            cur_sts = false;
            /* Invoke the below function to prepare and send a message */
            if(canfd_transmit_message(msg_type) != false)
            {
                /* wait for receive/error callback. */
                if(xTaskNotifyWait(NULL, CANFD_ALL_NOTIFICATIONS,
                                   &task_notified_value,
                                   portMAX_DELAY) != pdFALSE)
                {
                    /* Checks if both callbacks are successful */
                    if(task_notified_value & CANFD_RX_SUCCESS)
                    {
                        /* Invokes received message process function */
                        cur_sts = canfd_process_rx_message();
                    }
                    else if(task_notified_value & CANFD_ERROR)
                    {
                        /* Invoke the below function to check on errors */
                        canfd_check_error();
                        /* Sets the flag to stop the execution */
                        stop_execution = true;
                    }
                }
                msg_type++;
            }
        }
    }

power_off_canfd:
/* Powering OFF CANFD module */
    if(CANFD_instance->PowerControl(ARM_POWER_OFF) != ARM_DRIVER_OK)
    {
       printf("ERROR in CANFD power off\r\n");
    }

uninitialise_canfd:
    /*  Un-initialising CANFD module */
    if(CANFD_instance->Uninitialize() != ARM_DRIVER_OK)
    {
        printf("ERROR in CANFD un-initialization\r\n");
    }

    /* Disables the HFOSC clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                                              CLKEN_HFOSC,
                                              false,
                                              &service_error_code);
    if(error_code)
    {
        printf("SE Error: HFOSC clk disable = %d\n", (int)error_code);
        return;
    }
    /* Disables the 160MHz clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                                              CLKEN_CLK_160M,
                                              false,
                                              &service_error_code);
    if(error_code)
    {
        printf("SE Error: 160 MHz clk disable = %d\n", (int)error_code);
        return;
    }

    printf("*** CANFD Internal Loopback Demo is ended ***\r\n");

    /* Task delete */
    vTaskDelete( NULL );
}

/**
 * @fn      int main()
 * @brief   main function of the CANFD Application
 * @note    none
 * @param   none
 * @retval  software execution status
 */
int main()
{
    /* System Initialization */
    SystemCoreClockUpdate();

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

   /* Create application main thread */
   BaseType_t xReturned = xTaskCreate(canfd_lbi_demo_task, "CANFD_LBIM_Task",
                                      CANFD_TASK_STACK_SIZE, NULL,
                                      (configMAX_PRIORITIES - 1U),
                                      &canfd_xHandle);
   if(xReturned != pdPASS)
   {
      vTaskDelete(canfd_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();

   return 0;
}

/**
 * @fn      static void canfd_check_error(void)
 * @brief   Checks for the errors in CANFD
 * @note    none
 * @param   none
 * @retval  none
 */
static void canfd_check_error(void)
{
    ARM_CAN_STATUS cur_sts;
    if(error_occurred)
    {
        /* Getting the current CANFD status */
        cur_sts = CANFD_instance->GetStatus();

        printf("Error in CANFD->Error Code:%d\r\n", cur_sts.last_error_code);

        error_occurred = false;
    }
}

/**
 * @fn      static bool canfd_process_rx_message(void)
 * @brief   Processes the received messages
 * @note    none
 * @param   none
 * @retval  none
 */
static bool canfd_process_rx_message(void)
{
    uint8_t iter = 0U;

    if(rx_msg_error)
    {
        printf("Error in reading message \r\n");
        rx_msg_error = false; /* Discard the message */
        stop_execution = true;
        return false;
    }
    else
    {
        /* Checking if a new message is received. If yes
         * performs the below operations */
        if(rx_msg_header.rtr == 1U)
        {
            printf("Rx msg:\r\n    Type:Remote frame, Id:%lu",
                   (rx_msg_header.id & (~ARM_CAN_ID_IDE_Msk)));
        }
        else
        {
            printf("Rx msg:\r\n    Type:Data frame, ");

            /* Checks if expected Rx msg length is equal to actual length */
            if(rx_msg_size == canfd_len_dlc_map[rx_msg_header.dlc])
            {
                /* If transmitted message is matching with received message:
                 * performs the following operation */
                if(memcmp(tx_data, rx_data, rx_msg_size) == 0U)
                {
                    /* If any error is present in the Rx message */
                    if(rx_msg_header.esi)
                    {
                        printf("\r\n    Error Occurred in Rx message \r\n");
                        stop_execution = true;
                        return false;
                    }

                    printf("Id:%lu, Len:%d:\r\n    Data:",
                           (rx_msg_header.id & (~ARM_CAN_ID_IDE_Msk)),
                            rx_msg_size);
                    for(iter = 0; iter < rx_msg_size; iter++)
                    {
                        printf("%c", rx_data[iter]);
                    }
                }
                else
                {
                    printf("\r\n    Error: Tx and Rx message mismatch \r\n");
                    stop_execution = true;
                    return false;
                }
            }
            else
            {
                printf("\r\n    Error: Rx msg length is not as expected\r\n");
                stop_execution = true;
                return false;
            }
        }
        printf("\r\n");

        if(rx_buf_overrun)
        {
            printf("Error: Receiver buffer overrun \r\n");
            rx_buf_overrun = false;
        }
    }
    return true;
}

/**
 * @fn      static bool canfd_transmit_message(CANFD_FRAME msg_type)
 * @brief   Prepares and sends message
 * @note    none
 * @param   msg_type : Type of msg to send
 * @retval  none
 */
static bool canfd_transmit_message(const CANFD_FRAME msg_type)
{
    int32_t status = ARM_DRIVER_OK;
    uint8_t iter   = 0U;

    switch(msg_type)
    {
        case CANFD_FRAME_STD_ID_CLASSIC_DATA:
            /* Sending Classic CAN DATA message of
             * length 5 bytes with Message Id 0x5A5 */
            tx_msg_header.brs = 0x0U;
            tx_msg_header.dlc = 0x5U;
            tx_msg_header.id  = 0x5A5U;
            tx_msg_header.rtr = 0x0U;
            tx_msg_header.edl = 0x0U;
            tx_msg_size       = 0x5U;
            break;
        case CANFD_FRAME_STD_ID_RTR:
            /* Sending Classic CAN Remote request message
             * with Message Id 0x5A5 */
            tx_msg_header.brs = 0x0U;
            tx_msg_header.dlc = 0x0U;
            tx_msg_header.id  = 0x5A5U;
            tx_msg_header.rtr = 0x1U;
            tx_msg_header.edl = 0x0U;
            tx_msg_size       = 0x0U;
            break;
        case CANFD_FRAME_STD_ID_FD_DATA:
            /* Sending FD CAN DATA message of
             * length 64 bytes with Message Id 0x5A5 */
            tx_msg_header.brs = 0x1U;
            tx_msg_header.dlc = 0xFU;
            tx_msg_header.id  = 0x5A5U;
            tx_msg_header.rtr = 0x0U;
            tx_msg_header.edl = 0x1U;
            tx_msg_size       = 0x40U;
            break;
        case CANFD_FRAME_EXT_ID_RTR:
            /* Sending Classic CAN Remote request message
             * with Extended Message Id 0x1FF5A5AU */
            tx_msg_header.brs = 0x0U;
            tx_msg_header.dlc = 0x0U;
            tx_msg_header.id  = 0x81FF5A5AU;
            tx_msg_header.rtr = 0x1U;
            tx_msg_header.edl = 0x0U;
            tx_msg_size       = 0x0U;
            break;
        case CANFD_FRAME_EXT_ID_CLASSIC_DATA:
            /* Sending Classic CAN data message of
             * length 8 bytes with Extended Message Id 0x1FF5A5AU */
            tx_msg_header.brs = 0x0U;
            tx_msg_header.dlc = 0x8U;
            tx_msg_header.id  = 0x81FF5A5AU;
            tx_msg_header.rtr = 0x0U;
            tx_msg_header.edl = 0x0U;
            tx_msg_size       = 0x8U;
            break;
        case CANFD_FRAME_EXT_ID_FD_DATA:
            /* Sending FD CAN message of length 16 bytes
             * with Extended Message Id 0x1FF5A5AU */
            tx_msg_header.brs = 0x1U;
            tx_msg_header.dlc = 0xAU;
            tx_msg_header.id  = 0x81FF5A5AU;
            tx_msg_header.rtr = 0x0U;
            tx_msg_header.edl = 0x1U;
            tx_msg_size       = 0x10U;
            break;

        case CANFD_FRAME_OVER:
        default:
            stop_execution = true;
            return false;
    }
    /* Sends the message to CAN HAL Driver */
    status = CANFD_instance->MessageSend(tx_obj_id, &tx_msg_header,
                                         tx_data, tx_msg_size);
    if(status == ARM_DRIVER_OK)
    {
        rx_msg_size = tx_msg_size;
        printf("Tx Msg:\r\n    Id:%lu, Len:%d: \r\n    Data:",
                (tx_msg_header.id & (~ARM_CAN_ID_IDE_Msk)), tx_msg_size);
        for(iter = 0; iter < tx_msg_size; iter++)
        {
            printf("%c", tx_data[iter]);
        }
        printf("\r\n");
    }
    else
    {
        printf("Error: Failed to send message \n");
        stop_execution = true;
        return false;
    }
    return true;
}
