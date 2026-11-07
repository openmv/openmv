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
 * Font data.
 */
#ifndef __FONT_H__
#define __FONT_H__
#include <stdint.h>

typedef enum {
    FONT_DEFAULT = 0,
    FONT_8X10    = FONT_DEFAULT,
    FONT_12X16,
    FONT_LARGE = FONT_12X16,
    FONT_COUNT
} font_type_t;

typedef struct {
    uint8_t w;
    uint8_t h;
    // Bytes per glyph row. Glyph data is packed MSB-first within each byte.
    uint8_t row_stride;
    const uint8_t *data;
} glyph_t;

typedef struct {
    uint8_t first_char;
    uint8_t last_char;
    uint8_t line_height;
    uint8_t space_width;
    const glyph_t *glyphs;
} bitmap_font_t;

const bitmap_font_t *imlib_font_get(int font);
#endif // __FONT_H__
