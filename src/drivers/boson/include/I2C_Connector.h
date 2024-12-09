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

#ifndef I2C_CONNECTOR_H
#define I2C_CONNECTOR_H

#include <stdint.h>
#include "ReturnCodes.h"
#include <stdbool.h>

#define I2C_SLAVE_CP_FRAME_HEAD_SIZE        2
#define I2C_SALVE_CP_FRAME_BYTES_NUM_SIZE   2
#define I2C_SLAVE_CP_FRAME_HEADER_SIZE      (I2C_SLAVE_CP_FRAME_HEAD_SIZE + I2C_SALVE_CP_FRAME_BYTES_NUM_SIZE)

FLR_RESULT I2C_readFrame(uint8_t* readData, uint32_t* readBytes);
FLR_RESULT I2C_writeFrame(uint8_t* writeData, uint32_t writeBytes);

#endif // I2C_CONNECTOR_H
