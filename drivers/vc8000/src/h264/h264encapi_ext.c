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
 *  Abstract : H264 Encoder Extended API (just for testing)
 *
 ********************************************************************************
 */
#include "h264encapi.h"
#include "h264encapi_ext.h"
#include "H264Instance.h"
#include "H264TestId.h"

H264EncRet H264EncSetFilter(H264EncInst inst, const H264EncFilter * pEncCfg)
{

    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    if(pEncInst->picParameterSet.deblockingFilterControlPresent == ENCHW_NO)
        return H264ENC_INVALID_STATUS;

    pEncInst->slice.disableDeblocking = pEncCfg->disableDeblocking;

#if 0
    if(pEncCfg->disableDeblocking != 1)
    {
        pEncInst->slice.filterOffsetA = pEncCfg->filterOffsetA;
        pEncInst->slice.filterOffsetB = pEncCfg->filterOffsetB;
    }
    else
    {
        pEncInst->slice.filterOffsetA = 0;
        pEncInst->slice.filterOffsetB = 0;
    }
#else
    pEncInst->slice.filterOffsetA = pEncCfg->filterOffsetA;
    pEncInst->slice.filterOffsetB = pEncCfg->filterOffsetB;
#endif

    return H264ENC_OK;
}

H264EncRet H264EncGetFilter(H264EncInst inst, H264EncFilter * pEncCfg)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    pEncCfg->disableDeblocking = pEncInst->slice.disableDeblocking;
    pEncCfg->filterOffsetA = pEncInst->slice.filterOffsetA;
    pEncCfg->filterOffsetB = pEncInst->slice.filterOffsetB;

    return H264ENC_OK;
}

H264EncRet H264EncSetChromaQpIndexOffset(H264EncInst inst, i32 offset)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    ASSERT(inst != NULL);

    if(offset < -12 || offset > 12)
    {
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Check status, only INIT is allowed */
    if(pEncInst->encStatus != H264ENCSTAT_INIT)
    {
        return H264ENC_INVALID_STATUS;
    }

    pEncInst->picParameterSet.chromaQpIndexOffset = offset;

    return H264ENC_OK;
}

H264EncRet H264EncSetHwBurstSize(H264EncInst inst, u32 burst)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    ASSERT(inst != NULL);
    ASSERT(burst < 64);

    pEncInst->asic.regs.asicCfgReg &=  ~(63 << 8);
    pEncInst->asic.regs.asicCfgReg |=  ((burst & (63)) << 8);

    return H264ENC_OK;
}

H264EncRet H264EncSetHwBurstType(H264EncInst inst, u32 bursttype)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    ASSERT(inst != NULL);

    pEncInst->asic.regs.asicCfgReg &=  ~(1 << 6);
    pEncInst->asic.regs.asicCfgReg |=  ((bursttype & (1)) << 6);

    return H264ENC_OK;
}

H264EncRet H264EncTestInputLineBuf(H264EncInst inst)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    ASSERT(inst != NULL);

    H264InputLineBufDepthTest(pEncInst);

    return H264ENC_OK;
}

H264EncRet H264EncTestCropping(H264EncInst inst)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    ASSERT(inst != NULL);

    H264CroppingTest(pEncInst);

    return H264ENC_OK;
}

