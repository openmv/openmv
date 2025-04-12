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
 * BMP reader/writer.
 */
#include "imlib.h"
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)

#include <stdlib.h>
#include "py/obj.h"
#include "py/runtime.h"

#include "xalloc.h"
#include "file_utils.h"

// This function inits the geometry values of an image (opens file).
bool bmp_read_geometry(FIL *fp, image_t *img, const char *path, bmp_read_settings_t *rs) {
    file_read_check(fp, "BM", 2);

    uint32_t file_size;
    file_read(fp, &file_size, 4);
    file_read(fp, NULL, 4);

    uint32_t header_size;
    file_read(fp, &header_size, 4);
    if (file_size <= header_size) {
        file_raise_corrupted(fp);
    }

    uint32_t data_size = file_size - header_size;
    if (data_size % 4) {
        file_raise_corrupted(fp);
    }

    uint32_t header_type;
    file_read(fp, &header_type, 4);
    if ((header_type != 40) // BITMAPINFOHEADER
        && (header_type != 52) // BITMAPV2INFOHEADER
        && (header_type != 56) // BITMAPV3INFOHEADER
        && (header_type != 108) // BITMAPV4HEADER
        && (header_type != 124)) {
        file_raise_format(fp);  // BITMAPV5HEADER
    }
    file_read(fp, &rs->bmp_w, 4);
    file_read(fp, &rs->bmp_h, 4);
    if ((rs->bmp_w == 0) || (rs->bmp_h == 0)) {
        file_raise_corrupted(fp);
    }
    img->w = abs(rs->bmp_w);
    img->h = abs(rs->bmp_h);

    file_read_check(fp, &(uint32_t) {1}, 2);
    file_read(fp, &rs->bmp_bpp, 2);
    if ((rs->bmp_bpp != 8) && (rs->bmp_bpp != 16) && (rs->bmp_bpp != 24)) {
        file_raise_format(fp);
    }
    img->pixfmt = (rs->bmp_bpp == 8) ? PIXFORMAT_GRAYSCALE : PIXFORMAT_RGB565;

    file_read(fp, &rs->bmp_fmt, 4);
    if ((rs->bmp_fmt != 0) && (rs->bmp_fmt != 3)) {
        file_raise_format(fp);
    }

    file_read_check(fp, &data_size, 4);
    file_read(fp, NULL, 16);

    if (rs->bmp_bpp == 8) {
        if (rs->bmp_fmt != 0) {
            file_raise_format(fp);
        }
        if (header_type >= 52) {
            // Skip past the remaining BITMAPV2INFOHEADER bytes.
            for (int i = 0; i < 3; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 56) {
            // Skip past the remaining BITMAPV3INFOHEADER bytes.
            for (int i = 0; i < 1; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 108) {
            // Skip past the remaining BITMAPV4HEADER bytes.
            for (int i = 0; i < 13; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 124) {
            // Skip past the remaining BITMAPV5HEADER bytes.
            for (int i = 0; i < 4; i++) {
                file_read(fp, NULL, 4);
            }
        }
        // Color Table (1024 bytes)
        for (int i = 0; i < 256; i++) {
            file_read_check(fp, &(uint32_t) {((i) << 16) | ((i) << 8) | i}, 4);
        }
    } else if (rs->bmp_bpp == 16) {
        if (rs->bmp_fmt != 3) {
            file_raise_format(fp);
        }
        // Bit Masks (12 bytes)
        file_read_check(fp, (uint32_t [3]) {0x1F << 11, 0x3F << 5, 0x1F}, 12);
        if (header_type >= 56) {
            // Skip past the remaining BITMAPV3INFOHEADER bytes.
            for (int i = 0; i < 1; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 108) {
            // Skip past the remaining BITMAPV4HEADER bytes.
            for (int i = 0; i < 13; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 124) {
            // Skip past the remaining BITMAPV5HEADER bytes.
            for (int i = 0; i < 4; i++) {
                file_read(fp, NULL, 4);
            }
        }
    } else if (rs->bmp_bpp == 24) {
        if (rs->bmp_fmt == 3) {
            // Bit Masks (12 bytes)
            file_read_check(fp, (uint32_t [3]) {0xFF << 16, 0xFF << 8, 0xFF}, 12);
        } else if (header_type >= 52) {
            // Skip past the remaining BITMAPV2INFOHEADER bytes.
            for (int i = 0; i < 3; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 56) {
            // Skip past the remaining BITMAPV3INFOHEADER bytes.
            for (int i = 0; i < 1; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 108) {
            // Skip past the remaining BITMAPV4HEADER bytes.
            for (int i = 0; i < 13; i++) {
                file_read(fp, NULL, 4);
            }
        }
        if (header_type >= 124) {
            // Skip past the remaining BITMAPV5HEADER bytes.
            for (int i = 0; i < 4; i++) {
                file_read(fp, NULL, 4);
            }
        }
    }

    rs->bmp_row_bytes = (((img->w * rs->bmp_bpp) + 31) / 32) * 4;
    if (data_size != (rs->bmp_row_bytes * img->h)) {
        file_raise_corrupted(fp);
    }
    return (rs->bmp_h >= 0);
}

// This function reads the pixel values of an image.
void bmp_read_pixels(FIL *fp, image_t *img, int n_lines, bmp_read_settings_t *rs) {
    if (rs->bmp_bpp == 8) {
        if ((rs->bmp_h < 0) && (rs->bmp_w >= 0) && (img->w == rs->bmp_row_bytes)) {
            file_read(fp, img->pixels, n_lines * img->w);
        } else {
            for (int i = 0; i < n_lines; i++) {
                for (int j = 0; j < rs->bmp_row_bytes; j++) {
                    uint8_t pixel;
                    file_read(fp, &pixel, 1);
                    if (j < img->w) {
                        if (rs->bmp_h < 0) {
                            // vertical flip (BMP file perspective)
                            if (rs->bmp_w < 0) {
                                // horizontal flip (BMP file perspective)
                                IM_SET_GS_PIXEL(img, (img->w - j - 1), i, pixel);
                            } else {
                                IM_SET_GS_PIXEL(img, j, i, pixel);
                            }
                        } else {
                            if (rs->bmp_w < 0) {
                                IM_SET_GS_PIXEL(img, (img->w - j - 1), (img->h - i - 1), pixel);
                            } else {
                                IM_SET_GS_PIXEL(img, j, (img->h - i - 1), pixel);
                            }
                        }
                    }
                }
            }
        }
    } else if (rs->bmp_bpp == 16) {
        for (int i = 0; i < n_lines; i++) {
            for (int j = 0, jj = rs->bmp_row_bytes / 2; j < jj; j++) {
                uint16_t pixel;
                file_read(fp, &pixel, 2);
                if (j < img->w) {
                    if (rs->bmp_h < 0) {
                        // vertical flip (BMP file perspective)
                        if (rs->bmp_w < 0) {
                            // horizontal flip (BMP file perspective)
                            IM_SET_RGB565_PIXEL(img, (img->w - j - 1), i, pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (rs->bmp_w < 0) {
                            IM_SET_RGB565_PIXEL(img, (img->w - j - 1), (img->h - i - 1), pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, (img->h - i - 1), pixel);
                        }
                    }
                }
            }
        }
    } else if (rs->bmp_bpp == 24) {
        for (int i = 0; i < n_lines; i++) {
            for (int j = 0, jj = rs->bmp_row_bytes / 3; j < jj; j++) {
                uint8_t b, g, r;
                file_read(fp, &b, 1);
                file_read(fp, &g, 1);
                file_read(fp, &r, 1);
                uint16_t pixel = COLOR_R8_G8_B8_TO_RGB565(r, g, b);
                if (j < img->w) {
                    if (rs->bmp_h < 0) {
                        // vertical flip
                        if (rs->bmp_w < 0) {
                            // horizontal flip
                            IM_SET_RGB565_PIXEL(img, (img->w - j - 1), i, pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, i, pixel);
                        }
                    } else {
                        if (rs->bmp_w < 0) {
                            IM_SET_RGB565_PIXEL(img, (img->w - j - 1), (img->h - i - 1), pixel);
                        } else {
                            IM_SET_RGB565_PIXEL(img, j, (img->h - i - 1), pixel);
                        }
                    }
                }
            }
            for (int j = 0, jj = rs->bmp_row_bytes % 3; j < jj; j++) {
                file_read(fp, NULL, 1);
            }
        }
    }
}

void bmp_read(image_t *img, const char *path) {
    FIL fp;
    bmp_read_settings_t rs;
    file_open(&fp, path, true, FA_READ | FA_OPEN_EXISTING);
    bmp_read_geometry(&fp, img, path, &rs);
    if (!img->pixels) {
        image_xalloc(img, img->w * img->h * img->bpp);
    }
    bmp_read_pixels(&fp, img, img->h, &rs);
    file_close(&fp);
}

static void bmp_write_header(FIL *fp, uint32_t header_size, uint32_t data_size,
                             uint32_t bpp, uint32_t ncomp, rectangle_t *r) {
    // File Header (14 bytes)
    file_write(fp, "BM", 2);
    file_write_long(fp, 14 + 40 + header_size + data_size);
    file_write_long(fp, 0);
    file_write_long(fp, 14 + 40 + header_size);
    // Info Header (40 bytes)
    file_write_long(fp, 40);
    file_write_long(fp, r->w);
    file_write_long(fp, -r->h); // store the image flipped (correctly)
    file_write_short(fp, 1);
    file_write_short(fp, bpp);
    file_write_long(fp, ncomp);
    file_write_long(fp, data_size);
    file_write(fp, (uint8_t [16]) {0}, 16);
}

void bmp_write_subimg(image_t *img, const char *path, rectangle_t *r) {
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("No intersection!"));
    }

    FIL fp;
    file_open(&fp, path, true, FA_WRITE | FA_CREATE_ALWAYS);

    if (IM_IS_GS(img)) {
        const int row_bytes = (((rect.w * 8) + 31) / 32) * 4;
        const int data_size = (row_bytes * rect.h);
        const int waste = (row_bytes / sizeof(uint8_t)) - rect.w;
        // Write BMP file header
        bmp_write_header(&fp, 1024, data_size, 8, 0, &rect);
        // Write color Table (1024 bytes)
        for (int i = 0; i < 256; i++) {
            file_write_long(&fp, ((i) << 16) | ((i) << 8) | i);
        }
        if ((rect.x == 0) && (rect.w == img->w) && (img->w == row_bytes)) {
            file_write(&fp, img->pixels + (rect.y * img->w), rect.w * rect.h);
        } else {
            for (int i = 0; i < rect.h; i++) {
                file_write(&fp, img->pixels + ((rect.y + i) * img->w) + rect.x, rect.w);
                for (int j = 0; j < waste; j++) {
                    file_write_byte(&fp, 0);
                }
            }
        }
    } else {
        const int row_bytes = (((rect.w * 16) + 31) / 32) * 4;
        const int data_size = (row_bytes * rect.h);
        const int waste = (row_bytes / sizeof(uint16_t)) - rect.w;
        // Write BMP file header
        bmp_write_header(&fp, 12, data_size, 16, 3, &rect);
        // Write Bit Masks (12 bytes)
        file_write_long(&fp, 0x1F << 11);
        file_write_long(&fp, 0x3F << 5);
        file_write_long(&fp, 0x1F);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                file_write_short(&fp, IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i)));
            }
            for (int j = 0; j < waste; j++) {
                file_write_short(&fp, 0);
            }
        }
    }
    file_close(&fp);
}
#endif //IMLIB_ENABLE_IMAGE_FILE_IO
