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
 *  Abstract  :  JPEG Encoder API
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
       Version Information
------------------------------------------------------------------------------*/

#define JPEGENC_MAJOR_VERSION 5
#define JPEGENC_MINOR_VERSION 0

#define JPEGENC_BUILD_MAJOR 5
#define JPEGENC_BUILD_MINOR 0
#define JPEGENC_BUILD_REVISION 0
#define JPEGENC_SW_BUILD ((JPEGENC_BUILD_MAJOR * 1000000) + \
(JPEGENC_BUILD_MINOR * 1000) + JPEGENC_BUILD_REVISION)

#define JPEGENC_MAX_SIZE 261121     /* 511x511 = 261121 macroblocks */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "jpegencapi.h"

#include "EncJpegInit.h"
#include "EncJpegInstance.h"
#include "EncJpegCodeFrame.h"
#include "EncJpegQuantTables.h"
#include "EncJpegMarkers.h"

#include "EncJpegPutBits.h"
#include "encasiccontroller.h"
#include "ewl.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/* Tracing macro */
#ifdef JPEGENC_TRACE
#define APITRACE(str) JpegEnc_Trace(str)
#else
#define APITRACE(str)
#endif

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

static bool_e CheckJpegCfg(const JpegEncCfg * pEncCfg);
static i32 CheckThumbnailCfg(const JpegEncThumb * pCfgThumb);
static i32 CheckFullSize(const JpegEncCfg * pCfgFull);

/*******************************************************************************
 Function name : JpegEncGetApiVersion
 Description   :
 Return type   : JpegEncApiVersion
*******************************************************************************/
JpegEncApiVersion JpegEncGetApiVersion(void)
{
    JpegEncApiVersion ver;

    ver.major = JPEGENC_MAJOR_VERSION;
    ver.minor = JPEGENC_MINOR_VERSION;
    APITRACE("JpegEncGetVersion# OK\n");
    return ver;
}

/*******************************************************************************
 Function name : JpegEncGetBuild
 Description   :
 Return type   : JpegEncBuild
*******************************************************************************/
JpegEncBuild JpegEncGetBuild(void)
{
    JpegEncBuild ver;

    ver.swBuild = JPEGENC_SW_BUILD;
    ver.hwBuild = EWLReadAsicID();
    APITRACE("JpegEncGetBuild# OK\n");
    return (ver);
}

