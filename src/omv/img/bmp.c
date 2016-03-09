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

// This function inits the geometry values of an image (opens file).
bool bmp_read_geometry(FIL *fp, image_t *img, const char *path, bmp_read_settings_t *rs)
{
    read_byte_expect(fp, 'B');
    read_byte_expect(fp, 'M');

    uint32_t file_size;
    read_long(fp, &file_size);
    read_word_ignore(fp);
    read_word_ignore(fp);

    uint32_t header_size;
    read_long(fp, &header_size);
    if (file_size <= header_size) ff_file_corrupted(fp);

    uint32_t data_size = file_size - header_size;
    if (data_size % 4) ff_file_corrupted(fp);

    read_long_expect(fp, 40);
    read_long(fp, (uint32_t*) &rs->bmp_w);
    read_long(fp, (uint32_t*) &rs->bmp_h);
    if ((rs->bmp_w == 0) || (rs->bmp_h == 0)) ff_file_corrupted(fp);
    img->w = abs(rs->bmp_w);
    img->h = abs(rs->bmp_h);

    read_word_expect(fp, 1);
    read_word(fp, &rs->bmp_bpp);
    if ((rs->bmp_bpp != 8) && (rs->bmp_bpp != 16) && (rs->bmp_bpp != 24)) ff_unsupported_format(fp);
    img->bpp = (rs->bmp_bpp == 8) ? 1 : 2;

    read_long(fp, &rs->bmp_fmt);
    if ((rs->bmp_fmt != 0) && (rs->bmp_fmt != 3)) ff_unsupported_format(fp);

    read_long_expect(fp, data_size);
    read_long_ignore(fp);
    read_long_ignore(fp);
    read_long_ignore(fp);
    read_long_ignore(fp);

    if (rs->bmp_bpp == 8) {
        if (rs->bmp_fmt != 0) ff_unsupported_format(fp);
        // Color Table (1024 bytes)
        for (int i = 0; i < 256; i++) {
            read_long_expect(fp, ((i) << 16) | ((i) << 8) | i);
        }
    } else if (rs->bmp_bpp == 16) {
        if (rs->bmp_fmt != 3) ff_unsupported_format(fp);
        // Bit Masks (12 bytes)
        read_long_expect(fp, 0x1F << 11);
        read_long_expect(fp, 0x3F << 5);
        read_long_expect(fp, 0x1F);
    } else if (rs->bmp_bpp == 24) {
        if (rs->bmp_fmt == 3) {
            // Bit Masks (12 bytes)
            read_long_expect(fp, 0xFF << 16);
            read_long_expect(fp, 0xFF << 8);
            read_long_expect(fp, 0xFF);
        }
    }

    rs->bmp_row_bytes = (((img->w * rs->bmp_bpp) + 31) / 32) * 4;
    if (data_size != (rs->bmp_row_bytes * img->h)) ff_file_corrupted(fp);
    return (rs->bmp_h >= 0);
}

// This function reads the pixel values of an image.
void bmp_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, bmp_read_settings_t *rs)
{
    if (rs->bmp_bpp == 8) {
        if ((rs->bmp_h < 0) && (rs->bmp_w >= 0) && (img->w == rs->bmp_row_bytes)) {
            read_data(fp, // Super Fast - Zoom, Zoom!
                      img->pixels + (line_start * img->w),
                      (line_end - line_start) * img->w);
        } else {
            for (int i = line_start; i < line_end; i++) {
                for (int j = 0; j < rs->bmp_row_bytes; j++) {
                    uint8_t pixel;
                    read_byte(fp, &pixel);
                    if (j < img->w) {
                        if (rs->bmp_h < 0) { // vertical flip (BMP file perspective)
                            if (rs->bmp_w < 0) { // horizontal flip (BMP file perspective)
                                IM_SET_GS_PIXEL(img, (img->w-j-1), i, pixel);
                            } else {
                                IM_SET_GS_PIXEL(img, j, i, pixel);
                            }
                        } else {
                            if (rs->bmp_w < 0) {
                                IM_SET_GS_PIXEL(img, (img->w-j-1), (img->h-i-1), pixel);
                            } else {
                                IM_SET_GS_PIXEL(img, j, (img->h-i-1), pixel);
                            }
                        }
                    }
                }
            }
        }
    } else if (rs->bmp_bpp == 16) {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0, jj = rs->bmp_row_bytes / 2; j < jj; j++) {
                uint16_t pixel;
                read_word(fp, &pixel);
                pixel = IM_SWAP16(pixel);
                if (j < img->w) {
                    if (rs->bmp_h < 0) { // vertical flip (BMP file perspective)
                        if (rs->bmp_w < 0) { // horizontal flip (BMP file perspective)
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), i, pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (rs->bmp_w < 0) {
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), (img->h-i-1), pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, (img->h-i-1), pixel);
                        }
                    }
                }
            }
        }
    } else if (rs->bmp_bpp == 24) {
        for (int i = line_start; i < line_end; i++) {
            for (int j = 0, jj = rs->bmp_row_bytes / 3; j < jj; j++) {
                uint8_t r, g, b;
                read_byte(fp, &r);
                read_byte(fp, &g);
                read_byte(fp, &b);
                uint16_t pixel = IM_RGB565(IM_R825(r), IM_G826(g), IM_B825(b));
                if (j < img->w) {
                    if (rs->bmp_h < 0) { // vertical flip
                        if (rs->bmp_w < 0) { // horizontal flip
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), i, pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (rs->bmp_w < 0) {
                            IM_SET_RGB565_PIXEL(img, (img->w-j-1), (img->h-i-1), pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, (img->h-i-1), pixel);
                        }
                    }
                }
            }
            for (int j = 0, jj = rs->bmp_row_bytes % 3; j < jj; j++) {
                read_byte_ignore(fp);
            }
        }
    }
}

