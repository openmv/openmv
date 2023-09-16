/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Baseline JPEG decoder.
 */
#include "imlib_config.h"
#include "omv_boardconfig.h"
#include "py/obj.h"
#include "py/nlr.h"
#include "py/runtime.h"
#define TIME_JPEG    (0)
#if (TIME_JPEG == 1)
#include "py/mphal.h"
#include <stdio.h>
#endif
#include "imlib.h"

static bool jpeg_is_progressive(image_t *img) {
    uint8_t *data = img->data;
    for (uint32_t i = 0; i < img->size;) {
        uint8_t marker = data[i++];
        if (marker == 0xFF) {
            continue;
        } else if (marker == 0xD8) {
            //SOI
            continue;
        } else if (marker == 0xC0) {
            //SOF0/baseline
            return false;
        } else if (marker == 0xC2) {
            //SOF2/progressive
            return true;
        } else if (marker == 0xC2) {
            //EOI
            return false;
        } else if (marker == 0xDD) {
            //DRI 4 bytes payload.
            i += 4;
        } else if (marker >= 0xd0 && marker <= 0xd7) {
            //RSTn
            continue;
        } else if (i + 2 < img->size) {
            // Other markers, variable size payload.
            i += __REV16(*((uint16_t *) (&data[i])));
        } else {
            return false;
        }
    }
    return false;
}

#if 0 //(OMV_HARDWARE_JPEG == 1)
/* Hardware JPEG decoder */
#include STM32_HAL_H
#define JPEG_MCU_SIZE_444     (192)
#define JPEG_MCU_SIZE_422     (256)
#define JPEG_MCU_SIZE_420     (384)
#define JPEG_MCU_SIZE_GRAY    (64)
#define debug_printf(...)     //printf(__VA_ARGS__)

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t *pixels;
    uint32_t bpp;
    uint32_t error;
    pixformat_t pixfmt;
    uint32_t mcu_count;
    uint32_t mcu_width;
    uint32_t mcu_height;
    uint32_t mcu_per_line;
    uint8_t *mcu_buffer[2];
    uint32_t mcu_buffer_size;
} jpeg_state_t;

static jpeg_state_t *jpeg_state = NULL;
static JPEG_HandleTypeDef hjpeg = {0};
static DMA2D_HandleTypeDef hdma2d = {0};

static int dma2d_config(uint32_t pixfmt, uint32_t colorspace, uint32_t subsampling) {
    // Initial DMA2D configuration.
    hdma2d.Instance = DMA2D;
    HAL_DMA2D_DeInit(&hdma2d);
    hdma2d.XferCpltCallback = NULL;
    hdma2d.XferErrorCallback = NULL;

    // Configure DMA2D output.
    hdma2d.Init.OutputOffset = 0;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;                 // Ignored in M2M mode.
    hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;
    hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
    hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
    hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;

    // Configure DMA2D input.
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0xFF;
    hdma2d.LayerCfg[1].InputOffset = 0;
    hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
    hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
    hdma2d.LayerCfg[1].ChromaSubSampling = subsampling;

    if (pixfmt == PIXFORMAT_GRAYSCALE && colorspace == JPEG_GRAYSCALE_COLORSPACE) {
        hdma2d.Init.Mode = DMA2D_M2M;                  // GS -> GS, no PFC.
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8;
    } else if (pixfmt == PIXFORMAT_RGB565 && colorspace == JPEG_YCBCR_COLORSPACE) {
        hdma2d.Init.Mode = DMA2D_M2M_PFC;
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_YCBCR;
    } else {
        // Unsupported pixel format conversion
        return -1;
    }

    // DMA2D initialization.
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
    return 0;
}

static void dma2d_copy_mcu(uint32_t x, uint32_t y, uint32_t mcu_width, uint32_t mcu_height, uint8_t *mcu_buffer) {
    // Wait for previous transfer.
    if (HAL_DMA2D_PollForTransfer(&hdma2d, 1000) != HAL_OK) {
        // TODO abort transfer and decoding, and set error flag.
    }

    // Configure DMA2D output.
    hdma2d.Init.OutputOffset = jpeg_state->width - mcu_width;
    hdma2d.LayerCfg[1].InputOffset = jpeg_state->mcu_width - mcu_width;

    // DMA2D initialization.
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);

    uint8_t *dst = jpeg_state->pixels + (y * jpeg_state->width + x) * jpeg_state->bpp;

    // Output size in pixels per line and number of lines
    HAL_DMA2D_Start(&hdma2d, (uint32_t) mcu_buffer, (uint32_t) dst, mcu_width, mcu_height);
}

static void jpeg_info_ready_callback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *jpeg_info) {
    uint32_t subsampling = DMA2D_NO_CSS;
    debug_printf("colorspace %ld subsampling %ld width %ld height %ld quality %ld\n",
                 jpeg_info->ColorSpace, jpeg_info->ChromaSubsampling,
                 jpeg_info->ImageWidth, jpeg_info->ImageHeight, jpeg_info->ImageQuality);

    switch (jpeg_info->ChromaSubsampling) {
        case JPEG_420_SUBSAMPLING:
            subsampling = DMA2D_CSS_420;
            jpeg_state->mcu_width = 16;
            jpeg_state->mcu_height = 16;
            jpeg_state->mcu_buffer_size = JPEG_MCU_SIZE_420;
            break;
        case JPEG_422_SUBSAMPLING:
            subsampling = DMA2D_CSS_422;
            jpeg_state->mcu_width = 16;
            jpeg_state->mcu_height = 8;
            jpeg_state->mcu_buffer_size = JPEG_MCU_SIZE_422;
            break;
        case JPEG_444_SUBSAMPLING:
            subsampling = DMA2D_NO_CSS;
            jpeg_state->mcu_width = 8;
            jpeg_state->mcu_height = 8;
            if (jpeg_info->ColorSpace == JPEG_YCBCR_COLORSPACE) {
                jpeg_state->mcu_buffer_size = JPEG_MCU_SIZE_444;
            } else {
                // Grayscale
                jpeg_state->mcu_buffer_size = JPEG_MCU_SIZE_GRAY;
            }
            break;
        default: // unknown subsampling.
            goto config_error;
    }

    if (dma2d_config(jpeg_state->pixfmt, jpeg_info->ColorSpace, subsampling) != 0) {
        goto config_error;
    }

    // Allocate MCU buffer(s).
    jpeg_state->mcu_buffer[0] = fb_alloc(jpeg_state->mcu_buffer_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);
    jpeg_state->mcu_buffer[1] = fb_alloc(jpeg_state->mcu_buffer_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);
    jpeg_state->mcu_per_line = (jpeg_state->width / jpeg_state->mcu_width) + !!(jpeg_state->width % jpeg_state->mcu_width);

    // Set JPEG output buffer.
    HAL_JPEG_ConfigOutputBuffer(hjpeg, jpeg_state->mcu_buffer[0], jpeg_state->mcu_buffer_size);
    return;

config_error:
    HAL_JPEG_Abort(hjpeg);
    jpeg_state->error = true;
    return;
}

static void jpeg_data_ready_callback(JPEG_HandleTypeDef *hjpeg, uint8_t *mcu_buffer, uint32_t length) {
    uint32_t mcu_width = jpeg_state->mcu_width;
    uint32_t mcu_height = jpeg_state->mcu_height;
    uint32_t x = (jpeg_state->mcu_count % jpeg_state->mcu_per_line) * mcu_width;
    uint32_t y = (jpeg_state->mcu_count / jpeg_state->mcu_per_line) * mcu_height;

    if ((x + mcu_width) > jpeg_state->width) {
        mcu_width = jpeg_state->width - x;
        debug_printf("mcu: %lu mcu_perline %lu x:%lu y:%lu mcu_w:%lu mcu_cropped_w:%lu\n",
                     jpeg_state->mcu_count % jpeg_state->mcu_per_line, jpeg_state->mcu_per_line,
                     x, y, jpeg_state->mcu_width, mcu_width);
    }

    if ((y + mcu_height) > jpeg_state->height) {
        mcu_height = jpeg_state->height - y;
        debug_printf("mcu: %ld mcu_perline %lu x:%lu y:%lu mcu_h:%lu mcu_cropped_h:%lu\n",
                     jpeg_state->mcu_count % jpeg_state->mcu_per_line, jpeg_state->mcu_per_line,
                     x, y, jpeg_state->mcu_height, mcu_height);
    }

    // Flush MCU buffer cache for DMA2D.
    SCB_CleanDCache_by_Addr((uint32_t *) mcu_buffer, jpeg_state->mcu_buffer_size);

    // Copy MCU to target frame buffer.
    dma2d_copy_mcu(x, y, mcu_width, mcu_height, mcu_buffer);

    if (mcu_buffer == jpeg_state->mcu_buffer[0]) {
        HAL_JPEG_ConfigOutputBuffer(hjpeg, jpeg_state->mcu_buffer[1], jpeg_state->mcu_buffer_size);
    } else {
        HAL_JPEG_ConfigOutputBuffer(hjpeg, jpeg_state->mcu_buffer[0], jpeg_state->mcu_buffer_size);
    }
    jpeg_state->mcu_count++;
}

void jpeg_decompress(image_t *dst, image_t *src) {
    hjpeg.Instance = JPEG;
    HAL_JPEG_DeInit(&hjpeg);
    HAL_JPEG_Init(&hjpeg);

    // Register JPEG callbacks.
    HAL_JPEG_RegisterDataReadyCallback(&hjpeg, jpeg_data_ready_callback);
    HAL_JPEG_RegisterInfoReadyCallback(&hjpeg, jpeg_info_ready_callback);

    // Supports decoding to Grayscale or RGB565 only.
    if (dst->pixfmt != PIXFORMAT_GRAYSCALE && dst->pixfmt != PIXFORMAT_RGB565) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Unsupported format."));
    }

    // Supports decoding baseline JPEGs only.
    if (jpeg_is_progressive(src)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Progressive JPEG is not supported."));
    }

    // Set/reset JPEG state.
    jpeg_state_t jpeg_state_stk = {
        .width = dst->w,
        .height = dst->h,
        .pixels = dst->data,
        .bpp = dst->bpp,
        .error = false,
        .pixfmt = dst->pixfmt,
        .mcu_count = 0,
        .mcu_width = 0,
        .mcu_height = 0,
        .mcu_per_line = 0,
        .mcu_buffer = {NULL, NULL},
        .mcu_buffer_size = 0,
    };

    jpeg_state = &jpeg_state_stk;

    #if (TIME_JPEG == 1)
    mp_uint_t start = mp_hal_ticks_ms();
    #endif

    uint8_t buf[1024];
    // Start decoding JPEG headers.
    if (HAL_JPEG_Decode(&hjpeg, src->data, src->size, buf, sizeof(buf), 3000) != HAL_OK || jpeg_state->error) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("JPEG decoder failed"));
    }

    if (jpeg_state->mcu_buffer[0] != NULL) {
        // Free MCU buffer(s).
        fb_free();
        fb_free();
    }

    // Invalidate the dest image cache.
    SCB_InvalidateDCache_by_Addr(dst->pixels, image_size(dst));

    #if (TIME_JPEG == 1)
    printf("time: %u ms\n", mp_hal_ticks_ms() - start);
    #endif
}
#else
/* Software JPEG decoder */
#define FILE_HIGHWATER         1536
#define JPEG_FILE_BUF_SIZE     2048
#define HUFF_TABLEN            273
#define HUFF11SIZE             (1 << 11)
#define DC_TABLE_SIZE          1024
#define DCTSIZE                64
#define MAX_MCU_COUNT          6
#define MAX_COMPS_IN_SCAN      4
#define MAX_BUFFERED_PIXELS    2048

// Decoder options
#define JPEG_AUTO_ROTATE       1
#define JPEG_SCALE_HALF        2
#define JPEG_SCALE_QUARTER     4
#define JPEG_SCALE_EIGHTH      8
#define JPEG_LE_PIXELS         16
#define JPEG_EXIF_THUMBNAIL    32
#define JPEG_LUMA_ONLY         64

#define MCU0                   (DCTSIZE * 0)
#define MCU1                   (DCTSIZE * 1)
#define MCU2                   (DCTSIZE * 2)
#define MCU3                   (DCTSIZE * 3)
#define MCU4                   (DCTSIZE * 4)
#define MCU5                   (DCTSIZE * 5)

// Pixel types (defaults to little endian RGB565)
enum {
    RGB565_LITTLE_ENDIAN = 0,
    RGB565_BIG_ENDIAN,
    EIGHT_BIT_GRAYSCALE,
    ONE_BIT_GRAYSCALE,
    FOUR_BIT_DITHERED,
    TWO_BIT_DITHERED,
    ONE_BIT_DITHERED,
    INVALID_PIXEL_TYPE
};

enum {
    JPEG_MEM_RAM=0,
    JPEG_MEM_FLASH
};

// Error codes returned by getLastError()
enum {
    JPEG_SUCCESS = 0,
    JPEG_INVALID_PARAMETER,
    JPEG_DECODE_ERROR,
    JPEG_UNSUPPORTED_FEATURE,
    JPEG_INVALID_FILE
};

typedef struct buffered_bits {
    unsigned char *pBuf; // buffer pointer
    uint32_t ulBits;     // buffered bits
    uint32_t ulBitOff;   // current bit offset
} BUFFERED_BITS;

typedef struct jpeg_file_tag {
    int32_t iPos;   // current file position
    int32_t iSize;  // file size
    uint8_t *pData; // memory file pointer
    void *fHandle;  // class pointer to File/SdFat or whatever you want
} JPEGFILE;

typedef struct jpeg_draw_tag {
    int x, y;            // upper left corner of current MCU
    int iWidth, iHeight; // size of this MCU
    int iBpp;            // bit depth of the pixels (8 or 16)
    uint16_t *pPixels;   // 16-bit pixels
    void *pUser;
} JPEGDRAW;

// Callback function prototypes
typedef int32_t (JPEG_READ_CALLBACK) (JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen);
typedef int32_t (JPEG_SEEK_CALLBACK) (JPEGFILE *pFile, int32_t iPosition);
typedef int (JPEG_DRAW_CALLBACK) (JPEGDRAW *pDraw);
typedef void * (JPEG_OPEN_CALLBACK) (const char *szFilename, int32_t *pFileSize);
typedef void (JPEG_CLOSE_CALLBACK) (void *pHandle);

/* JPEG color component info */
typedef struct _jpegcompinfo {
    // These values are fixed over the whole image
    // For compression, they must be supplied by the user interface
    // for decompression, they are read from the SOF marker.
    unsigned char component_needed; /*  do we need the value of this component? */
    unsigned char component_id;     /* identifier for this component (0..255) */
    unsigned char component_index;  /* its index in SOF or cinfo->comp_info[] */
    // unsigned char h_samp_factor; /* horizontal sampling factor (1..4) */
    // unsigned char v_samp_factor; /* vertical sampling factor (1..4) */
    unsigned char quant_tbl_no;     /* quantization table selector (0..3) */
    // These values may vary between scans
    // For compression, they must be supplied by the user interface
    // for decompression, they are read from the SOS marker.
    unsigned char dc_tbl_no; /* DC entropy table selector (0..3) */
    unsigned char ac_tbl_no; /* AC entropy table selector (0..3) */
    // These values are computed during compression or decompression startup
    // int true_comp_width; /* component's image width in samples */
    // int true_comp_height; /* component's image height in samples */
    // the above are the logical dimensions of the downsampled image
    // These values are computed before starting a scan of the component
    // int MCU_width; /* number of blocks per MCU, horizontally */
    // int MCU_height; /* number of blocks per MCU, vertically */
    // int MCU_blocks; /* MCU_width * MCU_height */
    // int downsampled_width; /* image width in samples, after expansion */
    // int downsampled_height; /* image height in samples, after expansion */
    // the above are the true_comp_xxx values rounded up to multiples of
    // the MCU dimensions; these are the working dimensions of the array
    // as it is passed through the DCT or IDCT step.  NOTE: these values
    // differ depending on whether the component is interleaved or not!!
    // This flag is used only for decompression.  In cases where some of the
    // components will be ignored (eg grayscale output from YCbCr image),
    // we can skip IDCT etc. computations for the unused components.
} JPEGCOMPINFO;

