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
 * A simple GIF encoder.
 */
#include "imlib.h"
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)

#include "fb_alloc.h"
#include "file_utils.h"
#define BLOCK_SIZE    (126) // (2^7) - 2 // (DO NOT CHANGE!)

void gif_open(FIL *fp, int width, int height, bool color, bool loop) {
    file_buffer_on(fp);

    file_write(fp, "GIF89a", 6);
    file_write(fp, (uint16_t []) {width, height}, 4);
    file_write(fp, (uint8_t []) {0xF6, 0x00, 0x00}, 3);

    if (color) {
        for (int i = 0; i < 128; i++) {
            int red = ((((i & 0x60) >> 5) * 255) + 1.5) / 3;
            int green = ((((i & 0x1C) >> 2) * 255) + 3.5) / 7;
            int blue = (((i & 0x3) * 255) + 1.5) / 3;
            file_write(fp, (uint8_t []) {red, green, blue}, 3);
        }
    } else {
        for (int i = 0; i < 128; i++) {
            int gray = ((i * 255) + 63.5) / 127;
            file_write(fp, (uint8_t []) {gray, gray, gray}, 3);
        }
    }

    if (loop) {
        file_write(fp, (uint8_t []) {'!', 0xFF, 0x0B}, 3);
        file_write(fp, "NETSCAPE2.0", 11);
        file_write(fp, (uint8_t []) {0x03, 0x01, 0x00, 0x00, 0x00}, 5);
    }

    file_buffer_off(fp);
}

void gif_add_frame(FIL *fp, image_t *img, uint16_t delay) {
    file_buffer_on(fp);

    if (delay) {
        file_write(fp, (uint8_t []) {'!', 0xF9, 0x04, 0x04}, 4);
        file_write_short(fp, delay);
        file_write_short(fp, 0); // end
    }

    file_write_byte(fp, 0x2C);
    file_write_long(fp, 0);
    file_write(fp, (uint16_t []) {img->w, img->h}, 4);
    file_write(fp, (uint8_t []) {0x00, 0x07}, 2); // 7-bits

    int bytes = img->h * img->w;
    int blocks = (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (IM_IS_GS(img)) {
        for (int y = 0; y < blocks; y++) {
            int block_size = IM_MIN(BLOCK_SIZE, bytes - (y * BLOCK_SIZE));
            file_write_byte(fp, 1 + block_size);
            file_write_byte(fp, 0x80); // clear code
            for (int x = 0; x < block_size; x++) {
                file_write_byte(fp, img->pixels[(y * BLOCK_SIZE) + x] >> 1);
            }
        }
    } else if (IM_IS_RGB565(img)) {
        for (int y = 0; y < blocks; y++) {
            int block_size = IM_MIN(BLOCK_SIZE, bytes - (y * BLOCK_SIZE));
            file_write_byte(fp, 1 + block_size);
            file_write_byte(fp, 0x80); // clear code
            for (int x = 0; x < block_size; x++) {
                uint16_t pixel = ((uint16_t *) img->pixels)[(y * BLOCK_SIZE) + x];
                uint16_t r = COLOR_RGB565_TO_R5(pixel) >> 3;
                uint16_t g = COLOR_RGB565_TO_G6(pixel) >> 3;
                uint16_t b = COLOR_RGB565_TO_B5(pixel) >> 3;
                file_write_byte(fp, (r << 5) | (g << 2) | b);
            }
        }
    } else if (img->is_bayer || img->is_yuv) {
        for (int y = 0; y < blocks; y++) {
            int block_size = IM_MIN(BLOCK_SIZE, bytes - (y * BLOCK_SIZE));
            file_write_byte(fp, 1 + block_size);
            file_write_byte(fp, 0x80); // clear code
            uint16_t pixels[block_size];
            if (img->is_bayer) {
                imlib_debayer_line(0, block_size, y, pixels, PIXFORMAT_RGB565, img);
            } else {
                imlib_deyuv_line(0, block_size, y, pixels, PIXFORMAT_RGB565, img);
            }
            for (int x = 0; x < block_size; x++) {
                uint16_t pixel = pixels[2];
                uint16_t r = COLOR_RGB565_TO_R5(pixel) >> 3;
                uint16_t g = COLOR_RGB565_TO_G6(pixel) >> 3;
                uint16_t b = COLOR_RGB565_TO_B5(pixel) >> 3;
                file_write_byte(fp, (r << 5) | (g << 2) | b);
            }
        }
    }

    file_write(fp, (uint8_t []) {0x01, 0x81, 0x00}, 3); // end code

    file_buffer_off(fp);
}

void gif_close(FIL *fp) {
    file_write_byte(fp, ';');
    file_close(fp);
}
#endif //IMLIB_ENABLE_IMAGE_FILE_IO
