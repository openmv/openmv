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
 *  Abstract : Register enum auto generated.
 *
 ********************************************************************************
 */

#ifdef __REGISTERS_ENUM_FAKE_H__
typedef enum {
#endif /*__REGISTERS_ENUM_FAKE_H__*/

/* IRQ Source Map. */
    HEncIRQRfcBufOverflow,
    HEncIRQLowLatency,


/* HW synthesis config register 2, read-only */
    HEncHWAddress64Bits,
    HEncHWDnfSupport,
    HEncHWRfcSupport,
    HEncHWQEnhanceSupport,
    HEncHWInstantSupport,
    HEncHWSvctSupport,
    HEncHWAxiIdInSupport,
    HEncHWWriteOneClrIrqSupport,
    HEncHWInputLoopbackSupport,

/* HEncMBRC Control */
    HEncMBRCQpDeltaRange ,
    HEncMBRcSliceQpOffset ,

/* Segment 4 */
    HEncSeg4I4PrevModeFavor ,
    HEncSeg4I16Favor        ,
    HEncSeg4GoldenPenalty   ,
    HEncSeg4SkipPenalty     ,
    HEncSeg4InterFavor      ,
    HEncSeg4SplitPenalty16x8,
    HEncSeg4SplitPenalty8x8 ,
    HEncSeg4SplitPenalty8x4 ,
    HEncSeg4DMVPenaltyQp    ,
    HEncSeg4SplitPenalty4x4 ,

/* Segment 5 */
    HEncSeg5I4PrevModeFavor ,
    HEncSeg5I16Favor        ,
    HEncSeg5GoldenPenalty   ,
    HEncSeg5SkipPenalty     ,
    HEncSeg5InterFavor      ,
    HEncSeg5SplitPenalty16x8,
    HEncSeg5SplitPenalty8x8 ,
    HEncSeg5SplitPenalty8x4 ,
    HEncSeg5DMVPenaltyQp    ,
    HEncSeg5SplitPenalty4x4 ,

/* Segment 6 */
    HEncSeg6I4PrevModeFavor ,
    HEncSeg6I16Favor        ,
    HEncSeg6GoldenPenalty   ,
    HEncSeg6SkipPenalty     ,
    HEncSeg6InterFavor      ,
    HEncSeg6SplitPenalty16x8,
    HEncSeg6SplitPenalty8x8 ,
    HEncSeg6SplitPenalty8x4 ,
    HEncSeg6DMVPenaltyQp    ,
    HEncSeg6SplitPenalty4x4 ,

/* Segment 7 */
    HEncSeg7I4PrevModeFavor ,
    HEncSeg7I16Favor        ,
    HEncSeg7GoldenPenalty   ,
    HEncSeg7SkipPenalty     ,
    HEncSeg7InterFavor      ,
    HEncSeg7SplitPenalty16x8,
    HEncSeg7SplitPenalty8x8 ,
    HEncSeg7SplitPenalty8x4 ,
    HEncSeg7DMVPenaltyQp    ,
    HEncSeg7SplitPenalty4x4 ,

/* Segment 8 */
    HEncSeg8I4PrevModeFavor ,
    HEncSeg8I16Favor        ,
    HEncSeg8GoldenPenalty   ,
    HEncSeg8SkipPenalty     ,
    HEncSeg8InterFavor      ,
    HEncSeg8SplitPenalty16x8,
    HEncSeg8SplitPenalty8x8 ,
    HEncSeg8SplitPenalty8x4 ,
    HEncSeg8DMVPenaltyQp    ,
    HEncSeg8SplitPenalty4x4 ,

/* Segment 9 */
    HEncSeg9I4PrevModeFavor ,
    HEncSeg9I16Favor        ,
    HEncSeg9GoldenPenalty   ,
    HEncSeg9SkipPenalty     ,
    HEncSeg9InterFavor      ,
    HEncSeg9SplitPenalty16x8,
    HEncSeg9SplitPenalty8x8 ,
    HEncSeg9SplitPenalty8x4 ,
    HEncSeg9DMVPenaltyQp    ,
    HEncSeg9SplitPenalty4x4 ,

/* Segment 10 */
    HEncSeg10I4PrevModeFavor ,
    HEncSeg10I16Favor        ,
    HEncSeg10GoldenPenalty   ,
    HEncSeg10SkipPenalty     ,
    HEncSeg10InterFavor      ,
    HEncSeg10SplitPenalty16x8,
    HEncSeg10SplitPenalty8x8 ,
    HEncSeg10SplitPenalty8x4 ,
    HEncSeg10DMVPenaltyQp    ,
    HEncSeg10SplitPenalty4x4 ,

/* Segment 11 */
    HEncSeg11I4PrevModeFavor ,
    HEncSeg11I16Favor        ,
    HEncSeg11GoldenPenalty   ,
    HEncSeg11SkipPenalty     ,
    HEncSeg11InterFavor      ,
    HEncSeg11SplitPenalty16x8,
    HEncSeg11SplitPenalty8x8 ,
    HEncSeg11SplitPenalty8x4 ,
    HEncSeg11DMVPenaltyQp    ,
    HEncSeg11SplitPenalty4x4 ,

/* Segment 12 */
    HEncSeg12I4PrevModeFavor ,
    HEncSeg12I16Favor        ,
    HEncSeg12GoldenPenalty   ,
    HEncSeg12SkipPenalty     ,
    HEncSeg12InterFavor      ,
    HEncSeg12SplitPenalty16x8,
    HEncSeg12SplitPenalty8x8 ,
    HEncSeg12SplitPenalty8x4 ,
    HEncSeg12DMVPenaltyQp    ,
    HEncSeg12SplitPenalty4x4 ,

/* Segment 13 */
    HEncSeg13I4PrevModeFavor ,
    HEncSeg13I16Favor        ,
    HEncSeg13GoldenPenalty   ,
    HEncSeg13SkipPenalty     ,
    HEncSeg13InterFavor      ,
    HEncSeg13SplitPenalty16x8,
    HEncSeg13SplitPenalty8x8 ,
    HEncSeg13SplitPenalty8x4 ,
    HEncSeg13DMVPenaltyQp    ,
    HEncSeg13SplitPenalty4x4 ,

/* Segment 14 */
    HEncSeg14I4PrevModeFavor ,
    HEncSeg14I16Favor        ,
    HEncSeg14GoldenPenalty   ,
    HEncSeg14SkipPenalty     ,
    HEncSeg14InterFavor      ,
    HEncSeg14SplitPenalty16x8,
    HEncSeg14SplitPenalty8x8 ,
    HEncSeg14SplitPenalty8x4 ,
    HEncSeg14DMVPenaltyQp    ,
    HEncSeg14SplitPenalty4x4 ,

/* Segment 15 */
    HEncSeg15I4PrevModeFavor ,
    HEncSeg15I16Favor        ,
    HEncSeg15GoldenPenalty   ,
    HEncSeg15SkipPenalty     ,
    HEncSeg15InterFavor      ,
    HEncSeg15SplitPenalty16x8,
    HEncSeg15SplitPenalty8x8 ,
    HEncSeg15SplitPenalty8x4 ,
    HEncSeg15DMVPenaltyQp    ,
    HEncSeg15SplitPenalty4x4 ,

/* Segment 16 */
    HEncSeg16I4PrevModeFavor ,
    HEncSeg16I16Favor        ,
    HEncSeg16GoldenPenalty   ,
    HEncSeg16SkipPenalty     ,
    HEncSeg16InterFavor      ,
    HEncSeg16SplitPenalty16x8,
    HEncSeg16SplitPenalty8x8 ,
    HEncSeg16SplitPenalty8x4 ,
    HEncSeg16DMVPenaltyQp    ,
    HEncSeg16SplitPenalty4x4 ,

/* Segment 17 */
    HEncSeg17I4PrevModeFavor ,
    HEncSeg17I16Favor        ,
    HEncSeg17GoldenPenalty   ,
    HEncSeg17SkipPenalty     ,
    HEncSeg17InterFavor      ,
    HEncSeg17SplitPenalty16x8,
    HEncSeg17SplitPenalty8x8 ,
    HEncSeg17SplitPenalty8x4 ,
    HEncSeg17DMVPenaltyQp    ,
    HEncSeg17SplitPenalty4x4 ,

/* Segment 18 */
    HEncSeg18I4PrevModeFavor ,
    HEncSeg18I16Favor        ,
    HEncSeg18GoldenPenalty   ,
    HEncSeg18SkipPenalty     ,
    HEncSeg18InterFavor      ,
    HEncSeg18SplitPenalty16x8,
    HEncSeg18SplitPenalty8x8 ,
    HEncSeg18SplitPenalty8x4 ,
    HEncSeg18DMVPenaltyQp    ,
    HEncSeg18SplitPenalty4x4 ,

/* Segment 19 */
    HEncSeg19I4PrevModeFavor ,
    HEncSeg19I16Favor        ,
    HEncSeg19GoldenPenalty   ,
    HEncSeg19SkipPenalty     ,
    HEncSeg19InterFavor      ,
    HEncSeg19SplitPenalty16x8,
    HEncSeg19SplitPenalty8x8 ,
    HEncSeg19SplitPenalty8x4 ,
    HEncSeg19DMVPenaltyQp    ,
    HEncSeg19SplitPenalty4x4 ,

/* Segment 20 */
    HEncSeg20I4PrevModeFavor ,
    HEncSeg20I16Favor        ,
    HEncSeg20GoldenPenalty   ,
    HEncSeg20SkipPenalty     ,
    HEncSeg20InterFavor      ,
    HEncSeg20SplitPenalty16x8,
    HEncSeg20SplitPenalty8x8 ,
    HEncSeg20SplitPenalty8x4 ,
    HEncSeg20DMVPenaltyQp    ,
    HEncSeg20SplitPenalty4x4 ,

/* Segment 21 */
    HEncSeg21I4PrevModeFavor ,
    HEncSeg21I16Favor        ,
    HEncSeg21GoldenPenalty   ,
    HEncSeg21SkipPenalty     ,
    HEncSeg21InterFavor      ,
    HEncSeg21SplitPenalty16x8,
    HEncSeg21SplitPenalty8x8 ,
    HEncSeg21SplitPenalty8x4 ,
    HEncSeg21DMVPenaltyQp    ,
    HEncSeg21SplitPenalty4x4 ,

/* Segment 22 */
    HEncSeg22I4PrevModeFavor ,
    HEncSeg22I16Favor        ,
    HEncSeg22GoldenPenalty   ,
    HEncSeg22SkipPenalty     ,
    HEncSeg22InterFavor      ,
    HEncSeg22SplitPenalty16x8,
    HEncSeg22SplitPenalty8x8 ,
    HEncSeg22SplitPenalty8x4 ,
    HEncSeg22DMVPenaltyQp    ,
    HEncSeg22SplitPenalty4x4 ,

/* Segment 23 */
    HEncSeg23I4PrevModeFavor ,
    HEncSeg23I16Favor        ,
    HEncSeg23GoldenPenalty   ,
    HEncSeg23SkipPenalty     ,
    HEncSeg23InterFavor      ,
    HEncSeg23SplitPenalty16x8,
    HEncSeg23SplitPenalty8x8 ,
    HEncSeg23SplitPenalty8x4 ,
    HEncSeg23DMVPenaltyQp    ,
    HEncSeg23SplitPenalty4x4 ,

/* Segment 24 */
    HEncSeg24I4PrevModeFavor ,
    HEncSeg24I16Favor        ,
    HEncSeg24GoldenPenalty   ,
    HEncSeg24SkipPenalty     ,
    HEncSeg24InterFavor      ,
    HEncSeg24SplitPenalty16x8,
    HEncSeg24SplitPenalty8x8 ,
    HEncSeg24SplitPenalty8x4 ,
    HEncSeg24DMVPenaltyQp    ,
    HEncSeg24SplitPenalty4x4 ,

/* Segment 25 */
    HEncSeg25I4PrevModeFavor ,
    HEncSeg25I16Favor        ,
    HEncSeg25GoldenPenalty   ,
    HEncSeg25SkipPenalty     ,
    HEncSeg25InterFavor      ,
    HEncSeg25SplitPenalty16x8,
    HEncSeg25SplitPenalty8x8 ,
    HEncSeg25SplitPenalty8x4 ,
    HEncSeg25DMVPenaltyQp    ,
    HEncSeg25SplitPenalty4x4 ,

/* Segment 26 */
    HEncSeg26I4PrevModeFavor ,
    HEncSeg26I16Favor        ,
    HEncSeg26GoldenPenalty   ,
    HEncSeg26SkipPenalty     ,
    HEncSeg26InterFavor      ,
    HEncSeg26SplitPenalty16x8,
    HEncSeg26SplitPenalty8x8 ,
    HEncSeg26SplitPenalty8x4 ,
    HEncSeg26DMVPenaltyQp    ,
    HEncSeg26SplitPenalty4x4 ,

/* Segment 27 */
    HEncSeg27I4PrevModeFavor ,
    HEncSeg27I16Favor        ,
    HEncSeg27GoldenPenalty   ,
    HEncSeg27SkipPenalty     ,
    HEncSeg27InterFavor      ,
    HEncSeg27SplitPenalty16x8,
    HEncSeg27SplitPenalty8x8 ,
    HEncSeg27SplitPenalty8x4 ,
    HEncSeg27DMVPenaltyQp    ,
    HEncSeg27SplitPenalty4x4 ,

/* Segment 28 */
    HEncSeg28I4PrevModeFavor ,
    HEncSeg28I16Favor        ,
    HEncSeg28GoldenPenalty   ,
    HEncSeg28SkipPenalty     ,
    HEncSeg28InterFavor      ,
    HEncSeg28SplitPenalty16x8,
    HEncSeg28SplitPenalty8x8 ,
    HEncSeg28SplitPenalty8x4 ,
    HEncSeg28DMVPenaltyQp    ,
    HEncSeg28SplitPenalty4x4 ,

/* Segment 29 */
    HEncSeg29I4PrevModeFavor ,
    HEncSeg29I16Favor        ,
    HEncSeg29GoldenPenalty   ,
    HEncSeg29SkipPenalty     ,
    HEncSeg29InterFavor      ,
    HEncSeg29SplitPenalty16x8,
    HEncSeg29SplitPenalty8x8 ,
    HEncSeg29SplitPenalty8x4 ,
    HEncSeg29DMVPenaltyQp    ,
    HEncSeg29SplitPenalty4x4 ,

/* Segment 30 */
    HEncSeg30I4PrevModeFavor ,
    HEncSeg30I16Favor        ,
    HEncSeg30GoldenPenalty   ,
    HEncSeg30SkipPenalty     ,
    HEncSeg30InterFavor      ,
    HEncSeg30SplitPenalty16x8,
    HEncSeg30SplitPenalty8x8 ,
    HEncSeg30SplitPenalty8x4 ,
    HEncSeg30DMVPenaltyQp    ,
    HEncSeg30SplitPenalty4x4 ,

/* Segment 31 */
    HEncSeg31I4PrevModeFavor ,
    HEncSeg31I16Favor        ,
    HEncSeg31GoldenPenalty   ,
    HEncSeg31SkipPenalty     ,
    HEncSeg31InterFavor      ,
    HEncSeg31SplitPenalty16x8,
    HEncSeg31SplitPenalty8x8 ,
    HEncSeg31SplitPenalty8x4 ,
    HEncSeg31DMVPenaltyQp    ,
    HEncSeg31SplitPenalty4x4 ,

/* MBRC Control registers */
    HEncMBRCQpFrac            ,
    HEncMBComplexityOffset    ,
    HEncMBRCEnable            ,
    HEncMBQpDeltaGain         ,

/* HEncMBComplexity */
    HEncMBComplexityAverage   ,

/* Reference Frame Compression */
    HEncRfcLumBufLimit      ,
    HEncRfcIntEn            ,
    HEncRfcLumDiscreteMode  ,
    HEncRefChrCompress      ,
    HEncRefLumCompress      ,
    HEncBaseRefLumTbl       ,
    HEncBaseRefChrTbl       ,
    HEncBaseRecLumTbl       ,
    HEncBaseRecChrTbl       ,
    HEncBaseRefLumTbl2      ,
    HEncBaseRefChrTbl2      ,
    HEncRfcChrBufLimit      ,

/* Reference Management Commands */
    HEncRefReorderFlag       ,
    HEncRefReorderDiffPicNum,

/* AXI Read ID for Input */
    HEncAXIReadIDInC0,
    HEncAXIReadIDInC1,
    HEncAXIReadIDInC2,
    HEncAXIReadIDEnable,

/* MSB for 64bit address access. */
    HEncBaseRefLumTbl_MSB,
    HEncBaseRefChrTbl_MSB,
    HEncBaseRecLumTbl_MSB,
    HEncBaseRecChrTbl_MSB,
    HEncBaseRefLumTbl2_MSB,
    HEncBaseRefChrTbl2_MSB,
    HEncBaseStream_MSB,
    HEncBaseControl_MSB,
    HEncBaseRefLum_MSB,
    HEncBaseRefChr_MSB,
    HEncBaseRecLum_MSB,
    HEncBaseRecChr_MSB,
    HEncBaseInLum_MSB,
    HEncBaseInCb_MSB,
    HEncBaseInCr_MSB,
    HEncBaseRefLum2_MSB,
    HEncBaseRefChr2_MSB,
    HEncH264BaseRefLum2_MSB,
    HEncH264BaseRefChr2_MSB,
    HEncBaseNextLum_MSB,
    HEncBaseCabacCtx_MSB,
    HEncBaseMvWrite_MSB,
    HEncBasePartition1_MSB,
    HEncBasePartition2_MSB,
    HEncBaseVp8ProbCount_MSB,
    HEncBaseVp8SegmentMap_MSB,
    HEncBaseScaledOutLum_MSB,
    HEncBasePartition3_MSB,
    HEncBasePartition4_MSB,

/* Denoise Filter Control. */
    HEncDnfEnable,
    HEncDnfStregth,
    HEncDnfMbNum,
    HEncDnfNoiseMax,
    HEncDnfNoiseLevelInvC,
    HEncDnfNoiseLevelInvY,
    HEncDnfNoisePred,
    HEncDnfThresholdPred,

/* Denoise Filter Parameters. */
    HEncDnfParaS1_0,
    HEncDnfParaS1_1,
    HEncDnfParaS1_2,
    HEncDnfParaS1_3,
    HEncDnfParaS1_4,
    HEncDnfParaS1_5,
    HEncDnfParaS1_6,
    HEncDnfParaS1_7,
    HEncDnfParaS1_8,
    HEncDnfParaS1_9,
    HEncDnfParaS1_10,
    HEncDnfParaS1_11,
    HEncDnfParaS1_12,
    HEncDnfParaS1_13,
    HEncDnfParaS1_14,
    HEncDnfParaS1_15,
    HEncDnfParaS2_0,
    HEncDnfParaS2_1,
    HEncDnfParaS2_2,
    HEncDnfParaS2_3,
    HEncDnfParaS2_4,
    HEncDnfParaS2_5,
    HEncDnfParaS2_6,
    HEncDnfParaS2_7,
    HEncDnfParaS2_8,
    HEncDnfParaS2_9,
    HEncDnfParaS2_10,
    HEncDnfParaS2_11,
    HEncDnfParaS2_12,
    HEncDnfParaS2_13,
    HEncDnfParaS2_14,
    HEncDnfParaS2_15,
    HEncDnfParaS3_0,
    HEncDnfParaS3_1,
    HEncDnfParaS3_2,
    HEncDnfParaS3_3,
    HEncDnfParaS3_4,
    HEncDnfParaS3_5,
    HEncDnfParaS3_6,
    HEncDnfParaS3_7,
    HEncDnfParaS3_8,
    HEncDnfParaS3_9,
    HEncDnfParaS3_10,
    HEncDnfParaS3_11,
    HEncDnfParaS3_12,
    HEncDnfParaS3_13,
    HEncDnfParaS3_14,
    HEncDnfParaS3_15,
    HEncDnfParaS3_16,
    HEncDnfParaS3_17,
    HEncDnfParaS3_18,
    HEncDnfParaS3_19,
    HEncDnfParaS3_20,
    HEncDnfParaS3_21,
    HEncDnfParaS3_22,
    HEncDnfParaS3_23,
    HEncDnfParaS3_24,
    HEncDnfParaS3_25,
    HEncDnfParaS3_26,
    HEncDnfParaS3_27,
    HEncDnfParaS3_28,
    HEncDnfParaS3_29,
    HEncDnfParaS3_30,
    HEncDnfParaS3_31,
    HEncDnfParaS3_32,
    HEncDnfParaS3_33,
    HEncDnfParaS3_34,
    HEncDnfParaS3_35,
    HEncDnfParaS3_36,
    HEncDnfParaS3_37,
    HEncDnfParaS3_38,
    HEncDnfParaS3_39,
    HEncDnfParaS3_40,
    HEncDnfParaS3_41,
    HEncDnfParaS3_42,
    HEncDnfParaS3_43,
    HEncDnfParaS3_44,
    HEncDnfParaS3_45,
    HEncDnfParaS3_46,
    HEncDnfParaS3_47,
    HEncDnfParaS3_48,
    HEncDnfParaS3_49,
    HEncDnfParaS3_50,
    HEncDnfParaS3_51,
    HEncDnfParaS3_52,
    HEncDnfParaS3_53,
    HEncDnfParaS3_54,
    HEncDnfParaS3_55,
    HEncDnfParaS3_56,
    HEncDnfParaS3_57,
    HEncDnfParaS3_58,
    HEncDnfParaS3_59,
    HEncDnfParaS3_60,
    HEncDnfParaS3_61,
    HEncDnfParaS3_62,
    HEncDnfParaS3_63,
    HEncDnfParaS4_0,
    HEncDnfParaS4_1,
    HEncDnfParaS4_2,
    HEncDnfParaS4_3,
    HEncDnfParaS4_4,
    HEncDnfParaS4_5,
    HEncDnfParaS4_6,
    HEncDnfParaS4_7,
    HEncDnfParaS4_8,
    HEncDnfParaS4_9,
    HEncDnfParaS4_10,
    HEncDnfParaS4_11,
    HEncDnfParaS4_12,
    HEncDnfParaS4_13,
    HEncDnfParaS4_14,
    HEncDnfParaS4_15,
    HEncDnfParaS4_16,
    HEncDnfParaS4_17,
    HEncDnfParaS4_18,
    HEncDnfParaS4_19,
    HEncDnfParaS4_20,
    HEncDnfParaS4_21,
    HEncDnfParaS4_22,
    HEncDnfParaS4_23,
    HEncDnfParaS4_24,
    HEncDnfParaS4_25,
    HEncDnfParaS4_26,
    HEncDnfParaS4_27,
    HEncDnfParaS4_28,
    HEncDnfParaS4_29,
    HEncDnfParaS4_30,
    HEncDnfParaS4_31,
    HEncDnfParaS4_32,
    HEncDnfParaS4_33,
    HEncDnfParaS4_34,
    HEncDnfParaS4_35,
    HEncDnfParaS4_36,
    HEncDnfParaS4_37,
    HEncDnfParaS4_38,
    HEncDnfParaS4_39,
    HEncDnfParaS4_40,
    HEncDnfParaS4_41,
    HEncDnfParaS4_42,
    HEncDnfParaS4_43,
    HEncDnfParaS4_44,
    HEncDnfParaS4_45,
    HEncDnfParaS4_46,
    HEncDnfParaS4_47,
    HEncDnfParaS4_48,
    HEncDnfParaS4_49,
    HEncDnfParaS4_50,
    HEncDnfParaS4_51,
    HEncDnfParaS4_52,
    HEncDnfParaS4_53,
    HEncDnfParaS4_54,
    HEncDnfParaS4_55,
    HEncDnfParaS4_56,
    HEncDnfParaS4_57,
    HEncDnfParaS4_58,
    HEncDnfParaS4_59,
    HEncDnfParaS4_60,
    HEncDnfParaS4_61,
    HEncDnfParaS4_62,
    HEncDnfParaS4_63,

/* Low latency and Input Line buffer. */
    HEncMbWrPtr,
    HEncMbRdPtr,
    HEncLowLatencyHwSyncMbRows,
    HEncLowLatencyEn,
    HEncInputBufferLoopBackEn,
    HEncLowLatencyHwIntfEn,
    HEncLowLatencyIrqEn,

/* Last Fake Register for Register Counting. */

#ifdef __REGISTERS_ENUM_FAKE_H__
} regName;
#endif /*__REGISTERS_ENUM_FAKE_H__*/