void bmp_read(image_t *img, const char *path)
{
    FIL fp;
    bmp_read_settings_t rs;
    file_read_open(&fp, path);
    file_buffer_on(&fp);
    bmp_read_geometry(&fp, img, path, &rs);
    if (!img->pixels) img->pixels = xalloc(img->w * img->h * img->bpp);
    bmp_read_pixels(&fp, img, 0, img->h, &rs);
    file_buffer_off(&fp);
    file_close(&fp);
}

void bmp_write_subimg(image_t *img, const char *path, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) ff_no_intersection(NULL);
    FIL fp;
    file_write_open(&fp, path);

    file_buffer_on(&fp);
    if (IM_IS_GS(img)) {
        const int row_bytes = (((rect.w * 8) + 31) / 32) * 4;
        const int data_size = (row_bytes * rect.h);
        const int waste = (row_bytes / sizeof(uint8_t)) - rect.w;
        // File Header (14 bytes)
        write_byte(&fp, 'B');
        write_byte(&fp, 'M');
        write_long(&fp, 14 + 40 + 1024 + data_size);
        write_word(&fp, 0);
        write_word(&fp, 0);
        write_long(&fp, 14 + 40 + 1024);
        // Info Header (40 bytes)
        write_long(&fp, 40);
        write_long(&fp, rect.w);
        write_long(&fp, -rect.h); // store the image flipped (correctly)
        write_word(&fp, 1);
        write_word(&fp, 8);
        write_long(&fp, 0);
        write_long(&fp, data_size);
        write_long(&fp, 0);
        write_long(&fp, 0);
        write_long(&fp, 0);
        write_long(&fp, 0);
        // Color Table (1024 bytes)
        for (int i = 0; i < 256; i++) {
            write_long(&fp, ((i) << 16) | ((i) << 8) | i);
        }
        if ((rect.x == 0) && (rect.w == img->w) && (img->w == row_bytes)) {
            write_data(&fp, // Super Fast - Zoom, Zoom!
                       img->pixels + (rect.y * img->w),
                       rect.w * rect.h);
        } else {
            for (int i = 0; i < rect.h; i++) {
                write_data(&fp, img->pixels+((rect.y+i)*img->w)+rect.x, rect.w);
                for (int j = 0; j < waste; j++) {
                    write_byte(&fp, 0);
                }
            }
        }
    } else {
        const int row_bytes = (((rect.w * 16) + 31) / 32) * 4;
        const int data_size = (row_bytes * rect.h);
        const int waste = (row_bytes / sizeof(uint16_t)) - rect.w;
        // File Header (14 bytes)
        write_byte(&fp, 'B');
        write_byte(&fp, 'M');
        write_long(&fp, 14 + 40 + 12 + data_size);
        write_word(&fp, 0);
        write_word(&fp, 0);
        write_long(&fp, 14 + 40 + 12);
        // Info Header (40 bytes)
        write_long(&fp, 40);
        write_long(&fp, rect.w);
        write_long(&fp, -rect.h); // store the image flipped (correctly)
        write_word(&fp, 1);
        write_word(&fp, 16);
        write_long(&fp, 3);
        write_long(&fp, data_size);
        write_long(&fp, 0);
        write_long(&fp, 0);
        write_long(&fp, 0);
        write_long(&fp, 0);
        // Bit Masks (12 bytes)
        write_long(&fp, 0x1F << 11);
        write_long(&fp, 0x3F << 5);
        write_long(&fp, 0x1F);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                write_word(&fp, IM_SWAP16(IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i))));
            }
            for (int j = 0; j < waste; j++) {
                write_word(&fp, 0);
            }
        }
    }
    file_buffer_off(&fp);

    file_close(&fp);
}
