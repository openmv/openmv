/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2017, FLIR Systems                                          */
/*  All rights reserved.                                                      */
/*                                                                            */
/*  This document is controlled to FLIR Technology Level 2. The information   */
/*  contained in this document pertains to a dual use product controlled for  */
/*  export by the Export Administration Regulations (EAR). Diversion contrary */
/*  to US law is prohibited. US Department of Commerce authorization is not   */
/*  required prior to export or transfer to foreign persons or parties unless */
/*  otherwise prohibited.                                                     */
/*                                                                            */
/******************************************************************************/

/**
 * @file   dataServiceClient.c
 * @author Artur Tynecki
 * @date   April, 2017
 * @brief  Data Service Client source file
 *
 */

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include <string.h>
#include <stdio.h>

#include "UART_Connector.h"
#include "dataServiceClient.h"
#include "fifo.h"

#include "py/mphal.h"

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/
#define DO_TRACE(s, ...)                //printf("%s: " s, __FUNCTION__, __VA_ARGS__)
#define DO_INIT_TRACE(s, ...)           //printf("%s: " s, __FUNCTION__, __VA_ARGS__)
#define DO_DATA_SERVICE_TRACE(s, ...)   //printf("%s: " s, __FUNCTION__, __VA_ARGS__)
#define DEBUG_ERR(s, ...)               //printf("%s: " s, __FUNCTION__, __VA_ARGS__)
#define DEBUG_WARN(s, ...)              //printf("%s: " s, __FUNCTION__, __VA_ARGS__)

/******************************************************************************/
/** LOCAL DATA                                                               **/
/******************************************************************************/
static dataServiceController_t controller;
static dataServiceFrame_t sendFrame;
static dataServiceFrame_t receiveFrame;
static fifo_p receiveBuffer;

/******************************************************************************/
/** LOCAL FUNCTIONS DECLARATIONS                                             **/
/******************************************************************************/
static FLR_RESULT parseData(uint8_t *data, uint32_t data_len);

/******************************************************************************/
/** FSLP SERVICE FUNCTIONS                                                   **/
/******************************************************************************/
/**
 * @brief Send data by HostBinaryProtocol

 * @param [in] data - send data pointer
 * @param [in] data_len - send data size in bytes
 * @return FLR_OK if successful else return error
 */
static FLR_RESULT sendData(uint8_t* data, uint32_t data_len)
{
    if(!data || !data_len)
    {
        DEBUG_ERR("Incorrect arguments\n");
        return FLR_ERROR;
    }

    SendToCamera(DATA_SERVICE_NUMBER, data_len, data);

    DO_TRACE("Send data %d bytes:  [0x%02x | 0x%02x ...]\n",
                   data_len, data[0], data[1]);

    return FLR_OK;
}

/******************************************************************************/
/** DATA SERVICE CLIENT FUNCTIONS                                            **/
/******************************************************************************/
/**
 * @brief Reset Data Service Client controller

 * Set default value in controller structure
 * @return none
 */
static void resetController(void)
{
    controller.send_counter = 0;
    controller.receive_counter = 0;
    controller.state = CONTROLLER_READY;
}

/**
 * @brief Initialization Data Service Client controller

 * Reset controller and callback function
 * @return none
 */
static void initController(void)
{
    resetController();
    controller.callback = NULL;
}

/**
 * @brief Prepare Data Service Client frame

 * Prepare frame by set command, counter and data
 * @param [in] cmd - command code
 * @param [in] counter - frame counter
 * @param [in] data - data buffer
 * @param [in] data_len - data size in bytes
 * @return none
 */
static void prepareFrame(dataServiceFrameCommand_t cmd, uint16_t counter, uint8_t* data, uint32_t data_len)
{
    uint8_t counter_lsb, counter_msb;

    counter_msb = (uint8_t)(counter >> 8);
    counter_lsb = (uint8_t)(counter);

    sendFrame.buffer[0] = (cmd << 6) | (counter_msb & COUNTER_MASK);
    sendFrame.buffer[1] = counter_lsb;

    if(data_len > 0 && data != NULL)
    {
        memcpy(&sendFrame.buffer[2], data, data_len);
    }
    sendFrame.size = data_len + 2;
}

