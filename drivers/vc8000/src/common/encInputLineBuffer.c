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
#include <stdio.h>
#include "enccommon.h"
#include "ewl.h"
#include "encInputLineBuffer.h"
#include "H264Instance.h"

#ifdef PCIE_FPGA_VERI_LINEBUF
/* FPGA verification. get/set register value of input-line-buffer */
static u32 getInputLineBufReg (u32 *reg, u32 name)
{
    u32 value = (reg[lineBufRegisterDesc[name].base/4]&lineBufRegisterDesc[name].mask)>>lineBufRegisterDesc[name].lsb;
    return value;
}
static void setInputLineBufReg (u32 *reg, u32 name, u32 value)
{
    reg[lineBufRegisterDesc[name].base/4]=
        (reg[lineBufRegisterDesc[name].base/4] & ~(lineBufRegisterDesc[name].mask)) |
        ((value << lineBufRegisterDesc[name].lsb) & lineBufRegisterDesc[name].mask);
}
#endif

static u32 getMbLinesRdCnt (inputLineBufferCfg *cfg)
{
    u32 rdCnt = 0;
#ifdef PCIE_FPGA_VERI_LINEBUF
    if (cfg->hwHandShake)
    {
        rdCnt = getInputLineBufReg(cfg->reg, InputlineBufRdCntr);
        
        /* frame end, disable hardware handshake */
        //if ((rdCnt*16) >= cfg->encHeight)
            //setInputLineBufReg(cfg->reg,InputlineBufHwHandshake, 0);
    }
    else
#endif
    if (cfg->getMbLines)
        rdCnt = cfg->getMbLines(cfg->inst);

    return rdCnt;
}

static void DumpInputLineBufReg(inputLineBufferCfg *cfg)
{
#ifdef PCIE_FPGA_VERI_LINEBUF
    u32 *reg = cfg->reg;
    i32 i;
    printf ("==== SRAM HW-handshake REGISTERS ====\n");
    for (i = 0; i < LINE_BUF_SWREG_AMOUNT; i ++)
      printf ("     %08x: %08x\n", i*4, reg[i]);

    printf ("     rdCnt=%d, wrCnt=%d, depth=%d, hwMode=%d, picH=%d\n",
       getInputLineBufReg(reg, InputlineBufRdCntr),
       getInputLineBufReg(reg, InputlineBufWrCntr),
       getInputLineBufReg(reg, InputlineBufDepth),
       getInputLineBufReg(reg, InputlineBufHwHandshake),
       getInputLineBufReg(reg, InputlineBufPicHeight));
#endif
}

static void DumpH264LowLatencyReg(inputLineBufferCfg *cfg)
{
   h264Instance_s *pEncInst = (h264Instance_s *)(cfg->inst);

   printf ("==== ASIC Low-Latency Regs: %08x. rdCnt=%d, wrCnt=%d, depth=%d, En=%d, loopBack=%d, hwMode=%d, IrqEn=%d\n",
      EWLReadReg(pEncInst->asic.ewl, BASE_HEncInstantInput),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncMbRdPtr),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncMbWrPtr),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncLowLatencyHwSyncMbRows),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncLowLatencyEn),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncInputBufferLoopBackEn),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncLowLatencyHwIntfEn),
      EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncLowLatencyIrqEn));
}

static void setMbLinesWrCnt (inputLineBufferCfg *cfg)
{
    if (cfg->hwHandShake)
    {
#ifdef PCIE_FPGA_VERI_LINEBUF
        setInputLineBufReg(cfg->reg,InputlineBufWrCntr,cfg->wrCnt);
#endif
    }
    else if (cfg->setMbLines)
        cfg->setMbLines(cfg->inst, cfg->wrCnt);
}

/*------------------------------------------------------------------------------
    copyLineBuf
------------------------------------------------------------------------------*/
static void copyLineBuf (u8 *dst, u8 *src, i32 width, i32 height, u32 offset, u32 depth)
{
    i32 i, j;
    if (dst && src)
    {
        for (i = 0; i < height; i ++)
        {
            i32 srcOff = i + offset;
            i32 dstOff = (i + offset) % depth;
            u32 *dst32 = (u32 *)(dst + dstOff*width);
            u32 *src32 = (u32 *)(src + srcOff*width);
            for (j = 0; j < (width/4); j ++)
                dst32[j] = src32[j];
        }
    }
}