/*******************************************************************************
 Function name : JpegEncInit
 Description   :
 Return type   : JpegEncRet
 Argument      : JpegEncCfg * pEncCfg
 Argument      : JpegEncInst * instAddr
*******************************************************************************/
JpegEncRet JpegEncInit(const JpegEncCfg * pEncCfg, JpegEncInst * instAddr)
{
    JpegEncRet ret;
    jpegInstance_s *pEncInst = NULL;

    APITRACE("JpegEncInit#");

    /* check that right shift on negative numbers is performed signed */
    /*lint -save -e* following check causes multiple lint messages */
#if (((-1) >> 1) != (-1))
#error Right bit-shifting (>>) does not preserve the sign
#endif
    /*lint -restore */

    /* Check for illegal inputs */
    if(pEncCfg == NULL || instAddr == NULL)
    {
        APITRACE("JpegEncInit: ERROR null argument");
        return JPEGENC_NULL_ARGUMENT;
    }

    /* Check that configuration is valid */
    if(CheckJpegCfg(pEncCfg) == ENCHW_NOK)
    {
        APITRACE("JpegEncInit: ERROR invalid argument");
        return JPEGENC_INVALID_ARGUMENT;
    }

    /* Initialize encoder instance and allocate memories */
    ret = JpegInit(pEncCfg, &pEncInst);
    if(ret != JPEGENC_OK)
    {
        APITRACE("JpegEncInit: ERROR Initialization failed");
        return ret;
    }

    pEncInst->inputLineBuf.inputLineBufEn = pEncCfg->inputLineBufEn;
    pEncInst->inputLineBuf.inputLineBufLoopBackEn = pEncCfg->inputLineBufLoopBackEn;
    pEncInst->inputLineBuf.inputLineBufDepth = pEncCfg->inputLineBufDepth;
    pEncInst->inputLineBuf.inputLineBufHwModeEn = pEncCfg->inputLineBufHwModeEn;

    /* hardware config */
    pEncInst->asic.regs.qp = 0;
    pEncInst->asic.regs.constrainedIntraPrediction = 0;
    pEncInst->asic.regs.frameCodingType = ASIC_INTRA;
    pEncInst->asic.regs.roundingCtrl = 0;
    pEncInst->asic.regs.codingType = ASIC_JPEG;

    /* Pre processing */
    pEncInst->preProcess.lumWidthSrc = 0;
    pEncInst->preProcess.lumHeightSrc = 0;
    pEncInst->preProcess.lumWidth = 0;
    pEncInst->preProcess.lumHeight = 0;
    pEncInst->preProcess.horOffsetSrc = 0;
    pEncInst->preProcess.verOffsetSrc = 0;
    pEncInst->preProcess.rotation = 0;
    pEncInst->preProcess.videoStab = 0;

    pEncInst->preProcess.inputFormat = pEncCfg->frameType;
    pEncInst->preProcess.colorConversionType = pEncCfg->colorConversion.type;
    pEncInst->preProcess.colorConversionCoeffA = pEncCfg->colorConversion.coeffA;
    pEncInst->preProcess.colorConversionCoeffB = pEncCfg->colorConversion.coeffB;
    pEncInst->preProcess.colorConversionCoeffC = pEncCfg->colorConversion.coeffC;
    pEncInst->preProcess.colorConversionCoeffE = pEncCfg->colorConversion.coeffE;
    pEncInst->preProcess.colorConversionCoeffF = pEncCfg->colorConversion.coeffF;

    EncSetColorConversion(&pEncInst->preProcess, &pEncInst->asic);

#ifdef TRACE_STREAM
    /* Open stream tracing */
    EncOpenStreamTrace("stream.trc");
    traceStream.frameNum = 0;
#endif

    /* Status == INIT   Initialization succesful */
    pEncInst->encStatus = ENCSTAT_INIT;

    pEncInst->inst = pEncInst;  /* used as checksum */

    *instAddr = (JpegEncInst) pEncInst;

    APITRACE("JpegEncInit: OK");
    return JPEGENC_OK;
}

/*******************************************************************************
 Function name : JpegEncRelease
 Description   :
 Return type   : JpegEncRet
 Argument      : JpegEncInst inst
*******************************************************************************/
JpegEncRet JpegEncRelease(JpegEncInst inst)
{
    jpegInstance_s *pEncInst = (jpegInstance_s *) inst;

    APITRACE("JpegEncRelease#");

    /* Check for illegal inputs */
    if(pEncInst == NULL)
    {
        APITRACE("JpegEncRelease: ERROR null argument");
        return JPEGENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("JpegEncRelease: ERROR Invalid instance");
        return JPEGENC_INSTANCE_ERROR;
    }

#ifdef TRACE_STREAM
    EncCloseStreamTrace();
#endif

    JpegShutdown(pEncInst);

    APITRACE("JpegEncRelease: OK");

    return JPEGENC_OK;
}

/*******************************************************************************
 Function name : JpegEncSetThumbnail
 Description   :
 Return type   : JpegEncRet
 Argument      : JpegEncInst inst
 Argument      : JpegEncCfg * pEncCfg
*******************************************************************************/
JpegEncRet JpegEncSetThumbnail(JpegEncInst inst, const JpegEncThumb *pJpegThumb)
{
    jpegInstance_s *pEncInst = (jpegInstance_s *) inst;

    APITRACE("JpegEncSetThumbnail#");

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pJpegThumb == NULL))
    {
        APITRACE("JpegEncSetThumbnail: ERROR null argument");
        return JPEGENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("JpegEncSetThumbnail: ERROR Invalid instance");
        return JPEGENC_INSTANCE_ERROR;
    }

    if(CheckThumbnailCfg(pJpegThumb) != JPEGENC_OK)
    {
        APITRACE("JpegEncSetThumbnail: ERROR Invalid thumbnail");
        return JPEGENC_INVALID_ARGUMENT;
    }

    pEncInst->jpeg.appn.thumbEnable = 1;

    /* save the thumbnail config */
    (void)EWLmemcpy(&pEncInst->jpeg.thumbnail, pJpegThumb, sizeof(JpegEncThumb));
    
    APITRACE("JpegEncSetThumbnail: OK");

    return JPEGENC_OK;
}