//
// our private structure to hold a JPEG image decode state
//
typedef struct jpeg_image_tag {
    int iWidth, iHeight;           // image size
    int iThumbWidth, iThumbHeight; // thumbnail size (if present)
    int iThumbData;                // offset to image data
    int iXOffset, iYOffset;        // placement on the display
    void *pUser;
    uint8_t ucBpp, ucSubSample, ucHuffTableUsed;
    uint8_t ucMode, ucOrientation, ucHasThumb, b11Bit;
    uint8_t ucComponentsInScan, cApproxBitsLow, cApproxBitsHigh;
    uint8_t iScanStart, iScanEnd, ucFF, ucNumComponents;
    uint8_t ucACTable, ucDCTable, ucMaxACCol, ucMaxACRow;
    uint8_t ucMemType, ucPixelType;
    int iEXIF;                   // Offset to EXIF 'TIFF' file
    int iError;
    int iOptions;
    int iVLCOff;                 // current VLC data offset
    int iVLCSize;                // current quantity of data in the VLC buffer
    int iResInterval, iResCount; // restart interval
    int iMaxMCUs;                // max MCUs of pixels per JPEGDraw call
    JPEG_READ_CALLBACK *pfnRead;
    JPEG_SEEK_CALLBACK *pfnSeek;
    JPEG_DRAW_CALLBACK *pfnDraw;
    JPEG_OPEN_CALLBACK *pfnOpen;
    JPEG_CLOSE_CALLBACK *pfnClose;
    JPEGCOMPINFO JPCI[MAX_COMPS_IN_SCAN]; /* Max color components */
    JPEGFILE JPEGFile;
    BUFFERED_BITS bb;
    uint8_t *pImage;
    uint8_t *pDitherBuffer;                 // provided externally to do Floyd-Steinberg dithering
    uint16_t usPixels[MAX_BUFFERED_PIXELS];
    int16_t sMCUs[DCTSIZE * MAX_MCU_COUNT]; // 4:2:0 needs 6 DCT blocks per MCU
    int16_t sQuantTable[DCTSIZE * 4];       // quantization tables
    uint8_t ucFileBuf[JPEG_FILE_BUF_SIZE];  // holds temp data and pixel stack
    uint8_t ucHuffDC[DC_TABLE_SIZE * 2];    // up to 2 'short' tables
    uint16_t usHuffAC[HUFF11SIZE * 2];
} JPEGIMAGE;

int JPEG_openRAM(JPEGIMAGE *pJPEG, uint8_t *pData, int iDataSize, uint8_t *pImage);
int JPEG_openFile(JPEGIMAGE *pJPEG, const char *szFilename, JPEG_DRAW_CALLBACK *pfnDraw);
int JPEG_getWidth(JPEGIMAGE *pJPEG);
int JPEG_getHeight(JPEGIMAGE *pJPEG);
int JPEG_decode(JPEGIMAGE *pJPEG, int x, int y, int iOptions);
int JPEG_decodeDither(JPEGIMAGE *pJPEG, uint8_t *pDither, int iOptions);
void JPEG_close(JPEGIMAGE *pJPEG);
int JPEG_getLastError(JPEGIMAGE *pJPEG);
int JPEG_getOrientation(JPEGIMAGE *pJPEG);
int JPEG_getBpp(JPEGIMAGE *pJPEG);
int JPEG_getSubSample(JPEGIMAGE *pJPEG);
int JPEG_hasThumb(JPEGIMAGE *pJPEG);
int JPEG_getThumbWidth(JPEGIMAGE *pJPEG);
int JPEG_getThumbHeight(JPEGIMAGE *pJPEG);
int JPEG_getLastError(JPEGIMAGE *pJPEG);
void JPEG_setPixelType(JPEGIMAGE *pJPEG, int iType); // defaults to little endian
void JPEG_setMaxOutputSize(JPEGIMAGE *pJPEG, int iMaxMCUs);

// Due to unaligned memory causing an exception, we have to do these macros the slow way
#define INTELSHORT(p)     (*(uint16_t *) p)
#define INTELLONG(p)      (*(uint32_t *) p)
#define MOTOSHORT(p)      __builtin_bswap16(*(uint16_t *) p)
#define MOTOLONG(p)       __builtin_bswap32(*(uint32_t *) p)

// Must be a 32-bit target processor
#define REGISTER_WIDTH    32

// forward references
static int JPEGInit(JPEGIMAGE *pJPEG);
static int JPEGParseInfo(JPEGIMAGE *pPage, int bExtractThumb);
static void JPEGGetMoreData(JPEGIMAGE *pPage);
static int DecodeJPEG(JPEGIMAGE *pImage);
static int32_t readRAM(JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen);
static int32_t seekMem(JPEGFILE *pFile, int32_t iPosition);

