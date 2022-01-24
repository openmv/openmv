//
// Embedded-friendly PNG Encoder
//
// Copyright (c) 2000-2021 BitBank Software, Inc.
// Written by Larry Bank
// Project started 12/9/2000
//
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"

// Macro to simplify writing a big-endian 32-bit value on any CPU
#define WRITEMOTO32(p, o, val) {uint32_t l = val; p[o] = (unsigned char)(l >> 24); p[o+1] = (unsigned char)(l >> 16); p[o+2] = (unsigned char)(l >> 8); p[o+3] = (unsigned char)l;}

unsigned char PNGFindFilter(uint8_t *pCurr, uint8_t *pPrev, int iPitch, int iStride);
void PNGFilter(uint8_t ucFilter, uint8_t *pOut, uint8_t *pCurr, uint8_t *pPrev, int iStride, int iPitch);
//
// Calculate the PNG-style CRC value for a block of data
//
uint32_t PNGCalcCRC(unsigned char *buf, int len, uint32_t u32_start)
{
/* Table of CRCs of all 8-bit messages. */
   static uint32_t crc_table[256];
   static int crc_table_computed = 0;
    uint32_t crc = u32_start; //0xffffffff;
   int n;

   /* Make the table for a fast CRC. */
   if (crc_table_computed == 0)
   {
     uint32_t c;
     int n, k;
     
     for (n = 0; n < 256; n++) {
       c = (uint32_t) n;
       for (k = 0; k < 8; k++) {
         if (c & 1)
           c = 0xedb88320L ^ (c >> 1);
         else
           c = c >> 1;
       }
       crc_table[n] = c;
     }
     crc_table_computed = 1;
   }

   /* Update a running CRC with the bytes buf[0..len-1]--the CRC
      should be initialized to all 1's, and the transmitted value
      is the 1's complement of the final running CRC (see the
      crc() routine below)). */

     for (n = 0; n < len; n++)
     {
         crc = crc_table[(crc ^ buf[n]) & 0xff] ^ (crc >> 8);
     }

  return crc ^ 0xffffffffL;

} /* PNGCalcCRC() */