/*******************************************************************************
 Function name : JpegEncSetPictureSize
 Description   :
 Return type   : JpegEncRet
 Argument      : JpegEncInst inst
 Argument      : JpegEncCfg * pEncCfg
*******************************************************************************/
JpegEncRet JpegEncSetPictureSize(JpegEncInst inst, const JpegEncCfg * pEncCfg)
{
    u32 mbTotal = 0;
    u32 height = 0;
    u32 widthMbs;
    jpegInstance_s *pEncInst = (jpegInstance_s *) inst;

    APITRACE("JpegEncSetPictureSize#");

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pEncCfg == NULL))
    {
        APITRACE("JpegEncSetPictureSize: ERROR null argument");
        return JPEGENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("JpegEncSetPictureSize: ERROR Invalid instance");
        return JPEGENC_INSTANCE_ERROR;
    }

    if(CheckFullSize(pEncCfg) != JPEGENC_OK)
    {
        APITRACE
            ("JpegEncSetPictureSize: ERROR Out of range image dimension(s)");
        return JPEGENC_INVALID_ARGUMENT;
    }

    widthMbs = (pEncCfg->codingWidth + 15) / 16;
    if(((pEncCfg->restartInterval * 16) > pEncCfg->codingHeight) ||
       ((pEncCfg->restartInterval * widthMbs) > 0xFFFF))
    {
        APITRACE("JpegEncSetPictureSize: ERROR restart interval too big");
        return JPEGENC_INVALID_ARGUMENT;
    }

    /* Restart interval must be enabled for sliced encoding */
    if(pEncCfg->codingType == JPEGENC_SLICED_FRAME)
    {
        if(pEncCfg->rotation != JPEGENC_ROTATE_0)
        {
            APITRACE
                ("JpegEncSetPictureSize: ERROR rotation not allowed in sliced mode");
            return JPEGENC_INVALID_ARGUMENT;
        }

        if(pEncCfg->restartInterval == 0)
        {
            APITRACE
                ("JpegEncSetPictureSize: ERROR restart interval not set");
            return JPEGENC_INVALID_ARGUMENT;
        }

        /* Extra limitation for partial encoding: yOffset must be 
         * multiple of slice height in rows */
        if((pEncCfg->yOffset % (pEncCfg->restartInterval * 16)) != 0)
        {
            APITRACE("JpegEncSetPictureSize: ERROR yOffset not valid");
            return JPEGENC_INVALID_ARGUMENT;
        }
    }

    mbTotal =
        (u32) (((pEncCfg->codingWidth + 15) / 16) * ((pEncCfg->codingHeight +
                                                      15) / 16));

    pEncInst->jpeg.header = ENCHW_YES;
    pEncInst->jpeg.width = pEncCfg->codingWidth;
    pEncInst->jpeg.height = pEncCfg->codingHeight;
    /*pEncInst->jpeg.lastColumn = ((pEncInst->jpeg.width + 15) / 16);*/
    pEncInst->jpeg.mbPerFrame = mbTotal;

    /* Pre processing */
    pEncInst->preProcess.lumWidthSrc = pEncCfg->inputWidth;
    pEncInst->preProcess.lumHeightSrc = pEncCfg->inputHeight;
    pEncInst->preProcess.lumWidth = pEncCfg->codingWidth;
    pEncInst->preProcess.lumHeight = pEncCfg->codingHeight;
    pEncInst->preProcess.horOffsetSrc = pEncCfg->xOffset;
    pEncInst->preProcess.verOffsetSrc = pEncCfg->yOffset;
    pEncInst->preProcess.rotation = pEncCfg->rotation;

    /* Restart interval (MCU rows converted to macroblocks) */
    pEncInst->jpeg.restart.Ri = (u32)(pEncCfg->restartInterval * widthMbs);

    /* Coding type */
    if(pEncCfg->codingType == JPEGENC_WHOLE_FRAME)
    {
        pEncInst->jpeg.codingType = ENC_WHOLE_FRAME;
        height = pEncInst->jpeg.height;
    }
    else
    {
        /* Sliced mode */
        pEncInst->jpeg.codingType = ENC_PARTIAL_FRAME;
        pEncInst->jpeg.sliceRows = (u32) pEncCfg->restartInterval;
        height = (pEncCfg->restartInterval * 16);
    }

