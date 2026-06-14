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
 *  Abstract : Hantro H1 H.264 Encoder API
 *
 ********************************************************************************
 */

#ifndef __H264ENCAPI_H__
#define __H264ENCAPI_H__

#include "basetype.h"
#include "enccommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \mainpage Introduction
 * This document presents the Application Programming Interface (API) of the Hantro VC8000NanoE
 * H.264 hardware based encoder. The encoder is able to encode H.264 standard [1] baseline, main
 * or high profile video streams.
 *
 * The encoder conforms to the H.264 Baseline, Main and High profiles and can encode streams up
 * to a maximum picture size of 4080x4080. The maximum bitrate and frame rate depends on the
 * number of HW cores, core clock frequency and the DRAM latency.
 *
 * This document assumes that the reader understands the fundamentals of C-language and the
 * H.264 standard.
 */

/*------------------------------------------------------------------------------
    1. Type definition for encoder instance
------------------------------------------------------------------------------*/

    typedef const void *H264EncInst;

/*------------------------------------------------------------------------------
    2. Enumerations for API parameters
------------------------------------------------------------------------------*/

/** Function return values */
    typedef enum
    {
        H264ENC_OK = 0,
        H264ENC_FRAME_READY = 1,

        H264ENC_ERROR = -1,
        H264ENC_NULL_ARGUMENT = -2,
        H264ENC_INVALID_ARGUMENT = -3,
        H264ENC_MEMORY_ERROR = -4,
        H264ENC_EWL_ERROR = -5,
        H264ENC_EWL_MEMORY_ERROR = -6,
        H264ENC_INVALID_STATUS = -7,
        H264ENC_OUTPUT_BUFFER_OVERFLOW = -8,
        H264ENC_HW_BUS_ERROR = -9,
        H264ENC_HW_DATA_ERROR = -10,
        H264ENC_HW_TIMEOUT = -11,
        H264ENC_HW_RESERVED = -12,
        H264ENC_SYSTEM_ERROR = -13,
        H264ENC_INSTANCE_ERROR = -14,
        H264ENC_HRD_ERROR = -15,
        H264ENC_HW_RESET = -16,
        H264ENC_FUSE_ERROR = -17,
    } H264EncRet;

/* Stream type for initialization */
    typedef enum
    {
        H264ENC_BYTE_STREAM = 0,    /**< H.264 annex B: NAL unit starts with
                                     * hex bytes '00 00 00 01' */
        H264ENC_NAL_UNIT_STREAM = 1 /**< Plain NAL units without startcode */
    } H264EncStreamType;

/** Stream view mode and buffer requirement for initialization */
    typedef enum
    {
        H264ENC_BASE_VIEW_DOUBLE_BUFFER = 0,    /**< H.264 stream using
                                                   two reference frame buffers */
        H264ENC_BASE_VIEW_SINGLE_BUFFER = 1,    /**< H.264 stream using
                                                   one reference frame buffers,
                                                   HRD frame discard not possible */
        H264ENC_MVC_STEREO_INTER_VIEW_PRED = 2, /**< H.264 MVC Stereo stream using
                                                   one reference frame buffer,
                                                   no HRD frame discard,
                                                   second view is always
                                                   inter-view predicted */
        H264ENC_MVC_STEREO_INTER_PRED = 3,      /**< H.264 MVC Stereo stream using
                                                   two reference frame buffers,
                                                   no HRD frame discard,
                                                   second view can be inter
                                                   predicted */
        H264ENC_BASE_VIEW_MULTI_BUFFER = 4,     /**< H.264 stream using
                                                   two or more reference frame buffers */
        H264ENC_INTERLACED_FIELD = 5,           /**< H.264 stream coded as fields
                                                   of interlaced video */
    } H264EncViewMode;

