/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * A super simple MJPEG encoder.
 *
 */
#include "fb_alloc.h"
#include "ff_wrapper.h"
#include "imlib.h"

#define SIZE_OFFSET             (1*4)
#define MICROS_OFFSET           (8*4)
#define FRAMES_OFFSET           (12*4)
#define RATE_0_OFFSET           (19*4)
#define LENGTH_0_OFFSET         (21*4)
#define RATE_1_OFFSET           (33*4)
#define LENGTH_1_OFFSET         (35*4)
#define MOVI_OFFSET             (54*4)

void mjpeg_open(file_t *fp, int width, int height)
{
    file_write_data(fp, "RIFF", 4); // FOURCC fcc; - 0
    file_write_long(fp, 0); // DWORD cb; size - updated on close - 1
    file_write_data(fp, "AVI ", 4); // FOURCC fcc; - 2

    file_write_data(fp, "LIST", 4); // FOURCC fcc; - 3
    file_write_long(fp, 192); // DWORD cb; - 4
    file_write_data(fp, "hdrl", 4); // FOURCC fcc; - 5

    file_write_data(fp, "avih", 4); // FOURCC fcc; - 6
    file_write_long(fp, 56); // DWORD cb; - 7
    file_write_long(fp, 0); // DWORD dwMicroSecPerFrame; micros - updated on close - 8
    file_write_long(fp, 0); // DWORD dwMaxBytesPerSec; updated on close - 9
    file_write_long(fp, 4); // DWORD dwPaddingGranularity; - 10
    file_write_long(fp, 0); // DWORD dwFlags; - 11
    file_write_long(fp, 0); // DWORD dwTotalFrames; frames - updated on close - 12
    file_write_long(fp, 0); // DWORD dwInitialFrames; - 13
    file_write_long(fp, 1); // DWORD dwStreams; - 14
    file_write_long(fp, 0); // DWORD dwSuggestedBufferSize; - 15
    file_write_long(fp, width); // DWORD dwWidth; - 16
    file_write_long(fp, height); // DWORD dwHeight; - 17
    file_write_long(fp, 1000); // DWORD dwScale; - 18
    file_write_long(fp, 0); // DWORD dwRate; rate - updated on close - 19
    file_write_long(fp, 0); // DWORD dwStart; - 20
    file_write_long(fp, 0); // DWORD dwLength; length - updated on close - 21

    file_write_data(fp, "LIST", 4); // FOURCC fcc; - 22
    file_write_long(fp, 116); // DWORD cb; - 23
    file_write_data(fp, "strl", 4); // FOURCC fcc; - 24

    file_write_data(fp, "strh", 4); // FOURCC fcc; - 25
    file_write_long(fp, 56); // DWORD cb; - 26
    file_write_data(fp, "vids", 4); // FOURCC fccType; - 27
    file_write_data(fp, "MJPG", 4); // FOURCC fccHandler; - 28
    file_write_long(fp, 0); // DWORD dwFlags; - 29
    file_write_word(fp, 0); // WORD wPriority; - 30
    file_write_word(fp, 0); // WORD wLanguage; - 30.5
    file_write_long(fp, 0); // DWORD dwInitialFrames; - 31
    file_write_long(fp, 1000); // DWORD dwScale; - 32
    file_write_long(fp, 0); // DWORD dwRate; rate - updated on close - 33
    file_write_long(fp, 0); // DWORD dwStart; - 34
    file_write_long(fp, 0); // DWORD dwLength; length - updated on close - 35
    file_write_long(fp, 0); // DWORD dwSuggestedBufferSize; - 36
    file_write_long(fp, 10000); // DWORD dwQuality; - 37
    file_write_long(fp, 0); // DWORD dwSampleSize; - 38
    file_write_word(fp, 0); // short int left; - 39
    file_write_word(fp, 0); // short int top; - 39.5
    file_write_word(fp, 0); // short int right; - 40
    file_write_word(fp, 0); // short int bottom; - 40.5

    file_write_data(fp, "strf", 4); // FOURCC fcc; - 41
    file_write_long(fp, 40); // DWORD cb; - 42
    file_write_long(fp, 40); // DWORD biSize; - 43
    file_write_long(fp, width); // LONG biWidth; - 44
    file_write_long(fp, height); // LONG biHeight; - 45
    file_write_word(fp, 1); // WORD biPlanes; - 46
    file_write_word(fp, 24); // WORD biBitCount; - 46.5
    file_write_data(fp, "MJPG", 4); // DWORD biCompression; - 47
    file_write_long(fp, 0); // DWORD biSizeImage; - 48
    file_write_long(fp, 0); // LONG biXPelsPerMeter; - 49
    file_write_long(fp, 0); // LONG biYPelsPerMeter; - 50
    file_write_long(fp, 0); // DWORD biClrUsed; - 51
    file_write_long(fp, 0); // DWORD biClrImportant; - 52

    file_write_data(fp, "LIST", 4); // FOURCC fcc; - 53
    file_write_long(fp, 0); // DWORD cb; movi - updated on close - 54
    file_write_data(fp, "movi", 4); // FOURCC fcc; - 55
}