static unsigned char PAETH(unsigned char a, unsigned char b, unsigned char c)
{
    int pa, pb, pc;
#ifdef SLOW_WAY
    int p;
#endif // SLOW_WAY
    
#ifndef SLOW_WAY
    pc = c;
    pa = b - pc;
    pb = a - pc;
    pc = pa + pb;
    if (pa < 0) pa = -pa;
    if (pb < 0) pb = -pb;
    if (pc < 0) pc = -pc;
#else
    p = a + b - c; // initial estimate
    pa = abs(p - a); // distances to a, b, c
    pb = abs(p - b);
    pc = abs(p - c);
#endif
    // return nearest of a,b,c,
    // breaking ties in order a,b,c.
    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else return c;
    
} /* PAETH() */
//
// Write the PNG file header and, if needed, a color palette chunk
//
static int PNGStartFile(PNGIMAGE *pImage)
{
    int iError = PNG_SUCCESS;
    unsigned char *p;
    int iSize, i, iLen;
    uint32_t ulCRC;
        
    p = pImage->ucFileBuf;
    iSize = 0; // output data size
    WRITEMOTO32(p, iSize, 0x89504e47); // PNG File header
    iSize += 4;
    WRITEMOTO32(p, iSize, 0x0d0a1a0a);
    iSize += 4;
    // IHDR contains 13 data bytes
    WRITEMOTO32(p, iSize, 0x0000000d); // IHDR length
    iSize += 4;
    WRITEMOTO32(p, iSize, 0x49484452); // IHDR marker
    iSize += 4;
    WRITEMOTO32(p, iSize, pImage->iWidth); // Image Width
    iSize += 4;
    WRITEMOTO32(p, iSize, pImage->iHeight); // Image Height
    iSize += 4;
    p[iSize++] = (pImage->ucBpp > 8) ? 8:pImage->ucBpp; // Bit depth
    p[iSize++] = pImage->ucPixelType;
    p[iSize++] = 0; // compression method 0
    p[iSize++] = 0; // filter type 0
    p[iSize++] = 0; // interlace = no
    ulCRC = PNGCalcCRC(&p[iSize-17], 17, 0xffffffff); // store CRC for IHDR chunk
    WRITEMOTO32(p, iSize, ulCRC);
    iSize += 4;

    if (pImage->ucPixelType == PNG_PIXEL_INDEXED)
	   {
           // Write the palette
           iLen = (1 << pImage->ucBpp); // palette length
           WRITEMOTO32(p, iSize, iLen*3); // 3 bytes per entry
           iSize += 4;
           WRITEMOTO32(p, iSize, 0x504c5445/*'PLTE'*/);
           iSize += 4;
           for (i=0; i<iLen; i++)
           {
               p[iSize++] = pImage->ucPalette[i*3+2]; // red
               p[iSize++] = pImage->ucPalette[i*3+1]; // green
               p[iSize++] = pImage->ucPalette[i*3+0]; // blue
           }
           ulCRC = PNGCalcCRC(&p[iSize-(iLen*3)-4], 4+(iLen*3), 0xffffffff); // store CRC for PLTE chunk
           WRITEMOTO32(p, iSize, ulCRC);
           iSize += 4;
           if (pImage->iTransparent >= 0 || pImage->ucHasAlphaPalette) // add transparency chunk
           {
               if (pImage->ucPixelType == PNG_PIXEL_INDEXED) { // a set of palette alpha values
                    iLen = (1 << pImage->ucBpp); // palette length
               } else if (pImage->ucPixelType == PNG_PIXEL_GRAYSCALE) {
                   iLen = 2;
               } else {
                   iLen = 6; // truecolor single transparent color
               }
               WRITEMOTO32(p, iSize, iLen); // 1 byte per palette alpha entry
               iSize += 4;
               WRITEMOTO32(p, iSize, 0x74524e53 /*'tRNS'*/);
               iSize += 4;
               switch (iLen) {
                   case 2: // grayscale
                       p[iSize++] = 0; // 16-bit value (big endian)
                       p[iSize++] = (uint8_t)pImage->iTransparent;
                       break;
                   case 6: // truecolor
                       p[iSize++] = 0; // 16-bit value (big endian for color stimulus)
                       p[iSize++] = (uint8_t)pImage->iTransparent & 0xff;
                       p[iSize++] = 0;
                       p[iSize++] = (uint8_t)((pImage->iTransparent >> 8) & 0xff);
                       p[iSize++] = 0;
                       p[iSize++] = (uint8_t)((pImage->iTransparent >> 16) & 0xff);
                       p[iSize++] = 0;
                       break;
                   default: // palette colors
                       for (i = 0; i<iLen; i++) // write n alpha values to accompany the palette
                       {
                           p[iSize++] = pImage->ucPalette[768+i];
                       }
                       break;
               } // switch
               ulCRC = PNGCalcCRC(&p[iSize - iLen - 4], 4 + iLen, 0xffffffff); // store CRC for tRNS chunk
               WRITEMOTO32(p, iSize, ulCRC);
               iSize += 4;
           }
       }
    // IDAT
    WRITEMOTO32(p, iSize, 0/*iCompressedSize*/); // IDAT length
    iSize += 4;
    WRITEMOTO32(p, iSize, 0x49444154); // IDAT marker
    iSize += 4;
    pImage->iCompressedSize = 0;
    pImage->iHeaderSize = iSize; // keep the PNG header size for later
    if (pImage->pOutput) { // copy to ram?
        memcpy(pImage->pOutput, pImage->ucFileBuf, iSize);
    } else { // write it to the file
        (*pImage->pfnWrite)(&pImage->PNGFile, pImage->ucFileBuf, iSize);
    }
    return iError;
    
} /* PNGStartFile() */
//
// Finish PNG file data (updates IDAT chunk size+crc & writes END chunk)
//
int PNGEndFile(PNGIMAGE *pImage)
{
    int iSize=0;
    uint8_t *p;
    uint32_t ulCRC;
    
    if (pImage->pOutput) { // output buffer = easy to wrap up
        p = pImage->pOutput;
        iSize = pImage->iHeaderSize;
        WRITEMOTO32(p, iSize-8, pImage->iCompressedSize); // write IDAT chunk size
        iSize += pImage->iCompressedSize;
        ulCRC = PNGCalcCRC(&p[iSize-pImage->iCompressedSize-4], pImage->iCompressedSize+4, 0xffffffff); // store CRC for IDAT chunk
        WRITEMOTO32(p, iSize, ulCRC);
        iSize += 4;
        // Write the IEND chunk
        WRITEMOTO32(p, iSize, 0);
        iSize += 4;
        WRITEMOTO32(p, iSize, 0x49454e44/*'IEND'*/);
        iSize += 4;
        WRITEMOTO32(p, iSize, 0xae426082); // same CRC every time
        iSize += 4;
    } else { // file mode = not so easy
        uint32_t pu32[4];
        uint8_t *p;
        int i, iReadSize;
        
        p = (uint8_t *)&pu32[0];
        iSize = pImage->iHeaderSize;
        ulCRC = 0xffffffff;
        (*pImage->pfnSeek)(&pImage->PNGFile, iSize-8); // seek to compressed size
        WRITEMOTO32(p, 0, pImage->iCompressedSize); // save the actual IDAT size
        (*pImage->pfnWrite)(&pImage->PNGFile, (uint8_t *)pu32, 4);
        // From this point forward, we need to calculate the CRC of the IDAT chunk
        // and unfortunately that means reading back all of the compressed data
        i = pImage->iCompressedSize+4; // IDAT marker + data length
        while (i) {
            iReadSize = i;
            if (iReadSize > PNG_FILE_BUF_SIZE)
                iReadSize = PNG_FILE_BUF_SIZE;
            (*pImage->pfnRead)(&pImage->PNGFile, pImage->ucFileBuf, iReadSize);
            ulCRC = PNGCalcCRC(pImage->ucFileBuf, iReadSize, ulCRC);
            i -= iReadSize;
        }
        WRITEMOTO32(p, 0, ulCRC); // now write the CRC
        iSize += pImage->iCompressedSize + 4;
        // Write the IEND chunk
        WRITEMOTO32(p, 4, 0);
        iSize += 4;
        WRITEMOTO32(p, 8, 0x49454e44/*'IEND'*/);
        iSize += 4;
        WRITEMOTO32(p, 12, 0xae426082); // same CRC every time
        iSize += 4;
        // write the final data to the file
        (*pImage->pfnWrite)(&pImage->PNGFile, (uint8_t *)p, 16);
    }
    return iSize;
} /* PNGEndFile() */
//
// My internal alloc/free functions to work on simple embedded systems
//
voidpf ZLIB_INTERNAL myalloc (voidpf opaque, unsigned int items, unsigned int size)
{
    PNGIMAGE *pImage = (PNGIMAGE *)opaque;
    // allocate from our internal pool
    int iSize = items * size;
    void *p = &pImage->ucMemPool[pImage->iMemPool];
    pImage->iMemPool += iSize;
    return p;
} /* myalloc() */