/** Level for initialization */
    typedef enum
    {
        H264ENC_LEVEL_1 = 10,
        H264ENC_LEVEL_1_b = 99,
        H264ENC_LEVEL_1_1 = 11,
        H264ENC_LEVEL_1_2 = 12,
        H264ENC_LEVEL_1_3 = 13,
        H264ENC_LEVEL_2 = 20,
        H264ENC_LEVEL_2_1 = 21,
        H264ENC_LEVEL_2_2 = 22,
        H264ENC_LEVEL_3 = 30,
        H264ENC_LEVEL_3_1 = 31,
        H264ENC_LEVEL_3_2 = 32,
        H264ENC_LEVEL_4 = 40,
        H264ENC_LEVEL_4_1 = 41,
        H264ENC_LEVEL_4_2 = 42,
        H264ENC_LEVEL_5 = 50,
        H264ENC_LEVEL_5_1 = 51
    } H264EncLevel;

/** Picture YUV type for initialization */
    typedef enum
    {
        H264ENC_YUV420_PLANAR = 0,              /**< YYYY... UUUU... VVVV...  */
        H264ENC_YUV420_SEMIPLANAR,              /**< YYYY... UVUVUV...        */
        H264ENC_YUV420_SEMIPLANAR_VU,           /**< YYYY... VUVUVU...        */
        H264ENC_YUV422_INTERLEAVED_YUYV,        /**< YUYVYUYV...              */
        H264ENC_YUV422_INTERLEAVED_UYVY,        /**< UYVYUYVY...              */
        H264ENC_RGB565,                         /**< 16-bit RGB 16bpp         */
        H264ENC_BGR565,                         /**< 16-bit RGB 16bpp         */
        H264ENC_RGB555,                         /**< 15-bit RGB 16bpp         */
        H264ENC_BGR555,                         /**< 15-bit RGB 16bpp         */
        H264ENC_RGB444,                         /**< 12-bit RGB 16bpp         */
        H264ENC_BGR444,                         /**< 12-bit RGB 16bpp         */
        H264ENC_RGB888,                         /**< 24-bit RGB 32bpp         */
        H264ENC_BGR888,                         /**< 24-bit RGB 32bpp         */
        H264ENC_RGB101010,                      /**< 30-bit RGB 32bpp         */
        H264ENC_BGR101010,                      /**< 30-bit RGB 32bpp         */
        H264ENC_SP_101010,                      /**< yuv420 10bit, semi-planar */
        H264ENC_P010                            /**< yuv420 10bit, planar */
    } H264EncPictureType;

/** Picture rotation for pre-processing */
    typedef enum
    {
        H264ENC_ROTATE_0 = 0,
        H264ENC_ROTATE_90R = 1, /**< Rotate 90 degrees clockwise */
        H264ENC_ROTATE_90L = 2,  /**< Rotate 90 degrees counter-clockwise */
        H264ENC_ROTATE_180R = 3  /**< Rotate 180 degrees clockwise */
    } H264EncPictureRotation;

/** Picture color space conversion (RGB input) for pre-processing */
    typedef enum
    {
        H264ENC_RGBTOYUV_BT601 = 0, /**< Color conversion according to BT.601 */
        H264ENC_RGBTOYUV_BT709 = 1, /**< Color conversion according to BT.709 */
        H264ENC_RGBTOYUV_USER_DEFINED = 2   /**< User defined color conversion */
    } H264EncColorConversionType;

/** Picture type for encoding */
    typedef enum
    {
        H264ENC_INTRA_FRAME = 0,        /**< IDR-frame or field */
        H264ENC_PREDICTED_FRAME = 1,    /**< P-frame or field */
        H264ENC_NONIDR_INTRA_FRAME = 2, /**< Non-IDR I-frame or field */
        H264ENC_NOTCODED_FRAME          /**< Used just as a return value */
    } H264EncPictureCodingType;

/** Reference picture mode for reading and writing */
    typedef enum
    {
        H264ENC_NO_REFERENCE_NO_REFRESH = 0,
        H264ENC_REFERENCE = 1,
        H264ENC_REFRESH = 2,
        H264ENC_REFERENCE_AND_REFRESH = 3
    } H264EncRefPictureMode;