/*------------------------------------------------------------------------------
    writeInputLineBuf
------------------------------------------------------------------------------*/
static void writeInputLineBuf (inputLineBufferCfg *cfg, i32 lines)
{
    u8 *lumSrc = cfg->lumSrc;
    u8 *cbSrc  = cfg->cbSrc;
    u8 *crSrc  = cfg->crSrc;
    u8 *lumDst = cfg->lumBuf.buf;
    u8 *cbDst  = cfg->cbBuf.buf;
    u8 *crDst  = cfg->crBuf.buf;
    u32 format = cfg->inputFormat;
    u32 depth  = cfg->depth;
    u32 pixOnRow = cfg->pixOnRow;
    u32 wrCnt  = cfg->wrCnt;
    u32 offset = wrCnt * 16;
    u32 maxLine;

    if (!cfg->buf)
        return;

    if (cfg->loopBackEn)
        maxLine = depth * 16 * 2; /* ping-pong buffer */
    else
        maxLine = cfg->encHeight;

    if (format == 0)
    {
        copyLineBuf (lumDst, lumSrc, pixOnRow, lines, offset, maxLine);
        copyLineBuf (cbDst, cbSrc, pixOnRow/2, lines/2, offset/2, maxLine/2);
        copyLineBuf (crDst, crSrc, pixOnRow/2, lines/2, offset/2, maxLine/2);
    }
    else if (format <= 2)
    {
        copyLineBuf (lumDst, lumSrc, pixOnRow, lines, offset, maxLine);
        copyLineBuf (cbDst, cbSrc, pixOnRow, lines/2, offset/2, maxLine/2);
    }
    else if (format <= 10)
        copyLineBuf (lumDst, lumSrc, pixOnRow*2, lines, offset, maxLine);
    else
        copyLineBuf (lumDst, lumSrc, pixOnRow*4, lines, offset, maxLine);
}

/*------------------------------------------------------------------------------
    HEncInitInputLineBufSrcPtr
    -Initialize src picture related pointers
------------------------------------------------------------------------------*/
void HEncInitInputLineBufSrcPtr (inputLineBufferCfg *lineBufCfg)
{
    u32 format = lineBufCfg->inputFormat;
    u32 stride = lineBufCfg->pixOnRow;
    u8 *data = lineBufCfg->src;
    u32 picHeight = lineBufCfg->srcHeight;
    u32 verOffset = lineBufCfg->srcVerOffset;
    u32 bytes;

    if (!data)
        return;

    if (format <= 2) bytes = 1;
    else if (format <= 10) bytes = 2;
    else bytes = 4;

    lineBufCfg->lumSrc = data + verOffset * stride * bytes;

    if (format <= 2)
    {
        lineBufCfg->cbSrc = data + picHeight * stride;
        if (format == 0)
        {
            lineBufCfg->crSrc = lineBufCfg->cbSrc + (picHeight/2) * (stride/2);
            lineBufCfg->cbSrc += (verOffset/2) * (stride/2);
            lineBufCfg->crSrc += (verOffset/2) * (stride/2);
        }
        else
            lineBufCfg->cbSrc += (verOffset/2) * stride;
    }
}

/*------------------------------------------------------------------------------
    HEncInitInputLineBufPtr
    -Initialize line buffer related pointers
------------------------------------------------------------------------------*/
void HEncInitInputLineBufPtr (inputLineBufferCfg *lineBufCfg)
{
    u32 bufOffset = 0;
#ifdef PCIE_FPGA_VERI_LINEBUF
    bufOffset = LINE_BUF_SWREG_AMOUNT*4;
#endif

    if (!lineBufCfg->buf)
        return;

    lineBufCfg->lumBuf.buf = lineBufCfg->buf + bufOffset;
    lineBufCfg->lumBuf.busAddress = lineBufCfg->busAddress + bufOffset;
 
    if (lineBufCfg->inputFormat <= 2)
    {
        u32 lumaBufSize;
        if (lineBufCfg->loopBackEn)
            lumaBufSize = lineBufCfg->pixOnRow * lineBufCfg->depth*16*2;
        else
            lumaBufSize = lineBufCfg->pixOnRow * lineBufCfg->encHeight;

        lineBufCfg->cbBuf.buf = lineBufCfg->lumBuf.buf+ lumaBufSize;
        lineBufCfg->cbBuf.busAddress = lineBufCfg->lumBuf.busAddress + lumaBufSize;
 
        if (lineBufCfg->inputFormat == 0)
        {
            lineBufCfg->crBuf.buf = lineBufCfg->cbBuf.buf+ lumaBufSize/4;
            lineBufCfg->crBuf.busAddress = lineBufCfg->cbBuf.busAddress + lumaBufSize/4;
        }
    }
}