#ifdef JPEGENC_422_MODE_SUPPORTED
    if(pEncCfg->codingMode == JPEGENC_420_MODE)
        pEncInst->jpeg.codingMode = ENC_420_MODE;
    else
    {
        if ((pEncInst->preProcess.inputFormat != JPEGENC_YUV422_INTERLEAVED_YUYV) &&
            (pEncInst->preProcess.inputFormat != JPEGENC_YUV422_INTERLEAVED_UYVY))
        {
            APITRACE("JpegEncSetPictureSize: ERROR 4:2:0 input in 4:2:2 mode");
            return JPEGENC_INVALID_ARGUMENT;
        }
        if (pEncInst->preProcess.rotation != JPEGENC_ROTATE_0)
        {
            APITRACE("JpegEncSetPictureSize: ERROR rotation in 4:2:2 mode");
            return JPEGENC_INVALID_ARGUMENT;
        }
        pEncInst->jpeg.codingMode = ENC_422_MODE;
    }
#else
    pEncInst->jpeg.codingMode = ENC_420_MODE;
#endif

    /* Check that configuration is valid */
    if(EncPreProcessCheck(&pEncInst->preProcess) == ENCHW_NOK)
    {
        APITRACE
            ("JpegEncSetPictureSize: ERROR invalid pre-processing argument");
        return JPEGENC_INVALID_ARGUMENT;
    }

    /* Allocate internal SW/HW shared memories */
    if(EncAsicMemAlloc_V2(&pEncInst->asic, (u32) pEncInst->jpeg.width,
                          height, 0, 0, ASIC_JPEG, 0, 0) != ENCHW_OK)
    {
        APITRACE("JpegEncSetPictureSize: ERROR ewl memory allocation");
        return JPEGENC_EWL_MEMORY_ERROR;
    }

    APITRACE("JpegEncSetPictureSize: OK");

    return JPEGENC_OK;
}

