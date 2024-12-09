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
 * @file   fifo.h
 * @author Artur Tynecki
 * @date   May, 2017
 * @brief  FIFO implementation header file
 *
 * Simple FIFO implementation with the ability to create a buffer of any size
 * and thread safe operation on it.
 */

#ifndef _FIFO_H_
#define _FIFO_H_

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include <stdint.h>
#include "ReturnCodes.h"

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/

/**
 * @brief FIFO data structure
*/
typedef struct {
    uint32_t head;                                   /**< Buffer head index */
    uint32_t tail;                                   /**< Buffer tail index */
    uint32_t size;                                   /**< Buffer size */
    uint32_t length;                                 /**< Buffer data length */
    uint8_t *buffer;                                 /**< Data buffer */
} fifo_t, *fifo_p;

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

/**
 * @brief FIFO Create

 * Initialization FIFO structure, allocate memeory, set default parameters value
 * @param [in] size - FIFO buffer size
 * @return pointer to initialized fifo structure
 */
fifo_p fifoCreate(uint32_t size);

/**
 * @brief FIFO Destroy

 * Deinitialization FIFO structure, free memeory
 * @param [in] fifo - pointer to fifo structure
 * @return none
 */
void fifoDestroy(fifo_p fifo);

/**
 * @brief Write data to FIFO

 * Write data to FIFO buffer, thread safe
 * @param [out] fifo - pointer to fifo structure
 * @param [in] data - write data pointer
 * @param [in] data_len - write data size in bytes
 * @param [out] data_len_out - pointer to number of bytes which are properly write
 * @return FLR_OK if successful else return error
 */
FLR_RESULT fifoWrite(fifo_p fifo, uint8_t* data, uint32_t data_len, uint32_t* data_len_out);

/**
 * @brief Read data from fifo

 * Read data from FIFO buffer, thread safe
 * @param [out] fifo - pointer to fifo structure
 * @param [in] data - read data pointer
 * @param [in] data_len - read data size in bytes
 * @param [out] data_len_out - pointer to number of bytes which are properly read
 * @return FLR_OK if successful else return error
 */
FLR_RESULT fifoRead(fifo_p fifo, uint8_t* data, uint32_t data_len, uint32_t* data_len_out);

#endif // _FIFO_H_
