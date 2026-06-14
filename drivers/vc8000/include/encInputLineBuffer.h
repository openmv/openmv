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
 *  Abstract :  For test/fpga_verification/example purpose. operations on Input 
 *  line buffer.
 *
 ********************************************************************************
 */

#ifndef ENC_INPUTLINEBUFREGISTER_H
#define ENC_INPUTLINEBUFREGISTER_H

#include "basetype.h"

typedef u32 (*getHEncRdMbLines)(const void *inst);
typedef i32 (*setHEncWrMbLines)(const void *inst, u32 lines);

typedef struct
{
    u8 *buf;
    ptr_t busAddress;
} lineBufMem;

/* struct for input mb line buffer */
typedef struct
{
    /* src picture related pointers */
    u8 *src; /* source buffer */
    u8 *lumSrc;
    u8 *cbSrc;
    u8 *crSrc;

    /* line buffer related pointers */
    u8 *buf; /* line buffer virtual address */
    u32 *reg; /* virtual address of registers in line buffer, only for fpga verification purpose */
    ptr_t busAddress; /* line buffer bus address */
    lineBufMem lumBuf; /*luma address in line buffer */
    lineBufMem cbBuf; /*cb address in line buffer */
    lineBufMem crBuf; /*cr address in line buffer */   

    /* encoding parameters */
    u32 inputFormat; /* format of input video */
    u32 pixOnRow; /* pixels in one line */
    u32 encWidth;
    u32 encHeight;
    u32 srcHeight;
    u32 srcVerOffset;

    /* parameters of line buffer mode */
    i32 wrCnt;
    u32 depth;       /* number of MB_row lines in the input line buffer */
    u32 loopBackEn;
    u32 hwHandShake;

    /*functions */
    getHEncRdMbLines getMbLines; /* get read mb lines from encoder register */
    setHEncWrMbLines setMbLines; /* set written mb lines to encoder register */

    /* encoder instance */
    void *inst;
}inputLineBufferCfg;

#ifdef PCIE_FPGA_VERI_LINEBUF
#include "encswhwregisters.h"
/* HW Register field names */
typedef enum {
    InputlineBufWrCntr,
    InputlineBufDepth,
    InputlineBufHwHandshake,
    InputlineBufPicHeight,
    InputlineBufRdCntr,
} lineBufRegName;

#define LINE_BUF_SWREG_AMOUNT   4 /*4x 32-bit*/

static const regField_s lineBufRegisterDesc[] = {
/* HW ID register, read-only */
    {InputlineBufWrCntr       , 0x000, 0x000001ff,  0, 0, RW, "slice_wr_cntr. +slice_depth when one slice is filled into slice_fifo"},
    {InputlineBufDepth        , 0x000, 0x0003fe00,  9, 0, RW, "slice_depth. unit is MB line"},
    {InputlineBufHwHandshake  , 0x000, 0x00040000, 18, 0, RW, "slice_hw_mode_en. active high. enable bit of slice_fifo hardware mode. should be disabled before the start of next frame."},
    {InputlineBufPicHeight    , 0x000, 0x0ff80000, 19, 0, RW, "pic_height. same value of swreg14[18:10] in H1."},
    {InputlineBufRdCntr       , 0x008, 0x000001ff,  0, 0, RO, "slice_rd_cntr. read only"},
};
#endif

void HEncInitInputLineBufSrcPtr (inputLineBufferCfg *lineBufCfg);
void HEncInitInputLineBufPtr (inputLineBufferCfg *lineBufCfg);
i32 HEncInitInputLineBuffer(inputLineBufferCfg *lineBufCfg, const void *ewl);
void HEncStartInputLineBuffer(inputLineBufferCfg * lineBufCfg);
void HEncInputMBLineBufDone (void *pAppData);

#endif