/*******************************************************************************
 Function name : JpegEncEncode
 Description   :
 Return type   : JpegEncRet
 Argument      : JpegEncInst inst
 Argument      : JpegEncIn * pEncIn
 Argument      : JpegEncOut *pEncOut
*******************************************************************************/
JpegEncRet JpegEncEncode(JpegEncInst inst,
                               const JpegEncIn * pEncIn,
                               JpegEncOut * pEncOut,
                               EncInputMBLineBufCallBackFunc lineBufCbFunc,
                               void * pAppData)
{
    jpegInstance_s *pEncInst = (jpegInstance_s *) inst;
    jpegData_s *jpeg;
    asicData_s *asic;
    preProcess_s *preProcess;
    jpegEncodeFrame_e ret;

    APITRACE("JpegEncEncode#");

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pEncIn == NULL) || (pEncOut == NULL))
    {
        APITRACE("JpegEncEncode: ERROR null argument");
        return JPEGENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("JpegEncEncode: ERROR Invalid instance");
        return JPEGENC_INSTANCE_ERROR;
    }
    
    asic = &pEncInst->asic;
    jpeg = &pEncInst->jpeg;
    preProcess = &pEncInst->preProcess;

    /* Check for invalid input values */
    if((pEncIn->pOutBuf == NULL) || (pEncIn->outBufSize < 1024))
    {
        APITRACE("JpegEncEncode: ERROR Invalid output buffer");
        return JPEGENC_INVALID_ARGUMENT;
    }

    if((jpeg->appn.thumbEnable) &&
       (pEncIn->outBufSize < ((u32)jpeg->thumbnail.dataLength + 1024)))
    {
        APITRACE("JpegEncEncode: ERROR Invalid output buffer");
        return JPEGENC_INVALID_ARGUMENT;
    }

    /* Clear the output structure */
    pEncOut->jfifSize = 0;

    /* todo: check that thumbnail fits also */
    /* Set stream buffer, the size has been checked */
    if(EncJpegSetBuffer(&pEncInst->stream, (u8 *) pEncIn->pOutBuf,
                        (u32) pEncIn->outBufSize) == ENCHW_NOK)
    {
        APITRACE("JpegEncEncode: ERROR Invalid output buffer");
        return JPEGENC_INVALID_ARGUMENT;
    }

    /* Setup input line buffer */
    pEncInst->inputLineBuf.wrCnt = pEncIn->lineBufWrCnt;
    pEncInst->inputLineBuf.cbFunc = lineBufCbFunc;
    pEncInst->pAppData = pAppData;

    /* Set ASIC input image */
    asic->regs.inputLumBase = pEncIn->busLum;
    asic->regs.inputCbBase = pEncIn->busCb;
    asic->regs.inputCrBase = pEncIn->busCr;
    asic->regs.pixelsOnRow = (u32) preProcess->lumWidthSrc;
    asic->regs.outputStrmSize = pEncIn->outBufSize;
    asic->regs.outputStrmBase = pEncIn->busOutBuf;
    asic->regs.jpegMode = (jpeg->codingMode == ENC_420_MODE) ? 0 : 1;

    /* slice/restart information */
    if(jpeg->codingType == ENC_WHOLE_FRAME)
    {
        asic->regs.jpegSliceEnable = 0;
        asic->regs.jpegRestartInterval =
            (jpeg->restart.Ri / ((u32) jpeg->width / 16));
        asic->regs.jpegRestartMarker = 0;
    }
    else
    {
        asic->regs.jpegSliceEnable = 1;
        asic->regs.jpegRestartInterval = 0;
    }

    /* Check if this is start of a new frame */
    if(jpeg->sliceNum == 0)
    {
        jpeg->mbNum = 0;
        /*jpeg->column = 0;*/
        jpeg->row = 0;
    }

    /* For sliced frame, check if this slice should be encoded.
     * Vertical offset is multiple of slice height */
    if((jpeg->codingType == ENC_PARTIAL_FRAME) &&
       (preProcess->verOffsetSrc >
        (u32) (jpeg->sliceNum * jpeg->sliceRows * 16)))
    {
        jpeg->sliceNum++;
        APITRACE("JpegEncEncode: OK  restart interval");
        return JPEGENC_RESTART_INTERVAL;
    }

    /* Check if HW resource is available */
    if(EWLReserveHw(pEncInst->asic.ewl) == EWL_ERROR)
    {
        APITRACE("JpegEncEncode: ERROR hw resource unavailable");
        return JPEGENC_HW_RESERVED;
    }


    /* set the rst value for HW if RST wanted */
    if(jpeg->restart.Ri)
    {
        switch (jpeg->rstCount)
        {
        case 0:
            asic->regs.jpegRestartMarker = RST0;
            break;
        case 1:
            asic->regs.jpegRestartMarker = RST1;
            break;
        case 2:
            asic->regs.jpegRestartMarker = RST2;
            break;
        case 3:
            asic->regs.jpegRestartMarker = RST3;
            break;
        case 4:
            asic->regs.jpegRestartMarker = RST4;
            break;
        case 5:
            asic->regs.jpegRestartMarker = RST5;
            break;
        case 6:
            asic->regs.jpegRestartMarker = RST6;
            break;
        case 7:
            asic->regs.jpegRestartMarker = RST7;
            break;
        default:
            ASSERT(0);
        }

        jpeg->rstCount++;

        if(jpeg->rstCount > 7)
            jpeg->rstCount = 0;
    }

    if(jpeg->codingType == ENC_WHOLE_FRAME)
    {
        /* Adjust ASIC input image with pre-processing */
        EncPreProcess(asic, preProcess);
    }
    else
    {
        /* Set frame dimensions in slice mode for pre-processing */
        if((jpeg->row + jpeg->sliceRows) <= (jpeg->height / 16))
        {
            preProcess->lumHeight = (16 * jpeg->sliceRows);
        }
        else
        {
            preProcess->lumHeight = (jpeg->height % (jpeg->sliceRows * 16));
        }

        preProcess->verOffsetSrc = 0;

        /* Check if we need to update height for last slice (for ASIC) */
        if(((jpeg->height / 16) - jpeg->row) < jpeg->sliceRows)
        {
            asic->regs.mbsInCol = (((jpeg->height + 15) / 16) - jpeg->row);
        }

        /* check the last slice */
        i32 lastSlice = (jpeg->height / 16);
        if((jpeg->height % 16) != 0 )
            lastSlice = ((jpeg->height + 15) / 16);

        /* enable EOI writing in last slice */
        if((jpeg->row + jpeg->sliceRows) >= lastSlice)
        {
            asic->regs.jpegSliceEnable = 0;
        }

        /* Adjust ASIC input image with pre-processing */
        EncPreProcess(asic, preProcess);
    }

