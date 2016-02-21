/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * PPM/PGM reader/writer.
 *
 */
#include <stdio.h>
#include <ff.h>
#include "ff_wrapper.h"
#include "xalloc.h"
#include "imlib.h"

static uint8_t read_int_c;
static bool read_int_c_valid;
static void read_int_reset() {
    read_int_c_valid = false;
}

static int read_int(FIL *fp, uint32_t *i)
{
    enum { EAT_WHITESPACE, EAT_COMMENT, EAT_NUMBER } mode = EAT_WHITESPACE;
    for(*i = 0;;) {
        if (!read_int_c_valid) {
            if (f_eof(fp)) {
                return FR_OK;
            }
            int res = read_byte(fp, &read_int_c);
            if (res != FR_OK) {
                return res;
            }
            read_int_c_valid = true;
        }
        if (mode == EAT_WHITESPACE) {
            if (read_int_c == '#') {
                mode = EAT_COMMENT;
            } else if (('0' <= read_int_c) && (read_int_c <= '9')) {
                *i = read_int_c - '0';
                mode = EAT_NUMBER;
            }
        } else if (mode == EAT_COMMENT) {
            if ((read_int_c == '\n') || (read_int_c == '\r')) {
                mode = EAT_WHITESPACE;
            }
        } else if (mode == EAT_NUMBER) {
            if (('0' <= read_int_c) && (read_int_c <= '9')) {
                *i = (*i * 10) + read_int_c - '0';
            } else {
                return FR_OK; // read_int_c_valid==true on exit
            }
        }
        read_int_c_valid = false;
    }
}

#define READ_INT(fp, i) \
    ({ FRESULT _res = read_int((fp), (i)); \
       if (_res != FR_OK) { f_close((fp)); return _res; } })

////////////////////////////////////////////////////////////////////////////////

// Store mode settings internally, no need for callers to store this info.
static uint8_t ppm_fmt;

// This function inits the geometry values of an image.
int ppm_read_geometry(FIL *fp, image_t *img, const char *path)
{
    read_int_reset();
    FRESULT res = f_open(fp, path, FA_READ|FA_OPEN_EXISTING);
    if (res != FR_OK) {
        return res;
    }

    READ_BYTE_EXPECT(fp, 'P');
    READ_BYTE(fp, &ppm_fmt);
    if ((ppm_fmt!='2') && (ppm_fmt!='3') && (ppm_fmt!='5') && (ppm_fmt!='6')) {
        f_close(fp);
        return -1;
    }
    img->bpp = ((ppm_fmt == '2') || (ppm_fmt == '5')) ? 1 : 2;

    READ_INT(fp, (uint32_t *) &img->w);
    READ_INT(fp, (uint32_t *) &img->h);
    if ((img->w == 0) || (img->h == 0)) {
        f_close(fp);
        return -1;
    }

    uint32_t max;
    READ_INT(fp, &max);
    if (max != 255) {
        f_close(fp);
        return -1;
    }

    return FR_OK;
}

// This function reads the pixel values of an image.
int ppm_read_pixels(FIL *fp, image_t *img, int line_start, int line_end)
{
    if (ppm_fmt == '2') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint32_t pixel;
                READ_INT(fp, &pixel);
                IM_SET_GS_PIXEL(img, j, i, pixel);
            }
        }
    } else if (ppm_fmt == '3') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint32_t r, g, b;
                READ_INT(fp, &r);
                READ_INT(fp, &g);
                READ_INT(fp, &b);
                IM_SET_RGB565_PIXEL(img, j, i, IM_RGB565(IM_R825(r),
                                                         IM_G826(g),
                                                         IM_B825(b)));
            }
        }
    } else if (ppm_fmt == '5') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint8_t pixel;
                READ_BYTE(fp, &pixel);
                IM_SET_GS_PIXEL(img, j, i, pixel);
            }
        }
    } else if (ppm_fmt == '6') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint8_t r, g, b;
                READ_BYTE(fp, &r);
                READ_BYTE(fp, &g);
                READ_BYTE(fp, &b);
                IM_SET_RGB565_PIXEL(img, j, i, IM_RGB565(IM_R825(r),
                                                         IM_G826(g),
                                                         IM_B825(b)));
            }
        }
    }

    return FR_OK;
}

static int ppm_read_int(image_t *img, const char *path)
{
    FIL fp;

    int res = ppm_read_geometry(&fp, img, path);
    if (res != FR_OK) {
        return res;
    }

    if (!img->pixels) { // don't allocate if already allocated...
        img->pixels = xalloc(img->w * img->h * img->bpp);
    }

    res = ppm_read_pixels(&fp, img, 0, img->h);
    if (res != FR_OK) {
        return res;
    }

    return f_close(&fp);
}

int ppm_read(image_t *img, const char *path)
{
    const uint8_t *backup = img->pixels;
    const int res = ppm_read_int(img, path);
    // free image if I didn't start with one...
    if ((res != FR_OK) && (!backup) && (img->pixels)) {
        xfree(img->pixels);
    }
    return res;
}

int ppm_write_subimg(image_t *img, const char *path, rectangle_t *r)
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
        char buffer[20]; // exactly big enough for 5-digit w/h
        int len = snprintf(buffer, 20, "P5\n%d %d\n255\n", rect.w, rect.h);
        WRITE_DATA(&fp, buffer, len);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                WRITE_BYTE(&fp, IM_GET_GS_PIXEL(img, (rect.x + j), (rect.y + i)));
            }
        }
    } else {
        char buffer[20]; // exactly big enough for 5-digit w/h
        int len = snprintf(buffer, 20, "P6\n%d %d\n255\n", rect.w, rect.h);
        WRITE_DATA(&fp, buffer, len);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int pixel = IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i));
                char buff[3];
                buff[0] = IM_R528(IM_R565(pixel));
                buff[1] = IM_G628(IM_G565(pixel));
                buff[2] = IM_B528(IM_B565(pixel));
                WRITE_DATA(&fp, buff, 3);
            }
        }
    }

    return f_close(&fp);
}
