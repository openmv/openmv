/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * A simple MJPEG encoder.
 */
#include "imlib.h"
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)

#include "ff_wrapper.h"

#define SIZE_OFFSET        (1 * 4)
#define MICROS_OFFSET      (8 * 4)
#define FRAMES_OFFSET      (12 * 4)
#define RATE_0_OFFSET      (19 * 4)
#define LENGTH_0_OFFSET    (21 * 4)
#define RATE_1_OFFSET      (33 * 4)
#define LENGTH_1_OFFSET    (35 * 4)
#define MOVI_OFFSET        (54 * 4)

void mjpeg_open(FIL *fp, int width, int height) {
    write_data(fp, "RIFF", 4); // FOURCC fcc; - 0
    write_long(fp, 0); // DWORD cb; size - updated on close - 1
    write_data(fp, "AVI ", 4); // FOURCC fcc; - 2

    write_data(fp, "LIST", 4); // FOURCC fcc; - 3
    write_long(fp, 192); // DWORD cb; - 4
    write_data(fp, "hdrl", 4); // FOURCC fcc; - 5

    write_data(fp, "avih", 4); // FOURCC fcc; - 6
    write_long(fp, 56); // DWORD cb; - 7
    write_long(fp, 0); // DWORD dwMicroSecPerFrame; micros - updated on close - 8
    write_long(fp, 0); // DWORD dwMaxBytesPerSec; updated on close - 9
    write_long(fp, 4); // DWORD dwPaddingGranularity; - 10
    write_long(fp, 0); // DWORD dwFlags; - 11
    write_long(fp, 0); // DWORD dwTotalFrames; frames - updated on close - 12
    write_long(fp, 0); // DWORD dwInitialFrames; - 13
    write_long(fp, 1); // DWORD dwStreams; - 14
    write_long(fp, 0); // DWORD dwSuggestedBufferSize; - 15
    write_long(fp, width); // DWORD dwWidth; - 16
    write_long(fp, height); // DWORD dwHeight; - 17
    write_long(fp, 1000); // DWORD dwScale; - 18
    write_long(fp, 0); // DWORD dwRate; rate - updated on close - 19
    write_long(fp, 0); // DWORD dwStart; - 20
    write_long(fp, 0); // DWORD dwLength; length - updated on close - 21

    write_data(fp, "LIST", 4); // FOURCC fcc; - 22
    write_long(fp, 116); // DWORD cb; - 23
    write_data(fp, "strl", 4); // FOURCC fcc; - 24

    write_data(fp, "strh", 4); // FOURCC fcc; - 25
    write_long(fp, 56); // DWORD cb; - 26
    write_data(fp, "vids", 4); // FOURCC fccType; - 27
    write_data(fp, "MJPG", 4); // FOURCC fccHandler; - 28
    write_long(fp, 0); // DWORD dwFlags; - 29
    write_word(fp, 0); // WORD wPriority; - 30
    write_word(fp, 0); // WORD wLanguage; - 30.5
    write_long(fp, 0); // DWORD dwInitialFrames; - 31
    write_long(fp, 1000); // DWORD dwScale; - 32
    write_long(fp, 0); // DWORD dwRate; rate - updated on close - 33
    write_long(fp, 0); // DWORD dwStart; - 34
    write_long(fp, 0); // DWORD dwLength; length - updated on close - 35
    write_long(fp, 0); // DWORD dwSuggestedBufferSize; - 36
    write_long(fp, 10000); // DWORD dwQuality; - 37
    write_long(fp, 0); // DWORD dwSampleSize; - 38
    write_word(fp, 0); // short int left; - 39
    write_word(fp, 0); // short int top; - 39.5
    write_word(fp, 0); // short int right; - 40
    write_word(fp, 0); // short int bottom; - 40.5

    write_data(fp, "strf", 4); // FOURCC fcc; - 41
    write_long(fp, 40); // DWORD cb; - 42
    write_long(fp, 40); // DWORD biSize; - 43
    write_long(fp, width); // LONG biWidth; - 44
    write_long(fp, height); // LONG biHeight; - 45
    write_word(fp, 1); // WORD biPlanes; - 46
    write_word(fp, 24); // WORD biBitCount; - 46.5
    write_data(fp, "MJPG", 4); // DWORD biCompression; - 47
    write_long(fp, 0); // DWORD biSizeImage; - 48
    write_long(fp, 0); // LONG biXPelsPerMeter; - 49
    write_long(fp, 0); // LONG biYPelsPerMeter; - 50
    write_long(fp, 0); // DWORD biClrUsed; - 51
    write_long(fp, 0); // DWORD biClrImportant; - 52

    write_data(fp, "LIST", 4); // FOURCC fcc; - 53
    write_long(fp, 0); // DWORD cb; movi - updated on close - 54
    write_data(fp, "movi", 4); // FOURCC fcc; - 55
}