/**
 * @brief Send ACK

 * Prepare and send ACK frame
 * @param [in] counter - number of receive frame
 * @return FLR_OK if successful else return error
 */
static FLR_RESULT sendAck(uint16_t counter)
{
    prepareFrame(DATA_ACK, counter, NULL, 0);

    return sendData(sendFrame.buffer, sendFrame.size);
}

/**
 * @brief Calculate counter distance

 * @param [in] sender - counter value from sender
 * @param [in] sender - counter value from recipient
 * @return distance value
 */
static int calculateDistance(uint16_t sender, uint16_t recipient)
{
    sender-= (sender>(MAX_COUNTER_VALUE/2) ? (MAX_COUNTER_VALUE/2) : 0); \
    recipient-= (recipient>(MAX_COUNTER_VALUE/2) ? (MAX_COUNTER_VALUE/2) : 0); \
    return (int)sender-(int)recipient;
}

/**
 * @brief Parse received data

 * Parse received data: check command code, control received counter number,
 * call right function
 * @param [in] data - data buffer
 * @param [in] data_len - data size in bytes
 * @return FLR_OK if successful else return error
 */
static FLR_RESULT parseData(uint8_t *data, uint32_t data_len)
{
    uint32_t data_len_out;
    int counter_distance;

    if(!data || (data_len < 2))
    {
        DEBUG_ERR("Incorrect arguments\n");
        return FLR_ERROR;
    }

    /** parse received data */
    receiveFrame.cmd = (data[0] & CMD_MASK)>>6;
    receiveFrame.counter = ((uint16_t)(data[0] & COUNTER_MASK)) << 8 | (uint16_t) (data[1]);
    if(data_len > 2)
    {
        receiveFrame.size = data_len - 2;
    }

    switch(receiveFrame.cmd)
    {
        case DATA_SEND:
            /** calculate counter distance */
            counter_distance = calculateDistance(receiveFrame.counter, controller.receive_counter);

            /** received previous frame - ACK resend */
            if(counter_distance < 0)
            {
                if(sendAck(receiveFrame.counter) != FLR_OK)
                {
                    DEBUG_ERR("Send ACK error occurred\n");
                    return FLR_ERROR;
                }

                return FLR_OK;
            }
            /** received next frame: save data in buffer, send ACK, increment counter, run callback  */
            else if(counter_distance == 0)
            {
                if(fifoWrite(receiveBuffer, &data[2], receiveFrame.size, &data_len_out) != FLR_OK)
                {
                    DEBUG_ERR("Write to receive buffer error occurred\n");
                    return FLR_ERROR;
                }

                if(sendAck(receiveFrame.counter) != FLR_OK)
                {
                    DEBUG_ERR("Send ACK error occurred\n");
                    return FLR_ERROR;
                }

                controller.receive_counter = (controller.receive_counter+1)%MAX_COUNTER_VALUE;

                if(controller.callback != NULL)
                {
                    controller.callback(data_len_out);
                }

                return FLR_OK;
            }
            /* synchronization error */
            else
            {
                DEBUG_ERR("Synchronization error - incorrect counter distance [%d]\n", counter_distance);
                return FLR_ERROR;
            }

        case DATA_ACK:
            /** compare received counter with send counter if the same increment send counter */
            if(receiveFrame.counter == controller.send_counter)
            {
                controller.send_counter = (controller.send_counter+1)%MAX_COUNTER_VALUE;
                controller.state = CONTROLLER_READY;

                return FLR_OK;
            }
            /* synchronization error */
            else
            {
                DEBUG_ERR("Synchronization error\n");
                return FLR_ERROR;
            }

        case COMMUNICATION_RESET_ACK:
            controller.state = CONTROLLER_READY;
            return FLR_OK;

        default:
            DEBUG_ERR("Unknown frame status\n");
            return FLR_ERROR;
    }
}

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

/******************************************************************************/
FLR_RESULT dataServiceClientInit(void)
{
    initController();
    receiveBuffer = fifoCreate(DATA_BUFFER_SIZE);
    if(receiveBuffer == NULL)
    {
        DEBUG_ERR("Initialization receive buffer error occurred\n");
        return FLR_ERROR;
    }

    DO_INIT_TRACE("Initialization success\n");
    return FLR_OK;
}