/* JPEG tables */
// zigzag ordering of DCT coefficients
static const unsigned char cZigZag[64] = {
    0,  1,  5,  6,  14, 15, 27, 28,
    2,  4,  7,  13, 16, 26, 29, 42,
    3,  8,  12, 17, 25, 30, 41, 43,
    9,  11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

// un-zigzag ordering
static const unsigned char cZigZag2[64] = {
    0,  1,  8,  16, 9,  2, 3,  10,
    17, 24, 32, 25, 18, 11, 4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6,  7,  14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// For AA&N IDCT method, multipliers are equal to quantization
// coefficients scaled by scalefactor[row]*scalefactor[col], where
// scalefactor[0] = 1
// scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
// For integer operation, the multiplier table is to be scaled by
// IFAST_SCALE_BITS.
static const int iScaleBits[64] = {
    16384, 22725, 21407, 19266, 16384, 12873, 8867,  4520,
    22725, 31521, 29692, 26722, 22725, 17855, 12299, 6270,
    21407, 29692, 27969, 25172, 21407, 16819, 11585, 5906,
    19266, 26722, 25172, 22654, 19266, 15137, 10426, 5315,
    16384, 22725, 21407, 19266, 16384, 12873, 8867,  4520,
    12873, 17855, 16819, 15137, 12873, 10114, 6967,  3552,
    8867,  12299, 11585, 10426, 8867,  6967,  4799,  2446,
    4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};

// Range clip and shift for RGB565 output
// input value is 0 to 255, then another 256 for overflow to FF, then 512 more for negative values wrapping around
// Trims a few instructions off the final output stage
static const uint8_t ucRangeTable[] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

static const uint16_t usGrayTo565[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0020, 0x0020, 0x0020,
    0x0841, 0x0841, 0x0841, 0x0841, 0x0861, 0x0861, 0x0861, 0x0861,
    0x1082, 0x1082, 0x1082, 0x1082, 0x10a2, 0x10a2, 0x10a2, 0x10a2,
    0x18c3, 0x18c3, 0x18c3, 0x18c3, 0x18e3, 0x18e3, 0x18e3, 0x18e3,
    0x2104, 0x2104, 0x2104, 0x2104, 0x2124, 0x2124, 0x2124, 0x2124,
    0x2945, 0x2945, 0x2945, 0x2945, 0x2965, 0x2965, 0x2965, 0x2965,
    0x3186, 0x3186, 0x3186, 0x3186, 0x31a6, 0x31a6, 0x31a6, 0x31a6,
    0x39c7, 0x39c7, 0x39c7, 0x39c7, 0x39e7, 0x39e7, 0x39e7, 0x39e7,
    0x4208, 0x4208, 0x4208, 0x4208, 0x4228, 0x4228, 0x4228, 0x4228,
    0x4a49, 0x4a49, 0x4a49, 0x4a49, 0x4a69, 0x4a69, 0x4a69, 0x4a69,
    0x528a, 0x528a, 0x528a, 0x528a, 0x52aa, 0x52aa, 0x52aa, 0x52aa,
    0x5acb, 0x5acb, 0x5acb, 0x5acb, 0x5aeb, 0x5aeb, 0x5aeb, 0x5aeb,
    0x630c, 0x630c, 0x630c, 0x630c, 0x632c, 0x632c, 0x632c, 0x632c,
    0x6b4d, 0x6b4d, 0x6b4d, 0x6b4d, 0x6b6d, 0x6b6d, 0x6b6d, 0x6b6d,
    0x738e, 0x738e, 0x738e, 0x738e, 0x73ae, 0x73ae, 0x73ae, 0x73ae,
    0x7bcf, 0x7bcf, 0x7bcf, 0x7bcf, 0x7bef, 0x7bef, 0x7bef, 0x7bef,
    0x8410, 0x8410, 0x8410, 0x8410, 0x8430, 0x8430, 0x8430, 0x8430,
    0x8c51, 0x8c51, 0x8c51, 0x8c51, 0x8c71, 0x8c71, 0x8c71, 0x8c71,
    0x9492, 0x9492, 0x9492, 0x9492, 0x94b2, 0x94b2, 0x94b2, 0x94b2,
    0x9cd3, 0x9cd3, 0x9cd3, 0x9cd3, 0x9cf3, 0x9cf3, 0x9cf3, 0x9cf3,
    0xa514, 0xa514, 0xa514, 0xa514, 0xa534, 0xa534, 0xa534, 0xa534,
    0xad55, 0xad55, 0xad55, 0xad55, 0xad75, 0xad75, 0xad75, 0xad75,
    0xb596, 0xb596, 0xb596, 0xb596, 0xb5b6, 0xb5b6, 0xb5b6, 0xb5b6,
    0xbdd7, 0xbdd7, 0xbdd7, 0xbdd7, 0xbdf7, 0xbdf7, 0xbdf7, 0xbdf7,
    0xc618, 0xc618, 0xc618, 0xc618, 0xc638, 0xc638, 0xc638, 0xc638,
    0xce59, 0xce59, 0xce59, 0xce59, 0xce79, 0xce79, 0xce79, 0xce79,
    0xd69a, 0xd69a, 0xd69a, 0xd69a, 0xd6ba, 0xd6ba, 0xd6ba, 0xd6ba,
    0xdedb, 0xdedb, 0xdedb, 0xdedb, 0xdefb, 0xdefb, 0xdefb, 0xdefb,
    0xe71c, 0xe71c, 0xe71c, 0xe71c, 0xe73c, 0xe73c, 0xe73c, 0xe73c,
    0xef5d, 0xef5d, 0xef5d, 0xef5d, 0xef7d, 0xef7d, 0xef7d, 0xef7d,
    0xf79e, 0xf79e, 0xf79e, 0xf79e, 0xf7be, 0xf7be, 0xf7be, 0xf7be,
    0xffdf, 0xffdf, 0xffdf, 0xffdf, 0xffff, 0xffff, 0xffff, 0xffff
};

// Memory initialization
int JPEG_openRAM(JPEGIMAGE *pJPEG, uint8_t *pData, int iDataSize, uint8_t *pImage) {
    memset(pJPEG, 0, sizeof(JPEGIMAGE));
    pJPEG->ucMemType = JPEG_MEM_RAM;
    pJPEG->pfnRead = readRAM;
    pJPEG->pfnSeek = seekMem;
    pJPEG->pImage = pImage;
    pJPEG->pfnOpen = NULL;
    pJPEG->pfnClose = NULL;
    pJPEG->JPEGFile.iSize = iDataSize;
    pJPEG->JPEGFile.pData = pData;
    pJPEG->iMaxMCUs = 1000;  // set to an unnaturally high value to start
    return JPEGInit(pJPEG);
}

int JPEG_getLastError(JPEGIMAGE *pJPEG) {
    return pJPEG->iError;
}

int JPEG_getWidth(JPEGIMAGE *pJPEG) {
    return pJPEG->iWidth;
}

int JPEG_getHeight(JPEGIMAGE *pJPEG) {
    return pJPEG->iHeight;
}

int JPEG_getOrientation(JPEGIMAGE *pJPEG) {
    return (int) pJPEG->ucOrientation;
}

int JPEG_getBpp(JPEGIMAGE *pJPEG) {
    return (int) pJPEG->ucBpp;
}

int JPEG_getSubSample(JPEGIMAGE *pJPEG) {
    return (int) pJPEG->ucSubSample;
}

int JPEG_hasThumb(JPEGIMAGE *pJPEG) {
    return (int) pJPEG->ucHasThumb;
}

int JPEG_getThumbWidth(JPEGIMAGE *pJPEG) {
    return pJPEG->iThumbWidth;
}
int JPEG_getThumbHeight(JPEGIMAGE *pJPEG) {
    return pJPEG->iThumbHeight;
}

void JPEG_setPixelType(JPEGIMAGE *pJPEG, int iType) {
    pJPEG->ucPixelType = (uint8_t) iType;
}

void JPEG_setMaxOutputSize(JPEGIMAGE *pJPEG, int iMaxMCUs) {
    if (iMaxMCUs < 1) {
        iMaxMCUs = 1; // don't allow invalid value
    }
    pJPEG->iMaxMCUs = iMaxMCUs;
}

int JPEG_decode(JPEGIMAGE *pJPEG, int x, int y, int iOptions) {
    pJPEG->iXOffset = x;
    pJPEG->iYOffset = y;
    pJPEG->iOptions = iOptions;
    return DecodeJPEG(pJPEG);
}

int JPEG_decodeDither(JPEGIMAGE *pJPEG, uint8_t *pDither, int iOptions) {
    pJPEG->iOptions = iOptions;
    pJPEG->pDitherBuffer = pDither;
    return DecodeJPEG(pJPEG);
}

// Helper functions for memory based images
static int32_t readRAM(JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen) {
    int32_t iBytesRead;

    iBytesRead = iLen;
    if ((pFile->iSize - pFile->iPos) < iLen) {
        iBytesRead = pFile->iSize - pFile->iPos;
    }
    if (iBytesRead <= 0) {
        return 0;
    }
    memcpy(pBuf, &pFile->pData[pFile->iPos], iBytesRead);
    pFile->iPos += iBytesRead;
    return iBytesRead;
}

static int32_t seekMem(JPEGFILE *pFile, int32_t iPosition) {
    if (iPosition < 0) {
        iPosition = 0;
    } else if (iPosition >= pFile->iSize) {
        iPosition = pFile->iSize - 1;
    }
    pFile->iPos = iPosition;
    return iPosition;
}

// The following functions are written in plain C and have no
// 3rd party dependencies, not even the C runtime library
//
// Initialize a JPEG file and callback access from a file on SD or memory
// returns 1 for success, 0 for failure
// Fills in the basic image info fields of the JPEGIMAGE structure
static int JPEGInit(JPEGIMAGE *pJPEG) {
    return JPEGParseInfo(pJPEG, 0); // gather info for image
}

// Unpack the Huffman tables
static int JPEGGetHuffTables(uint8_t *pBuf, int iLen, JPEGIMAGE *pJPEG) {
    int i, j, iOffset, iTableOffset;
    uint8_t ucTable, *pHuffVals;

    iOffset = 0;
    pHuffVals = (uint8_t *) pJPEG->usPixels; // temp holding area to save RAM
    while (iLen > 17) {
        // while there are tables to copy (we may have combined more than 1 table together)
        ucTable = pBuf[iOffset++];           // get table index
        if (ucTable & 0x10) {
            // convert AC offset of 0x10 into offset of 4
            ucTable ^= 0x14;
        }
        pJPEG->ucHuffTableUsed |= (1 << ucTable); // mark this table as being defined
        if (ucTable <= 7) {
            // tables are 0-3, AC+DC
            iTableOffset = ucTable * HUFF_TABLEN;
            j = 0; // total bits
            for (i = 0; i < 16; i++) {
                j += pBuf[iOffset];
                pHuffVals[iTableOffset + i] = pBuf[iOffset++];
            }
            iLen -= 17; // subtract length of bit lengths
            if (j == 0 || j > 256 || j > iLen) {
                // bogus bit lengths
                return -1;
            }
            iTableOffset += 16;
            for (i = 0; i < j; i++) {
                // copy huffman table
                pHuffVals[iTableOffset + i] = pBuf[iOffset++];
            }
            iLen -= j;
        }
    }
    return 0;
}

// Expand the Huffman tables for fast decoding
// returns 1 for success, 0 for failure
static int JPEGMakeHuffTables(JPEGIMAGE *pJPEG, int bThumbnail) {
    int code, repeat, count, codestart;
    int j;
    int iLen, iTable;
    uint16_t *pTable, *pShort, *pLong;
    uint8_t *pHuffVals, *pucTable, *pucShort, *pucLong;
    uint32_t ul, *pLongTable;
    int iBitNum; // current code bit length
    int cc;      // code
    uint8_t *p, *pBits, ucCode;
    int iMaxLength, iMaxMask;
    int iTablesUsed;

    iTablesUsed = 0;
    pHuffVals = (uint8_t *) pJPEG->usPixels;
    for (j = 0; j < 4; j++) {
        if (pJPEG->ucHuffTableUsed & (1 << j)) {
            iTablesUsed++;
        }
    }
    // first do DC components (up to 4 tables of 12-bit codes)
    // we can save time and memory for the DC codes by knowing that there exist short codes (<= 6 bits)
    // and long codes (>6 bits, but the first 5 bits are 1's).  This allows us to create 2 tables: a 6-bit
    // and 7 or 8-bit to handle any DC codes
    iMaxLength = 12;   // assume DC codes can be 12-bits
    iMaxMask = 0x7f;   // lower 7 bits after truncate 5 leading 1's
    for (iTable = 0; iTable < 4; iTable++) {
        if (pJPEG->ucHuffTableUsed & (1 << iTable)) {
            //         pJPEG->huffdcFast[iTable] = (int *)PILIOAlloc(0x180); // short table = 128 bytes, long table =
            // 256 bytes
            pucShort = &pJPEG->ucHuffDC[iTable * DC_TABLE_SIZE];
            //         pJPEG->huffdc[iTable] = pJPEG->huffdcFast[iTable] + 0x20; // 0x20 longs = 128 bytes
            pucLong = &pJPEG->ucHuffDC[iTable * DC_TABLE_SIZE + 128];
            pBits = &pHuffVals[iTable * HUFF_TABLEN];
            p = pBits;
            p += 16;             // point to bit data
            cc = 0;              // start with a code of 0
            for (iBitNum = 1; iBitNum <= 16; iBitNum++) {
                iLen = *pBits++; // get number of codes for this bit length
                if (iBitNum > iMaxLength && iLen > 0) {
                    // we can't handle codes longer a certain length
                    return 0;
                }
                while (iLen) {
                    //               if (iBitNum > 6) // do long table
                    if ((cc >> (iBitNum - 5)) == 0x1f) {
                        // first 5 bits are 1 - use long table
                        count = iMaxLength - iBitNum;
                        codestart = cc << count;
                        pucTable = &pucLong[codestart & iMaxMask];  // use lower 7/8 bits of code
                    } else {
                        // do short table
                        count = 6 - iBitNum;
                        if (count < 0) {
                            return 0; // DEBUG - something went wrong
                        }
                        codestart = cc << count;
                        pucTable = &pucShort[codestart];
                    }
                    ucCode = *p++;  // get actual huffman code
                    // does precalculating the DC value save time on ARM?
                    if (ucCode != 0 && (ucCode + iBitNum) <= 6 && pJPEG->ucMode != 0xc2) {
                        // we can fit the magnitude value in the code lookup (not for progressive)
                        int k, iLoop;
                        unsigned char ucCoeff;
                        unsigned char *d = &pucTable[512];
                        unsigned char ucMag = ucCode;
                        ucCode |= ((iBitNum + ucCode) << 4); // add magnitude bits to length
                        repeat = 1 << ucMag;
                        iLoop = 1 << (count - ucMag);
                        for (j = 0; j < repeat; j++) {
                            // calculate the magnitude coeff already
                            if (j & 1 << (ucMag - 1)) {
                                // positive number
                                ucCoeff = (unsigned char) j;
                            } else {
                                // negative number
                                ucCoeff = (unsigned char) (j - ((1 << ucMag) - 1));
                            }
                            for (k = 0; k < iLoop; k++) {
                                *d++ = ucCoeff;
                            } // for k
                        }     // for j
                    } else {
                        ucCode |= (iBitNum << 4);
                    }
                    if (count) {
                        // do it as dwords to save time
                        repeat = (1 << count);
                        memset(pucTable, ucCode, repeat);
                    } else {
                        pucTable[0] = ucCode;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            }
        } // if table defined
    }
    // now do AC components (up to 4 tables of 16-bit codes)
    // We split the codes into a short table (9 bits or less) and a long table (first 5 bits are 1)
    for (iTable = 0; iTable < 4; iTable++) {
        if (pJPEG->ucHuffTableUsed & (1 << (iTable + 4))) {
            // if this table is defined
            pBits = &pHuffVals[(iTable + 4) * HUFF_TABLEN];
            p = pBits;
            p += 16;             // point to bit data
            pShort = &pJPEG->usHuffAC[iTable * HUFF11SIZE];
            pLong = &pJPEG->usHuffAC[iTable * HUFF11SIZE + 1024];
            cc = 0;              // start with a code of 0
            // construct the decode table
            for (iBitNum = 1; iBitNum <= 16; iBitNum++) {
                iLen = *pBits++; // get number of codes for this bit length
                while (iLen) {
                    if ((cc >> (iBitNum - 6)) == 0x3f) {
                        // first 6 bits are 1 - use long table
                        count = 16 - iBitNum;
                        codestart = cc << count;
                        pTable = &pLong[codestart & 0x3ff];    // use lower 10 bits of code
                    } else {
                        count = 10 - iBitNum;
                        if (count < 0) {
                            // an 11/12-bit? code - that doesn't fit our optimized
                            // scheme, see if we can do a bigger table version
                            if (count == -1 && iTablesUsed <= 4) {
                                return 0;
                            } else {
                                return 0; // DEBUG - fatal error, more than 2 big tables we currently don't support
                            }
                        }
                        codestart = cc << count;
                        pTable = &pShort[codestart];    // 10 bits or shorter
                    }
                    code = *p++;                        // get actual huffman code
                    if (bThumbnail && code != 0) {
                        // add "extra" bits to code length since we skip these codes
                        // get rid of extra bits in code and add increment (1) for AC index
                        code = ((iBitNum + (code & 0xf)) << 8) | ((code >> 4) + 1);
                    } else {
                        code |= (iBitNum << 8);
                    }
                    if (count) {
                        // do it as dwords to save time
                        repeat = 1 << (count - 1);     // store as dwords (/2)
                        ul = code | (code << 16);
                        pLongTable = (uint32_t *) pTable;
                        for (j = 0; j < repeat; j++) {
                            *pLongTable++ = ul;
                        }
                    } else {
                        pTable[0] = (unsigned short) code;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            } // for each bit length
        }     // if table defined
    }
    return 1;
}

// TIFFSHORT
// read a 16-bit unsigned integer from the given pointer
// and interpret the data as big endian (Motorola) or little endian (Intel)
static uint16_t TIFFSHORT(unsigned char *p, int bMotorola) {
    unsigned short s;

    if (bMotorola) {
        s = *p * 0x100 + *(p + 1); // big endian (AKA Motorola byte order)
    } else {
        s = *p + *(p + 1) * 0x100; // little endian (AKA Intel byte order)
    }
    return s;
}

// TIFFLONG
// read a 32-bit unsigned integer from the given pointer
// and interpret the data as big endian (Motorola) or little endian (Intel)
static uint32_t TIFFLONG(unsigned char *p, int bMotorola) {
    uint32_t l;

    if (bMotorola) {
        l = *p * 0x1000000 + *(p + 1) * 0x10000 + *(p + 2) * 0x100 + *(p + 3); // big endian
    } else {
        l = *p + *(p + 1) * 0x100 + *(p + 2) * 0x10000 + *(p + 3) * 0x1000000; // little endian
    }
    return l;
}

// TIFFVALUE
// read an integer value encoded in a TIFF TAG (12-byte structure)
// and interpret the data as big endian (Motorola) or little endian (Intel)
static int TIFFVALUE(unsigned char *p, int bMotorola) {
    int i, iType;

    iType = TIFFSHORT(p + 2, bMotorola);
    /* If pointer to a list of items, must be a long */
    if (TIFFSHORT(p + 4, bMotorola) > 1) {
        iType = 4;
    }
    switch (iType) {
        case 3:  /* Short */
            i = TIFFSHORT(p + 8, bMotorola);
            break;
        case 4:  /* Long */
        case 7:  // undefined (treat it as a long since it's usually a multibyte buffer)
            i = TIFFLONG(p + 8, bMotorola);
            break;
        case 6:  // signed byte
            i = (signed char) p[8];
            break;
        case 2:  /* ASCII */
        case 5:  /* Unsigned Rational */
        case 10: /* Signed Rational */
            i = TIFFLONG(p + 8, bMotorola);
            break;
        default: /* to suppress compiler warning */
            i = 0;
            break;
    }
    return i;

}

static void GetTIFFInfo(JPEGIMAGE *pPage, int bMotorola, int iOffset) {
    int iTag, iTagCount, i;
    uint8_t *cBuf = pPage->ucFileBuf;

    iTagCount = TIFFSHORT(&cBuf[iOffset], bMotorola); /* Number of tags in this dir */
    if (iTagCount < 1 || iTagCount > 256) {
        // invalid tag count
        return;                                       /* Bad header info */
    }
    /*--- Search the TIFF tags ---*/
    for (i = 0; i < iTagCount; i++) {
        unsigned char *p = &cBuf[iOffset + (i * 12) + 2];
        iTag = TIFFSHORT(p, bMotorola);  /* current tag value */
        if (iTag == 274) {
            // orientation tag
            pPage->ucOrientation = TIFFVALUE(p, bMotorola);
        } else if (iTag == 256) {
            // width of thumbnail
            pPage->iThumbWidth = TIFFVALUE(p, bMotorola);
        } else if (iTag == 257) {
            // height of thumbnail
            pPage->iThumbHeight = TIFFVALUE(p, bMotorola);
        } else if (iTag == 513) {
            // offset to JPEG data
            pPage->iThumbData = TIFFVALUE(p, bMotorola);
        }
    }
}

static int JPEGGetSOS(JPEGIMAGE *pJPEG, int *iOff) {
    int16_t sLen;
    int iOffset = *iOff;
    int i, j;
    uint8_t uc, c, cc;
    uint8_t *buf = pJPEG->ucFileBuf;

    sLen = MOTOSHORT(&buf[iOffset]);
    iOffset += 2;

    // Assume no components in this scan
    for (i = 0; i < 4; i++) {
        pJPEG->JPCI[i].component_needed = 0;
    }

    uc = buf[iOffset++];    // get number of components
    pJPEG->ucComponentsInScan = uc;
    sLen -= 3;
    if (uc < 1 || uc > MAX_COMPS_IN_SCAN || sLen != (uc * 2 + 3)) {
        // check length of data packet
        return 1; // error
    }
    for (i = 0; i < uc; i++) {
        cc = buf[iOffset++];
        c = buf[iOffset++];
        sLen -= 2;
        for (j = 0; j < 4; j++) {
            // search for component id
            if (pJPEG->JPCI[j].component_id == cc) {
                break;
            }
        }
        if (j == 4) {
            // error, not found
            return 1;
        }
        if ((c & 0xf) > 3 || (c & 0xf0) > 0x30) {
            return 1; // bogus table numbers
        }
        pJPEG->JPCI[j].dc_tbl_no = c >> 4;
        pJPEG->JPCI[j].ac_tbl_no = c & 0xf;
        pJPEG->JPCI[j].component_needed = 1; // mark this component as being included in the scan
    }
    pJPEG->iScanStart = buf[iOffset++];      // Get the scan start (or lossless predictor) for this scan
    pJPEG->iScanEnd = buf[iOffset++];        // Get the scan end for this scan
    c = buf[iOffset++];                      // successive approximation bits
    pJPEG->cApproxBitsLow = c & 0xf;         // also point transform in lossless mode
    pJPEG->cApproxBitsHigh = c >> 4;

    *iOff = iOffset;
    return 0;

}

// Remove markers from the data stream to allow faster decode
// Stuffed zeros and restart interval markers aren't needed to properly decode
// the data, but they make reading VLC data slower, so I pull them out first
static int JPEGFilter(uint8_t *pBuf, uint8_t *d, int iLen, uint8_t *bFF) {
    // since we have the entire jpeg buffer in memory already, we can just change it in place
    unsigned char c, *s, *pEnd, *pStart;

    pStart = d;
    s = pBuf;
    pEnd = &s[iLen - 1];   // stop just shy of the end to not miss a final marker/stuffed 0
    if (*bFF) {
        // last byte was a FF, check the next one
        if (s[0] == 0) {
            // stuffed 0, keep the FF
            *d++ = 0xff;
        }
        s++;
        *bFF = 0;
    }
    while (s < pEnd) {
        c = *d++ = *s++;
        if (c == 0xff) {
            // marker or stuffed zeros?
            if (s[0] != 0) {
                // it's a marker, skip both
                d--;
            }
            s++; // for stuffed 0's, store the FF, skip the 00
        }
    }
    if (s == pEnd) {
        // need to test the last byte
        c = s[0];
        if (c == 0xff) {
            // last byte is FF, take care of it next time through
            *bFF = 1;          // take care of it next time through
        } else {
            *d++ = c;          // nope, just store it
        }
    }
    return (int) (d - pStart); // filtered output length
}

// Read and filter more VLC data for decoding
static void JPEGGetMoreData(JPEGIMAGE *pPage) {
    int iDelta = pPage->iVLCSize - pPage->iVLCOff;
    // move any existing data down
    if (iDelta >= (JPEG_FILE_BUF_SIZE - 64) || iDelta < 0) {
        return; // buffer is already full; no need to read more data
    }
    if (pPage->iVLCOff != 0) {
        memcpy(pPage->ucFileBuf, &pPage->ucFileBuf[pPage->iVLCOff], pPage->iVLCSize - pPage->iVLCOff);
        pPage->iVLCSize -= pPage->iVLCOff;
        pPage->iVLCOff = 0;
        pPage->bb.pBuf = pPage->ucFileBuf;   // reset VLC source pointer too
    }
    if (pPage->JPEGFile.iPos < pPage->JPEGFile.iSize && pPage->iVLCSize < JPEG_FILE_BUF_SIZE - 64) {
        int i;
        // Try to read enough to fill the buffer
        // max length we can read
        i = (*pPage->pfnRead) (&pPage->JPEGFile, &pPage->ucFileBuf[pPage->iVLCSize], JPEG_FILE_BUF_SIZE - pPage->iVLCSize);
        // Filter out the markers
        pPage->iVLCSize += JPEGFilter(&pPage->ucFileBuf[pPage->iVLCSize], &pPage->ucFileBuf[pPage->iVLCSize], i, &pPage->ucFF);
    }
}

// Parse the JPEG header, gather necessary info to decode the image
// Returns 1 for success, 0 for failure
static int JPEGParseInfo(JPEGIMAGE *pPage, int bExtractThumb) {
    int iBytesRead;
    int i, iOffset, iTableOffset;
    uint8_t ucTable, *s = pPage->ucFileBuf;
    uint16_t usMarker, usLen = 0;
    int iFilePos = 0;

    if (bExtractThumb) {
        // seek to the start of the thumbnail image
        iFilePos = pPage->iThumbData;
        (*pPage->pfnSeek) (&pPage->JPEGFile, iFilePos);
    }
    iBytesRead = (*pPage->pfnRead) (&pPage->JPEGFile, s, JPEG_FILE_BUF_SIZE);
    if (iBytesRead < 256) {
        // a JPEG file this tiny? probably bad
        pPage->iError = JPEG_INVALID_FILE;
        return 0;
    }
    iFilePos += iBytesRead;
    if (MOTOSHORT(pPage->ucFileBuf) != 0xffd8) {
        pPage->iError = JPEG_INVALID_FILE;
        return 0; // not a JPEG file
    }
    iOffset = 2;  /* Start at offset of first marker */
    usMarker = 0; /* Search for SOFx (start of frame) marker */
    while (usMarker != 0xffda && iOffset < pPage->JPEGFile.iSize) {
        if (iOffset >= JPEG_FILE_BUF_SIZE / 2) {
            // too close to the end, read more data
            // Do we need to seek first?
            if (iOffset >= JPEG_FILE_BUF_SIZE) {
                iFilePos += (iOffset - iBytesRead);
                iOffset = 0;
                (*pPage->pfnSeek) (&pPage->JPEGFile, iFilePos);
                iBytesRead = 0; // throw away any old data
            }
            // move existing bytes down
            if (iOffset) {
                memcpy(pPage->ucFileBuf, &pPage->ucFileBuf[iOffset], iBytesRead - iOffset);
                iBytesRead -= iOffset;
                iOffset = 0;
            }
            i = (*pPage->pfnRead) (&pPage->JPEGFile, &pPage->ucFileBuf[iBytesRead], JPEG_FILE_BUF_SIZE - iBytesRead);
            iFilePos += i;
            iBytesRead += i;
        }
        usMarker = MOTOSHORT(&s[iOffset]);
        iOffset += 2;
        usLen = MOTOSHORT(&s[iOffset]);    // marker length

        if (usMarker < 0xffc0 || usMarker == 0xffff) {
            // invalid marker, could be generated by "Arles Image Web Page Creator" or Accusoft
            iOffset++;
            continue; // skip 1 byte and try to resync
        }
        switch (usMarker) {
            case 0xffc1:
            case 0xffc2:
            case 0xffc3:
                pPage->iError = JPEG_UNSUPPORTED_FEATURE;
                return 0; // currently unsupported modes

            case 0xffe1:  // App1 (EXIF?)
                if (s[iOffset + 2] == 'E' && s[iOffset + 3] == 'x'
                    && (s[iOffset + 8] == 'M' || s[iOffset + 8] == 'I')) {
                    // the EXIF data we want
                    int bMotorola, IFD, iTagCount;
                    pPage->iEXIF = iFilePos - iBytesRead + iOffset + 8; // start of TIFF file
                    // Get the orientation value (if present)
                    bMotorola = (s[iOffset + 8] == 'M');
                    IFD = TIFFLONG(&s[iOffset + 12], bMotorola);
                    iTagCount = TIFFSHORT(&s[iOffset + 16], bMotorola);
                    GetTIFFInfo(pPage, bMotorola, IFD + iOffset + 8);
                    // The second IFD defines the thumbnail (if present)
                    if (iTagCount >= 1 && iTagCount < 32) {
                        // valid number of tags for EXIF data 'page'
                        // point to next IFD
                        IFD += (12 * iTagCount) + 2;
                        IFD = TIFFLONG(&s[IFD + iOffset + 8], bMotorola);
                        if (IFD != 0) {
                            // Thumbnail present?
                            pPage->ucHasThumb = 1;
                            GetTIFFInfo(pPage, bMotorola, IFD + iOffset + 8); // info for second 'page' of TIFF
                            pPage->iThumbData += iOffset + 8;                 // absolute offset in the file
                        }
                    }
                }
                break;
            case 0xffc0:                         // SOFx - start of frame
                pPage->ucMode = (uint8_t) usMarker;
                pPage->ucBpp = s[iOffset + 2];   // bits per sample
                pPage->iHeight = MOTOSHORT(&s[iOffset + 3]);
                pPage->iWidth = MOTOSHORT(&s[iOffset + 5]);
                pPage->ucNumComponents = s[iOffset + 7];
                pPage->ucBpp = pPage->ucBpp * pPage->ucNumComponents;   // Bpp = number of components * bits per sample
                if (pPage->ucNumComponents == 1) {
                    pPage->ucSubSample = 0;                             // use this to differentiate from color 1:1
                } else {
                    usLen -= 8;
                    iOffset += 8;
                    for (i = 0; i < pPage->ucNumComponents; i++) {
                        uint8_t ucSamp;
                        pPage->JPCI[i].component_id = s[iOffset++];
                        pPage->JPCI[i].component_index = (unsigned char) i;
                        ucSamp = s[iOffset++]; // get the h+v sampling factor
                        if (i == 0) {
                            // Y component?
                            pPage->ucSubSample = ucSamp;
                        }
                        pPage->JPCI[i].quant_tbl_no = s[iOffset++]; // quantization table number
                        usLen -= 3;
                    }
                }
                break;
            case 0xffdd: // Restart Interval
                if (usLen == 4) {
                    pPage->iResInterval = MOTOSHORT(&s[iOffset + 2]);
                }
                break;
            case 0xffc4: /* M_DHT */ // get Huffman tables
                iOffset += 2;        // skip length
                usLen -= 2;          // subtract length length
                if (JPEGGetHuffTables(&s[iOffset], usLen, pPage) != 0) {
                    // bad tables?
                    pPage->iError = JPEG_DECODE_ERROR;
                    return 0;               // error
                }
                break;
            case 0xffdb:                    /* M_DQT */
                /* Get the quantization tables */
                /* first byte has PPPPNNNN where P = precision and N = table number 0-3 */
                iOffset += 2;               // skip length
                usLen -= 2;                 // subtract length length
                while (usLen > 0) {
                    ucTable = s[iOffset++]; // table number
                    if ((ucTable & 0xf) > 3) {
                        // invalid table number
                        pPage->iError = JPEG_DECODE_ERROR;
                        return 0;
                    }
                    iTableOffset = (ucTable & 0xf) * DCTSIZE;
                    if (ucTable & 0xf0) {
                        // if word precision
                        for (i = 0; i < DCTSIZE; i++) {
                            pPage->sQuantTable[i + iTableOffset] = MOTOSHORT(&s[iOffset]);
                            iOffset += 2;
                        }
                        usLen -= (DCTSIZE * 2 + 1);
                    } else {
                        // byte precision
                        for (i = 0; i < DCTSIZE; i++) {
                            pPage->sQuantTable[i + iTableOffset] = (unsigned short) s[iOffset++];
                        }
                        usLen -= (DCTSIZE + 1);
                    }
                }
                break;
        } // switch on JPEG marker
        iOffset += usLen;
    }     // while
    if (usMarker == 0xffda) {
        // start of image
        if (pPage->ucBpp != 8) {
            // need to match up table IDs
            iOffset -= usLen;
            JPEGGetSOS(pPage, &iOffset); // get Start-Of-Scan info for decoding
        }
        if (!JPEGMakeHuffTables(pPage, 0)) {
            //int bThumbnail) DEBUG
            pPage->iError = JPEG_UNSUPPORTED_FEATURE;
            return 0;
        }
        // Now the offset points to the start of compressed data
        i = JPEGFilter(&pPage->ucFileBuf[iOffset], pPage->ucFileBuf, iBytesRead - iOffset, &pPage->ucFF);
        pPage->iVLCOff = 0;
        pPage->iVLCSize = i;
        JPEGGetMoreData(pPage); // read more VLC data
        return 1;
    }
    pPage->iError = JPEG_DECODE_ERROR;
    return 0;
}

// Fix and reorder the quantization table for faster decoding.*
static void JPEGFixQuantD(JPEGIMAGE *pJPEG) {
    int iTable, iTableOffset;
    signed short sTemp[DCTSIZE];
    int i;
    uint16_t *p;

    for (iTable = 0; iTable < pJPEG->ucNumComponents; iTable++) {
        iTableOffset = iTable * DCTSIZE;
        p = (uint16_t *) &pJPEG->sQuantTable[iTableOffset];
        for (i = 0; i < DCTSIZE; i++) {
            sTemp[i] = p[cZigZag[i]];
        }
        memcpy(&pJPEG->sQuantTable[iTableOffset], sTemp, DCTSIZE * sizeof(short)); // copy back to original spot

        // Prescale for DCT multiplication
        p = (uint16_t *) &pJPEG->sQuantTable[iTableOffset];
        for (i = 0; i < DCTSIZE; i++) {
            p[i] = (uint16_t) ((p[i] * iScaleBits[i]) >> 12);
        }
    }
}

// Decode the 64 coefficients of the current DCT block
static int JPEGDecodeMCU(JPEGIMAGE *pJPEG, int iMCU, int *iDCPredictor) {
    uint32_t ulCode, ulTemp;
    uint8_t *pZig;
    signed char cCoeff;
    unsigned short *pFast;
    unsigned char ucHuff, *pucFast;
    uint32_t usHuff;           // this prevents an unnecessary & 65535 for shorts
    uint32_t ulBitOff, ulBits; // local copies to allow compiler to use register vars
    uint8_t *pBuf, *pEnd, *pEnd2;
    signed short *pMCU = &pJPEG->sMCUs[iMCU];
    uint8_t ucMaxACCol, ucMaxACRow;

#define MIN_DCT_THRESHOLD    8

    ulBitOff = pJPEG->bb.ulBitOff;
    ulBits = pJPEG->bb.ulBits;
    pBuf = pJPEG->bb.pBuf;

    pZig = (unsigned char *) &cZigZag2[1];
    pEnd = (unsigned char *) &cZigZag2[64];

    if (ulBitOff > (REGISTER_WIDTH - 17)) {
        // need to get more data
        pBuf += (ulBitOff >> 3);
        ulBitOff &= 7;
        ulBits = MOTOLONG(pBuf);
    }
    if (pJPEG->iOptions & (JPEG_SCALE_QUARTER | JPEG_SCALE_EIGHTH)) {
        // reduced size DCT
        pMCU[1] = pMCU[8] = pMCU[9] = 0;
        pEnd2 = (uint8_t *) &cZigZag2[5];    // we only need to store the 4 elements we care about
    } else {
        memset(pMCU, 0, 64 * sizeof(short)); // pre-fill with zero since we may skip coefficients
        pEnd2 = (uint8_t *) &cZigZag2[64];
    }
    ucMaxACCol = ucMaxACRow = 0;
    pZig = (unsigned char *) &cZigZag2[1];
    pEnd = (unsigned char *) &cZigZag2[64];

    // get the DC component
    pucFast = &pJPEG->ucHuffDC[pJPEG->ucDCTable * DC_TABLE_SIZE];
    ulCode = (ulBits >> (REGISTER_WIDTH - 12 - ulBitOff)) & 0xfff;  // get as lower 12 bits
    if (ulCode >= 0xf80) {
        // it's a long code
        ulCode = (ulCode & 0xff);                                   // point to long table and trim to 7-bits + 0x80
                                                                    // offset into long table
    } else {
        ulCode >>= 6;                                               // it's a short code, use first 6 bits only
    }
    ucHuff = pucFast[ulCode];
    cCoeff = (signed char) pucFast[ulCode + 512];                   // get pre-calculated extra bits for "small" values
    if (ucHuff == 0) {
        // invalid code
        return -1;
    }
    ulBitOff += (ucHuff >> 4); // add the Huffman length
    ucHuff &= 0xf;             // get the actual code (SSSS)
    if (ucHuff) {
        // if there is a change to the DC value
        // get the 'extra' bits
        if (cCoeff) {
            (*iDCPredictor) += cCoeff;
        } else {
            if (ulBitOff > (REGISTER_WIDTH - 17)) {
                // need to get more data
                pBuf += (ulBitOff >> 3);
                ulBitOff &= 7;
                ulBits = MOTOLONG(pBuf);
            }
            ulCode = ulBits << ulBitOff;
            ulTemp = ~(uint32_t) (((int32_t) ulCode) >> 31);    // slide sign bit across other 31 bits
            ulCode >>= (REGISTER_WIDTH - ucHuff);
            ulCode -= ulTemp >> (REGISTER_WIDTH - ucHuff);
            ulBitOff += ucHuff;                                 // add bit length
            (*iDCPredictor) += (int) ulCode;
        }
    }
    pMCU[0] = (short) *iDCPredictor; // store in MCU[0]
    // Now get the other 63 AC coefficients
    pFast = &pJPEG->usHuffAC[pJPEG->ucACTable * HUFF11SIZE];
    if (pJPEG->b11Bit) {
        // 11-bit "slow" tables used
        while (pZig < pEnd) {
            if (ulBitOff > (REGISTER_WIDTH - 17)) {
                // need to get more data
                pBuf += (ulBitOff >> 3);
                ulBitOff &= 7;
                ulBits = MOTOLONG(pBuf);
            }
            ulCode = (ulBits >> (REGISTER_WIDTH - 16 - ulBitOff)) & 0xffff; // get as lower 16 bits
            if (ulCode >= 0xf000) {
                // first 4 bits = 1, use long table
                ulCode = (ulCode & 0x1fff);
            } else {
                ulCode >>= 4; // use lower 12 bits (short table)
            }
            usHuff = pFast[ulCode];
            if (usHuff == 0) {
                // invalid code
                return -1;
            }
            ulBitOff += (usHuff >> 8); // add length
            usHuff &= 0xff;            // get code (RRRR/SSSS)
            if (usHuff == 0) {
                // no more AC components
                goto mcu_done;
            }
            if (ulBitOff > (REGISTER_WIDTH - 17)) {
                // need to get more data
                pBuf += (ulBitOff >> 3);
                ulBitOff &= 7;
                ulBits = MOTOLONG(pBuf);
            }
            pZig += (usHuff >> 4);   // get the skip amount (RRRR)
            usHuff &= 0xf;           // get (SSSS) - extra length
            if (pZig < pEnd && usHuff) {
                // && piHisto)
                ulCode = ulBits << ulBitOff;
                // slide sign bit across other 63 bits
                ulTemp = ~(uint32_t) (((int32_t) ulCode) >> (REGISTER_WIDTH - 1));
                ulCode >>= (REGISTER_WIDTH - usHuff);
                ulCode -= ulTemp >> (REGISTER_WIDTH - usHuff);
                ucMaxACCol |= 1 << (*pZig & 7);                             // keep track of occupied columns
                if (*pZig >= 0x20) {
                    // if more than 4 rows used in a col, mark it
                    ucMaxACRow |= 1 << (*pZig & 7);                         // keep track of the max AC term
                                                                            // row
                }
                pMCU[*pZig] = (signed short) ulCode;                        // store AC coefficient (already
                                                                            // reordered)
            }
            ulBitOff += usHuff;                                             // add (SSSS) extra length
            pZig++;
        } // while
    } else {
        // 10-bit "fast" tables used
        while (pZig < pEnd) {
            if (ulBitOff > (REGISTER_WIDTH - 17)) {
                // need to get more data
                pBuf += (ulBitOff >> 3);
                ulBitOff &= 7;
                ulBits = MOTOLONG(pBuf);
            }
            ulCode = (ulBits >> (REGISTER_WIDTH - 16 - ulBitOff)) & 0xffff; // get as lower 16 bits
            if (ulCode >= 0xfc00) {
                // first 6 bits = 1, use long table
                ulCode = (ulCode & 0x7ff);                                  // (ulCode & 0x3ff) + 0x400;
            } else {
                ulCode >>= 6;                                               // use lower 10 bits (short table)
            }
            usHuff = pFast[ulCode];
            if (usHuff == 0) {
                // invalid code
                return -1;
            }
            ulBitOff += (usHuff >> 8); // add length
            usHuff &= 0xff;            // get code (RRRR/SSSS)
            if (usHuff == 0) {
                // no more AC components
                goto mcu_done;
            }
            if (ulBitOff > (REGISTER_WIDTH - 17)) {
                // need to get more data
                pBuf += (ulBitOff >> 3);
                ulBitOff &= 7;
                ulBits = MOTOLONG(pBuf);
            }
            pZig += (usHuff >> 4);                                                      // get the skip amount (RRRR)
            usHuff &= 0xf;                                                              // get (SSSS) - extra length
            if (pZig < pEnd2 && usHuff) {
                ulCode = ulBits << ulBitOff;
                ulTemp = ~(uint32_t) (((int32_t) ulCode) >> (REGISTER_WIDTH - 1));      // slide sign bit across other
                                                                                        // 63 bits
                ulCode >>= (REGISTER_WIDTH - usHuff);
                ulCode -= ulTemp >> (REGISTER_WIDTH - usHuff);
                ucMaxACCol |= 1 << (*pZig & 7);                                         // keep track of occupied
                                                                                        // columns
                if (*pZig >= 0x20) {
                    // if more than 4 rows used in a col, mark it
                    ucMaxACRow |= 1 << (*pZig & 7);                                     // keep track of the max AC term
                                                                                        // row
                }
                pMCU[*pZig] = (signed short) ulCode;                                    // store AC coefficient (already
                                                                                        // reordered)
            }
            ulBitOff += usHuff;                                                         // add (SSSS) extra length
            pZig++;
        } // while
    } // 10-bit tables
mcu_done:
    pJPEG->bb.pBuf = pBuf;
    pJPEG->iVLCOff = (int) (pBuf - pJPEG->ucFileBuf);
    pJPEG->bb.ulBitOff = ulBitOff;
    pJPEG->bb.ulBits = ulBits;
    pJPEG->ucMaxACCol = ucMaxACCol;
    pJPEG->ucMaxACRow = ucMaxACRow;  // DEBUG
    return 0;
}

// Inverse DCT
static void JPEGIDCT(JPEGIMAGE *pJPEG, int iMCUOffset, int iQuantTable, int iACFlags) {
    int iRow;
    unsigned char ucColMask;
    int iCol;
    signed int tmp6, tmp7, tmp10, tmp11, tmp12, tmp13;
    signed int z5, z10, z11, z12, z13;
    signed int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
    signed short *pQuant;
    unsigned char *pOutput;
    unsigned char ucMaxACRow, ucMaxACCol;
    int16_t *pMCUSrc = &pJPEG->sMCUs[iMCUOffset];

    ucMaxACRow = (unsigned char) (iACFlags >> 8);
    ucMaxACCol = iACFlags & 0xff;

    // my shortcut method appears to violate patent 20020080052
    // but the patent is invalidated by prior art:
    // http://netilium.org/~mad/dtj/DTJ/DTJK04/
    pQuant = &pJPEG->sQuantTable[iQuantTable * DCTSIZE];
    if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
        // special case
        /* Column 0 */
        tmp4 = pMCUSrc[0] * pQuant[0];
        tmp5 = pMCUSrc[8] * pQuant[8];
        tmp0 = tmp4 + tmp5;
        tmp2 = tmp4 - tmp5;
        /* Column 1 */
        tmp4 = pMCUSrc[1] * pQuant[1];
        tmp5 = pMCUSrc[9] * pQuant[9];
        tmp1 = tmp4 + tmp5;
        tmp3 = tmp4 - tmp5;
        /* Pass 2: process 2 rows, store into output array. */
        /* Row 0 */
        pOutput = (unsigned char *) pMCUSrc;    // store output pixels back into MCU
        pOutput[0] = ucRangeTable[(((tmp0 + tmp1) >> 5) & 0x3ff)];
        pOutput[1] = ucRangeTable[(((tmp0 - tmp1) >> 5) & 0x3ff)];
        /* Row 1 */
        pOutput[2] = ucRangeTable[(((tmp2 + tmp3) >> 5) & 0x3ff)];
        pOutput[3] = ucRangeTable[(((tmp2 - tmp3) >> 5) & 0x3ff)];
        return;
    }
    // do columns first
    ucColMask = ucMaxACCol | 1; // column 0 must always be calculated
    for (iCol = 0; iCol < 8 && ucColMask; iCol++) {
        if (ucColMask & (1 << iCol)) {
            // column has data in it
            ucColMask &= ~(1 << iCol); // unmark this col after use
            if (!(ucMaxACRow & (1 << iCol))) {
                // simpler calculations if only half populated
                // even part
                tmp10 = pMCUSrc[iCol] * pQuant[iCol];
                tmp1 = pMCUSrc[iCol + 16] * pQuant[iCol + 16];  // get 2nd row
                tmp12 = ((tmp1 * 106) >> 8);                    // used to be 362 - 1 (256)
                tmp0 = tmp10 + tmp1;
                tmp3 = tmp10 - tmp1;
                tmp1 = tmp10 + tmp12;
                tmp2 = tmp10 - tmp12;
                // odd part
                tmp4 = pMCUSrc[iCol + 8] * pQuant[iCol + 8];  // get 1st row
                tmp5 = pMCUSrc[iCol + 24];
                if (tmp5) {
                    // this value is usually 0
                    tmp5 *= pQuant[iCol + 24];            // get 3rd row
                    tmp7 = tmp4 + tmp5;
                    tmp11 = (((tmp4 - tmp5) * 362) >> 8); // 362>>8 = 1.414213562
                    z5 = (((tmp4 - tmp5) * 473) >> 8);    // 473>>8 = 1.8477
                    tmp12 = ((-tmp5 * -669) >> 8) + z5;   // -669>>8 = -2.6131259
                    tmp6 = tmp12 - tmp7;
                    tmp5 = tmp11 - tmp6;
                    tmp10 = ((tmp4 * 277) >> 8) - z5;     // 277>>8 = 1.08239
                    tmp4 = tmp10 + tmp5;
                } else {
                    // simpler case when we only have 1 odd row to calculate
                    tmp7 = tmp4;
                    tmp5 = (145 * tmp4) >> 8;
                    tmp6 = (217 * tmp4) >> 8;
                    tmp4 = (-51 * tmp4) >> 8;
                }
                pMCUSrc[iCol] = (short) (tmp0 + tmp7);      // row0
                pMCUSrc[iCol + 8] = (short) (tmp1 + tmp6);  // row 1
                pMCUSrc[iCol + 16] = (short) (tmp2 + tmp5); // row 2
                pMCUSrc[iCol + 24] = (short) (tmp3 - tmp4); // row 3
                pMCUSrc[iCol + 32] = (short) (tmp3 + tmp4); // row 4
                pMCUSrc[iCol + 40] = (short) (tmp2 - tmp5); // row 5
                pMCUSrc[iCol + 48] = (short) (tmp1 - tmp6); // row 6
                pMCUSrc[iCol + 56] = (short) (tmp0 - tmp7); // row 7
            } else {
                // need to do full column calculation
                // even part
                tmp0 = pMCUSrc[iCol] * pQuant[iCol];
                tmp2 = pMCUSrc[iCol + 32]; // get 4th row
                if (tmp2) {
                    // 4th row is most likely 0
                    tmp2 = tmp2 * pQuant[iCol + 32];
                    tmp10 = tmp0 + tmp2;
                    tmp11 = tmp0 - tmp2;
                } else {
                    tmp10 = tmp11 = tmp0;
                }
                tmp1 = pMCUSrc[iCol + 16] * pQuant[iCol + 16]; // get 2nd row
                tmp3 = pMCUSrc[iCol + 48];                     // get 6th row
                if (tmp3) {
                    // 6th row is most likely 0
                    tmp3 = tmp3 * pQuant[iCol + 48];
                    tmp13 = tmp1 + tmp3;
                    tmp12 = (((tmp1 - tmp3) * 362) >> 8) - tmp13;  // 362>>8 = 1.414213562
                } else {
                    tmp13 = tmp1;
                    tmp12 = ((tmp1 * 362) >> 8) - tmp1;
                }
                tmp0 = tmp10 + tmp13;
                tmp3 = tmp10 - tmp13;
                tmp1 = tmp11 + tmp12;
                tmp2 = tmp11 - tmp12;
                // odd part
                tmp5 = pMCUSrc[iCol + 24] * pQuant[iCol + 24]; // get 3rd row
                tmp6 = pMCUSrc[iCol + 40];                     // get 5th row
                if (tmp6) {
                    // very likely that row 5 = 0
                    tmp6 = tmp6 * pQuant[iCol + 40];
                    z13 = tmp6 + tmp5;
                    z10 = tmp6 - tmp5;
                } else {
                    z13 = tmp5;
                    z10 = -tmp5;
                }
                tmp4 = pMCUSrc[iCol + 8] * pQuant[iCol + 8]; // get 1st row
                tmp7 = pMCUSrc[iCol + 56];                   // get 7th row
                if (tmp7) {
                    // very likely that row 7 = 0
                    tmp7 = tmp7 * pQuant[iCol + 56];
                    z11 = tmp4 + tmp7;
                    z12 = tmp4 - tmp7;
                } else {
                    z11 = z12 = tmp4;
                }
                tmp7 = z11 + z13;
                tmp11 = (((z11 - z13) * 362) >> 8);         // 362>>8 = 1.414213562
                z5 = (((z10 + z12) * 473) >> 8);            // 473>>8 = 1.8477
                tmp12 = ((z10 * -669) >> 8) + z5;           // -669>>8 = -2.6131259
                tmp6 = tmp12 - tmp7;
                tmp5 = tmp11 - tmp6;
                tmp10 = ((z12 * 277) >> 8) - z5;            // 277>>8 = 1.08239
                tmp4 = tmp10 + tmp5;
                pMCUSrc[iCol] = (short) (tmp0 + tmp7);      // row0
                pMCUSrc[iCol + 8] = (short) (tmp1 + tmp6);  // row 1
                pMCUSrc[iCol + 16] = (short) (tmp2 + tmp5); // row 2
                pMCUSrc[iCol + 24] = (short) (tmp3 - tmp4); // row 3
                pMCUSrc[iCol + 32] = (short) (tmp3 + tmp4); // row 4
                pMCUSrc[iCol + 40] = (short) (tmp2 - tmp5); // row 5
                pMCUSrc[iCol + 48] = (short) (tmp1 - tmp6); // row 6
                pMCUSrc[iCol + 56] = (short) (tmp0 - tmp7); // row 7
            } // full calculation needed
        } // if column has data in it
    } // for each column
      // now do rows
    pOutput = (unsigned char *) pMCUSrc; // store output pixels back into MCU
    for (iRow = 0; iRow < 64; iRow += 8) {
        // all rows must be calculated
        // even part
        if (ucMaxACCol < 0x10) {
            // quick and dirty calculation (right 4 columns are all 0's)
            if (ucMaxACCol < 0x04) {
                // very likely case (1 or 2 columns occupied)
                // even part
                tmp0 = tmp1 = tmp2 = tmp3 = pMCUSrc[iRow + 0];
                // odd part
                tmp7 = pMCUSrc[iRow + 1];
                tmp6 = (tmp7 * 217) >> 8;   // * 0.8477
                tmp5 = (tmp7 * 145) >> 8;   // * 0.5663
                tmp4 = -((tmp7 * 51) >> 8); // * -0.199
            } else {
                tmp10 = pMCUSrc[iRow + 0];
                tmp13 = pMCUSrc[iRow + 2];
                tmp12 = ((tmp13 * 106) >> 8); // 2-6 * 1.414
                tmp0 = tmp10 + tmp13;
                tmp3 = tmp10 - tmp13;
                tmp1 = tmp10 + tmp12;
                tmp2 = tmp10 - tmp12;
                // odd part
                z13 = pMCUSrc[iRow + 3];
                z11 = pMCUSrc[iRow + 1];
                tmp7 = z11 + z13;
                tmp11 = ((z11 - z13) * 362) >> 8; // * 1.414
                z5 = ((z11 - z13) * 473) >> 8;    // * 1.8477
                tmp10 = ((z11 * 277) >> 8) - z5;  // * 1.08239
                tmp12 = ((z13 * 669) >> 8) + z5;  // * 2.61312
                tmp6 = tmp12 - tmp7;
                tmp5 = tmp11 - tmp6;
                tmp4 = tmp10 + tmp5;
            }
        } else {
            // need to do the full calculation
            tmp10 = pMCUSrc[iRow + 0] + pMCUSrc[iRow + 4];
            tmp11 = pMCUSrc[iRow + 0] - pMCUSrc[iRow + 4];
            tmp13 = pMCUSrc[iRow + 2] + pMCUSrc[iRow + 6];
            tmp12 = (((pMCUSrc[iRow + 2] - pMCUSrc[iRow + 6]) * 362) >> 8) - tmp13; // 2-6 * 1.414
            tmp0 = tmp10 + tmp13;
            tmp3 = tmp10 - tmp13;
            tmp1 = tmp11 + tmp12;
            tmp2 = tmp11 - tmp12;
            // odd part
            z13 = pMCUSrc[iRow + 5] + pMCUSrc[iRow + 3];
            z10 = pMCUSrc[iRow + 5] - pMCUSrc[iRow + 3];
            z11 = pMCUSrc[iRow + 1] + pMCUSrc[iRow + 7];
            z12 = pMCUSrc[iRow + 1] - pMCUSrc[iRow + 7];
            tmp7 = z11 + z13;
            tmp11 = ((z11 - z13) * 362) >> 8; // * 1.414
            z5 = ((z10 + z12) * 473) >> 8;    // * 1.8477
            tmp10 = ((z12 * 277) >> 8) - z5;  // * 1.08239
            tmp12 = ((z10 * -669) >> 8) + z5; // * 2.61312
            tmp6 = tmp12 - tmp7;
            tmp5 = tmp11 - tmp6;
            tmp4 = tmp10 + tmp5;
        }
        // final output stage - scale down and range limit
        pOutput[0] = ucRangeTable[(((tmp0 + tmp7) >> 5) & 0x3ff)];
        pOutput[1] = ucRangeTable[(((tmp1 + tmp6) >> 5) & 0x3ff)];
        pOutput[2] = ucRangeTable[(((tmp2 + tmp5) >> 5) & 0x3ff)];
        pOutput[3] = ucRangeTable[(((tmp3 - tmp4) >> 5) & 0x3ff)];
        pOutput[4] = ucRangeTable[(((tmp3 + tmp4) >> 5) & 0x3ff)];
        pOutput[5] = ucRangeTable[(((tmp2 - tmp5) >> 5) & 0x3ff)];
        pOutput[6] = ucRangeTable[(((tmp1 - tmp6) >> 5) & 0x3ff)];
        pOutput[7] = ucRangeTable[(((tmp0 - tmp7) >> 5) & 0x3ff)];
        pOutput += 8;
    } // for each row
}

// render grayscale MCU as either 1-bit or RGB565
static void JPEGPutMCUGray(JPEGIMAGE *pJPEG, int x, int y) {
    int i, j, xcount, ycount;
    uint8_t *pSrc = (uint8_t *) &pJPEG->sMCUs[0];

    // For odd-sized JPEGs, don't draw past the edge of the image bounds
    xcount = ycount = 8;
    if (x + 8 > pJPEG->iWidth) {
        xcount = pJPEG->iWidth & 7;
    }
    if (y + 8 > pJPEG->iHeight) {
        ycount = pJPEG->iHeight & 7;
    }
    if (pJPEG->ucPixelType == ONE_BIT_GRAYSCALE) {
        const int iPitch = ((pJPEG->iWidth + 31) >> 3) & 0xfffc;
        uint8_t *pDest = (uint8_t *) &pJPEG->pImage[(y * iPitch) + (x >> 3)];

        for (i = 0; i < ycount; i++) {
            // do up to 8 rows
            uint8_t ucPixels = 0;
            for (j = 0; j < xcount; j++) {
                if (pSrc[j] > 127) {
                    ucPixels |= (1 << j);
                }
            }
            pDest[0] = ucPixels; // one byte holds the 8 pixels
            pSrc += 8;
            pDest += iPitch;   // next line
        }
    } else {
        // must be RGB565 output
        const int iPitch = pJPEG->iWidth;
        uint16_t *usDest = (uint16_t *) &pJPEG->pImage[(y * iPitch * 2) + x * 2];

        for (i = 0; i < ycount; i++) {
            // do up to 8 rows
            for (j = 0; j < xcount; j++) {
                *usDest++ = usGrayTo565[*pSrc++];
            }
            pSrc += (8 - xcount);
            usDest -= xcount;
            usDest += iPitch; // next line
        }
    } // RGB565
}

static void JPEGPutMCU8BitGray(JPEGIMAGE *pJPEG, int x, int y) {
    int i, j, xcount, ycount;
    const int iPitch = pJPEG->iWidth;
    uint8_t *pDest, *pSrc = (uint8_t *) &pJPEG->sMCUs[0];
    pDest = (uint8_t *) &pJPEG->pImage[(y * iPitch) + x];
    if (pJPEG->ucSubSample <= 0x11) {
        // single Y
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            // special handling of 1/2 size (pixel averaging)
            int pix;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    pix = (pSrc[0] + pSrc[1] + pSrc[8] + pSrc[9] + 2) >> 2;      // average 2x2 block
                    pDest[j] = (uint8_t) pix;
                    pSrc += 2;
                }
                pSrc += 8;  // skip extra line
                pDest += iPitch;
            }
            return;
        }
        xcount = ycount = 8; // debug
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            xcount = ycount = 2;
        } else if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            xcount = ycount = 1;
        }
        if ((x + 8) > pJPEG->iWidth) {
            xcount = pJPEG->iWidth & 7;
        }
        if ((y + 8) > pJPEG->iHeight) {
            ycount = pJPEG->iHeight & 7;
        }
        for (i = 0; i < ycount; i++) {
            // do up to 8 rows
            for (j = 0; j < xcount; j++) {
                *pDest++ = *pSrc++;
            }
            pSrc += (8 - xcount);
            pDest -= xcount;
            pDest += iPitch; // next line
        }
        return;
    } // single Y source
    if (pJPEG->ucSubSample == 0x21) {
        // stacked horizontally
        if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            // only 2 pixels emitted
            pDest[0] = pSrc[0];
            pDest[1] = pSrc[128];
            return;
        } /* 1/8 */
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    int pix;
                    pix = (pSrc[j * 2] + pSrc[j * 2 + 1] + pSrc[j * 2 + 8] + pSrc[j * 2 + 9] + 2) >> 2;
                    pDest[j] = (uint8_t) pix;
                    pix = (pSrc[j * 2 + 128] + pSrc[j * 2 + 129] + pSrc[j * 2 + 136] + pSrc[j * 2 + 137] + 2) >> 2;
                    pDest[j + 4] = (uint8_t) pix;
                }
                pSrc += 16;
                pDest += iPitch;
            }
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            // each MCU contributes a 2x2 block
            pDest[0] = pSrc[0]; // Y0
            pDest[1] = pSrc[1];
            pDest[iPitch] = pSrc[2];
            pDest[iPitch + 1] = pSrc[3];

            pDest[2] = pSrc[128]; // Y`
            pDest[3] = pSrc[129];
            pDest[iPitch + 2] = pSrc[130];
            pDest[iPitch + 3] = pSrc[131];
            return;
        }
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                pDest[j] = pSrc[j];
                pDest[j + 8] = pSrc[128 + j];
            }
            pSrc += 8;
            pDest += iPitch;
        }
    } // 0x21
    if (pJPEG->ucSubSample == 0x12) {
        // stacked vertically
        if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            // only 2 pixels emitted
            pDest[0] = pSrc[0];
            pDest[iPitch] = pSrc[128];
            return;
        } /* 1/8 */
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    int pix;
                    pix = (pSrc[j * 2] + pSrc[j * 2 + 1] + pSrc[j * 2 + 8] + pSrc[j * 2 + 9] + 2) >> 2;
                    pDest[j] = (uint8_t) pix;
                    pix = (pSrc[j * 2 + 128] + pSrc[j * 2 + 129] + pSrc[j * 2 + 136] + pSrc[j * 2 + 137] + 2) >> 2;
                    pDest[4 * iPitch + j] = (uint8_t) pix;
                }
                pSrc += 16;
                pDest += iPitch;
            }
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            // each MCU contributes a 2x2 block
            pDest[0] = pSrc[0]; // Y0
            pDest[1] = pSrc[1];
            pDest[iPitch] = pSrc[2];
            pDest[iPitch + 1] = pSrc[3];

            pDest[iPitch * 2] = pSrc[128];     // Y`
            pDest[iPitch * 2 + 1] = pSrc[129];
            pDest[iPitch * 3] = pSrc[130];
            pDest[iPitch * 3 + 1] = pSrc[131];
            return;
        }
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                pDest[j] = pSrc[j];
                pDest[8 * iPitch + j] = pSrc[128 + j];
            }
            pSrc += 8;
            pDest += iPitch;
        }
    } // 0x12
    if (pJPEG->ucSubSample == 0x22) {
        if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            // each MCU contributes 1 pixel
            pDest[0] = pSrc[0];            // Y0
            pDest[1] = pSrc[128];          // Y1
            pDest[iPitch] = pSrc[256];     // Y2
            pDest[iPitch + 1] = pSrc[384]; // Y3
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            // each MCU contributes 2x2 pixels
            pDest[0] = pSrc[0]; // Y0
            pDest[1] = pSrc[1];
            pDest[iPitch] = pSrc[2];
            pDest[iPitch + 1] = pSrc[3];

            pDest[2] = pSrc[128]; // Y1
            pDest[3] = pSrc[129];
            pDest[iPitch + 2] = pSrc[130];
            pDest[iPitch + 3] = pSrc[131];

            pDest[iPitch * 2] = pSrc[256];     // Y2
            pDest[iPitch * 2 + 1] = pSrc[257];
            pDest[iPitch * 3] = pSrc[258];
            pDest[iPitch * 3 + 1] = pSrc[259];

            pDest[iPitch * 2 + 2] = pSrc[384]; // Y3
            pDest[iPitch * 2 + 3] = pSrc[385];
            pDest[iPitch * 3 + 2] = pSrc[386];
            pDest[iPitch * 3 + 3] = pSrc[387];
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    int pix;
                    pix = (pSrc[j * 2] + pSrc[j * 2 + 1] + pSrc[j * 2 + 8] + pSrc[j * 2 + 9] + 2) >> 2;
                    pDest[j] = (uint8_t) pix;                  // Y0
                    pix = (pSrc[j * 2 + 128] + pSrc[j * 2 + 129] + pSrc[j * 2 + 136] + pSrc[j * 2 + 137] + 2) >> 2;
                    pDest[j + 4] = (uint8_t) pix;              // Y1
                    pix = (pSrc[j * 2 + 256] + pSrc[j * 2 + 257] + pSrc[j * 2 + 264] + pSrc[j * 2 + 265] + 2) >> 2;
                    pDest[iPitch * 4 + j] = (uint8_t) pix;     // Y2
                    pix = (pSrc[j * 2 + 384] + pSrc[j * 2 + 385] + pSrc[j * 2 + 392] + pSrc[j * 2 + 393] + 2) >> 2;
                    pDest[iPitch * 4 + j + 4] = (uint8_t) pix; // Y3
                }
                pSrc += 16;
                pDest += iPitch;
            }
            return;
        }
        xcount = ycount = 16;
        if ((x + 16) > pJPEG->iWidth) {
            xcount = pJPEG->iWidth & 15;
        }
        if ((y + 16) > pJPEG->iHeight) {
            ycount = pJPEG->iHeight & 15;
        }
        // The source MCUs are 64 bytes of data at offsets of 0, 128, 256, 384
        // The 4 8x8 MCUs are looping through using a single pass of x/y by
        // using the 0/8 bit of the coordinate to adjust the source data offset
        for (i = 0; i < ycount; i++) {
            for (j = 0; j < xcount; j++) {
                pDest[j] = pSrc[j + ((i & 8) * 24) + ((j & 8) * 15)];
            }
            pSrc += 8;
            pDest += iPitch;
        }
    } // 0x22
}