#ifdef TRACE_STREAM
    traceStream.id = 0; /* Stream generated by SW */
    traceStream.bitCnt = 0;  /* New frame */
#endif

    /* Enable/disable jfif header generation */
    if(pEncIn->frameHeader)
        jpeg->frame.header = ENCHW_YES;
    else
        jpeg->frame.header = ENCHW_NO;

    /* Encode one image or one slice */
    ret = EncJpegCodeFrame(pEncInst);

    if(ret != JPEGENCODE_OK)
    {
        /* Error has occured and the frame is invalid */
        JpegEncRet to_user;

        /* Error has occured and the image is invalid.
         * The image size is passed to the user and can be used for debugging */
        pEncOut->jfifSize = (i32) pEncInst->stream.byteCnt;

        switch (ret)
        {
        case JPEGENCODE_TIMEOUT:
            APITRACE("JpegEncEncode: ERROR HW timeout");
            to_user = JPEGENC_HW_TIMEOUT;
            break;
        case JPEGENCODE_HW_RESET:
            APITRACE("JpegEncEncode: ERROR HW reset detected");
            to_user = JPEGENC_HW_RESET;
            break;
        case JPEGENCODE_HW_ERROR:
            APITRACE("JpegEncEncode: ERROR HW failure");
            to_user = JPEGENC_HW_BUS_ERROR;
            break;
        case JPEGENCODE_SYSTEM_ERROR:
        default:
            /* System error has occured, encoding can't continue */
            pEncInst->encStatus = ENCSTAT_ERROR;
            APITRACE("JpegEncEncode: ERROR Fatal system error");
            to_user = JPEGENC_SYSTEM_ERROR;
        }

        return to_user;
    }

    /* Store the stream size in output structure */
    pEncOut->jfifSize = (i32) pEncInst->stream.byteCnt;

    /* Check for stream buffer overflow */
    if(pEncInst->stream.overflow == ENCHW_YES)
    {
        /* The rest of the frame is lost */
        jpeg->sliceNum = 0;
        APITRACE("JpegEncEncode: ERROR stream buffer overflow");
        return JPEGENC_OUTPUT_BUFFER_OVERFLOW;
    }

    /* Check if this is end of slice or end of frame */
    if(jpeg->mbNum < jpeg->mbPerFrame)
    {
        jpeg->sliceNum++;
        APITRACE("JpegEncEncode: OK  restart interval");
        return JPEGENC_RESTART_INTERVAL;
    }
    jpeg->sliceNum = 0;
    jpeg->rstCount = 0;

    APITRACE("JpegEncEncode: OK  frame ready");
    return JPEGENC_FRAME_READY;
}

/*******************************************************************************
 Function name : JpegEncGetBitsPerPixel
 Description   : Returns the amount of bits per pixel for given format.
 Return type   : u32 bitsPerPixel
 Argument      : JpegEncFrameType
*******************************************************************************/
u32 JpegEncGetBitsPerPixel(JpegEncFrameType type)
{
    switch (type)
    {
        case JPEGENC_YUV420_PLANAR:
        case JPEGENC_YUV420_SEMIPLANAR:
        case JPEGENC_YUV420_SEMIPLANAR_VU:
            return 12;
        case JPEGENC_YUV422_INTERLEAVED_YUYV:
        case JPEGENC_YUV422_INTERLEAVED_UYVY:
        case JPEGENC_RGB565:
        case JPEGENC_BGR565:
        case JPEGENC_RGB555:
        case JPEGENC_BGR555:
        case JPEGENC_RGB444:
        case JPEGENC_BGR444:
            return 16;
        case JPEGENC_RGB888:
        case JPEGENC_BGR888:
        case JPEGENC_RGB101010:
        case JPEGENC_BGR101010:
            return 32;
        default:
            return 0;
    }
}

