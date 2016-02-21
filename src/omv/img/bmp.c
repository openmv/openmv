/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * BMP reader/writer.
 *
 */
#include <arm_math.h>
#include <stdlib.h>
#include <ff.h>
#include "ff_wrapper.h"
#include "xalloc.h"
#include "imlib.h"

// Store mode settings internally, no need for callers to store this info.
static int32_t bmp_w;
static int32_t bmp_h;
static uint16_t bmp_bpp;
static uint32_t bmp_fmt;
static uint32_t bmp_row_bytes;

// This function inits the geometry values of an image.
int bmp_read_geometry(FIL *fp, image_t *img, const char *path)
{
    FRESULT res = f_open(fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) {
        return res;
    }

    READ_BYTE_EXPECT(fp, 'B');
    READ_BYTE_EXPECT(fp, 'M');
    uint32_t file_size;
    READ_LONG(fp, &file_size);
    READ_LONG_IGNORE(fp);
    READ_LONG_IGNORE(fp);

    uint32_t header_size;
    READ_LONG(fp, &header_size);
    if (file_size <= header_size) {
        f_close(fp);
        return -1;
    }

    uint32_t data_size = file_size - header_size;
    if (data_size % 4) { // must be some number of dwords
        f_close(fp);
        return -1;
    }

    READ_LONG_EXPECT(fp, 40);
    READ_LONG(fp, (uint32_t*) &bmp_w);
    READ_LONG(fp, (uint32_t*) &bmp_h);
    if ((bmp_w == 0) || (bmp_h == 0)) {
        f_close(fp);
        return -1;
    }
    img->w = abs(bmp_w);
    img->h = abs(bmp_h);

    READ_WORD_EXPECT(fp, 1);
    READ_WORD(fp, &bmp_bpp);
    if ((bmp_bpp != 8) && (bmp_bpp != 16) && (bmp_bpp != 24)) {
        f_close(fp);
        return -1;
    }
    img->bpp = (bmp_bpp == 8) ? 1 : 2;

    READ_LONG(fp, &bmp_fmt);
    if ((bmp_fmt != 0) && (bmp_fmt != 3)) {
        f_close(fp);
        return -1;
    }

    READ_LONG_EXPECT(fp, data_size);
    READ_LONG_IGNORE(fp);
    READ_LONG_IGNORE(fp);
    READ_LONG_IGNORE(fp);
    READ_LONG_IGNORE(fp);

    if (bmp_bpp == 8) {
        if (bmp_fmt != 0) {
            f_close(fp);
            return -1;
        }
        // Color Table (1024 bytes)
        for (int i = 0; i < 256; i++) {
            READ_LONG_EXPECT(fp, ((i) << 16) | ((i) << 8) | i);
        }
    } else if (bmp_bpp == 16) {
        if (bmp_fmt != 3) {
            f_close(fp);
            return -1;
        }
        // Bit Masks (12 bytes)
        READ_LONG_EXPECT(fp, 0x1F << 11);
        READ_LONG_EXPECT(fp, 0x3F << 5);
        READ_LONG_EXPECT(fp, 0x1F);
    } else if (bmp_bpp == 24) {
        if (bmp_fmt == 3) {
            // Bit Masks (12 bytes)
            READ_LONG_EXPECT(fp, 0xFF << 16);
            READ_LONG_EXPECT(fp, 0xFF << 8);
            READ_LONG_EXPECT(fp, 0xFF);
        }

    }

    bmp_row_bytes = (((img->w * bmp_bpp) + 31) / 32) * 4;
    if (data_size != (bmp_row_bytes * img->h)) {
        f_close(fp);
        return -1;
    }

    return FR_OK;
}

// This function reads the pixel values of an image.
int bmp_read_pixels(FIL *fp, image_t *img, int line_start, int line_end)
{
    if (bmp_bpp == 8) {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < bmp_row_bytes; j++) {
                uint8_t pixel;
                READ_BYTE(fp, &pixel);
                if (j < img->w) {
                    if (bmp_h < 0) { // vertical flip
                        if (bmp_w < 0) { // horizontal flip
                            IM_SET_GS_PIXEL(img, (img->w-j-1), i, pixel);
                        } else {
                            IM_SET_GS_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (bmp_w < 0) {
                            IM_SET_GS_PIXEL(img, (img->w-j-1), (img->h-i-1), pixel);
                        } else {
                            IM_SET_GS_PIXEL(img, j, (img->h-i-1), pixel);
                        }
                    }
                }
            }
        }
    } else if (bmp_bpp == 16) {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0, jj = bmp_row_bytes / 2; j < jj; j++) {
                uint16_t pixel;
                READ_WORD(fp, &pixel);
                IM_SWAP16(pixel);
                if (j < img->w) {
                    if (bmp_h < 0) { // vertical flip
                        if (bmp_w < 0) { // horizontal flip
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), i, pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (bmp_w < 0) {
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), (img->h-i-1), pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, (img->h-i-1), pixel);
                        }
                    }
                }
            }
        }
    } else if (bmp_bpp == 24) {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0, jj = bmp_row_bytes / 3; j < jj; j++) {
                uint8_t r, g, b;
                READ_BYTE(fp, &r);
                READ_BYTE(fp, &g);
                READ_BYTE(fp, &b);
                uint16_t pixel = IM_RGB565(IM_R825(r), IM_G826(g), IM_B825(b));
                if (j < img->w) {
                    if (bmp_h < 0) { // vertical flip
                        if (bmp_w < 0) { // horizontal flip
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), i, pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (bmp_w < 0) {
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), (img->h-i-1), pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, (img->h-i-1), pixel);
                        }
                    }
                }
            }
            for (int j = 0, jj = bmp_row_bytes % 3; j < jj; j++) {
                READ_BYTE_IGNORE(fp);
            }
        }
    }

    return FR_OK;
}