static void JPEGPutMCU1BitGray(JPEGIMAGE *pJPEG, int x, int y) {
    int i, j, xcount, ycount;
    const int iPitch = ((pJPEG->iWidth + 31) >> 3) & 0xfffc;
    uint8_t *pDest, *pSrc = (uint8_t *) &pJPEG->sMCUs[0];
    pDest = (uint8_t *) &pJPEG->pImage[(y * iPitch) + (x >> 3)];
    if (pJPEG->ucSubSample <= 0x11) {
        // single Y
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            // special handling of 1/2 size (pixel averaging)
            int pix;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    pix = (pSrc[0] + pSrc[1] + pSrc[8] + pSrc[9] + 2) >> 2;      // average 2x2 block
                    pDest[j] = (uint8_t) pix;
                    pSrc += 2;
                }
                pSrc += 8;  // skip extra line
                pDest += iPitch;
            }
            return;
        }
        xcount = ycount = 8; // debug
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            xcount = ycount = 2;
        } else if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            xcount = ycount = 1;
        }
        for (i = 0; i < ycount; i++) {
            // do up to 8 rows
            uint8_t ucPixels = 0;
            for (j = 0; j < xcount; j++) {
                if (pSrc[j] > 127) {
                    ucPixels |= (1 << j);
                }
            }
            pDest[0] = ucPixels;
            pSrc += xcount;
            pDest += iPitch;   // next line
        }
        return;
    } // single Y source
    if (pJPEG->ucSubSample == 0x21) {
        // stacked horizontally
        if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            // only 2 pixels emitted
            pDest[0] = pSrc[0];
            pDest[1] = pSrc[128];
            return;
        } /* 1/8 */
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    int pix;
                    pix = (pSrc[j * 2] + pSrc[j * 2 + 1] + pSrc[j * 2 + 8] + pSrc[j * 2 + 9] + 2) >> 2;
                    pDest[j] = (uint8_t) pix;
                    pix = (pSrc[j * 2 + 128] + pSrc[j * 2 + 129] + pSrc[j * 2 + 136] + pSrc[j * 2 + 137] + 2) >> 2;
                    pDest[j + 4] = (uint8_t) pix;
                }
                pSrc += 16;
                pDest += iPitch;
            }
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            // each MCU contributes a 2x2 block
            pDest[0] = pSrc[0]; // Y0
            pDest[1] = pSrc[1];
            pDest[iPitch] = pSrc[2];
            pDest[iPitch + 1] = pSrc[3];

            pDest[2] = pSrc[128]; // Y`
            pDest[3] = pSrc[129];
            pDest[iPitch + 2] = pSrc[130];
            pDest[iPitch + 3] = pSrc[131];
            return;
        }
        for (i = 0; i < 8; i++) {
            uint8_t uc0 = 0, uc1 = 0;
            for (j = 0; j < 8; j++) {
                if (pSrc[j] > 127) {
                    uc0 |= (1 << j);
                }
                if (pSrc[128 + j] > 127) {
                    uc1 |= (1 << j);
                }
            }
            pDest[0] = uc0;
            pDest[1] = uc1;
            pSrc += 8;
            pDest += iPitch;
        }
    } // 0x21
    if (pJPEG->ucSubSample == 0x12) {
        // stacked vertically
        if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            // only 2 pixels emitted
            pDest[0] = pSrc[0];
            pDest[iPitch] = pSrc[128];
            return;
        } /* 1/8 */
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    int pix;
                    pix = (pSrc[j * 2] + pSrc[j * 2 + 1] + pSrc[j * 2 + 8] + pSrc[j * 2 + 9] + 2) >> 2;
                    pDest[j] = (uint8_t) pix;
                    pix = (pSrc[j * 2 + 128] + pSrc[j * 2 + 129] + pSrc[j * 2 + 136] + pSrc[j * 2 + 137] + 2) >> 2;
                    pDest[4 * iPitch + j] = (uint8_t) pix;
                }
                pSrc += 16;
                pDest += iPitch;
            }
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            // each MCU contributes a 2x2 block
            pDest[0] = pSrc[0]; // Y0
            pDest[1] = pSrc[1];
            pDest[iPitch] = pSrc[2];
            pDest[iPitch + 1] = pSrc[3];

            pDest[iPitch * 2] = pSrc[128];     // Y`
            pDest[iPitch * 2 + 1] = pSrc[129];
            pDest[iPitch * 3] = pSrc[130];
            pDest[iPitch * 3 + 1] = pSrc[131];
            return;
        }
        for (i = 0; i < 8; i++) {
            uint8_t uc0 = 0, uc1 = 0;
            for (j = 0; j < 8; j++) {
                if (pSrc[j] > 127) {
                    uc0 |= (1 << j);
                }
                if (pSrc[128 + j] > 127) {
                    uc1 |= (1 << j);
                }
            }
            pDest[0] = uc0;
            pDest[8 * iPitch] = uc1;
            pSrc += 8;
            pDest += iPitch;
        }
    } // 0x12
    if (pJPEG->ucSubSample == 0x22) {
        if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
            // each MCU contributes 1 pixel
            pDest[0] = pSrc[0];            // Y0
            pDest[1] = pSrc[128];          // Y1
            pDest[iPitch] = pSrc[256];     // Y2
            pDest[iPitch + 1] = pSrc[384]; // Y3
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
            // each MCU contributes 2x2 pixels
            pDest[0] = pSrc[0]; // Y0
            pDest[1] = pSrc[1];
            pDest[iPitch] = pSrc[2];
            pDest[iPitch + 1] = pSrc[3];

            pDest[2] = pSrc[128]; // Y1
            pDest[3] = pSrc[129];
            pDest[iPitch + 2] = pSrc[130];
            pDest[iPitch + 3] = pSrc[131];

            pDest[iPitch * 2] = pSrc[256];     // Y2
            pDest[iPitch * 2 + 1] = pSrc[257];
            pDest[iPitch * 3] = pSrc[258];
            pDest[iPitch * 3 + 1] = pSrc[259];

            pDest[iPitch * 2 + 2] = pSrc[384]; // Y3
            pDest[iPitch * 2 + 3] = pSrc[385];
            pDest[iPitch * 3 + 2] = pSrc[386];
            pDest[iPitch * 3 + 3] = pSrc[387];
            return;
        }
        if (pJPEG->iOptions & JPEG_SCALE_HALF) {
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    int pix;
                    pix = (pSrc[j * 2] + pSrc[j * 2 + 1] + pSrc[j * 2 + 8] + pSrc[j * 2 + 9] + 2) >> 2;
                    pDest[j] = (uint8_t) pix;                  // Y0
                    pix = (pSrc[j * 2 + 128] + pSrc[j * 2 + 129] + pSrc[j * 2 + 136] + pSrc[j * 2 + 137] + 2) >> 2;
                    pDest[j + 4] = (uint8_t) pix;              // Y1
                    pix = (pSrc[j * 2 + 256] + pSrc[j * 2 + 257] + pSrc[j * 2 + 264] + pSrc[j * 2 + 265] + 2) >> 2;
                    pDest[iPitch * 4 + j] = (uint8_t) pix;     // Y2
                    pix = (pSrc[j * 2 + 384] + pSrc[j * 2 + 385] + pSrc[j * 2 + 392] + pSrc[j * 2 + 393] + 2) >> 2;
                    pDest[iPitch * 4 + j + 4] = (uint8_t) pix; // Y3
                }
                pSrc += 16;
                pDest += iPitch;
            }
            return;
        }
        for (i = 0; i < 8; i++) {
            uint8_t uc00 = 0, uc10 = 0, uc01 = 0, uc11 = 0;
            for (j = 0; j < 8; j++) {
                if (pSrc[j] > 127) {
                    uc00 |= (1 << j);     // Y0
                }
                if (pSrc[j + 128] > 127) {
                    uc10 |= (1 << j);     // Y1
                }
                if (pSrc[j + 256] > 127) {
                    uc01 |= (1 << j);     // Y2
                }
                if (pSrc[j + 384] > 127) {
                    uc11 |= (1 << j);     // Y3
                }
            }
            pDest[0] = uc00;              // Y0
            pDest[1] = uc10;              // Y1
            pDest[iPitch * 8] = uc01;     // Y2
            pDest[iPitch * 8 + 1] = uc11; // Y3
            pSrc += 8;
            pDest += iPitch;
        }
    } // 0x22
}

