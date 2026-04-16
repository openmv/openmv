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
    1. Include headers
------------------------------------------------------------------------------*/
#include "H264Denoise.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

#ifdef TRACE_DENOISE_INTERNAL
#define APIDNF_DBG_LEVEL    20
#define APIDNF_DBG_PARAM    10
#define APIDNF_DBG_UPDATE   30

#define DBG(l, ...) if (l <= APIDNF_DBG_LEVEL) printf(__VA_ARGS__)
#else
#define DBG(l, ...)
#endif

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function name : H264EncDnfInit
    Description   : Init denoise parameters.
    Return type   : H264EncRet
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
H264EncRet H264EncDnfInit(h264Instance_s  *inst)
{
    inst->dnfEnable = 0;
    inst->dnfNoiseLevelLow = 5 << FIX_POINT_BIT_WIDTH;
    inst->dnfNoiseLevelY = 10 << FIX_POINT_BIT_WIDTH;
    inst->dnfNoiseYCRatio = (1<<FIX_POINT_BIT_WIDTH)*100/100;
    inst->dnfTableLoaded = 0;

    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name : H264EncDnfSetParameters
    Description   : Set DNF parameters according to coding parameters.
    Return type   : H264EncRet
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
H264EncRet H264EncDnfSetParameters(h264Instance_s *inst, const H264EncCodingCtrl *pCodeParams)
{
  inst->dnfEnable = pCodeParams->noiseReductionEnable;
  inst->dnfNoiseLevelLow = pCodeParams->noiseLow;
  inst->dnfNoiseLevelY  = pCodeParams->noiseLevel;

  return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name : H264EncDnfGetParameters
    Description   : Get DNF parameters.
    Return type   : H264EncRet
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
H264EncRet H264EncDnfGetParameters(h264Instance_s *inst, H264EncCodingCtrl *pCodeParams)
{
  pCodeParams->noiseReductionEnable = inst->dnfEnable;
  pCodeParams->noiseLow =  inst->dnfNoiseLevelLow;
  pCodeParams->noiseLevel = inst->dnfNoiseLevelY;

  return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name : H264EncDnfPrepare
    Description   : run before HW run, set register's value according to coding settings
    Return type   : H264EncRet
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
H264EncRet H264EncDnfPrepare(h264Instance_s *inst)
{
#define SETPSIx2(x) (64+x/2)
#define SETPSIx1(x) (x)
#define MAP8x8(a, x) a[(x)/16*2+((x)/4)%2][(x)%4+4*(((x)/8)%2)]

  const int g_psi4e[4][4] =
  {
      {128, 114, 90, 41},
      {114, 102, 81, 36},
      { 90, 81, 64, 29},
      { 41, 36, 29, 13}
  };
  const int g_psi8e[8][8] =
  {
      {128, 125, 118, 107, 86, 58, 41, 28},
      {125, 122, 115, 105, 84, 56, 40, 27},
      {118, 115, 109,  99, 80, 53, 38, 26},
      {107, 105,  99,  90, 72, 48, 34, 23},
      { 86,  84,  80,  72, 58, 39, 27, 19},
      { 58,  56,  53,  48, 39, 26, 18, 12},
      { 41,  40,  38,  34, 27, 18, 13, 9},
      { 28,  27,  26,  23, 19, 12,  9, 6}
  };
  /* 4096/level, 12bits */
  const unsigned int g_table_sigReci[200] =
  {
      803, 788, 773, 759, 745, 731, 719, 706, 694, 683,
      671, 661, 650, 640, 630, 621, 611, 602, 594, 585,
      577, 569, 561, 554, 546, 539, 532, 525, 518, 512,
      506, 500, 493, 488, 482, 476, 471, 465, 460, 455,
      450, 445, 440, 436, 431, 427, 422, 418, 414, 410,
      406, 402, 398, 394, 390, 386, 383, 379, 376, 372,
      369, 366, 362, 359, 356, 353, 350, 347, 344, 341,
      339, 336, 333, 330, 328, 325, 323, 320, 318, 315,
      313, 310, 308, 306, 303, 301, 299, 297, 295, 293,
      290, 288, 286, 284, 282, 281, 279, 277, 275, 273,
      271, 269, 268, 266, 264, 263, 261, 259, 258, 256,
      254, 253, 251, 250, 248, 247, 245, 244, 242, 241,
      240, 238, 237, 235, 234, 233, 231, 230, 229, 228,
      226, 225, 224, 223, 221, 220, 219, 218, 217, 216,
      214, 213, 212, 211, 210, 209, 208, 207, 206, 205,
      204, 203, 202, 201, 200, 199, 198, 197, 196, 195,
      194, 193, 192, 191, 191, 190, 189, 188, 187, 186,
      185, 185, 184, 183, 182, 181, 180, 180, 179, 178,
      177, 177, 176, 175, 174, 174, 173, 172, 171, 171,
      170, 169, 169, 168, 167, 167, 166, 165, 164, 164
  };

  asicData_s *asic = &inst->asic;
  i32 qp = inst->rateControl.qpHdr>>QP_FRACTIONAL_BITS;
  
  /* fill dnf table */
  if (inst->dnfTableLoaded==0)
  {
    #ifdef DNF_PARAM_REGS
    /* load table from registers */
    int i;
    /* intra 4x4 */
    for(i=0; i<16; i++) asic->regs.dnfParamS1[i] = SETPSIx2(g_psi4e[i/4][i%4]);
    /* inter 4x4 */
    for(i=0; i<16; i++) asic->regs.dnfParamS2[i] = SETPSIx1(g_psi4e[i/4][i%4]);
    /* intra 8x8 */
    for(i=0; i<64; i++) asic->regs.dnfParamS4[i] = SETPSIx2(MAP8x8(g_psi8e,i));
    /* inter 8x8 */
    for(i=0; i<64; i++) asic->regs.dnfParamS4[i] = SETPSIx1(MAP8x8(g_psi8e,i));
    #else
    /* load table from DDR */
    int i;
    u8 *base;
    /* intra 4x4 */
    base = (u8*)inst->asic.dnfTable.virtualAddress;
    for(i=0; i<16; i++) base[i] = SETPSIx2(g_psi4e[i/4][i%4]);
    /* inter 4x4 */
    base += 16;
    for(i=0; i<16; i++) base[i] = SETPSIx1(g_psi4e[i/4][i%4]);
    /* intra 8x8 */
    base += 16;
    for(i=0; i<64; i++) base[i] = SETPSIx2(MAP8x8(g_psi8e,i));
    /* inter 8x8 */
    base += 64;
    for(i=0; i<64; i++) base[i] = SETPSIx1(MAP8x8(g_psi8e,i));
    #endif

    inst->dnfTableLoaded = 1;
  }  

  if (inst->dnfEnable==0)
    return H264ENC_OK;
  
  if ((inst->dnfNoiseLevelY <= inst->dnfNoiseLevelLow) ||(inst->dnfNoiseLevelY <= MAX(80 * qp, inst->dnfNoiseLevelLow)))
  {
      asic->regs.dnfStrength = FILTER_STRENGTH_H;
      asic->regs.dnfNoiseLevelInvertY = BITMASK(DNF_LEVELIVT_BITS);
      asic->regs.dnfNoiseLevelInvertC = BITMASK(DNF_LEVELIVT_BITS);
  }
  else
  {
      inst->dnfNoiseLevelC = (inst->dnfNoiseLevelY * inst->dnfNoiseYCRatio) >> FIX_POINT_BIT_WIDTH;
      
      asic->regs.dnfStrength = CLIP3((FILTER_STRENGTH_H - ((FILTER_STRENGTH_PARAM * (MAX(inst->dnfNoiseLevelY - inst->dnfNoiseLevelLow, 0))) >> FIX_POINT_BIT_WIDTH)),
                                      FILTER_STRENGTH_L, FILTER_STRENGTH_H);
      asic->regs.dnfNoiseLevelInvertY = g_table_sigReci[CLIP3((((inst->dnfNoiseLevelY*1311)>>17) - 51), 0, 199)];
      asic->regs.dnfNoiseLevelInvertC = g_table_sigReci[CLIP3((((inst->dnfNoiseLevelC*1311)>>17) - 51), 0, 199)];
  }

  ASSERT(asic->regs.dnfStrength <= BITMASK(DNF_STENGTH_BITS));
  ASSERT(asic->regs.dnfNoiseLevelInvertY <= BITMASK(DNF_LEVELIVT_BITS));
  ASSERT(asic->regs.dnfNoiseLevelInvertC <= BITMASK(DNF_LEVELIVT_BITS));
  
  asic->regs.dnfEnable = (inst->dnfEnable!=0);
  asic->regs.dnfNoiseLevelMax = inst->dnfNoiseLevelMax;
  ASSERT(asic->regs.dnfNoiseLevelMax < BITMASK(DNF_LEVELMAX_BITS));

  DBG(APIDNF_DBG_PARAM, "DNF Parameters     :\n");
  DBG(APIDNF_DBG_PARAM, " Noise level is %.2f\n", (float)inst->dnfNoiseLevelY/1024.0);
  DBG(APIDNF_DBG_PARAM, " Noise Low   is %.2f\n", (float)inst->dnfNoiseLevelLow/1024.0);
  DBG(APIDNF_DBG_PARAM, " Noise Max   is %.2f\n", (float)inst->dnfNoiseLevelMax/1024.0);

  return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name : H264EncDnfUpdate
    Description   : run after HW finish one frame, update collected parameters
    Return type   : H264EncRet
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
H264EncRet H264EncDnfUpdate(h264Instance_s *inst)
{
  const int SMOOTHFACTOR[5] = {1024, 512, 341, 256, 205};
  const int KK=160;
  
  asicData_s *asic = &inst->asic;
  unsigned int j = 0;
  int qp = inst->asic.regs.qp;
  int frameCodingType = inst->asic.regs.frameCodingType;
  unsigned int frameCnt = inst->dnfFrameNum++;
  int noiseSum = 0;
  int noiseMbNum;
  int noiseLevel;
  int noiseNext;
  int noiseMax;

  /**
   * check pre-conditions  
   * 1. disable or without noise estimation.
   * 2. no stream generated, estimation fail.
   */
  if (inst->dnfEnable != 1 || inst->stream.byteCnt == 0)
    return H264ENC_OK;

  /* calculate the noise level */
  noiseMbNum = asic->regs.dnfNoiseMbNum;
  noiseLevel = (noiseMbNum!=0u)?(int)(asic->regs.dnfNoiseLevelPred/noiseMbNum):-1;
  
  inst->dnfQpPrev = qp;

  /* estimate noise level */  
  if (noiseLevel == -1)
  {
    noiseLevel = inst->dnfNoiseLevelY;
  }
  noiseLevel = (noiseLevel * KK) >> 7;
  inst->dnfNoiseLevels[(frameCnt)%SIGMA_SMOOTH_NUM] = noiseLevel;
  for (j = 0; j < (MIN(SIGMA_SMOOTH_NUM - 1, frameCnt) + 1); j++)
  {
    noiseSum += inst->dnfNoiseLevels[j];
    DBG(APIDNF_DBG_UPDATE, " %.2f", (float)inst->dnfNoiseLevels[j]/1024.0);
  }
  DBG(APIDNF_DBG_UPDATE, "\n");
  noiseNext = (noiseSum * SMOOTHFACTOR[MIN(SIGMA_SMOOTH_NUM-1, frameCnt)]) >> 10;
  inst->dnfNoiseLevelY = CLIP3(noiseNext, 0, (SIGMA_MAX << FIX_POINT_BIT_WIDTH));
  DBG(APIDNF_DBG_UPDATE, "DNF Update     :\n");
  DBG(APIDNF_DBG_UPDATE, " Pedict Noise level is %.2f\n", (float)noiseLevel/1024.0);
  DBG(APIDNF_DBG_UPDATE, " Smooth Noise Level is %.2f by smoothing %d frames\n", (float)noiseNext/1024.0, frameCnt);
  DBG(APIDNF_DBG_UPDATE, " Final Noise Level as %.2f\n", (float)inst->dnfNoiseLevelY/1024.0);

  /* estimate noise max */
  noiseMax = (frameCodingType != (int)ASIC_INTRA) ? (int)(asic->regs.dnfNoiseMaxPred/inst->mbPerFrame) : inst->dnfNoiseMaxPrev;
  if (noiseMax > BITMASK(DNF_LEVELMAX_BITS)) 
  {
    noiseMax = BITMASK(DNF_LEVELMAX_BITS);
  }  
  inst->dnfNoiseLevelMax = inst->dnfNoiseMaxPred = noiseMax;
  DBG(APIDNF_DBG_UPDATE, " Predict Noise Max is %.2f\n", (float)noiseMax/1024.0);

  return H264ENC_OK;
}