static int bmp_read_int(image_t *img, const char *path)
{
    FIL fp;

    int res = bmp_read_geometry(&fp, img, path);
    if (res != FR_OK) {
        return res;
    }

    if (!img->pixels) { // don't allocate if already allocated...
        img->pixels = xalloc(img->w * img->h * img->bpp);
    }

    res = bmp_read_pixels(&fp, img, 0, img->h);
    if (res != FR_OK) {
        return res;
    }

    return f_close(&fp);
}

int bmp_read(image_t *img, const char *path)
{
    const uint8_t *backup = img->pixels;
    const int res = bmp_read_int(img, path);
    // free image if I didn't start with one...
    if ((res != FR_OK) && (!backup) && (img->pixels)) {
        xfree(img->pixels);
    }
    return res;
}

int bmp_write_subimg(image_t *img, const char *path, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return -1; // no image intersection
    }

    FIL fp;
    FRESULT res = f_open(&fp, path, FA_WRITE|FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        return res;
    }

    if (IM_IS_GS(img)) {
        const int row_bytes = (((rect.w * 8) + 31) / 32) * 4;
        const int data_size = (row_bytes * rect.h);
        const int waste = (row_bytes / sizeof(uint8_t)) - rect.w;
        // File Header (14 bytes)
        WRITE_BYTE(&fp, 'B');
        WRITE_BYTE(&fp, 'M');
        WRITE_LONG(&fp, 14 + 40 + 1024 + data_size);
        WRITE_WORD(&fp, 0);
        WRITE_WORD(&fp, 0);
        WRITE_LONG(&fp, 14 + 40 + 1024);
        // Info Header (40 bytes)
        WRITE_LONG(&fp, 40);
        WRITE_LONG(&fp, rect.w);
        WRITE_LONG(&fp, -rect.h); // store the image flipped (correctly)
        WRITE_WORD(&fp, 1);
        WRITE_WORD(&fp, 8);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, data_size);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, 0);
        // Color Table (1024 bytes)
        for (int i = 0; i < 256; i++) {
            WRITE_LONG(&fp, ((i) << 16) | ((i) << 8) | i);
        }
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                WRITE_BYTE(&fp, IM_GET_GS_PIXEL(img, (rect.x + j), (rect.y + i)));
            }
            for (int j = 0; j < waste; j++) {
                WRITE_BYTE(&fp, 0);
            }
        }
    } else {
        const int row_bytes = (((rect.w * 16) + 31) / 32) * 4;
        const int data_size = (row_bytes * rect.h);
        const int waste = (row_bytes / sizeof(uint16_t)) - rect.w;
        // File Header (14 bytes)
        WRITE_BYTE(&fp, 'B');
        WRITE_BYTE(&fp, 'M');
        WRITE_LONG(&fp, 14 + 40 + 12 + data_size);
        WRITE_WORD(&fp, 0);
        WRITE_WORD(&fp, 0);
        WRITE_LONG(&fp, 14 + 40 + 12);
        // Info Header (40 bytes)
        WRITE_LONG(&fp, 40);
        WRITE_LONG(&fp, rect.w);
        WRITE_LONG(&fp, -rect.h); // store the image flipped (correctly)
        WRITE_WORD(&fp, 1);
        WRITE_WORD(&fp, 16);
        WRITE_LONG(&fp, 3);
        WRITE_LONG(&fp, data_size);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, 0);
        WRITE_LONG(&fp, 0);
        // Bit Masks (12 bytes)
        WRITE_LONG(&fp, 0x1F << 11);
        WRITE_LONG(&fp, 0x3F << 5);
        WRITE_LONG(&fp, 0x1F);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                WRITE_WORD(&fp, IM_SWAP16(IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i))));
            }
            for (int j = 0; j < waste; j++) {
                WRITE_WORD(&fp, 0);
            }
        }
    }

    return f_close(&fp);
}