static void JPEGPixelLE(uint16_t *pDest, int iY, int iCb, int iCr) {
    uint32_t ulPixel;
    uint32_t ulCbCr = (iCb | (iCr << 16));
    uint32_t ulTmp;                                   // for green calc
    ulTmp = -1409;
    ulTmp = (ulTmp & 0xffff) | (-2925 << 16);
    ulCbCr = __SSUB16(ulCbCr, 0x00800080);            // dual 16-bit subtraction
    ulPixel = __SMLAD(ulCbCr, ulTmp, iY) >> 14;       // G
    ulPixel = __USAT16(ulPixel, 6) << 5;              // range limit to 6 bits
    ulTmp = __SMLAD(7258, ulCbCr, iY) >> 15;          // Blue
    ulTmp = __USAT16(ulTmp, 5);                       // range limit to 5 bits
    ulPixel |= ulTmp;                                 // now we have G + B
    ulTmp = __SMLAD(5742, ulCbCr >> 16, iY) >> 15;    // Red
    ulTmp = __USAT16(ulTmp, 5);                       // range limit to 5 bits
    ulPixel |= (ulTmp << 11);                         // now we have R + G + B
    pDest[0] = (uint16_t) ulPixel;
}

static void JPEGPixel2LE(uint16_t *pDest, int iY1, int iY2, int iCb, int iCr) {
    uint32_t ulPixel1, ulPixel2;
    uint32_t ulCbCr = (iCb | (iCr << 16));
    uint32_t ulTmp2, ulTmp;                             // for green calc
    ulTmp = -1409;
    ulTmp = (ulTmp & 0xffff) | (-2925 << 16);
    ulCbCr = __SSUB16(ulCbCr, 0x00800080);              // dual 16-bit subtraction
    ulPixel1 = __SMLAD(ulCbCr, ulTmp, iY1) >> 14;       // G for pixel 1
    ulPixel2 = __SMLAD(ulCbCr, ulTmp, iY2) >> 14;       // G for pixel 2
    ulPixel1 |= (ulPixel2 << 16);
    ulPixel1 = __USAT16(ulPixel1, 6) << 5;              // range limit both to 6 bits
    ulTmp = __SMLAD(7258, ulCbCr, iY1) >> 15;           // Blue 1
    ulTmp2 = __SMLAD(7258, ulCbCr, iY2) >> 15;          // Blue 2
    ulTmp = __USAT16(ulTmp | (ulTmp2 << 16), 5);        // range limit both to 5 bits
    ulPixel1 |= ulTmp;                                  // now we have G + B
    ulTmp = __SMLAD(5742, ulCbCr >> 16, iY1) >> 15;     // Red 1
    ulTmp2 = __SMLAD(5742, ulCbCr >> 16, iY2) >> 15;    // Red 2
    ulTmp = __USAT16(ulTmp | (ulTmp2 << 16), 5);        // range limit both to 5 bits
    ulPixel1 |= (ulTmp << 11);                          // now we have R + G + B
    *(uint32_t *) &pDest[0] = ulPixel1;
}