/*------------------------------------------------------------------------------

    HEncInitInputLineBuffer
    -get line buffer params for IRQ handle
    -get address of input line buffer
------------------------------------------------------------------------------*/
i32 HEncInitInputLineBuffer(inputLineBufferCfg *lineBufCfg, const void *ewl)
{
    i32 ret;
    EWLLinearMem_t lineBufSRAM;
    
    if (lineBufCfg->depth == 0)
    {
      i32 mbPerRow = (lineBufCfg->encWidth + 15)/16;
      if (mbPerRow & 1)
        lineBufCfg->depth = 4;
      else if (mbPerRow & 2)
        lineBufCfg->depth = 2;
      else
        lineBufCfg->depth = 1;
    }

    /* setup pointers of source picture */
    HEncInitInputLineBufSrcPtr (lineBufCfg);

    /* setup addresses of line buffer */
    lineBufCfg->buf = NULL;
    ret = EWLGetInputLineBufferBase(ewl, &lineBufSRAM);
    lineBufCfg->buf = (u8 *)(lineBufSRAM.virtualAddress);
    lineBufCfg->busAddress = lineBufSRAM.busAddress;

    if (ret != EWL_OK)
        return -1;

    if (!lineBufCfg->buf)
        return 0;

#ifdef PCIE_FPGA_VERI_LINEBUF
    lineBufCfg->reg = (u32 *)lineBufCfg->buf;
    /* clean sram registers */
    lineBufCfg->reg[0] = 0;
#endif

    if (lineBufCfg->loopBackEn)
        HEncInitInputLineBufPtr(lineBufCfg);

#if 0
    /* To test sram */
    if (lineBufSRAM.virtualAddress)
    {
        u32 *sram = (u32 *)(lineBufSRAM.virtualAddress + LINE_BUF_SWREG_AMOUNT);
        i32 size = lineBufSRAM.size/4 - LINE_BUF_SWREG_AMOUNT;
        i32 i;
        //write
        for (i=0; i<size; i++)
            *sram++ = i;
        //read
        sram = (u32 *)(lineBufSRAM.virtualAddress + LINE_BUF_SWREG_AMOUNT);
        for (i=0; i<size; i++)
        {
            i32 r = *sram++;
            if (r != i)
                printf ("====== SRAM Test Error at %d: w=%d, r=%d\n", i, i, r);
        }
    }
#endif

    return 0;
}

/*------------------------------------------------------------------------------

    HEncStartInputLineBuffer
    -setup inputLineBufferCfg
    -initialize line buffer

------------------------------------------------------------------------------*/
void HEncStartInputLineBuffer(inputLineBufferCfg * lineBufCfg)
{
    /* write line buffer to start */
    u32 lines = MIN(lineBufCfg->depth*16*2, lineBufCfg->encHeight);
    lineBufCfg->wrCnt = ((lines+15)/16);
    if (lineBufCfg->loopBackEn)
        writeInputLineBuf (lineBufCfg, lines);

#ifdef PCIE_FPGA_VERI_LINEBUF
    /* init sram regs */
    if (lineBufCfg->reg)
    {
        i32 i;
        for (i = 0; i < LINE_BUF_SWREG_AMOUNT; i ++)
            lineBufCfg->reg[i] = 0; 

        if (lineBufCfg->hwHandShake)
        {
            setInputLineBufReg(lineBufCfg->reg, InputlineBufPicHeight,  (lineBufCfg->encHeight+15)/16);
            setInputLineBufReg(lineBufCfg->reg, InputlineBufDepth,       lineBufCfg->depth);
            setInputLineBufReg(lineBufCfg->reg, InputlineBufWrCntr,      lineBufCfg->wrCnt);
            setInputLineBufReg(lineBufCfg->reg, InputlineBufHwHandshake, 1);

            //DumpInputLineBufReg(lineBufCfg);
        }
    }
#endif
    return;
}

/*------------------------------------------------------------------------------
    An example of Line buffer Callback function
    called by the encoder SW after receive "input line buffer done" interruption from HW.
    Used for test/fpga_verification currently.
------------------------------------------------------------------------------*/
void HEncInputMBLineBufDone (void *pAppData)
{
    if (pAppData)
    {
        inputLineBufferCfg *cfg = (inputLineBufferCfg *)pAppData;
        i32 rdCnt = 0;
        i32 wrCnt  = cfg->wrCnt;
        i32 depth  = cfg->depth;
        i32 height = cfg->encHeight;
        i32 lines = depth * 16;
        i32 offset = wrCnt * 16;

        /* get rd counter */
        rdCnt = getMbLinesRdCnt(cfg);

        /* write line buffer */    
        lines = MIN(lines, height-offset);
        if ((lines>0) && (cfg->wrCnt <= (rdCnt+depth)))
        {
            if (cfg->loopBackEn)
                writeInputLineBuf (cfg, lines);
            cfg->wrCnt += ((lines+15)/16);
        }

        /* update write counter */
        setMbLinesWrCnt (cfg);

        printf ("    #<---- Line_Buf_Done:  encHeight=%d, depth=%d, rdCnt=%d, wrCnt=%d-->%d\n",
            height, depth, rdCnt, wrCnt, cfg->wrCnt);

        //DumpInputLineBufReg(cfg);
        //DumpH264LowLatencyReg(cfg);
    }
}

