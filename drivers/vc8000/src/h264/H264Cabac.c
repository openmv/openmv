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
 *  Description : H264 CABAC context table initialization
 *
 ********************************************************************************
 */
#include "H264Cabac.h"
#include "H264CabacContext.h"
#include "enccommon.h"
#include "enccfg.h"

/* Swap byte and/or 32-bit endianess as defined in enccfg.h.
 * Input and output buffers can be the same. */
static void SwapEndianess(u32 * bufIn, u32 * buf, u32 sizeBytes)
{
#if (ENCH1_OUTPUT_SWAP_8 == 1)
    u32 i = 0;
    i32 words = sizeBytes / 4;

    ASSERT((sizeBytes % 8) == 0);

    while(words > 0)
    {
        u32 val = bufIn[i];
        u32 tmp = 0;

        tmp |= (val & 0xFF) << 24;
        tmp |= (val & 0xFF00) << 8;
        tmp |= (val & 0xFF0000) >> 8;
        tmp |= (val & 0xFF000000) >> 24;

#if(ENCH1_OUTPUT_SWAP_32 == 1)    /* need this for 64-bit HW */
        {
            u32 val2 = bufIn[i + 1];
            u32 tmp2 = 0;

            tmp2 |= (val2 & 0xFF) << 24;
            tmp2 |= (val2 & 0xFF00) << 8;
            tmp2 |= (val2 & 0xFF0000) >> 8;
            tmp2 |= (val2 & 0xFF000000) >> 24;

            buf[i] = tmp2;
            words--;
            i++;
        }
#endif
        buf[i] = tmp;
        words--;
        i++;
    }
#else   /* No swapping, copy buffer if needed. */
    if (bufIn != buf)
        EWLmemcpy(buf, bufIn, 52 * 2 * 464);
#endif

}

#ifndef H1_MEMCPY_CABAC_INIT

#ifndef H1_PRECALC_CABAC_INIT
u32 H264CabacInit(u32 * contextTable, u32 cabac_init_idc)
{
    const i32(*ctx)[460][2];
    int i, j, qp;
    u8 *table = (u8 *) contextTable;

    for(qp = 0; qp < 52; qp++)  /* All QP values */
    {
        for(j = 0; j < 2; j++)  /* Intra/Inter */
        {
            if(j == 0)
                ctx = /*lint -e(545) */ &h264ContextInitIntra;
            else
                ctx = /*lint -e(545) */ &h264ContextInit[cabac_init_idc];

            for(i = 0; i < 460; i++)
            {
                i32 m = (i32) (*ctx)[i][0];
                i32 n = (i32) (*ctx)[i][1];

                i32 preCtxState = CLIP3(((m * (i32) qp) >> 4) + n, 1, 126);

                if(preCtxState <= 63)
                {
                    table[qp * 464 * 2 + j * 464 + i] =
                        (u8) ((63 - preCtxState) << 1);
                }
                else
                {
                    table[qp * 464 * 2 + j * 464 + i] =
                        (u8) (((preCtxState - 64) << 1) | 1);
                }
            }
        }
    }

    SwapEndianess(contextTable, contextTable, 52 * 2 * 464);
    return 0;
}

#else
u32 H264CabacInit(u32 * contextTable, u32 cabac_init_idc)
{
    SwapEndianess((u32*)preCalculatedContextTable[cabac_init_idc],
            contextTable, 52 * 2 * 464);
    return 0;
}
#endif

#else
u32 H264CabacInit(u32 * contextTable, u32 cabac_init_idc)
{
    const i32(*ctx)[460][2];
    int i, j, qp;

    u8 *tempCabacTable = (u8*)EWLmalloc(52 * 2 * 464);
    if(tempCabacTable == NULL)
    {
        return 1;
    }

    for(qp = 0; qp < 52; qp++) /* All QP values */
    {
        for(j = 0; j < 2; j++) /* Intra/Inter */
        {
            if(j == 0)
                ctx = /*lint -e(545) */ &h264ContextInitIntra;
            else
                ctx = /*lint -e(545) */ &h264ContextInit[cabac_init_idc];

            for(i = 0; i < 460; i++)
            {
                i32 m = (i32) (*ctx)[i][0];
                i32 n = (i32) (*ctx)[i][1];

                i32 preCtxState = CLIP3(((m * (i32) qp) >> 4) + n, 1, 126);

                if(preCtxState <= 63)
                {
                    tempCabacTable [qp * 464 * 2 + j * 464 + i] =
                        (u8) ((63 - preCtxState) << 1);
                }
                else
                {
                    tempCabacTable [qp * 464 * 2 + j * 464 + i] =
                        (u8) (((preCtxState - 64) << 1) | 1);
                }
            }
        }
    }

    SwapEndianess((u32*)tempCabacTable, (u32*)tempCabacTable, 52 * 2 * 464);
    EWLmemcpy(contextTable, tempCabacTable, 52 * 2 * 464);
    EWLfree(tempCabacTable);
    return 0;
}
#endif