static void JPEGPutMCU11(JPEGIMAGE *pJPEG, int x, int y) {
    int iCr, iCb;
    signed int Y;
    int iCol;
    int iRow;
    const int iPitch = pJPEG->iWidth;
    uint8_t *pY, *pCr, *pCb;
    uint16_t *pOutput = (uint16_t *) &pJPEG->pImage[(y * iPitch * 2) + x * 2];

    pY = (unsigned char *) &pJPEG->sMCUs[0 * DCTSIZE];
    pCb = (unsigned char *) &pJPEG->sMCUs[1 * DCTSIZE];
    pCr = (unsigned char *) &pJPEG->sMCUs[2 * DCTSIZE];

    if (pJPEG->iOptions & JPEG_SCALE_HALF) {
        for (iRow = 0; iRow < 4; iRow++) {
            // up to 8 rows to do
            for (iCol = 0; iCol < 4; iCol++) {
                // up to 4x2 cols to do
                iCr = (pCr[0] + pCr[1] + pCr[8] + pCr[9] + 2) >> 2;
                iCb = (pCb[0] + pCb[1] + pCb[8] + pCb[9] + 2) >> 2;
                Y = (pY[0] + pY[1] + pY[8] + pY[9]) << 10;
                JPEGPixelLE(pOutput + iCol, Y, iCb, iCr);
                pCr += 2;
                pCb += 2;
                pY += 2;
            } // for col
            pCr += 8;
            pCb += 8;
            pY += 8;
            pOutput += iPitch;
        } // for row
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
        // special case for 1/8 scaling
        // only 4 pixels to draw, so no looping needed
        iCr = pCr[0];
        iCb = pCb[0];
        Y = (int) (pY[0]) << 12;
        JPEGPixelLE(pOutput, Y, iCb, iCr);
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
        // special case for 1/4 scaling
        iCr = *pCr++;
        iCb = *pCb++;
        Y = (int) (*pY++) << 12;
        JPEGPixelLE(pOutput, Y, iCb, iCr);
        iCr = *pCr++;
        iCb = *pCb++;
        Y = (int) (*pY++) << 12;
        JPEGPixelLE(pOutput + 1, Y, iCb, iCr);
        iCr = *pCr++;
        iCb = *pCb++;
        Y = (int) (*pY++) << 12;
        JPEGPixelLE(pOutput + iPitch, Y, iCb, iCr);
        iCr = *pCr++;
        iCb = *pCb++;
        Y = (int) (*pY++) << 12;
        JPEGPixelLE(pOutput + 1 + iPitch, Y, iCb, iCr);
        return;
    }
    for (iRow = 0; iRow < 8; iRow++) {
        // up to 8 rows to do
        for (iCol = 0; iCol < 8; iCol++) {
            // up to 4x2 cols to do
            iCr = *pCr++;
            iCb = *pCb++;
            Y = (int) (*pY++) << 12;
            JPEGPixelLE(pOutput + iCol, Y, iCb, iCr);
        } // for col
        pOutput += iPitch;
    }     // for row
}         /* JPEGPutMCU11() */