void ZLIB_INTERNAL myfree (voidpf opaque, voidpf ptr)
{
    (void)opaque;
    (void)ptr; // doesn't do anything since the memory is from an internal pool
} /* myfree() */
//
// Compress one line of image at a time and write the compressed data
// incrementally to the output file. This allows the system to not need an
// input nor output buffer larger than 2 lines of image data
//
int PNGAddLine(PNGIMAGE *pImage, uint8_t *pSrc, int y)
{
    unsigned char ucFilter; // filter type
    unsigned char *pOut;
    int iStride;
    int err;
    int iPitch;
    
    iStride = pImage->ucBpp >> 3; // bytes per pixel
    iPitch = (pImage->iWidth * pImage->ucBpp) >> 3;
    if (iStride < 1)
        iStride = 1; // 1,4 bpp
    pOut = pImage->ucCurrLine;
    ucFilter = PNGFindFilter(pSrc, (y == 0) ? NULL : pImage->ucPrevLine, iPitch, iStride); // find best filter
    *pOut++ = ucFilter; // store filter byte
    PNGFilter(ucFilter, pOut, pSrc, pImage->ucPrevLine, iStride, iPitch); // filter the current line of image data and store
    memcpy(pImage->ucPrevLine, pSrc, iPitch);
    // Compress the filtered image data
    if (y == 0) // first block, initialize zlib
    {
        PNGStartFile(pImage);
        memset(&pImage->c_stream, 0, sizeof(z_stream));
        pImage->c_stream.zalloc = myalloc; // use internal alloc/free
        pImage->c_stream.zfree = myfree; // to use our memory pool
        pImage->c_stream.opaque = (voidpf)pImage;
        pImage->iMemPool = 0;
        // ZLIB compression levels: 1 = fastest, 9 = most compressed (slowest)
//        err = deflateInit(&pImage->c_stream, pImage->ucCompLevel); // might as well use max compression
        err = deflateInit2_(&pImage->c_stream, pImage->ucCompLevel, Z_DEFLATED, MAX_WBITS-MEM_SHRINK, DEF_MEM_LEVEL-MEM_SHRINK, Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream)); // might as well use max compression
        pImage->c_stream.total_out = 0;
        pImage->c_stream.next_out = pImage->ucFileBuf;
        pImage->c_stream.avail_out = PNG_FILE_BUF_SIZE;
    }
    pImage->c_stream.next_in  = (Bytef*)pImage->ucCurrLine;
    pImage->c_stream.total_in = 0;
    pImage->c_stream.avail_in = iPitch+1; // compress entire buffer in 1 shot
    err = deflate(&pImage->c_stream, Z_SYNC_FLUSH);
    if (err != Z_OK) { // something went wrong with the data compression, stop
        pImage->iError = PNG_ENCODE_ERROR;
        return PNG_ENCODE_ERROR;
    }
    if (y == pImage->iHeight - 1) // last line, clean up
    {
        err = deflate(&pImage->c_stream, Z_FINISH);
        err = deflateEnd(&pImage->c_stream);
    }
    // Write the data to memory or a file
    //
    // A bunch of extra logic has been added below to minimize the total number
    // of calls to 'write'. Each compressed scanline might generate only a few
    // bytes of flate output and calling write() for a few bytes at a time can
    // slow things to a crawl.
    if (pImage->c_stream.total_out >= PNG_FILE_HIGHWATER) {
        if (pImage->pOutput) { // memory
            if ((pImage->iHeaderSize + pImage->iCompressedSize + pImage->c_stream.total_out) > pImage->iBufferSize) {
                // output buffer not large enough
                pImage->iError = PNG_MEM_ERROR;
                return PNG_MEM_ERROR;
            }
            memcpy(&pImage->pOutput[pImage->iHeaderSize + pImage->iCompressedSize], pImage->ucFileBuf, pImage->c_stream.total_out);
        } else { // file
            (*pImage->pfnWrite)(&pImage->PNGFile, pImage->ucFileBuf, (int)pImage->c_stream.total_out);
        }
        pImage->iCompressedSize += (int)pImage->c_stream.total_out;
        // reset zlib output buffer to start
        pImage->c_stream.total_out = 0;
        pImage->c_stream.next_out = pImage->ucFileBuf;
        pImage->c_stream.avail_out = PNG_FILE_BUF_SIZE;
    } // highwater hit
    if (y == pImage->iHeight -1) { // last line, finish file
        // if any remaining data in output buffer, write it
        if (pImage->c_stream.total_out > 0) {
            if (pImage->pOutput) { // memory
                if ((pImage->iHeaderSize + pImage->iCompressedSize + pImage->c_stream.total_out) > pImage->iBufferSize) {
                    // output buffer not large enough
                    pImage->iError = PNG_MEM_ERROR;
                    return PNG_MEM_ERROR;
                }
                memcpy(&pImage->pOutput[pImage->iHeaderSize + pImage->iCompressedSize], pImage->ucFileBuf, pImage->c_stream.total_out);
            } else { // file
                (*pImage->pfnWrite)(&pImage->PNGFile, pImage->ucFileBuf, (int)pImage->c_stream.total_out);
            }
            pImage->iCompressedSize += (int)pImage->c_stream.total_out;
        }
        pImage->iDataSize = PNGEndFile(pImage);
    }    
    return PNG_SUCCESS; // DEBUG
    
} /* PNGAddLine() */
//
// Find the best filter method for the given scanline
// Try each filter algorithm in turn and use SAD (sum of absolute differences)
// to choose the one with the lowest sum (a reasonable proxy for entropy)
//
unsigned char PNGFindFilter(uint8_t *pCurr, uint8_t *pPrev, int iPitch, int iStride)
{
int i;
unsigned char a, b, c, ucDiff, ucFilter;
uint32_t ulSum[5]  = {0,0,0,0,0}; // individual sums for the 4 types of filters
uint32_t ulMin;

    ucFilter = 0;
    for (i=0; i<iPitch; i++)
    {
       ucDiff = pCurr[i]; // no filter
       ulSum[0] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       // Sub
       if (i >= iStride)
       {
          ucDiff = pCurr[i]-pCurr[i-iStride];
          ulSum[1] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       else
       {
           ucDiff = pCurr[i];
           ulSum[1] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       // Up
       if (pPrev)
       {
          ucDiff = pCurr[i]-pPrev[i];
          ulSum[2] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       else // not available
       {
           ucDiff = pCurr[i];
           ulSum[2] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       // Average
       if (!pPrev || i < iStride)
       {
          if (!pPrev)
          {
             if (i < iStride)
                a = 0;
             else
                a = pCurr[i-iStride]>>1;
          }
          else
             a = pPrev[i]>>1;
       }
       else
       {
          a = (pCurr[i-iStride] + pPrev[i])>>1;
       }
       ucDiff = pCurr[i] - a;
       ulSum[3] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       // Paeth
       if (i < iStride)
          a = 0;
       else
          a = pCurr[i-iStride]; // left
       if (pPrev == NULL)
          b = 0; // above doesn't exist
       else
          b = pPrev[i];
       if (!pPrev || i < iStride)
          c = 0;
       else
          c = pPrev[i-iStride]; // above left
       ucDiff = pCurr[i] - PAETH(a,b,c);
       ulSum[4] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       } // for i
       // Pick the best filter (or NONE if they're all bad)
       ulMin = iPitch * 255; // max value
       for (a=0; a<5; a++)
       {
          if (ulSum[a] < ulMin)
          {
             ulMin = ulSum[a];
             ucFilter = a; // current winner
          }
       } // for
       return ucFilter;

} /* PNGFindFilter() */
//
// Apply the given filter algorithm to a line of image data
//
void PNGFilter(uint8_t ucFilter, uint8_t *pOut, uint8_t *pCurr, uint8_t *pPrev, int iStride, int iPitch)
{
int j;

   switch (ucFilter)
      {
      case 0: // no filter, just copy
         memcpy(pOut, pCurr, iPitch);
         break;
      case 1: // sub
         j = 0;
         while (j < iStride)
         {
             pOut[j] = pCurr[j];
             j++;
         }
         while (j < (int)iPitch)
         {
            pOut[j] = pCurr[j]-pCurr[j-iStride];
            j++;
         }
         break;
      case 2: // up
         if (pPrev)
         {
            for (j=0;j<iPitch;j++)
            {
               pOut[j] = pCurr[j]-pPrev[j];
            }
         }
         else
            memcpy(pOut, pCurr, iPitch);
         break;
      case 3: // average
         for (j=0; j<iPitch; j++)
        {
            int a;
            if (!pPrev || j < iStride)
            {
               if (!pPrev)
               {
                  if (j < iStride)
                     a = 0;
                  else
                     a = pCurr[j-iStride]>>1;
               }
               else
                  a = pPrev[j]>>1;
            }
            else
            {
               a = (pCurr[j-iStride] + pPrev[j])>>1;
            }
            pOut[j] = (uint8_t)(pCurr[j] - a);
         }
         break;
      case 4: // paeth
         for (j=0; j<iPitch; j++)
         {
            uint8_t a,b,c;
            if (j < iStride)
               a = 0;
            else
               a = pCurr[j-iStride]; // left
            if (!pPrev)
               b = 0; // above doesn't exist
            else
               b = pPrev[j]; // above
            if (!pPrev || j < iStride)
               c = 0;
            else
               c = pPrev[j-iStride]; // above left
            pOut[j] = pCurr[j] - PAETH(a,b,c);
         }
         break;
      } // switch
} /* PNGFilter() */
