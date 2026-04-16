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
 *  Abstract  :    JPEG Code Frame control
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "enccommon.h"
#include "ewl.h"

#include "EncJpegCodeFrame.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
static void NextHeader(stream_s * stream, jpegData_s * jpeg);
static void EndRi(stream_s * stream, jpegData_s * jpeg);
static void jpegSetNewFrame(jpegInstance_s * inst);

/*------------------------------------------------------------------------------

    EncJpegCodeFrame

------------------------------------------------------------------------------*/
jpegEncodeFrame_e EncJpegCodeFrame(jpegInstance_s * inst)
{
    i32 status = ASIC_STATUS_ERROR;
    jpegEncodeFrame_e ret;
    asicData_s *asic = &inst->asic;

    /* set output stream start point in case of whole encoding mode
     * and no rst used */
    if(inst->stream.byteCnt == 0)
    {
        inst->jpeg.streamStartAddress = inst->stream.stream;
    }

    /* set new frame encoding parameters */
    jpegSetNewFrame(inst);

    /* start hw encoding */
    EncAsicFrameStart(inst->asic.ewl, &inst->asic.regs);
    do {
        /* Encode one frame */
        i32 ewl_ret;

#ifndef JPEG_SIMULATION
        /* Wait for IRQ */
        ewl_ret = EWLWaitHwRdy(asic->ewl, NULL);
#else
        return JPEGENCODE_OK;
#endif
        if(ewl_ret != EWL_OK)
        {
            status = ASIC_STATUS_ERROR;

            if(ewl_ret == EWL_ERROR)
            {
                /* IRQ error => Stop and release HW */
                ret = JPEGENCODE_SYSTEM_ERROR;
            }
            else    /*if(ewl_ret == EWL_HW_WAIT_TIMEOUT) */
            {
                /* IRQ Timeout => Stop and release HW */
                ret = JPEGENCODE_TIMEOUT;
            }

            EncAsicStop(asic->ewl);
            /* Release HW so that it can be used by other codecs */
            EWLReleaseHw(asic->ewl);

        }
        else
        {
            status = EncAsicCheckStatus_V2(asic);

            switch (status)
            {
            case ASIC_STATUS_ERROR:
                ret = JPEGENCODE_HW_ERROR;
                break;
            case ASIC_STATUS_BUFF_FULL:
                ret = JPEGENCODE_OK;
                inst->stream.overflow = ENCHW_YES;
                break;
            case ASIC_STATUS_HW_RESET:
                ret = JPEGENCODE_HW_RESET;
                break;
            case ASIC_STATUS_FRAME_READY:
                inst->stream.byteCnt &= (~0x07);    /* last not full 64-bit counted in HW data */
                inst->stream.byteCnt += asic->regs.outputStrmSize;
                ret = JPEGENCODE_OK;
                break;
            case ASIC_STATUS_LINE_BUFFER_DONE:
                ret = JPEGENCODE_OK;
                /* SW handshaking:
                              Software will clear the slice ready interrupt bit
                              and then update the line buffer write pointer, when the next slice is ready. 
                              Then the encoder will continue.*/
                if (!inst->inputLineBuf.inputLineBufHwModeEn)
                {
                    EncAsicClearStatusBit(asic->ewl, ASIC_STATUS_LINE_BUFFER_DONE);
                    if (inst->inputLineBuf.cbFunc)
                        inst->inputLineBuf.cbFunc(inst->pAppData);
                }
                break;
            default:
                /* should never get here */
                ASSERT(0);
                ret = JPEGENCODE_HW_ERROR;
            }
        }
    }while (status == ASIC_STATUS_LINE_BUFFER_DONE);

    /* Handle EOI */
    if(ret == JPEGENCODE_OK)
    {
        /* update mcu count */
        if(inst->jpeg.codingType == ENC_PARTIAL_FRAME)
        {
            if((inst->jpeg.mbNum + inst->jpeg.restart.Ri) <
               ((u32) inst->jpeg.mbPerFrame))
            {
                inst->jpeg.mbNum += inst->jpeg.restart.Ri;
                inst->jpeg.row += inst->jpeg.sliceRows;
            }
            else
            {
                inst->jpeg.mbNum += (inst->jpeg.mbPerFrame - inst->jpeg.mbNum);
            }
        }
        else
        {
            inst->jpeg.mbNum += inst->jpeg.mbPerFrame;
        }

        EndRi(&inst->stream, &inst->jpeg);
    }

    return ret;
}