/*------------------------------------------------------------------------------
    3. Structures for API function parameters
------------------------------------------------------------------------------*/

/**
 * Configuration info for initialization
 *
 * Width and height are picture dimensions after rotation
 * Width and height are restricted by level limitations
 * Stream Profile will be automatically decided based on parameters:
 *  - CABAC            -> Main/High Profile,
 *  - 8x8-transform    -> High Profile,
 *  - MVC              -> Stereo High Profile
 */
    typedef struct
    {
        H264EncStreamType streamType;   /**< Byte stream / Plain NAL units */
        H264EncViewMode viewMode;       /**< Mode of stream to be generated and
                                           the corresponding amount of encoder
                                           internal frame buffers required */
        H264EncLevel level;
        u32 width;           /**< Encoded picture width in pixels, multiple of 4 */
        u32 height;          /**< Encoded picture height in pixels, multiple of 2 */
        u32 frameRateNum;    /**< The stream time scale, [1..1048575] */
        u32 frameRateDenom;  /**< Maximum frame rate is frameRateNum/frameRateDenom
                              * in frames/second. The actual frame rate will be
                              * defined by timeIncrement of encoded pictures,
                              * [1..frameRateNum] */
        u32 scaledWidth;    /**< Optional down-scaled output picture width,
                               multiple of 4. 0=disabled. [16..width] */
        u32 scaledHeight;   /**< Optional down-scaled output picture height,
                               multiple of 2. [96..height]                    */
        u32 refFrameAmount; /**< Amount of reference frame buffers, [1..2]
                             * 1 = only last frame buffered,
                             * 2 = last and long term frames buffered */
        u32 refFrameCompress; /**< reference frame compress: 0=disable; 1=enable */
        u32 rfcLumBufLimit;  /**< Limit of luma RFC buffer in percent of original reference frame size. */
        u32 rfcChrBufLimit;  /**< Limit of chroma RFC buffer in percent of original reference frame size. */
        u32 svctLevel;       /**< [0~3] Max Layers number SVC Temporal Scalable Coding. */
        u32 enableSvctPrefix;       /**< [0~1] Enable SVCT prefix 0-Disable 1-Enable */
    } H264EncConfig;

/** Defining rectangular macroblock area in encoder picture */
    typedef struct
    {
        u32 enable;         /**< [0,1] Enables this area */
        u32 top;            /**< Top macroblock row inside area [0..heightMbs-1] */
        u32 left;           /**< Left macroblock row inside area [0..widthMbs-1] */
        u32 bottom;         /**< Bottom macroblock row inside area [top..heightMbs-1] */
        u32 right;          /**< Right macroblock row inside area [left..widthMbs-1] */
    } H264EncPictureArea;