static void JPEGPutMCU22(JPEGIMAGE *pJPEG, int x, int y) {
    uint32_t Cr, Cb;
    signed int Y1, Y2, Y3, Y4;
    int iRow, iRowLimit, iCol, iXCount1, iXCount2, iYCount;
    unsigned char *pY, *pCr, *pCb;
    const int iPitch = pJPEG->iWidth;
    int bUseOdd1, bUseOdd2; // special case where 24bpp odd sized image can clobber first column
    uint16_t *pOutput = (uint16_t *) &pJPEG->pImage[(y * iPitch * 2) + x * 2];

    pY = (unsigned char *) &pJPEG->sMCUs[0 * DCTSIZE];
    pCb = (unsigned char *) &pJPEG->sMCUs[4 * DCTSIZE];
    pCr = (unsigned char *) &pJPEG->sMCUs[5 * DCTSIZE];

    if (pJPEG->iOptions & JPEG_SCALE_HALF) {
        // special handling of 1/2 size (pixel averaging)
        for (iRow = 0; iRow < 4; iRow++) {
            // 16x16 becomes 8x8 of 2x2 pixels
            for (iCol = 0; iCol < 4; iCol++) {
                Y1 = (pY[iCol * 2] + pY[iCol * 2 + 1] + pY[iCol * 2 + 8] + pY[iCol * 2 + 9]) << 10;
                Cb = pCb[iCol];
                Cr = pCr[iCol];
                JPEGPixelLE(pOutput + iCol, Y1, Cb, Cr); // top left
                Y1 = (pY[iCol * 2 + (DCTSIZE * 2)] + pY[iCol * 2 + 1 + (DCTSIZE * 2)]
                      + pY[iCol * 2 + 8 + (DCTSIZE * 2)] + pY[iCol * 2 + 9 + (DCTSIZE * 2)]) << 10;
                Cb = pCb[iCol + 4];
                Cr = pCr[iCol + 4];
                JPEGPixelLE(pOutput + iCol + 4, Y1, Cb, Cr); // top right
                Y1 = (pY[iCol * 2 + (DCTSIZE * 4)] + pY[iCol * 2 + 1 + (DCTSIZE * 4)]
                      + pY[iCol * 2 + 8 + (DCTSIZE * 4)] + pY[iCol * 2 + 9 + (DCTSIZE * 4)]) << 10;
                Cb = pCb[iCol + 32];
                Cr = pCr[iCol + 32];
                JPEGPixelLE(pOutput + iCol + iPitch * 4, Y1, Cb, Cr); // bottom left
                Y1 = (pY[iCol * 2 + (DCTSIZE * 6)] + pY[iCol * 2 + 1 + (DCTSIZE * 6)]
                      + pY[iCol * 2 + 8 + (DCTSIZE * 6)] + pY[iCol * 2 + 9 + (DCTSIZE * 6)]) << 10;
                Cb = pCb[iCol + 32 + 4];
                Cr = pCr[iCol + 32 + 4];
                JPEGPixelLE(pOutput + iCol + 4 + iPitch * 4, Y1, Cb, Cr); // bottom right
            }
            pY += 8;
            pCb += 8;
            pCr += 8;
            pOutput += iPitch;
        }
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
        Y1 = pY[0] << 12;  // scale to level of conversion table
        Cb = pCb[0];
        Cr = pCr[0];
        JPEGPixelLE(pOutput, Y1, Cb, Cr);
        // top right block
        Y1 = pY[DCTSIZE * 2] << 12; // scale to level of conversion table
        JPEGPixelLE(pOutput + 1, Y1, Cb, Cr);
        // bottom left block
        Y1 = pY[DCTSIZE * 4] << 12; // scale to level of conversion table
        JPEGPixelLE(pOutput + iPitch, Y1, Cb, Cr);
        // bottom right block
        Y1 = pY[DCTSIZE * 6] << 12; // scale to level of conversion table
        JPEGPixelLE(pOutput + 1 + iPitch, Y1, Cb, Cr);
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
        // special case of 1/4
        for (iRow = 0; iRow < 2; iRow++) {
            for (iCol = 0; iCol < 2; iCol++) {
                // top left block
                Y1 = pY[iCol] << 12;  // scale to level of conversion table
                Cb = pCb[0];
                Cr = pCr[0];
                JPEGPixelLE(pOutput + iCol, Y1, Cb, Cr);
                // top right block
                Y1 = pY[iCol + (DCTSIZE * 2)] << 12; // scale to level of conversion table
                Cb = pCb[1];
                Cr = pCr[1];
                JPEGPixelLE(pOutput + 2 + iCol, Y1, Cb, Cr);
                // bottom left block
                Y1 = pY[iCol + DCTSIZE * 4] << 12;  // scale to level of conversion table
                Cb = pCb[2];
                Cr = pCr[2];
                JPEGPixelLE(pOutput + iPitch * 2 + iCol, Y1, Cb, Cr);
                // bottom right block
                Y1 = pY[iCol + DCTSIZE * 6] << 12; // scale to level of conversion table
                Cb = pCb[3];
                Cr = pCr[3];
                JPEGPixelLE(pOutput + iPitch * 2 + 2 + iCol, Y1, Cb, Cr);
            } // for each column
            pY += 2; // skip 1 line of source pixels
            pOutput += iPitch;
        }
        return;
    }
    /* Convert YCC pixels into RGB pixels and store in output image */
    iYCount = 4;
    iRowLimit = 16; // assume all rows possible to draw
    if ((y + 15) >= pJPEG->iHeight) {
        iRowLimit = pJPEG->iHeight & 15;
        if (iRowLimit < 8) {
            iYCount = iRowLimit >> 1;
        }
    }
    bUseOdd1 = bUseOdd2 = 1; // assume odd column can be used
    if ((x + 15) >= pJPEG->iWidth) {
        iCol = (((pJPEG->iWidth & 15) + 1) >> 1);
        if (iCol >= 4) {
            iXCount1 = 4;
            iXCount2 = iCol - 4;
            if (pJPEG->iWidth & 1 && (iXCount2 * 2) + 8 + (x * 16) > pJPEG->iWidth) {
                bUseOdd2 = 0;
            }
        } else {
            iXCount1 = iCol;
            iXCount2 = 0;
            if (pJPEG->iWidth & 1 && (iXCount1 * 2) + (x * 16) > pJPEG->iWidth) {
                bUseOdd1 = 0;
            }
        }
    } else {
        iXCount1 = iXCount2 = 4;
    }
    for (iRow = 0; iRow < iYCount; iRow++) {
        // up to 4 rows to do
        for (iCol = 0; iCol < iXCount1; iCol++) {
            // up to 4 cols to do
            // for top left block
            Y1 = pY[iCol * 2];
            Y2 = pY[iCol * 2 + 1];
            Y3 = pY[iCol * 2 + 8];
            Y4 = pY[iCol * 2 + 9];
            Y1 <<= 12;  // scale to level of conversion table
            Y2 <<= 12;
            Y3 <<= 12;
            Y4 <<= 12;
            Cb = pCb[iCol];
            Cr = pCr[iCol];
            if (bUseOdd1 || iCol != (iXCount1 - 1)) {
                // only render if it won't go off the right edge
                JPEGPixel2LE(pOutput + (iCol << 1), Y1, Y2, Cb, Cr);
                JPEGPixel2LE(pOutput + iPitch + (iCol << 1), Y3, Y4, Cb, Cr);
            } else {
                JPEGPixelLE(pOutput + (iCol << 1), Y1, Cb, Cr);
                JPEGPixelLE(pOutput + iPitch + (iCol << 1), Y3, Cb, Cr);
            }
            // for top right block
            if (iCol < iXCount2) {
                Y1 = pY[iCol * 2 + DCTSIZE * 2];
                Y2 = pY[iCol * 2 + 1 + DCTSIZE * 2];
                Y3 = pY[iCol * 2 + 8 + DCTSIZE * 2];
                Y4 = pY[iCol * 2 + 9 + DCTSIZE * 2];
                Y1 <<= 12;  // scale to level of conversion table
                Y2 <<= 12;
                Y3 <<= 12;
                Y4 <<= 12;
                Cb = pCb[iCol + 4];
                Cr = pCr[iCol + 4];
                if (bUseOdd2 || iCol != (iXCount2 - 1)) {
                    // only render if it won't go off the right edge
                    JPEGPixel2LE(pOutput + 8 + (iCol << 1), Y1, Y2, Cb, Cr);
                    JPEGPixel2LE(pOutput + iPitch + 8 + (iCol << 1), Y3, Y4, Cb, Cr);
                } else {
                    JPEGPixelLE(pOutput + 8 + (iCol << 1), Y1, Cb, Cr);
                    JPEGPixelLE(pOutput + iPitch + 8 + (iCol << 1), Y3, Cb, Cr);
                }
            }
            if (iRowLimit > 8) {
                // for bottom left block
                Y1 = pY[iCol * 2 + DCTSIZE * 4];
                Y2 = pY[iCol * 2 + 1 + DCTSIZE * 4];
                Y3 = pY[iCol * 2 + 8 + DCTSIZE * 4];
                Y4 = pY[iCol * 2 + 9 + DCTSIZE * 4];
                Y1 <<= 12; // scale to level of conversion table
                Y2 <<= 12;
                Y3 <<= 12;
                Y4 <<= 12;
                Cb = pCb[iCol + 32];
                Cr = pCr[iCol + 32];
                if (bUseOdd1 || iCol != (iXCount1 - 1)) {
                    // only render if it won't go off the right edge
                    JPEGPixel2LE(pOutput + iPitch * 8 + (iCol << 1), Y1, Y2, Cb, Cr);
                    JPEGPixel2LE(pOutput + iPitch * 9 + (iCol << 1), Y3, Y4, Cb, Cr);
                } else {
                    JPEGPixelLE(pOutput + iPitch * 8 + (iCol << 1), Y1, Cb, Cr);
                    JPEGPixelLE(pOutput + iPitch * 9 + (iCol << 1), Y3, Cb, Cr);
                }
                // for bottom right block
                if (iCol < iXCount2) {
                    Y1 = pY[iCol * 2 + DCTSIZE * 6];
                    Y2 = pY[iCol * 2 + 1 + DCTSIZE * 6];
                    Y3 = pY[iCol * 2 + 8 + DCTSIZE * 6];
                    Y4 = pY[iCol * 2 + 9 + DCTSIZE * 6];
                    Y1 <<= 12; // scale to level of conversion table
                    Y2 <<= 12;
                    Y3 <<= 12;
                    Y4 <<= 12;
                    Cb = pCb[iCol + 36];
                    Cr = pCr[iCol + 36];
                    if (bUseOdd2 || iCol != (iXCount2 - 1)) {
                        // only render if it won't go off the right edge
                        JPEGPixel2LE(pOutput + iPitch * 8 + 8 + (iCol << 1), Y1, Y2, Cb, Cr);
                        JPEGPixel2LE(pOutput + iPitch * 9 + 8 + (iCol << 1), Y3, Y4, Cb, Cr);
                    } else {
                        JPEGPixelLE(pOutput + iPitch * 8 + 8 + (iCol << 1), Y1, Cb, Cr);
                        JPEGPixelLE(pOutput + iPitch * 9 + 8 + (iCol << 1), Y3, Cb, Cr);
                    }
                }
            } // row limit > 8
        } // for each column
        pY += 16;      // skip to next line of source pixels
        pCb += 8;
        pCr += 8;
        pOutput += iPitch * 2;
    }
}

static void JPEGPutMCU12(JPEGIMAGE *pJPEG, int x, int y) {
    uint32_t Cr, Cb;
    signed int Y1, Y2;
    int iRow, iCol, iXCount, iYCount;
    uint8_t *pY, *pCr, *pCb;
    const int iPitch = pJPEG->iWidth;
    uint16_t *pOutput = (uint16_t *) &pJPEG->pImage[(y * iPitch * 2) + x * 2];

    pY = (uint8_t *) &pJPEG->sMCUs[0 * DCTSIZE];
    pCb = (uint8_t *) &pJPEG->sMCUs[2 * DCTSIZE];
    pCr = (uint8_t *) &pJPEG->sMCUs[3 * DCTSIZE];

    if (pJPEG->iOptions & JPEG_SCALE_HALF) {
        for (iRow = 0; iRow < 4; iRow++) {
            for (iCol = 0; iCol < 4; iCol++) {
                Y1 = (pY[0] + pY[1] + pY[8] + pY[9]) << 10;
                Cb = (pCb[0] + pCb[1] + 1) >> 1;
                Cr = (pCr[0] + pCr[1] + 1) >> 1;
                JPEGPixelLE(pOutput + iCol, Y1, Cb, Cr);
                Y1 = (pY[DCTSIZE * 2] + pY[DCTSIZE * 2 + 1] + pY[DCTSIZE * 2 + 8] + pY[DCTSIZE * 2 + 9]) << 10;
                Cb = (pCb[32] + pCb[33] + 1) >> 1;
                Cr = (pCr[32] + pCr[33] + 1) >> 1;
                JPEGPixelLE(pOutput + iCol + iPitch, Y1, Cb, Cr);
                pCb += 2;
                pCr += 2;
                pY += 2;
            }
            pY += 8;
            pOutput += iPitch * 2;
        }
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
        Y1 = pY[0] << 12;
        Y2 = pY[DCTSIZE * 2] << 12;
        Cb = pCb[0];
        Cr = pCr[0];
        JPEGPixelLE(pOutput, Y1, Cb, Cr);
        JPEGPixelLE(pOutput + iPitch, Y2, Cb, Cr);
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
        // draw a 2x4 block
        Y1 = pY[0] << 12;
        Y2 = pY[2] << 12;
        Cb = pCb[0];
        Cr = pCr[0];
        JPEGPixelLE(pOutput, Y1, Cb, Cr);
        JPEGPixelLE(pOutput + iPitch, Y2, Cb, Cr);
        Y1 = pY[1] << 12;
        Y2 = pY[3] << 12;
        Cb = pCb[1];
        Cr = pCr[1];
        JPEGPixelLE(pOutput + 1, Y1, Cb, Cr);
        JPEGPixelLE(pOutput + 1 + iPitch, Y2, Cb, Cr);
        pY += DCTSIZE * 2; // next Y block below
        Y1 = pY[0] << 12;
        Y2 = pY[2] << 12;
        Cb = pCb[2];
        Cr = pCr[2];
        JPEGPixelLE(pOutput + iPitch * 2, Y1, Cb, Cr);
        JPEGPixelLE(pOutput + iPitch * 3, Y2, Cb, Cr);
        Y1 = pY[1] << 12;
        Y2 = pY[3] << 12;
        Cb = pCb[3];
        Cr = pCr[3];
        JPEGPixelLE(pOutput + 1 + iPitch * 2, Y1, Cb, Cr);
        JPEGPixelLE(pOutput + 1 + iPitch * 3, Y2, Cb, Cr);
        return;
    }
    /* Convert YCC pixels into RGB pixels and store in output image */
    iYCount = 16;
    iXCount = 8;
    for (iRow = 0; iRow < iYCount; iRow += 2) {
        // up to 16 rows to do
        for (iCol = 0; iCol < iXCount; iCol++) {
            // up to 8 cols to do
            Y1 = pY[iCol];
            Y2 = pY[iCol + 8];
            Y1 <<= 12;  // scale to level of conversion table
            Y2 <<= 12;
            Cb = pCb[iCol];
            Cr = pCr[iCol];
            JPEGPixelLE(pOutput + iCol, Y1, Cb, Cr);
            JPEGPixelLE(pOutput + iPitch + iCol, Y2, Cb, Cr);
        }
        pY += 16; // skip to next 2 lines of source pixels
        if (iRow == 6) {
            // next MCU block, skip ahead to correct spot
            pY += (128 - 64);
        }
        pCb += 8;
        pCr += 8;
        pOutput += iPitch * 2; // next 2 lines of dest pixels
    }
}

static void JPEGPutMCU21(JPEGIMAGE *pJPEG, int x, int y) {
    int iCr, iCb;
    signed int Y1, Y2;
    int iCol;
    int iRow;
    uint8_t *pY, *pCr, *pCb;
    const int iPitch = pJPEG->iWidth;
    uint16_t *pOutput = (uint16_t *) &pJPEG->pImage[(y * iPitch * 2) + x * 2];

    pY = (uint8_t *) &pJPEG->sMCUs[0 * DCTSIZE];
    pCb = (uint8_t *) &pJPEG->sMCUs[2 * DCTSIZE];
    pCr = (uint8_t *) &pJPEG->sMCUs[3 * DCTSIZE];

    if (pJPEG->iOptions & JPEG_SCALE_HALF) {
        for (iRow = 0; iRow < 4; iRow++) {
            for (iCol = 0; iCol < 4; iCol++) {
                // left block
                iCr = (pCr[0] + pCr[8] + 1) >> 1;
                iCb = (pCb[0] + pCb[8] + 1) >> 1;
                Y1 = (signed int) (pY[0] + pY[1] + pY[8] + pY[9]) << 10;
                JPEGPixelLE(pOutput + iCol, Y1, iCb, iCr);
                // right block
                iCr = (pCr[4] + pCr[12] + 1) >> 1;
                iCb = (pCb[4] + pCb[12] + 1) >> 1;
                Y1 = (signed int) (pY[128] + pY[129] + pY[136] + pY[137]) << 10;
                JPEGPixelLE(pOutput + iCol + 4, Y1, iCb, iCr);
                pCb++;
                pCr++;
                pY += 2;
            }
            pCb += 12;
            pCr += 12;
            pY += 8;
            pOutput += iPitch;
        }
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
        // draw 2 pixels
        iCr = pCr[0];
        iCb = pCb[0];
        Y1 = (signed int) (pY[0]) << 12;
        Y2 = (signed int) (pY[DCTSIZE * 2]) << 12;
        JPEGPixel2LE(pOutput, Y1, Y2, iCb, iCr);
        return;
    }
    if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
        // draw 4x2 pixels
        // top left
        iCr = pCr[0];
        iCb = pCb[0];
        Y1 = (signed int) (pY[0]) << 12;
        Y2 = (signed int) (pY[1]) << 12;
        JPEGPixel2LE(pOutput, Y1, Y2, iCb, iCr);
        // top right
        iCr = pCr[1];
        iCb = pCb[1];
        Y1 = (signed int) pY[DCTSIZE * 2] << 12;
        Y2 = (signed int) pY[DCTSIZE * 2 + 1] << 12;
        JPEGPixel2LE(pOutput + 2, Y1, Y2, iCb, iCr);
        // bottom left
        iCr = pCr[2];
        iCb = pCb[2];
        Y1 = (signed int) (pY[2]) << 12;
        Y2 = (signed int) (pY[3]) << 12;
        JPEGPixel2LE(pOutput + iPitch, Y1, Y2, iCb, iCr);
        // bottom right
        iCr = pCr[3];
        iCb = pCb[3];
        Y1 = (signed int) pY[DCTSIZE * 2 + 2] << 12;
        Y2 = (signed int) pY[DCTSIZE * 2 + 3] << 12;
        JPEGPixel2LE(pOutput + iPitch + 2, Y1, Y2, iCb, iCr);
        return;
    }
    /* Convert YCC pixels into RGB pixels and store in output image */
    for (iRow = 0; iRow < 8; iRow++) {
        // up to 8 rows to do
        for (iCol = 0; iCol < 4; iCol++) {
            // up to 4x2 cols to do
            // left block
            iCr = *pCr++;
            iCb = *pCb++;
            Y1 = (signed int) (*pY++) << 12;
            Y2 = (signed int) (*pY++) << 12;
            JPEGPixel2LE(pOutput + iCol * 2, Y1, Y2, iCb, iCr);
            // right block
            iCr = pCr[3];
            iCb = pCb[3];
            Y1 = (signed int) pY[126] << 12;
            Y2 = (signed int) pY[127] << 12;
            JPEGPixel2LE(pOutput + 8 + iCol * 2, Y1, Y2, iCb, iCr);
        } // for col
        pCb += 4;
        pCr += 4;
        pOutput += iPitch;
    } // for row
}

