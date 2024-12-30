/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * PPM/PGM reader/writer.
 */

#include "imlib.h"
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)

#include <stdio.h>
#include "py/obj.h"
#include "py/runtime.h"

#include "xalloc.h"
#include "imlib.h"
#include "file_utils.h"

static void read_int_reset(ppm_read_settings_t *rs) {
    rs->read_int_c_valid = false;
}

static void read_int(FIL *fp, uint32_t *i, ppm_read_settings_t *rs) {
    enum {
        EAT_WHITESPACE, EAT_COMMENT, EAT_NUMBER
    }
    mode = EAT_WHITESPACE;
    for (*i = 0;;) {
        if (!rs->read_int_c_valid) {
            if (file_tell(fp) == file_size(fp)) {
                return;
            }
            file_read(fp, &rs->read_int_c, 1);
            rs->read_int_c_valid = true;
        }
        if (mode == EAT_WHITESPACE) {
            if (rs->read_int_c == '#') {
                mode = EAT_COMMENT;
            } else if (('0' <= rs->read_int_c) && (rs->read_int_c <= '9')) {
                *i = rs->read_int_c - '0';
                mode = EAT_NUMBER;
            }
        } else if (mode == EAT_COMMENT) {
            if ((rs->read_int_c == '\n') || (rs->read_int_c == '\r')) {
                mode = EAT_WHITESPACE;
            }
        } else if (mode == EAT_NUMBER) {
            if (('0' <= rs->read_int_c) && (rs->read_int_c <= '9')) {
                *i = (*i * 10) + rs->read_int_c - '0';
            } else {
                return; // read_int_c_valid==true on exit
            }
        }
        rs->read_int_c_valid = false;
    }
}

// This function inits the geometry values of an image.
void ppm_read_geometry(FIL *fp, image_t *img, const char *path, ppm_read_settings_t *rs) {
    read_int_reset(rs);
    file_read_check(fp, "P", 1);
    file_read(fp, &rs->ppm_fmt, 1);

    if ((rs->ppm_fmt != '2') && (rs->ppm_fmt != '3') && (rs->ppm_fmt != '5') && (rs->ppm_fmt != '6')) {
        file_raise_format(fp);
    }

    img->pixfmt = ((rs->ppm_fmt == '2') || (rs->ppm_fmt == '5')) ? PIXFORMAT_GRAYSCALE : PIXFORMAT_RGB565;

    read_int(fp, (uint32_t *) &img->w, rs);
    read_int(fp, (uint32_t *) &img->h, rs);

    if ((img->w == 0) || (img->h == 0)) {
        file_raise_corrupted(fp);
    }

    uint32_t max;
    read_int(fp, &max, rs);
    if (max != 255) {
        file_raise_format(fp);
    }
}

// This function reads the pixel values of an image.
void ppm_read_pixels(FIL *fp, image_t *img, int n_lines, ppm_read_settings_t *rs) {
    if (rs->ppm_fmt == '2') {
        for (int i = 0; i < n_lines; i++) {
            for (int j = 0; j < img->w; j++) {
                uint32_t pixel;
                read_int(fp, &pixel, rs);
                IM_SET_GS_PIXEL(img, j, i, pixel);
            }
        }
    } else if (rs->ppm_fmt == '3') {
        for (int i = 0; i < n_lines; i++) {
            for (int j = 0; j < img->w; j++) {
                uint32_t r, g, b;
                read_int(fp, &r, rs);
                read_int(fp, &g, rs);
                read_int(fp, &b, rs);
                IM_SET_RGB565_PIXEL(img, j, i, COLOR_R8_G8_B8_TO_RGB565(r, g, b));
            }
        }
    } else if (rs->ppm_fmt == '5') {
        file_read(fp, img->pixels, n_lines * img->w);
    } else if (rs->ppm_fmt == '6') {
        for (int i = 0; i < n_lines; i++) {
            for (int j = 0; j < img->w; j++) {
                uint8_t r, g, b;
                file_read(fp, &r, 1);
                file_read(fp, &g, 1);
                file_read(fp, &b, 1);
                IM_SET_RGB565_PIXEL(img, j, i, COLOR_R8_G8_B8_TO_RGB565(r, g, b));
            }
        }
    }
}

void ppm_read(image_t *img, const char *path) {
    FIL fp;
    ppm_read_settings_t rs;

    file_open(&fp, path, true, FA_READ | FA_OPEN_EXISTING);
    ppm_read_geometry(&fp, img, path, &rs);

    if (!img->pixels) {
        image_xalloc(img, img->w * img->h * img->bpp);
    }
    ppm_read_pixels(&fp, img, img->h, &rs);
    file_close(&fp);
}

void ppm_write_subimg(image_t *img, const char *path, rectangle_t *r) {
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("No intersection!"));
    }

    FIL fp;
    file_open(&fp, path, true, FA_WRITE | FA_CREATE_ALWAYS);

    if (IM_IS_GS(img)) {
        char buffer[20]; // exactly big enough for 5-digit w/h
        int len = snprintf(buffer, 20, "P5\n%d %d\n255\n", rect.w, rect.h);
        file_write(&fp, buffer, len);
        if ((rect.x == 0) && (rect.w == img->w)) {
            file_write(&fp, img->pixels + (rect.y * img->w), rect.w * rect.h);
        } else {
            for (int i = 0; i < rect.h; i++) {
                file_write(&fp, img->pixels + ((rect.y + i) * img->w) + rect.x, rect.w);
            }
        }
    } else {
        char buffer[20]; // exactly big enough for 5-digit w/h
        int len = snprintf(buffer, 20, "P6\n%d %d\n255\n", rect.w, rect.h);
        file_write(&fp, buffer, len);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int pixel = IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i));
                char buff[3];
                buff[0] = COLOR_RGB565_TO_R8(pixel);
                buff[1] = COLOR_RGB565_TO_G8(pixel);
                buff[2] = COLOR_RGB565_TO_B8(pixel);
                file_write(&fp, buff, 3);
            }
        }
    }
    file_close(&fp);
}
#endif //IMLIB_ENABLE_IMAGE_FILE_IO