void mjpeg_write(FIL *fp, int width, int height, uint32_t *frames, uint32_t *bytes,
                 image_t *img, int quality, rectangle_t *roi, int rgb_channel, int alpha,
                 const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint) {
    float xscale = width / ((float) roi->w);
    float yscale = height / ((float) roi->h);
    // MAX == KeepAspectRationByExpanding - MIN == KeepAspectRatio
    float scale = IM_MIN(xscale, yscale);

    image_t dst_img = {
        .w = width,
        .h = height,
        .pixfmt = PIXFORMAT_JPEG,
        .size = 0,
        .data = NULL
    };

    bool simple = (xscale == 1) &&
                  (yscale == 1) &&
                  (roi->x == 0) &&
                  (roi->y == 0) &&
                  (roi->w == img->w) &&
                  (roi->h == img->h) &&
                  (rgb_channel == -1) &&
                  (alpha == 256) &&
                  (color_palette == NULL) &&
                  (alpha_palette == NULL);

    fb_alloc_mark();

    if ((dst_img.pixfmt != img->pixfmt) || (!simple)) {
        image_t temp;
        memcpy(&temp, img, sizeof(image_t));

        if (img->is_compressed || (!simple)) {
            temp.w = dst_img.w;
            temp.h = dst_img.h;
            temp.pixfmt = PIXFORMAT_RGB565; // TODO PIXFORMAT_ARGB8888
            temp.size = 0;
            temp.data = fb_alloc(image_size(&temp), FB_ALLOC_NO_HINT);

            int center_x = fast_floorf((width - (roi->w * scale)) / 2);
            int center_y = fast_floorf((height - (roi->h * scale)) / 2);

            int x0, x1, y0, y1;
            bool black = !imlib_draw_image_rectangle(&temp, img, center_x, center_y, scale, scale, roi,
                                                     alpha, alpha_palette, hint, &x0, &x1, &y0, &y1);

            if (black) {
                // zero the whole image
                memset(temp.data, 0, temp.w * temp.h * sizeof(uint16_t));
            } else {
                // Zero the top rows
                if (y0) {
                    memset(temp.data, 0, temp.w * y0 * sizeof(uint16_t));
                }

                if (x0) {
                    for (int i = y0; i < y1; i++) {
                        // Zero left
                        memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp, i), 0, x0 * sizeof(uint16_t));
                    }
                }

                imlib_draw_image(&temp, img, center_x, center_y, scale, scale, roi,
                                 rgb_channel, alpha, color_palette, alpha_palette,
                                 (hint & (~IMAGE_HINT_CENTER)) | IMAGE_HINT_BLACK_BACKGROUND,
                                 NULL, NULL);

                if (temp.w - x1) {
                    for (int i = y0; i < y1; i++) {
                        // Zero right
                        memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp, i) + x1,
                               0, (temp.w - x1) * sizeof(uint16_t));
                    }
                }

                // Zero the bottom rows
                if (temp.h - y1) {
                    memset(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp, y1),
                           0, temp.w * (temp.h - y1) * sizeof(uint16_t));
                }
            }
        }

        // When jpeg_compress needs more memory than in currently allocated it
        // will try to realloc. MP will detect that the pointer is outside of
        // the heap and return NULL which will cause an out of memory error.
        jpeg_compress(&temp, &dst_img, quality, true);
    } else {
        dst_img.size = img->size;
        dst_img.data = img->data;
    }

    uint32_t size_padded = (((dst_img.size + 3) / 4) * 4);
    write_data(fp, "00dc", 4); // FOURCC fcc;
    write_long(fp, size_padded); // DWORD cb;
    write_data(fp, dst_img.data, size_padded); // reading past okay

    *frames += 1;
    *bytes += size_padded;

    fb_alloc_free_till_mark();
}