/*------------------------------------------------------------------------------

    Write the header data (frame header or restart marker) to the stream. 
    
------------------------------------------------------------------------------*/
void NextHeader(stream_s * stream, jpegData_s * jpeg)
{
    if(jpeg->mbNum == 0)
    {
        (void) EncJpegHdr(stream, jpeg);
    }
}

/*------------------------------------------------------------------------------

    Write the end of current coding unit (RI / FRAME) into stream.
    
------------------------------------------------------------------------------*/
void EndRi(stream_s * stream, jpegData_s * jpeg)
{
    /* not needed anymore, ASIC generates EOI marker */
}

/*------------------------------------------------------------------------------

    Set encoding parameters at the beginning of a new frame.

------------------------------------------------------------------------------*/
void jpegSetNewFrame(jpegInstance_s * inst)
{
    regValues_s *regs = &inst->asic.regs;

    /* Write next header if needed */
    NextHeader(&inst->stream, &inst->jpeg);

    /* calculate output start point for hw */
    regs->outputStrmSize -= inst->stream.byteCnt;
    regs->outputStrmSize /= 8;  /* 64-bit addresses */
    regs->outputStrmSize &= (~0x07);    /* 8 multiple size */

    /* 64-bit aligned stream base address */
    regs->outputStrmBase += (inst->stream.byteCnt & (~0x07));
    /* bit offset in the last 64-bit word */
    regs->firstFreeBit = (inst->stream.byteCnt & 0x07) * 8;

    /* header remainder is byte aligned, max 7 bytes = 56 bits */
    if(regs->firstFreeBit != 0)
    {
        /* 64-bit aligned stream pointer */
        u8 *pTmp = (u8 *) ((ptr_t) (inst->stream.stream) & (~0x07));
        u32 val;

        /* Clear remaining bits */
        for (val = 6; val >= regs->firstFreeBit/8; val--)
            pTmp[val] = 0;

        val = pTmp[0] << 24;
        val |= pTmp[1] << 16;
        val |= pTmp[2] << 8;
        val |= pTmp[3];

        regs->strmStartMSB = val;  /* 32 bits to MSB */

        if(regs->firstFreeBit > 32)
        {
            val = pTmp[4] << 24;
            val |= pTmp[5] << 16;
            val |= pTmp[6] << 8;

            regs->strmStartLSB = val;
        }
        else
            regs->strmStartLSB = 0;
    }
    else
    {
        regs->strmStartMSB = regs->strmStartLSB = 0;
    }

    /* configure the input buffer control related register.*/
    regs->lineBufferEn = inst->inputLineBuf.inputLineBufEn;
    regs->lineBufferHwHandShake = inst->inputLineBuf.inputLineBufHwModeEn;
    if (regs->lineBufferHwHandShake)
    {
      regs->lineBufferIrqEnable = 0;
    }
    else
    {
      regs->lineBufferIrqEnable = 1;
    }
    regs->lineBufferLoopBackEn = inst->inputLineBuf.inputLineBufLoopBackEn;
    regs->lineBufferDepth = inst->inputLineBuf.inputLineBufDepth;
    regs->mbWrPtr = inst->inputLineBuf.wrCnt;  /* MB write pointer. */
    regs->mbRrPtr = 0;  /* clear the MB read pointer. */
}