/** Coding control parameters */
    typedef struct
    {
        u32 sliceSize;       /**< Slice size in macroblock rows,
                              * 0 to encode each picture in one slice,
                              * [0..height/16]
                              */
        u32 seiMessages;     /**< Insert picture timing and buffering
                              * period SEI messages into the stream,
                              * [0,1]
                              */
        u32 idrHeader;       /**< Insert SPS/PPS header to stream either
                              * for every IDR frame or only to the beginning
                              * of the sequence.
                              * [0,1]
                              */
        u32 videoFullRange;  /**< Input video signal sample range, [0,1]
                              * 0 = Y range in [16..235],
                              * Cb&Cr range in [16..240]
                              * 1 = Y, Cb and Cr range in [0..255]
                              */
        u32 constrainedIntraPrediction; /**< 0 = No constrains,
                                         * 1 = Only use intra neighbours */
        u32 disableDeblockingFilter;    /**< 0 = Filter enabled,
                                         * 1 = Filter disabled,
                                         * 2 = Filter disabled on slice edges */
        u32 sampleAspectRatioWidth; /**< Horizontal size of the sample aspect
                                     * ratio (in arbitrary units), 0 for
                                     * unspecified, [0..65535]
                                     */
        u32 sampleAspectRatioHeight;    /**< Vertical size of the sample aspect ratio
                                         * (in same units as sampleAspectRatioWidth)
                                         * 0 for unspecified, [0..65535]
                                         */
        u32 enableCabac;     /* 0 = CAVLC - Baseline profile,
                              * 1 = CABAC - Main profile,
                              * 2 = CABAC/CAVLC frame based -
                              *     Performance optimized Main profile with
                              *     Intra frames encoded using CAVLC and
                              *     Inter frames encoded using CABAC */
        u32 cabacInitIdc;    /**< [0,2] CABAC table initial value */
        u32 transform8x8Mode;   /**< Enable 8x8 transform mode, High profile
                                 * 0=disabled, 1=adaptive 8x8, 2=always 8x8 */
        u32 quarterPixelMv;     /**< 1/4 pixel motion estimation
                                 * 0=disabled, 1=adaptive, 2=enabled */
        u32 cirStart;           /**< [0..mbTotal] First macroblock for
                                   Cyclic Intra Refresh */
        u32 cirInterval;        /**< [0..mbTotal] Macroblock interval for
                                   Cyclic Intra Refresh, 0=disabled */
        u32 intraSliceMap1;     /**< Bitmap for forcing slices 0..31 to intra,
                                   LSB=slice 0, MSB=slice 31, 1=intra. */
        u32 intraSliceMap2;     /**< Bitmap for forcing slices 32..63 to intra,
                                   LSB=slice 32, MSB=slice 63, 1=intra. */
        u32 intraSliceMap3;     /**< Bitmap for forcing slices 64..95 to intra,
                                   LSB=slice 64, MSB=slice 95, 1=intra. */
        H264EncPictureArea intraArea;   /**< Area for forcing intra macroblocks */
        H264EncPictureArea roi1Area;    /**< Area for 1st Region-Of-Interest */
        H264EncPictureArea roi2Area;    /**< Area for 2nd Region-Of-Interest */
        i32 roi1DeltaQp;                /**< [-15..0] QP delta value for 1st ROI */
        i32 roi2DeltaQp;                /**< [-15..0] QP delta value for 2nd ROI */
        i32 adaptiveRoi;        /**< [-51..0] QP delta value for adaptive ROI */
        i32 adaptiveRoiColor;   /**< [-10..10] Color temperature sensitivity
                                     * for adaptive ROI skin detection.
                                     * -10 = 2000K, 0=3000K, 10=5000K        */
        i32 roiMapEnable;       /**< ROI map status, 0=disable, 1=enable. */
        i32 qpOffset[3];        /**< when roiMapEnable is 1, the qp offset for index 1,2,3. */
        u32 fieldOrder;         /**< Field order for interlaced coding,
                                   0 = bottom field first, 1 = top field first */
        u32 gdrDuration;        /**< how many pictures it will take to do GDR, if 0, not do GDR*/
        u32 svctLevel;          /**< [0~3] Max Layers number SVC Temporal Scalable Coding. */

        /* wiener denoise parameters */
        u32 noiseReductionEnable; /**<0 = disable noise reduction; 1 = enable noise reduction */
        u32 noiseLow;               /**< valid value range :[1,30] , default: 5 */
        u32 noiseLevel;             /**< valid value range :[1,30] , default :10*/

        u32 inputLineBufEn;            /**< enable input image control signals */
        u32 inputLineBufLoopBackEn;    /**< input buffer loopback mode enable */
        u32 inputLineBufDepth;         /**< input buffer depth in mb lines */
        u32 inputLineBufHwModeEn;        /**< hw handshake*/
        u32 nBaseLayerPID;             /**< priority_id of base temporal layer */
        u32 level;
        u32 enableSVC;
    } H264EncCodingCtrl;