// Decode the image
// returns 0 for error, 1 for success
static int DecodeJPEG(JPEGIMAGE *pJPEG) {
    int cx, cy, x, y, mcuCX, mcuCY;
    int iLum0, iLum1, iLum2, iLum3, iCr, iCb;
    signed int iDCPred0, iDCPred1, iDCPred2;
    int i, iQuant1, iQuant2, iQuant3, iErr;
    uint8_t c;
    int iMCUCount, /*xoff, iPitch,*/ bThumbnail = 0;
    int bContinue = 1; // early exit if the DRAW callback wants to stop
    uint32_t l, *pl;
    unsigned char cDCTable0, cACTable0, cDCTable1, cACTable1, cDCTable2, cACTable2;
    int iMaxFill = 16, iScaleShift = 0;

    // Requested the Exif thumbnail
    if (pJPEG->iOptions & JPEG_EXIF_THUMBNAIL) {
        if (pJPEG->iThumbData == 0 || pJPEG->iThumbWidth == 0) {
            // doesn't exist
            pJPEG->iError = JPEG_INVALID_PARAMETER;
            return 0;
        }
        if (!JPEGParseInfo(pJPEG, 1)) {
            // parse the embedded thumbnail file header
            return 0; // something went wrong
        }
    }
    // Fast downscaling options
    if (pJPEG->iOptions & JPEG_SCALE_HALF) {
        iScaleShift = 1;
    } else if (pJPEG->iOptions & JPEG_SCALE_QUARTER) {
        iScaleShift = 2;
        iMaxFill = 1;
    } else if (pJPEG->iOptions & JPEG_SCALE_EIGHTH) {
        iScaleShift = 3;
        iMaxFill = 1;
        bThumbnail = 1;
    }

    // reorder and fix the quantization table for decoding
    JPEGFixQuantD(pJPEG);
    pJPEG->bb.ulBits = MOTOLONG(&pJPEG->ucFileBuf[0]);   // preload first 4 bytes
    pJPEG->bb.pBuf = pJPEG->ucFileBuf;
    pJPEG->bb.ulBitOff = 0;

    cDCTable0 = pJPEG->JPCI[0].dc_tbl_no;
    cACTable0 = pJPEG->JPCI[0].ac_tbl_no;
    cDCTable1 = pJPEG->JPCI[1].dc_tbl_no;
    cACTable1 = pJPEG->JPCI[1].ac_tbl_no;
    cDCTable2 = pJPEG->JPCI[2].dc_tbl_no;
    cACTable2 = pJPEG->JPCI[2].ac_tbl_no;
    iDCPred0 = iDCPred1 = iDCPred2 = mcuCX = mcuCY = 0;

    switch (pJPEG->ucSubSample) {
        // set up the parameters for the different subsampling options
        case 0x00:                            // fake value to handle grayscale
        case 0x01:                            // fake value to handle sRGB/CMYK
        case 0x11:
            cx = (pJPEG->iWidth + 7) >> 3;    // number of MCU blocks
            cy = (pJPEG->iHeight + 7) >> 3;
            iCr = MCU1;
            iCb = MCU2;
            mcuCX = mcuCY = 8;
            break;
        case 0x12:
            cx = (pJPEG->iWidth + 7) >> 3;    // number of MCU blocks
            cy = (pJPEG->iHeight + 15) >> 4;
            iCr = MCU2;
            iCb = MCU3;
            mcuCX = 8;
            mcuCY = 16;
            break;
        case 0x21:
            cx = (pJPEG->iWidth + 15) >> 4;    // number of MCU blocks
            cy = (pJPEG->iHeight + 7) >> 3;
            iCr = MCU2;
            iCb = MCU3;
            mcuCX = 16;
            mcuCY = 8;
            break;
        case 0x22:
            cx = (pJPEG->iWidth + 15) >> 4;    // number of MCU blocks
            cy = (pJPEG->iHeight + 15) >> 4;
            iCr = MCU4;
            iCb = MCU5;
            mcuCX = mcuCY = 16;
            break;
        default: // to suppress compiler warning
            cx = cy = 0;
            iCr = iCb = 0;
            break;
    }
    // Scale down the MCUs by the requested amount
    mcuCX >>= iScaleShift;
    mcuCY >>= iScaleShift;

    iQuant1 = pJPEG->sQuantTable[pJPEG->JPCI[0].quant_tbl_no * DCTSIZE];   // DC quant values
    iQuant2 = pJPEG->sQuantTable[pJPEG->JPCI[1].quant_tbl_no * DCTSIZE];
    iQuant3 = pJPEG->sQuantTable[pJPEG->JPCI[2].quant_tbl_no * DCTSIZE];
    // luminance values are always in these positions
    iLum0 = MCU0;
    iLum1 = MCU1;
    iLum2 = MCU2;
    iLum3 = MCU3;
    iErr = 0;
    pJPEG->iResCount = pJPEG->iResInterval;
    // Calculate how many MCUs we can fit in the pixel buffer to maximize LCD drawing speed
    iMCUCount = MAX_BUFFERED_PIXELS / (mcuCX * mcuCY);
    if (pJPEG->ucPixelType == EIGHT_BIT_GRAYSCALE) {
        iMCUCount *= 2; // each pixel is only 1 byte
    }
    if (iMCUCount > cx) {
        iMCUCount = cx; // don't go wider than the image
    }
    if (iMCUCount > pJPEG->iMaxMCUs) {
        // did the user set an upper bound on how many pixels per JPEGDraw callback?
        iMCUCount = pJPEG->iMaxMCUs;
    }
    if (pJPEG->ucPixelType > EIGHT_BIT_GRAYSCALE) {
        // dithered, override the max MCU count
        iMCUCount = cx; // do the whole row
    }
    for (y = 0; y < cy && bContinue; y++) {
        for (x = 0; x < cx && bContinue && iErr == 0; x++) {
            pJPEG->ucACTable = cACTable0;
            pJPEG->ucDCTable = cDCTable0;
            // do the first luminance component
            iErr = JPEGDecodeMCU(pJPEG, iLum0, &iDCPred0);
            if (pJPEG->ucMaxACCol == 0 || bThumbnail) {
                // no AC components, save some time
                pl = (uint32_t *) &pJPEG->sMCUs[iLum0];
                c = ucRangeTable[((iDCPred0 * iQuant1) >> 5) & 0x3ff];
                l = c | ((uint32_t) c << 8) | ((uint32_t) c << 16) | ((uint32_t) c << 24);
                // dct stores byte values
                for (i = 0; i < iMaxFill; i++) {
                    // 8x8 bytes = 16 longs
                    pl[i] = l;
                }
            } else {
                // first quantization table
                JPEGIDCT(pJPEG, iLum0, pJPEG->JPCI[0].quant_tbl_no, (pJPEG->ucMaxACCol | (pJPEG->ucMaxACRow << 8)));
            }
            // do the second luminance component
            if (pJPEG->ucSubSample > 0x11) {
                // subsampling
                iErr |= JPEGDecodeMCU(pJPEG, iLum1, &iDCPred0);
                if (pJPEG->ucMaxACCol == 0 || bThumbnail) {
                    // no AC components, save some time
                    c = ucRangeTable[((iDCPred0 * iQuant1) >> 5) & 0x3ff];
                    l = c | ((uint32_t) c << 8) | ((uint32_t) c << 16) | ((uint32_t) c << 24);
                    // dct stores byte values
                    pl = (uint32_t *) &pJPEG->sMCUs[iLum1];
                    for (i = 0; i < iMaxFill; i++) {
                        // 8x8 bytes = 16 longs
                        pl[i] = l;
                    }
                } else {
                    // first quantization table
                    JPEGIDCT(pJPEG, iLum1, pJPEG->JPCI[0].quant_tbl_no, (pJPEG->ucMaxACCol | (pJPEG->ucMaxACRow << 8)));
                }
                if (pJPEG->ucSubSample == 0x22) {
                    iErr |= JPEGDecodeMCU(pJPEG, iLum2, &iDCPred0);
                    if (pJPEG->ucMaxACCol == 0 || bThumbnail) {
                        // no AC components, save some time
                        c = ucRangeTable[((iDCPred0 * iQuant1) >> 5) & 0x3ff];
                        l = c | ((uint32_t) c << 8) | ((uint32_t) c << 16) | ((uint32_t) c << 24);
                        // dct stores byte values
                        pl = (uint32_t *) &pJPEG->sMCUs[iLum2];
                        for (i = 0; i < iMaxFill; i++) {
                            // 8x8 bytes = 16 longs
                            pl[i] = l;
                        }
                    } else {
                        // first quantization table
                        JPEGIDCT(pJPEG, iLum2, pJPEG->JPCI[0].quant_tbl_no, (pJPEG->ucMaxACCol | (pJPEG->ucMaxACRow << 8)));
                    }
                    iErr |= JPEGDecodeMCU(pJPEG, iLum3, &iDCPred0);
                    if (pJPEG->ucMaxACCol == 0 || bThumbnail) {
                        // no AC components, save some time
                        c = ucRangeTable[((iDCPred0 * iQuant1) >> 5) & 0x3ff];
                        l = c | ((uint32_t) c << 8) | ((uint32_t) c << 16) | ((uint32_t) c << 24);
                        // dct stores byte values
                        pl = (uint32_t *) &pJPEG->sMCUs[iLum3];
                        for (i = 0; i < iMaxFill; i++) {
                            // 8x8 bytes = 16 longs
                            pl[i] = l;
                        }
                    } else {
                        // first quantization table
                        JPEGIDCT(pJPEG, iLum3, pJPEG->JPCI[0].quant_tbl_no, (pJPEG->ucMaxACCol | (pJPEG->ucMaxACRow << 8)));
                    }
                } // if 2:2 subsampling
            } // if subsampling used
            if (pJPEG->ucSubSample && pJPEG->ucNumComponents == 3) {
                // if color (not CMYK)
                // first chroma
                pJPEG->ucACTable = cACTable1;
                pJPEG->ucDCTable = cDCTable1;
                iErr |= JPEGDecodeMCU(pJPEG, iCr, &iDCPred1);
                if (pJPEG->ucMaxACCol == 0 || bThumbnail) {
                    // no AC components, save some time
                    c = ucRangeTable[((iDCPred1 * iQuant2) >> 5) & 0x3ff];
                    l = c | ((uint32_t) c << 8) | ((uint32_t) c << 16) | ((uint32_t) c << 24);
                    // dct stores byte values
                    pl = (uint32_t *) &pJPEG->sMCUs[iCr];
                    for (i = 0; i < iMaxFill; i++) {
                        // 8x8 bytes = 16 longs
                        pl[i] = l;
                    }
                } else {
                    // second quantization table
                    JPEGIDCT(pJPEG, iCr, pJPEG->JPCI[1].quant_tbl_no, (pJPEG->ucMaxACCol | (pJPEG->ucMaxACRow << 8)));
                }
                // second chroma
                pJPEG->ucACTable = cACTable2;
                pJPEG->ucDCTable = cDCTable2;
                iErr |= JPEGDecodeMCU(pJPEG, iCb, &iDCPred2);
                if (pJPEG->ucMaxACCol == 0 || bThumbnail) {
                    // no AC components, save some time
                    c = ucRangeTable[((iDCPred2 * iQuant3) >> 5) & 0x3ff];
                    l = c | ((uint32_t) c << 8) | ((uint32_t) c << 16) | ((uint32_t) c << 24);
                    // dct stores byte values
                    pl = (uint32_t *) &pJPEG->sMCUs[iCb];
                    for (i = 0; i < iMaxFill; i++) {
                        // 8x8 bytes = 16 longs
                        pl[i] = l;
                    }
                } else {
                    JPEGIDCT(pJPEG, iCb, pJPEG->JPCI[2].quant_tbl_no, (pJPEG->ucMaxACCol | (pJPEG->ucMaxACRow << 8)));
                }
            } // if color components present
            if (pJPEG->ucPixelType == EIGHT_BIT_GRAYSCALE) {
                JPEGPutMCU8BitGray(pJPEG, x * mcuCX, y * mcuCY);
            } else if (pJPEG->ucPixelType == ONE_BIT_GRAYSCALE) {
                JPEGPutMCU1BitGray(pJPEG, x * mcuCX, y * mcuCY);
            } else {
                switch (pJPEG->ucSubSample) {
                    case 0x00: // grayscale
                        JPEGPutMCUGray(pJPEG, x * mcuCX, y * mcuCY);
                        break; // not used
                    case 0x11:
                        JPEGPutMCU11(pJPEG, x * mcuCX, y * mcuCY);
                        break;
                    case 0x12:
                        JPEGPutMCU12(pJPEG, x * mcuCX, y * mcuCY);
                        break;
                    case 0x21:
                        JPEGPutMCU21(pJPEG, x * mcuCX, y * mcuCY);
                        break;
                    case 0x22:
                        JPEGPutMCU22(pJPEG, x * mcuCX, y * mcuCY);
                        break;
                } // switch on color option
            }
            if (pJPEG->iResInterval) {
                if (--pJPEG->iResCount == 0) {
                    pJPEG->iResCount = pJPEG->iResInterval;
                    iDCPred0 = iDCPred1 = iDCPred2 = 0;                       // reset DC predictors
                    if (pJPEG->bb.ulBitOff & 7) {
                        // need to start at the next even byte
                        // new restart interval starts on byte boundary
                        pJPEG->bb.ulBitOff += (8 - (pJPEG->bb.ulBitOff & 7));
                    }
                } // if restart interval needs to reset
            } // if there is a restart interval
              // See if we need to feed it more data
            if (pJPEG->iVLCOff >= FILE_HIGHWATER) {
                JPEGGetMoreData(pJPEG); // need more 'filtered' VLC data
            }
        } // for x
    } // for y
    if (iErr != 0) {
        pJPEG->iError = JPEG_DECODE_ERROR;
    }
    return (iErr == 0);
}

void jpeg_decompress(image_t *dst, image_t *src) {
    JPEGIMAGE jpg;

    #if (TIME_JPEG == 1)
    mp_uint_t start = mp_hal_ticks_ms();
    #endif

    // Supports decoding baseline JPEGs only.
    if (jpeg_is_progressive(src)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Progressive JPEG is not supported."));
    }

    if (JPEG_openRAM(&jpg, src->data, src->size, dst->data) == 0) {
        // failed to parse the header
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("JPEG decoder failed."));
    }

    switch (dst->pixfmt) {
        case PIXFORMAT_BINARY:
            // Force 1-bit (binary) output in the draw function.
            jpg.ucPixelType = ONE_BIT_GRAYSCALE;
            break;
        case PIXFORMAT_GRAYSCALE:
            // Force 8-bit grayscale output.
            jpg.ucPixelType = EIGHT_BIT_GRAYSCALE;
            break;
        case PIXFORMAT_RGB565:
            // Force output to be RGB565
            jpg.ucPixelType = RGB565_LITTLE_ENDIAN;
            break;
        default:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Unsupported format."));
    }

    // Set up dest image params
    jpg.pUser = (void *) dst;

    // Fill buffer with 0's so we only need to write "set" bits
    memset(dst->data, 0, image_size(dst));

    // Start decoding.
    if (JPEG_decode(&jpg, 0, 0, 0) == 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("JPEG decoder failed."));
    }

    #if (TIME_JPEG == 1)
    printf("time: %u ms\n", mp_hal_ticks_ms() - start);
    #endif
}
#endif
