//
// PNG Encoder
//
// written by Larry Bank
// bitbank@pobox.com
// Arduino port started 6/27/2021
// Original PNG code written 20+ years ago :)
// The goal of this code is to decode PNG images on embedded systems
//
// Copyright 2021 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
//
#ifndef __PNGENC__
#define __PNGENC__
#if 1 //defined( __MACH__ ) || defined( __LINUX__ ) || defined( __MCUXPRESSO )
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define memcpy_P memcpy
#define PROGMEM
#else
#include <Arduino.h>
#endif
#include "zutil.h"
#include "deflate.h"
//
// PNG Encoder
// Written by Larry Bank
// Copyright (c) 2021 BitBank Software, Inc.
// 
// Designed to encode PNG files from source images (1-32 bpp)
// using less than 100K of RAM
//
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif
// Number of bits to reduce the zlib window size
// 0 = 256K + 6K needed
// 1 = 128K + 6K needed
// 2 = 64K ...
#define MEM_SHRINK 3

/* Defines and variables */
#define PNG_FILE_BUF_SIZE 2048
#define PNG_FILE_HIGHWATER ((PNG_FILE_BUF_SIZE * 3)/4)
// Number of bytes to reserve for current and previous lines
// Defaults to 640 32-bit pixels max width
#define PNG_MAX_BUFFERED_PIXELS (640*4 + 1)
// PNG filter type
#ifndef __PNG_FILTER_TYPES__
#define __PNG_FILTER_TYPES__
enum {
    PNG_FILTER_NONE=0,
    PNG_FILTER_SUB,
    PNG_FILTER_UP,
    PNG_FILTER_AVG,
    PNG_FILTER_PAETH,
    PNG_FILTER_COUNT
};
#endif
// decode options
enum {
    PNG_CHECK_CRC = 1,
    PNG_FAST_PALETTE = 2
};

#ifndef __PNG_PIXEL_TYPES__
#define __PNG_PIXEL_TYPES__
// source pixel type
enum {
  PNG_PIXEL_GRAYSCALE=0,
    PNG_PIXEL_TRUECOLOR=2,
    PNG_PIXEL_INDEXED=3,
    PNG_PIXEL_GRAY_ALPHA=4,
    PNG_PIXEL_TRUECOLOR_ALPHA=6
};
#endif
// Error codes returned by getLastError()
#ifndef __PNG_ERROR_TYPES__
#define __PNG_ERROR_TYPES__
enum {
    PNG_SUCCESS = 0,
    PNG_INVALID_PARAMETER,
    PNG_ENCODE_ERROR,
    PNG_DECODE_ERROR,
    PNG_MEM_ERROR,
    PNG_NO_BUFFER,
    PNG_UNSUPPORTED_FEATURE,
    PNG_INVALID_FILE,
    PNG_TOO_BIG
};
#endif

#ifndef __PNG_FILE_STRUCT__
#define __PNG_FILE_STRUCT__
typedef struct png_file_tag
{
  int32_t iPos; // current file position
  int32_t iSize; // file size
  uint8_t *pData; // memory file pointer
  void * fHandle; // class pointer to File/SdFat or whatever you want
} PNGFILE;

// Callback function prototypes
typedef int32_t (PNG_READ_CALLBACK)(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen);
typedef int32_t (PNG_SEEK_CALLBACK)(PNGFILE *pFile, int32_t iPosition);
typedef void * (PNG_OPEN_CALLBACK)(const char *szFilename);
typedef void (PNG_CLOSE_CALLBACK)(PNGFILE *pFile);
#endif // __PNG_FILE_STRUCT__
typedef int32_t (PNG_WRITE_CALLBACK)(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen);

//
// our private structure to hold a JPEG image decode state
//
typedef struct png_enc_image_tag
{
    int iWidth, iHeight, y, iTransparent; // image size
    uint8_t ucBpp, ucPixelType, ucCompLevel, ucHasAlphaPalette;
    uint8_t ucMemType;
    uint8_t *pOutput;
    int iBufferSize; // output buffer size provided by caller
    int iHeaderSize; // size of the PNG header
    int iCompressedSize; // size of flate output
    int iDataSize; // total output file size
    int iMemPool; // memory allocated out of memory pool
    int iPitch; // bytes per line
    int iError;
    PNG_READ_CALLBACK *pfnRead;
    PNG_WRITE_CALLBACK *pfnWrite;
    PNG_SEEK_CALLBACK *pfnSeek;
    PNG_OPEN_CALLBACK *pfnOpen;
    PNG_CLOSE_CALLBACK *pfnClose;
    PNGFILE PNGFile;
    z_stream c_stream; /* compression stream */
    uint8_t ucPalette[1024];
    uint8_t ucMemPool[sizeof(deflate_state) + (0x40000 >> MEM_SHRINK)]; // RAM needed for deflate
    uint8_t ucPrevLine[PNG_MAX_BUFFERED_PIXELS];
    uint8_t ucCurrLine[PNG_MAX_BUFFERED_PIXELS];
    uint8_t ucFileBuf[PNG_FILE_BUF_SIZE]; // holds temp file data
} PNGIMAGE;

#ifdef __cplusplus
#define PNG_STATIC static
//
// The PNG class wraps portable C code which does the actual work
//
class PNG
{
  public:
    int open(const char *szFilename, PNG_OPEN_CALLBACK *pfnOpen, PNG_CLOSE_CALLBACK *pfnClose, PNG_READ_CALLBACK *pfnRead, PNG_WRITE_CALLBACK *pfnWrite, PNG_SEEK_CALLBACK *pfnSeek);
    int open(uint8_t *pOutput, int iBufferSize);
    int close();
    int encodeBegin(int iWidth, int iHeight, uint8_t iPixelType, uint8_t iBpp, uint8_t *pPalette, uint8_t iCompLevel);
    int addLine(uint8_t *pPixels);
    int setTransparentColor(uint32_t u32Color);
    int setAlphaPalette(uint8_t *pPalette);
    int getLastError();

  private:
    PNGIMAGE _png;
};
#else
#define PNG_STATIC
int PNG_openRAM(PNGIMAGE *pPNG, uint8_t *pData, int iDataSize);
int PNG_openFile(PNGIMAGE *pPNG, const char *szFilename);
int PNG_encodeBegin(PNGIMAGE *pPNG, int iWidth, int iHeight, uint8_t ucPixelType, uint8_t iBpp, uint8_t *pPalette, uint8_t ucCompLevel);
void PNG_encodeEnd(PNGIMAGE *pPNG);
int addLine(PNGIMAGE *,uint8_t *pPixels);
int setTransparentColor(PNGIMAGE *pPNG, uint32_t u32Color);
int setAlphaPalette(PNGIMAGE *pPNG, uint8_t *pPalette);
int PNG_getLastError(PNGIMAGE *pPNG);
#endif // __cplusplus

// Due to unaligned memory causing an exception, we have to do these macros the slow way
#define INTELSHORT(p) ((*p) + (*(p+1)<<8))
#define INTELLONG(p) ((*p) + (*(p+1)<<8) + (*(p+2)<<16) + (*(p+3)<<24))
#define MOTOSHORT(p) (((*(p))<<8) + (*(p+1)))
#define MOTOLONG(p) (((*p)<<24) + ((*(p+1))<<16) + ((*(p+2))<<8) + (*(p+3)))

// Must be a 32-bit target processor
#define REGISTER_WIDTH 32

#endif // __PNGENC__