/*******************************************************************************
 Function name : CheckFullSize
 Description   : Check that full image size is valid
 Return type   : JPEGENC_OK for success
 Argument      : JpegEncCfg
*******************************************************************************/
i32 CheckFullSize(const JpegEncCfg * pCfgFull)
{
    if((pCfgFull->inputWidth > 8192) || (pCfgFull->inputHeight > 8192))
    {
        return JPEGENC_ERROR;
    }

    if((pCfgFull->codingWidth < 96) || (pCfgFull->codingWidth > (511 * 16)))
    {
        return JPEGENC_ERROR;
    }

    if((pCfgFull->codingHeight < 32) || (pCfgFull->codingHeight > (511 * 16)))
    {
        return JPEGENC_ERROR;
    }

    if(((pCfgFull->codingWidth + 15) >> 4) *
       ((pCfgFull->codingHeight + 15) >> 4) > JPEGENC_MAX_SIZE)
    {
        return JPEGENC_ERROR;
    }

    if((pCfgFull->codingWidth & (3)) != 0)
    {
        return JPEGENC_ERROR;
    }

    if((pCfgFull->codingHeight & (1)) != 0)
    {
        return JPEGENC_ERROR;
    }

    if((pCfgFull->inputWidth & (15)) != 0)
    {
        return JPEGENC_ERROR;
    }

    if((pCfgFull->rotation == 0 || pCfgFull->rotation == 3) &&
    (pCfgFull->inputWidth < ((pCfgFull->codingWidth + 15) & (~15))))
    {
        return JPEGENC_ERROR;
    }
 
    if((pCfgFull->rotation == 1 || pCfgFull->rotation == 2) &&
    (pCfgFull->inputWidth < ((pCfgFull->codingHeight + 15) & (~15))))
    {
        return JPEGENC_ERROR;
    }

    return JPEGENC_OK;
}

/*******************************************************************************
 Function name : CheckThumbnailCfg
 Description   : Check that thumbnail data is valid
 Return type   : JPEGENC_OK for success
 Argument      : JpegEncCfg
*******************************************************************************/
i32 CheckThumbnailCfg(const JpegEncThumb * pCfgThumb)
{
    u16 dataLength;
    if(pCfgThumb->width < 16)  /* max size limit by data range, 8-bits */
    {
        return JPEGENC_ERROR;
    }
    if(pCfgThumb->height < 16)  /* max size limit by data range, 8-bits */
    {
        return JPEGENC_ERROR;
    }
    if(pCfgThumb->data == NULL)
    {
        return JPEGENC_ERROR;
    }

    switch(pCfgThumb->format)
    {
    case JPEGENC_THUMB_JPEG:
        {
            dataLength = ((1<<16) - 1) - 8; /* 16 bits minus the APP0 ext field count */
            if(pCfgThumb->dataLength > dataLength)
                return JPEGENC_ERROR;
        }
        break;
    case  JPEGENC_THUMB_PALETTE_RGB8:
        {
            dataLength = 3*256 + (pCfgThumb->width*pCfgThumb->height);
            if((dataLength > (((1<<16) - 1) - 10)) || /* 16 bits minus the APP0 ext field count */
               (pCfgThumb->dataLength != dataLength))
                return JPEGENC_ERROR;
        }
        break;
    case JPEGENC_THUMB_RGB24:
        {
            dataLength = (3*pCfgThumb->width*pCfgThumb->height);
            if((dataLength > (((1<<16) - 1) - 10)) || /* 16 bits minus the APP0 ext field count */
               (pCfgThumb->dataLength != dataLength))
                return JPEGENC_ERROR;
        }
        break;
    default:
        return JPEGENC_ERROR;
    }

    return JPEGENC_OK;
}