/* Rate control parameters */
    typedef struct
    {
        u32 pictureRc;       /**< Adjust QP between pictures, [0,1] */
        u32 mbRc;            /**< Adjust QP inside picture, [0,1] */
        u32 pictureSkip;     /**< Allow rate control to skip pictures, [0,1] */
        i32 qpHdr;           /**< QP for next encoded picture, [-1..51]
                              * -1 = Let rate control calculate initial QP
                              * This QP is used for all pictures if
                              * HRD and pictureRc and mbRc are disabled
                              * If HRD is enabled it may override this QP
                              */
        u32 qpMin;           /**< Minimum QP for any picture, [0..51] */
        u32 qpMax;           /**< Maximum QP for any picture, [0..51] */
        u32 bitPerSecond;    /**< Target bitrate in bits/second, this is
                              * needed if pictureRc, mbRc, pictureSkip or
                              * hrd is enabled [10000..60000000]
                              */
        u32 hrd;             /**< Hypothetical Reference Decoder model, [0,1]
                              * restricts the instantaneous bitrate and
                              * total bit amount of every coded picture.
                              * Enabling HRD will cause tight constrains
                              * on the operation of the rate control
                              */
        u32 hrdCpbSize;      /**< Size of Coded Picture Buffer in HRD (bits) */
        u32 gopLen;          /**< Length for Group of Pictures, indicates
                              * the distance of two intra pictures,
                              * including first intra [1..300]
                              */
        i32 intraQpDelta;    /**< Intra QP delta. intraQP = QP + intraQpDelta
                              * This can be used to change the relative quality
                              * of the Intra pictures or to lower the size
                              * of Intra pictures. [-12..12]
                              */
        u32 fixedIntraQp;    /**< Fixed QP value for all Intra pictures, [0..51]
                              * 0 = Rate control calculates intra QP.
                              */
        i32 mbQpAdjustment;  /**< Encoder uses MAD thresholding to recognize
                              * macroblocks with least details. This value is
                              * used to adjust the QP of these macroblocks
                              * increasing the subjective quality. [-8..7]
                              */
        i32 longTermPicRate; /**< period between long term pic refreshes */
        i32 mbQpAutoBoost;   /**< Encoder uses motion vectors and variance to
                              * recognize background/object macroblocks.
                              * This value is used to enable the auto QP
                              * boost of these macroblocks. [0,1]
                              */
    } H264EncRateCtrl;

/* Encoder input structure */
    typedef struct
    {
        ptr_t busLuma;         /**< Bus address for input picture
                              * planar format: luminance component
                              * semiplanar format: luminance component
                              * interleaved format: whole picture
                              */
        ptr_t busChromaU;      /**< Bus address for input chrominance
                              * planar format: cb component
                              * semiplanar format: both chrominance
                              * interleaved format: not used
                              */
        ptr_t busChromaV;      /**< Bus address for input chrominance
                              * planar format: cr component
                              * semiplanar format: not used
                              * interleaved format: not used
                              */
        u32 timeIncrement;   /**< The previous picture duration in units
                              * of 1/frameRateNum. 0 for the very first picture
                              * and typically equal to frameRateDenom for the rest.
                              */
        u32 *pOutBuf;        /**< Pointer to output stream buffer */
        size_t busOutBuf;       /**< Bus address of output stream buffer */
        u32 outBufSize;      /**< Size of output stream buffer in bytes */
        H264EncPictureCodingType codingType;    /**< Proposed picture coding type,
                                                 * INTRA/PREDICTED
                                                 */
        size_t busLumaStab;     /**< bus address of next picture to stabilize (luminance) */
        H264EncRefPictureMode ipf;  /**< Immediately previous == last frame */
        H264EncRefPictureMode ltrf; /**< Long term reference frame */

        u32 lineBufWrCnt;    /**< The number of MB lines already in input MB line buffer */

        u32 sendAUD;        /**< 0=NOT send AUD, 1= send AUD */
    } H264EncIn;

