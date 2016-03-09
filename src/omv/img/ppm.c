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

static void read_int_reset(ppm_read_settings_t *rs)
{
    rs->read_int_c_valid = false;
}

static void read_int(FIL *fp, uint32_t *i, ppm_read_settings_t *rs)
{
    enum { EAT_WHITESPACE, EAT_COMMENT, EAT_NUMBER } mode = EAT_WHITESPACE;
    for(*i = 0;;) {
        if (!rs->read_int_c_valid) {
            if (file_tell_w_buf(fp) == file_size_w_buf(fp)) return;
            read_byte(fp, &rs->read_int_c);
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
void ppm_read_geometry(FIL *fp, image_t *img, const char *path, ppm_read_settings_t *rs)
{
    read_int_reset(rs);
    read_byte_expect(fp, 'P');
    read_byte(fp, &rs->ppm_fmt);
    if ((rs->ppm_fmt!='2') && (rs->ppm_fmt!='3') && (rs->ppm_fmt!='5') && (rs->ppm_fmt!='6')) ff_unsupported_format(fp);
    img->bpp = ((rs->ppm_fmt == '2') || (rs->ppm_fmt == '5')) ? 1 : 2;

    read_int(fp, (uint32_t *) &img->w, rs);
    read_int(fp, (uint32_t *) &img->h, rs);
    if ((img->w == 0) || (img->h == 0)) ff_file_corrupted(fp);

    uint32_t max;
    read_int(fp, &max, rs);
    if (max != 255) ff_unsupported_format(fp);
}

// This function reads the pixel values of an image.
void ppm_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, ppm_read_settings_t *rs)
{
    if (rs->ppm_fmt == '2') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint32_t pixel;
                read_int(fp, &pixel, rs);
                IM_SET_GS_PIXEL(img, j, i, pixel);
            }
        }
    } else if (rs->ppm_fmt == '3') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint32_t r, g, b;
                read_int(fp, &r, rs);
                read_int(fp, &g, rs);
                read_int(fp, &b, rs);
                IM_SET_RGB565_PIXEL(img, j, i, IM_RGB565(IM_R825(r),
                                                         IM_G826(g),
                                                         IM_B825(b)));
            }
        }
    } else if (rs->ppm_fmt == '5') {
        read_data(fp, // Super Fast - Zoom, Zoom!
                  img->pixels + (line_start * img->w),
                  (line_end - line_start) * img->w);
    } else if (rs->ppm_fmt == '6') {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0; j < img->w; j++) {
                uint8_t r, g, b;
                read_byte(fp, &r);
                read_byte(fp, &g);
                read_byte(fp, &b);
                IM_SET_RGB565_PIXEL(img, j, i, IM_RGB565(IM_R825(r),
                                                         IM_G826(g),
                                                         IM_B825(b)));
            }
        }
    }
}

void ppm_read(image_t *img, const char *path)
{
    FIL fp;
    ppm_read_settings_t rs;
    file_read_open(&fp, path);
    file_buffer_on(&fp);
    ppm_read_geometry(&fp, img, path, &rs);
    if (!img->pixels) img->pixels = xalloc(img->w * img->h * img->bpp);
    ppm_read_pixels(&fp, img, 0, img->h, &rs);
    file_buffer_off(&fp);
    file_close(&fp);
}

void ppm_write_subimg(image_t *img, const char *path, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) ff_no_intersection(NULL);
    FIL fp;
    file_write_open(&fp, path);

    file_buffer_on(&fp);
    if (IM_IS_GS(img)) {
        char buffer[20]; // exactly big enough for 5-digit w/h
        int len = snprintf(buffer, 20, "P5\n%d %d\n255\n", rect.w, rect.h);
        write_data(&fp, buffer, len);
        if ((rect.x == 0) && (rect.w == img->w)) {
            write_data(&fp, // Super Fast - Zoom, Zoom!
                       img->pixels + (rect.y * img->w),
                       rect.w * rect.h);
        } else {
            for (int i = 0; i < rect.h; i++) {
                write_data(&fp, img->pixels+((rect.y+i)*img->w)+rect.x, rect.w);
            }
        }
    } else {
        char buffer[20]; // exactly big enough for 5-digit w/h
        int len = snprintf(buffer, 20, "P6\n%d %d\n255\n", rect.w, rect.h);
        write_data(&fp, buffer, len);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int pixel = IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i));
                char buff[3];
                buff[0] = IM_R528(IM_R565(pixel));
                buff[1] = IM_G628(IM_G565(pixel));
                buff[2] = IM_B528(IM_B565(pixel));
                write_data(&fp, buff, 3);
            }
        }
    }
    file_buffer_off(&fp);

    file_close(&fp);
}