void mjpeg_add_frame(file_t *fp, uint32_t *frames, uint32_t *bytes, image_t *img, int quality)
{
    file_write_data(fp, "00dc", 4); // FOURCC fcc;
    *frames += 1;
    if (IM_IS_JPEG(img)) {
        int pad = (((img->bpp + 3) / 4) * 4) - img->bpp;
        file_write_long(fp, img->bpp + pad); // DWORD cb;
        file_write_data(fp, img->pixels, img->bpp + pad); // reading past okay
        *bytes += img->bpp + pad;
    } else {
        uint32_t size;
        uint8_t *buffer = fb_alloc_all(&size);
        image_t out = { .w=img->w, .h=img->h, .bpp=size, .pixels=buffer };
        // When jpeg_compress needs more memory than in currently allocated it
        // will try to realloc. MP will detect that the pointer is outside of
        // the heap and return NULL which will cause an out of memory error.
        jpeg_compress(img, &out, quality, true);
        int pad = (((out.bpp + 3) / 4) * 4) - out.bpp;
        file_write_long(fp, out.bpp + pad); // DWORD cb;
        file_write_data(fp, out.pixels, out.bpp + pad); // reading past okay
        *bytes += out.bpp + pad;
        fb_free();
    }
}

void mjpeg_close(file_t *fp, uint32_t *frames, uint32_t *bytes, float fps)
{
    // Needed
    file_seek(fp, SIZE_OFFSET);
    file_write_long(fp, 216 + (*frames * 8) + *bytes);
    // Needed
    file_seek(fp, MICROS_OFFSET);
    file_write_long(fp, (!fast_roundf(fps)) ? 0 :
            fast_roundf(1000000 / fps));
    file_write_long(fp, (!(*frames)) ? 0 :
            fast_roundf((((*frames * 8) + *bytes) * fps) / *frames));
    // Needed
    file_seek(fp, FRAMES_OFFSET);
    file_write_long(fp, *frames);
    // Probably not needed but writing it just in case.
    file_seek(fp, RATE_0_OFFSET);
    file_write_long(fp, fast_roundf(fps * 1000));
    // Probably not needed but writing it just in case.
    file_seek(fp, LENGTH_0_OFFSET);
    file_write_long(fp, (!fast_roundf(fps)) ? 0 :
            fast_roundf((*frames * 1000) / fps));
    // Probably not needed but writing it just in case.
    file_seek(fp, RATE_1_OFFSET);
    file_write_long(fp, fast_roundf(fps * 1000));
    // Probably not needed but writing it just in case.
    file_seek(fp, LENGTH_1_OFFSET);
    file_write_long(fp, (!fast_roundf(fps)) ? 0 :
            fast_roundf((*frames * 1000) / fps));
    // Needed
    file_seek(fp, MOVI_OFFSET);
    file_write_long(fp, 4 + (*frames * 8) + *bytes);
    file_close(fp);
}
