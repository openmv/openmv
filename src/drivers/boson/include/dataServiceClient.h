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
 * @file   dataServiceClient.h
 * @author Artur Tynecki
 * @date   April, 2017
 * @brief  Data Service Client header file
 *
 * Includes function used to send with confirmation data packets.
 * Communcation base on HostBinaryProtocol module. Confirmation base on send ACK
 * and control packet counter.
 * Implementation:
 * 1. call dataServiceClientInit() function during system initialization
 * 2. register receive data callback by dataServiceClientRegisterCallback() function
 * 3. register receive HostBinaryProtcol callback by set dataServiceClientReceive()
 *    function as an argument in RegisterServiceDataCallback() call
 * 3. send your data by dataServiceClientSend() function
 * 4. on data receive: callback function with number of receive bytes
 *    will be called. Read receive data buffer by dataServiceClientRead() function
 * 5. reset communication by call dataServiceClientReset()
 */

#ifndef _DATA_SERVICE_H
#define _DATA_SERVICE_H

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include <stdint.h>
#include "ReturnCodes.h"

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/
#define DATA_SERVICE_NUMBER             1
#define MAX_PAYLOAD_BYTES               768

#define CMD_MASK                        0xC0
#define COUNTER_MASK                    0x3F
#define MAX_COUNTER_VALUE               0x3FFF
#define DATA_SEND_TIMEOUT_MS            3000
#define DATA_BUFFER_SIZE                UINT16_MAX

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/
/**
 * @brief definition of callback function type
 * @param [in] number of received bytes
*/
typedef void (*receive_callback) (uint32_t);

/**
 * @brief Frame command definition
*/
typedef enum {
    DATA_SEND = (uint8_t) 0,    ///< send data
    DATA_ACK = (uint8_t) 1,     ///< send ACK response
    COMMUNICATION_RESET = (uint8_t) 2,  ///< reset communication
    COMMUNICATION_RESET_ACK = (uint8_t) 3
}dataServiceFrameCommand_t;

/**
 * @brief Controller state definition
*/
typedef enum {
    CONTROLLER_READY = (uint8_t) 0,     ///< controller is ready for action
    CONTROLLER_ACK_WAIT = (uint8_t) 1   ///< controller is busy
}dataServiceControllerState_t;

/**
 * @brief Data Service Client controller structure
*/
typedef struct {
    uint16_t send_counter;          ///< Send data counter
    uint16_t receive_counter;       ///< Received data counter
    dataServiceControllerState_t state;     /**< Controller state */
    receive_callback callback;              /**< Received data callback */
} dataServiceController_t, *dataServiceController_p;

/**
 * @brief Data Service Client frame structure
*/
typedef struct {
    dataServiceFrameCommand_t cmd;          /**< Command code */
    uint16_t counter;                       /**< Frame counter */
    uint8_t buffer[MAX_PAYLOAD_BYTES];      /**< Frame buffer */
    uint32_t size;                          /**< Frame buffer size */
} dataServiceFrame_t, *dataServiceFrame_p;

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

/**
 * @brief Initialization Data Service Client

 * Initalization service controller and receive data buffer
 * @return FLR_OK if successful else return error
 */
FLR_RESULT dataServiceClientInit(void);

/**
 * @brief Register receive data callback

 * Add callback that is called when new data receive on Data Service
 * @param [in] callback - callback function definition
 * @return FLR_OK if successful else return error
 */
FLR_RESULT dataServiceClientRegisterCallback(receive_callback callback);

/**
 * @brief Data send

 * @param [in] data - send data buffer
 * @param [in] data_len - send data size in bytes
 * @param [out] data_len_out - pointer to number of bytes which are properly sent
 * @return FLR_OK if successful else return error
 */
FLR_RESULT dataServiceClientSend(uint8_t* data, uint32_t data_len, uint32_t* data_len_out);

/**
 * @brief Data read

 * @param [out] data - read data buffer
 * @param [in] data_len - number of bytes to read
 * @param [out] data_len_out - pointer to number of bytes which are properly read
 * @return FLR_OK if successful else return error
 */
FLR_RESULT dataServiceClientRead(uint8_t* data, uint32_t data_len, uint32_t* data_len_out);

/**
 * @brief Reset Data Service communication

 * Send reset command and reset controller parameters after ACK received
 * @return FLR_OK if successful else return error
 */
FLR_RESULT dataServiceClientReset(void);

/**
 * @brief Receive data HostBinaryProtocol callback

 * Callback that is called from BinaryProtcol module
 * when new data receive on Data Service Client
 * Add by set as an argument in RegisterServiceDataCallback() call
 * @param [in] receiveBytes - number of receive bytes
 * @param [in] receiveData - pointer to data buffer
 * @return 0 if successful else return -1
 */
int32_t dataServiceClientReceive(uint32_t receiveBytes, const uint8_t* receiveData);

#endif // _DATA_SERVICE_H