/** Encoder output structure */
    typedef struct
    {
        H264EncPictureCodingType codingType;    /**< Realized picture coding type,
                                                 * INTRA/PREDICTED/NOTCODED
                                                 */
        u32 streamSize;      /**< Size of output stream in bytes */
        i8 *motionVectors;   /**< One pixel motion vector x and y and corresponding
                                SAD value for every macroblock.
                                Format: mb0x mb0y mb0sadMsb mb0sadLsb mb1x .. */
        u32 *pNaluSizeBuf;   /**< Output buffer for NAL unit sizes
                              * pNaluSizeBuf[0] = NALU 0 size in bytes
                              * pNaluSizeBuf[1] = NALU 1 size in bytes
                              * etc
                              * Zero value is written after last NALU.
                              */
        u32 numNalus;        /**< Amount of NAL units */
        u32 mse_mul256;      /**< Encoded frame Mean Squared Error
                                multiplied by 256.                            */
        size_t busScaledLuma;   /**< Bus address for scaled encoder picture luma   */
        u8 *scaledPicture;   /**< Pointer for scaled encoder picture            */
        H264EncRefPictureMode ipf; /**< Immediately previous == last frame */
        H264EncRefPictureMode ltrf; /**< Long term reference frame */
    } H264EncOut;

/** Input pre-processing */
    typedef struct
    {
        H264EncColorConversionType type;
        u16 coeffA;          /**< User defined color conversion coefficient */
        u16 coeffB;          /**< User defined color conversion coefficient */
        u16 coeffC;          /**< User defined color conversion coefficient */
        u16 coeffE;          /**< User defined color conversion coefficient */
        u16 coeffF;          /**< User defined color conversion coefficient */
    } H264EncColorConversion;

    typedef struct
    {
        u32 origWidth;                          /**< Input camera picture width */
        u32 origHeight;                         /**< Input camera picture height*/
        u32 xOffset;                            /**< Horizontal offset          */
        u32 yOffset;                            /**< Vertical offset            */
        H264EncPictureType inputType;           /**< Input picture color format */
        H264EncPictureRotation rotation;        /**< Input picture rotation     */
        u32 videoStabilization;                 /**< Enable video stabilization */
        H264EncColorConversion colorConversion; /**< Define color conversion
                                                   parameters for RGB input   */
        u32 scaledOutput;                       /**< Enable output of down-scaled
                                                   encoder picture. Dimensions
                                                   specified at Init.         */
        u32 interlacedFrame;                    /**< Enable input frame format
                                                   with two interlaced fields.
                                                   Even pictures will be read
                                                   from top field, odd pictures
                                                   from bottom field.         */
    } H264EncPreProcessingCfg;

/** Callback struct and function type. The callback is made by the encoder
 * when a slice is completed and available in the encoder stream output buffer. */

    typedef struct
    {
        u32 slicesReadyPrev;/**< Indicates how many slices were completed at
                               previous callback. This is given because
                               several slices can be completed between
                               the callbacks. */
        u32 slicesReady;    /**< Indicates how many slices are completed. */
        u32 *sliceSizes;    /**< Holds the size (bytes) of every completed slice. */
        u32 *pOutBuf;       /**< Pointer to beginning of output stream buffer. */
        void *pAppData;     /**< Pointer to application data. */
    } H264EncSliceReady;

    typedef void (*H264EncSliceReadyCallBackFunc)(H264EncSliceReady *sliceReady);

/** Version information */
    typedef struct
    {
        u32 major;           /**< Encoder API major version */
        u32 minor;           /**< Encoder API minor version */
    } H264EncApiVersion;

    typedef struct
    {
        u32 swBuild;         /**< Software build ID */
        u32 hwBuild;         /**< Hardware build ID */
    } H264EncBuild;

