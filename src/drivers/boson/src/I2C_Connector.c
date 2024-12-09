/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2018, FLIR Systems                                          */
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

#include "I2C_Connector.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern FLR_RESULT I2C_read(uint8_t* readData, uint32_t readBytes);
extern FLR_RESULT I2C_write(uint8_t* writeData, uint32_t writeBytes);

static uint8_t frameHead[I2C_SLAVE_CP_FRAME_HEAD_SIZE] = {0x8E, 0xA1};

static void addToShiftBuffer(uint8_t* buffer, uint32_t bufferSize, uint8_t value)
{
    for(uint32_t index = 0; index < (bufferSize - 1); ++index)
    {
        buffer[index] = buffer[index + 1];
    }
    buffer[bufferSize - 1] = value;
}

FLR_RESULT I2C_readFrame(uint8_t* readData, uint32_t* readBytes)
{
    bool frameNotReady = true, headerFound = false;
    uint8_t retByte;
    uint8_t* readBuffer;
    uint32_t bytesNumber, readSize;
    uint8_t headerBuffer[I2C_SLAVE_CP_FRAME_HEADER_SIZE];

    if(readData == NULL || readBytes == NULL)
    {
        return FLR_BAD_ARG_POINTER_ERROR;
    }

    readBuffer = &retByte;
    readSize = 1;
    do
    {
        if(I2C_read(readBuffer, readSize) != FLR_OK)
        {
            return FLR_COMM_ERROR_READING_COMM;
        }

        if(headerFound)
        {
            *readBytes = readSize;
            frameNotReady = false;
        }
        else
        {
            addToShiftBuffer(headerBuffer, I2C_SLAVE_CP_FRAME_HEADER_SIZE, retByte);
            if(memcmp(headerBuffer, frameHead, I2C_SLAVE_CP_FRAME_HEAD_SIZE) == 0)
            {
                bytesNumber = ((uint32_t)headerBuffer[I2C_SLAVE_CP_FRAME_HEAD_SIZE]) << 8 | (uint32_t)headerBuffer[I2C_SLAVE_CP_FRAME_HEAD_SIZE + 1];
                readSize = bytesNumber;
                readBuffer = readData;
                headerFound = true;
            }
        }
    } while(frameNotReady);

    return FLR_OK;
}

FLR_RESULT I2C_writeFrame(uint8_t* writeData, uint32_t writeBytes)
{
    if(writeData == NULL)
    {
        return FLR_BAD_ARG_POINTER_ERROR;
    }

    if(writeBytes < 1)
    {
        return FLR_ERROR;
    }

    uint8_t sendFrame[writeBytes + I2C_SLAVE_CP_FRAME_HEADER_SIZE];
    uint8_t* ptr = sendFrame;

    memcpy(ptr, frameHead, I2C_SLAVE_CP_FRAME_HEAD_SIZE);
    ptr += I2C_SLAVE_CP_FRAME_HEAD_SIZE;
    *ptr = (uint8_t)((writeBytes >> 8) & 0xFF);
    ptr++;
    *ptr = (uint8_t)(writeBytes & 0xFF);
    ptr++;
    memcpy(ptr, writeData, writeBytes);

    ptr = sendFrame;
    if(I2C_write(ptr, sizeof(sendFrame)) != FLR_OK)
    {
            return FLR_COMM_ERROR_WRITING_COMM;
    }

    return FLR_OK;
}