/*******************************************************************************
 Function name : CheckJpegCfg
 Description   : Check that all values in the input structure are valid
 Return type   : bool_e
 Argument      : JpegEncCfg
*******************************************************************************/
bool_e CheckJpegCfg(const JpegEncCfg * pEncCfg)
{
    /* check HW limitations */
    {
        EWLHwConfig_t cfg = EWLReadAsicConfig();

        /* is JPEG encoding supported */
        if(cfg.jpegEnabled == EWL_HW_CONFIG_NOT_SUPPORTED)
        {
            return ENCHW_NOK;
        }
        if(cfg.rgbEnabled == EWL_HW_CONFIG_NOT_SUPPORTED &&
           pEncCfg->frameType > JPEGENC_YUV422_INTERLEAVED_UYVY)
        {
            return ENCHW_NOK;
        }
    }

    if(pEncCfg->qLevel > 10)
        return ENCHW_NOK;

    if(pEncCfg->frameType > JPEGENC_BGR101010)
        return ENCHW_NOK;

    if(pEncCfg->codingType != JPEGENC_WHOLE_FRAME &&
       pEncCfg->codingType != JPEGENC_SLICED_FRAME)
        return ENCHW_NOK;

    /* Units type must be valid */
    if(pEncCfg->unitsType != JPEGENC_NO_UNITS &&
       pEncCfg->unitsType != JPEGENC_DOTS_PER_INCH &&
       pEncCfg->unitsType != JPEGENC_DOTS_PER_CM)
        return ENCHW_NOK;

    /* Xdensity and Ydensity must valid */
    if((pEncCfg->xDensity > 0xFFFFU) ||
       (pEncCfg->yDensity > 0xFFFFU))
        return ENCHW_NOK;

    /* COM header length */
    if((pEncCfg->comLength > 0xFFFDU) ||
       (pEncCfg->comLength  != 0 && pEncCfg->pCom == NULL))
        return ENCHW_NOK;

    /* Marker type must be valid */
    if(pEncCfg->markerType != JPEGENC_SINGLE_MARKER &&
       pEncCfg->markerType != JPEGENC_MULTI_MARKER)
        return ENCHW_NOK;

    /* Input Line buffer check */
    if(pEncCfg->inputLineBufEn)
    {
       /* check zero depth */
       if ((pEncCfg->inputLineBufDepth == 0) &&
           (pEncCfg->inputLineBufLoopBackEn || pEncCfg->inputLineBufHwModeEn))
          return ENCHW_NOK;

       /* check depth*mb_width align to 4 */
       {
           i32 mbW = (pEncCfg->codingWidth+15)/16;
           if ((mbW * pEncCfg->inputLineBufDepth) & 3)
               return ENCHW_NOK;
       }

       /* not support ratation */
       if (pEncCfg->rotation)
           return ENCHW_NOK;
    }

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------
    Function name : JpegEncGetEncodedMbLines
    Description   : Get how many MB lines has been encoded by encoder.
    Return type   : u32
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
u32 JpegEncGetEncodedMbLines(JpegEncInst inst)
{
	jpegInstance_s *pEncInst = (jpegInstance_s *) inst;
	u32 lines;

	APITRACE("JpegEncGetEncodedMbLines#");

	/* Check for illegal inputs */
	if (!pEncInst) {
		APITRACE("JpegEncGetEncodedMbLines: ERROR Null argument");
		return JPEGENC_NULL_ARGUMENT;
	}

	if (!pEncInst->inputLineBuf.inputLineBufEn) {
		APITRACE("JpegEncGetEncodedMbLines: ERROR Invalid mode for input control");
		return JPEGENC_INVALID_ARGUMENT;
	}

    lines = EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncMbRdPtr);
	return lines;
}

/*------------------------------------------------------------------------------
    Function name : JpegEncSetInputMBLines
    Description   : Set the input buffer lines available of current picture.
    Return type   : JpegEncRet
    Argument      : inst - encoder instance
    Argument      : lines - number of macroblock lines
------------------------------------------------------------------------------*/
JpegEncRet JpegEncSetInputMBLines(JpegEncInst inst, u32 lines)
{
	jpegInstance_s *pEncInst = (jpegInstance_s *) inst;

	APITRACE("JpegEncSetInputMBLines#");

	/* Check for illegal inputs */
	if (!pEncInst) {
		APITRACE("JpegEncSetInputMBLines: ERROR Null argument");
		return JPEGENC_NULL_ARGUMENT;
	}

	if (!pEncInst->inputLineBuf.inputLineBufEn) {
		APITRACE("JpegEncSetInputMBLines: ERROR Invalid mode for input control");
		return JPEGENC_INVALID_ARGUMENT;
	}

	EncAsicWriteRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncMbWrPtr, lines);
	return JPEGENC_OK;
}