/*------------------------------------------------------------------------------
    4. Encoder API function prototypes
------------------------------------------------------------------------------*/

/** Version information */
    H264EncApiVersion H264EncGetApiVersion(void);
    H264EncBuild H264EncGetBuild(void);

/** Helper for input format bit-depths */
    u32 H264EncGetBitsPerPixel(H264EncPictureType type);

/** Initialization & release */
    H264EncRet H264EncInit(const H264EncConfig * pEncConfig,
                           H264EncInst * instAddr);
    H264EncRet H264EncRelease(H264EncInst inst);

/** Encoder configuration before stream generation */
    H264EncRet H264EncSetCodingCtrl(H264EncInst inst, const H264EncCodingCtrl *
                                    pCodingParams);
    H264EncRet H264EncGetCodingCtrl(H264EncInst inst, H264EncCodingCtrl *
                                    pCodingParams);

/** Encoder configuration before and during stream generation */
    H264EncRet H264EncSetRateCtrl(H264EncInst inst,
                                  const H264EncRateCtrl * pRateCtrl);
    H264EncRet H264EncGetRateCtrl(H264EncInst inst,
                                  H264EncRateCtrl * pRateCtrl);

    H264EncRet H264EncSetPreProcessing(H264EncInst inst,
                                       const H264EncPreProcessingCfg *
                                       pPreProcCfg);
    H264EncRet H264EncGetPreProcessing(H264EncInst inst,
                                       H264EncPreProcessingCfg * pPreProcCfg);

    H264EncRet H264EncSetRoiMap(H264EncInst inst, u8 *map);

/** Encoder user data insertion during stream generation */
    H264EncRet H264EncSetSeiUserData(H264EncInst inst, const u8 * pUserData,
                                     u32 userDataSize);

/** Stream generation */

/** H264EncStrmStart generates the SPS and PPS. SPS is the first NAL unit and PPS
 * is the second NAL unit. NaluSizeBuf indicates the size of NAL units.
 */
    H264EncRet H264EncStrmStart(H264EncInst inst, const H264EncIn * pEncIn,
                                H264EncOut * pEncOut);

/** H264EncStrmEncode encodes one video frame. If SEI messages are enabled the
 * first NAL unit is a SEI message. When MVC mode is selected first encoded
 * frame belongs to view=0 and second encoded frame belongs to view=1 and so on.
 * When MVC mode is selected a prefix NAL unit is generated before view=0 frames.
 */
    H264EncRet H264EncStrmEncode(H264EncInst inst, const H264EncIn * pEncIn,
                                 H264EncOut * pEncOut,
                                 H264EncSliceReadyCallBackFunc cbFunc,
                                 EncInputMBLineBufCallBackFunc lineBufCbFunc,
                                 void * pAppData);

/** H264EncStrmEnd ends a stream with an EOS code. */
    H264EncRet H264EncStrmEnd(H264EncInst inst, const H264EncIn * pEncIn,
                              H264EncOut * pEncOut);

/** Hantro internal encoder testing */
    H264EncRet H264EncSetTestId(H264EncInst inst, u32 testId);

/** Set valid input MB lines for encoder to work */
    H264EncRet H264EncSetInputMbLines(H264EncInst inst, u32 lines);

/** Get encoded lines information from encoder */
    u32 H264EncGetEncodedMbLines(H264EncInst inst);

/** Set motionVectors field of H264EncOut structure to point macroblock mbNum */
    H264EncRet H264EncGetMbInfo(H264EncInst inst, H264EncOut * pEncOut,
                                u32 mbNum);

/** Add AUD. */
    void H264AccessUnitDelimiter(stream_s *b, u32 byte_stream, u32 primary_pic_type);

/*------------------------------------------------------------------------------
    5. Encoder API tracing callback function
------------------------------------------------------------------------------*/

    void H264EncTrace(const char *msg);

#ifdef __cplusplus
}
#endif

#endif /*__H264ENCAPI_H__*/