/******************************************************************************/
FLR_RESULT dataServiceClientRegisterCallback(receive_callback callback)
{
    if(!callback)
    {
       DEBUG_ERR("Incorrect arguments - null pointer to callback\n");
       return FLR_ERROR;
    }
    controller.callback = callback;

    return FLR_OK;
}

/******************************************************************************/
FLR_RESULT dataServiceClientSend(uint8_t* data, uint32_t data_len, uint32_t* data_len_out)
{
    uint32_t timeout = 0;

    if(!data || !data_len || !data_len_out)
    {
        DEBUG_ERR("Incorrect arguments\n");
        return FLR_ERROR;
    }

    *data_len_out = 0;

    if(data_len > MAX_PAYLOAD_BYTES - 3)
    {
       data_len = MAX_PAYLOAD_BYTES - 3;
    }

    prepareFrame(DATA_SEND, controller.send_counter, data, data_len);

    if(sendData(sendFrame.buffer, sendFrame.size) != FLR_OK)
    {
        DEBUG_ERR("Send data error occurred\n");
        return FLR_ERROR;
    }

    controller.state = CONTROLLER_ACK_WAIT;

    /** wait for ACK */
    while ((controller.state == CONTROLLER_ACK_WAIT) && (timeout < DATA_SEND_TIMEOUT_MS))
    {
        timeout++;
        mp_hal_delay_ms(1);
    }

    /** ACK received */
    if(controller.state == CONTROLLER_READY)
    {
        *data_len_out = data_len;
    }
    /** timeout */
    else
    {
       controller.state = CONTROLLER_READY;
       DO_TRACE("Send data timeout\n");
       return FLR_COMM_TIMEOUT_ERROR;
    }

    return FLR_OK;
}

/******************************************************************************/
FLR_RESULT dataServiceClientRead(uint8_t* data, uint32_t data_len, uint32_t* data_len_out)
{
    if(!data || !data_len || !data_len_out)
    {
       DEBUG_ERR("Incorrect arguments\n");
        return FLR_ERROR;
    }

    if(fifoRead(receiveBuffer, data, data_len, data_len_out) != FLR_OK)
    {
        DEBUG_ERR("Read from receive buffer error occurred\n");
        return FLR_ERROR;
    }

    return FLR_OK;
}

/******************************************************************************/
FLR_RESULT dataServiceClientReset(void)
{
    uint32_t timeout = 0;

    prepareFrame(COMMUNICATION_RESET, 0, NULL, 0);

    if(sendData(sendFrame.buffer, sendFrame.size) != FLR_OK)
    {
        DEBUG_ERR("Send data error occurred\n");
        return FLR_ERROR;
    }

    controller.state = CONTROLLER_ACK_WAIT;

    while ((controller.state == CONTROLLER_ACK_WAIT) && (timeout < DATA_SEND_TIMEOUT_MS))
    {
        timeout++;
        mp_hal_delay_ms(1);
    }

    if(controller.state == CONTROLLER_READY)
    {
        resetController();
    }
    else
    {
       controller.state = CONTROLLER_READY;
       DO_TRACE("Send data timeout\n");
       return FLR_COMM_TIMEOUT_ERROR;
    }

    return FLR_OK;
}

/******************************************************************************/
int32_t dataServiceClientReceive(uint32_t receiveBytes, const uint8_t* receiveData)
{
    uint8_t fslpData[MAX_PAYLOAD_BYTES];
    uint32_t fslpDataSize = 0;

    if (!receiveData || (receiveBytes < 2))
    {
         DEBUG_ERR("Incorrect arguments\n");
         return -1;
    }

    ReadFrame(DATA_SERVICE_NUMBER, &fslpDataSize, fslpData);

    DO_TRACE("Received data %d bytes:  [0x%02x | 0x%02x ...]\n",
                   fslpDataSize, fslpData[0], fslpData[1]);

    if(parseData(fslpData, fslpDataSize) != FLR_OK)
    {
        DEBUG_ERR("Parse data error occurred\n");
        return -1;
    }

    return 0;
}