void mjpeg_sync(FIL *fp, uint32_t *frames, uint32_t *bytes, float fps) {
    uint32_t position = f_tell(fp);
    // Needed
    file_seek(fp, SIZE_OFFSET);
    write_long(fp, 216 + (*frames * 8) + *bytes);
    // Needed
    file_seek(fp, MICROS_OFFSET);
    write_long(fp, (!fast_roundf(fps)) ? 0 :
               fast_roundf(1000000 / fps));
    write_long(fp, (!(*frames)) ? 0 :
               fast_roundf((((*frames * 8) + *bytes) * fps) / *frames));
    // Needed
    file_seek(fp, FRAMES_OFFSET);
    write_long(fp, *frames);
    // Probably not needed but writing it just in case.
    file_seek(fp, RATE_0_OFFSET);
    write_long(fp, fast_roundf(fps * 1000));
    // Probably not needed but writing it just in case.
    file_seek(fp, LENGTH_0_OFFSET);
    write_long(fp, (!fast_roundf(fps)) ? 0 :
               fast_roundf((*frames * 1000) / fps));
    // Probably not needed but writing it just in case.
    file_seek(fp, RATE_1_OFFSET);
    write_long(fp, fast_roundf(fps * 1000));
    // Probably not needed but writing it just in case.
    file_seek(fp, LENGTH_1_OFFSET);
    write_long(fp, (!fast_roundf(fps)) ? 0 :
               fast_roundf((*frames * 1000) / fps));
    // Needed
    file_seek(fp, MOVI_OFFSET);
    write_long(fp, 4 + (*frames * 8) + *bytes);
    file_sync(fp);
    file_seek(fp, position);
}

void mjpeg_close(FIL *fp, uint32_t *frames, uint32_t *bytes, float fps) {
    // Needed
    file_seek(fp, SIZE_OFFSET);
    write_long(fp, 216 + (*frames * 8) + *bytes);
    // Needed
    file_seek(fp, MICROS_OFFSET);
    write_long(fp, (!fast_roundf(fps)) ? 0 :
               fast_roundf(1000000 / fps));
    write_long(fp, (!(*frames)) ? 0 :
               fast_roundf((((*frames * 8) + *bytes) * fps) / *frames));
    // Needed
    file_seek(fp, FRAMES_OFFSET);
    write_long(fp, *frames);
    // Probably not needed but writing it just in case.
    file_seek(fp, RATE_0_OFFSET);
    write_long(fp, fast_roundf(fps * 1000));
    // Probably not needed but writing it just in case.
    file_seek(fp, LENGTH_0_OFFSET);
    write_long(fp, (!fast_roundf(fps)) ? 0 :
               fast_roundf((*frames * 1000) / fps));
    // Probably not needed but writing it just in case.
    file_seek(fp, RATE_1_OFFSET);
    write_long(fp, fast_roundf(fps * 1000));
    // Probably not needed but writing it just in case.
    file_seek(fp, LENGTH_1_OFFSET);
    write_long(fp, (!fast_roundf(fps)) ? 0 :
               fast_roundf((*frames * 1000) / fps));
    // Needed
    file_seek(fp, MOVI_OFFSET);
    write_long(fp, 4 + (*frames * 8) + *bytes);
    file_close(fp);
}

#endif // IMLIB_ENABLE_IMAGE_FILE_IO
