/*
 * Copyright (c) 2015-2022, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 *
 ********************************************************************************
 *
 * This software is distributed under the terms of
 * BSD-3-Clause. The following provisions apply :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 *
 *  Abstract  : 
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------

    Table of contents 
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef __ENC_JPEG_MARKERS_H__
#define __ENC_JPEG_MARKERS_H__

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/* JPEG markers, table B.1 page 32 */

enum
{
    SOI = 0xFFD8,   /* Start of Image                    */
    DQT = 0xFFDB,   /* Define Quantization Table(s)      */
    SOF0 = 0xFFC0,  /* Start of Frame                    */
    DRI = 0xFFDD,   /* Define Restart Interval           */
    RST0 = 0xFFD0,  /* Restart marker 0                  */
    RST1 = 0xFFD1,  /* Restart marker 1                  */
    RST2 = 0xFFD2,  /* Restart marker 2                  */
    RST3 = 0xFFD3,  /* Restart marker 3                  */
    RST4 = 0xFFD4,  /* Restart marker 4                  */
    RST5 = 0xFFD5,  /* Restart marker 5                  */
    RST6 = 0xFFD6,  /* Restart marker 6                  */
    RST7 = 0xFFD7,  /* Restart marker 7                  */
    DHT = 0xFFC4,   /* Define Huffman Table(s)           */
    SOS = 0xFFDA,   /* Start of Scan                     */
    EOI = 0xFFD9,   /* End of Image                      */
    APP0 = 0xFFE0,  /* APP0 Marker                       */
    COM = 0xFFFE    /* Comment marker                    */
};

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

#endif
