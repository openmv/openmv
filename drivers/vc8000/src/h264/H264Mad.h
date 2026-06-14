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
 *  Description : MAD threshold calculation
 *
 ********************************************************************************
 */

#ifndef H264_MAD_H
#define H264_MAD_H

#include "enccommon.h"

#define DSCY                      32 /* n * 32 */
#define I32_MAX           2147483647 /* 2 ^ 31 - 1 */
#define DIV(a, b)       (((a) + (SIGN(a) * (b)) / 2) / (b))
#define MAD_TABLE_LEN              5

typedef struct {
    i32  a1;               /* model parameter, y = a1*x + a2 */
    i32  a2;               /* model parameter */
    i32  th[MAD_TABLE_LEN];     /* mad threshold */
    i32  count[MAD_TABLE_LEN];  /* number of macroblocks under threshold */
    i32  pos;              /* current position */
    i32  len;              /* current lenght */
    i32  threshold[3];     /* current frame thresholds: high/mid/low */
    i32  mbPerFrame;       /* number of macroblocks per frame */
} madTable_s;

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

void H264MadInit(madTable_s *mad, u32 mbPerFrame);

void H264MadThreshold(madTable_s *madTable, u32 *madCount);

#endif

