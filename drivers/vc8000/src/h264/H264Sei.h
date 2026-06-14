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
 * Description : H.264 SEI Messages.
 *
 ********************************************************************************
 */

#ifndef H264_SEI_H
#define H264_SEI_H

#include "basetype.h"
#include "H264PutBits.h"

typedef struct
{
    u32 fts;    /* Full time stamp */
    u32 timeScale;
    u32 nuit;   /* number of units in tick */
    u32 time;   /* Modulo time */
    u32 secf;
    u32 sec;    /* Seconds */
    u32 minf;
    u32 min;    /* Minutes */
    u32 hrf;
    u32 hr; /* Hours */
} timeStamp_s;

typedef struct
{
    timeStamp_s ts;
    u32 nalUnitSize;
    u32 enabled;
    true_e byteStream;
    u32 hrd;    /* HRD conformance */
    u32 seqId;
    u32 icrd;   /* initial cpb removal delay */
    u32 icrdLen;
    u32 icrdo;  /* initial cpb removal delay offset */
    u32 icrdoLen;
    u32 crd;    /* CPB removal delay */
    u32 crdLen;
    u32 dod;    /* DPB removal delay */
    u32 dodLen;
    u32 psp;
    u32 ps;
    u32 cts;
    u32 cntType;
    u32 cdf;
    u32 nframes;
    u32 toffs;
    u32 toffsLen;
    u32 userDataEnabled;
    const u8 * pUserData;
    u32 userDataSize;
    u32 insertRecoveryPointMessage;
    u32 recoveryFrameCnt;
} sei_s;

void H264InitSei(sei_s * sei, true_e byteStream, u32 hrd, u32 timeScale,
                 u32 nuit);
void H264UpdateSeiTS(sei_s * sei, u32 timeInc);
void H264FillerSei(stream_s * sp, sei_s * sei, i32 cnt);
void H264BufferingSei(stream_s * stream, sei_s * sei);
void H264PicTimingSei(stream_s * stream, sei_s * sei);
void H264UserDataUnregSei(stream_s * sp, sei_s * sei);
void H264RecoveryPointSei(stream_s * sp, sei_s * sei);
void H264ScalabilityInfoSei(stream_s * sp, i32 svctLevel, i32 frameRate);

#endif
