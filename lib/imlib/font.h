/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
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
 * TrueType font loader, rasterizer and text layout.
 *
 * Reads the glyf/loca/cmap/hmtx/hhea/head/maxp tables of a TrueType font straight
 * out of memory (ROMFS mapped flash or a RAM copy), decodes quadratic outlines,
 * including composite glyphs, and rasterizes them with exact signed-area coverage
 * (no hinting, no supersampling). Text is laid out into an 8-bit coverage image
 * that the caller composites with imlib_draw_image(). Arabic is shaped here too:
 * letters take their contextual form from the font's Unicode presentation forms
 * and Arabic runs are reordered right to left, so callers pass logical order.
 */
#ifndef __FONT_H__
#define __FONT_H__
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct image;

// Errors are returned as codes, never raised, so this file has no runtime dependencies
// beyond the allocator; the caller owns the messages. FONT_OK is zero.
typedef enum font_err {
    FONT_OK = 0,
    FONT_ERR_NOT_TRUETYPE,      // Not an sfnt with TrueType outlines.
    FONT_ERR_COLLECTION,        // .ttc
    FONT_ERR_CFF,               // .otf with CFF outlines
    FONT_ERR_TABLE_BOUNDS,      // A table lies outside the file.
    FONT_ERR_TABLE_MISSING,     // A required table is absent or too short.
    FONT_ERR_UNITS,             // unitsPerEm is zero.
    FONT_ERR_NO_GLYPHS,
    FONT_ERR_METRICS,           // Bad numberOfHMetrics.
    FONT_ERR_NO_CMAP,           // No Unicode cmap subtable in format 4 or 12.
    FONT_ERR_GLYPH,             // A glyph record is corrupt.
} font_err_t;

// A typeface at a size: everything needed to measure and draw text with it.
typedef struct font {
    const uint8_t *data;    // The whole .ttf file.
    uint32_t size;          // Of the file, in bytes.
    float px;               // Em size in pixels.
    uint32_t loca;          // Table offsets into data.
    uint32_t glyf;
    uint32_t glyf_len;
    uint32_t hmtx;
    uint32_t cmap;          // Offset of the selected cmap subtable.
    uint16_t units_per_em;
    uint16_t num_glyphs;
    uint16_t num_hmetrics;
    uint16_t max_points;    // Points in the largest simple glyph.
    uint16_t max_contours;
    int16_t ascender;       // Font units.
    int16_t descender;
    int16_t line_gap;
    uint8_t loca_long;      // 0 = 16-bit loca entries, 1 = 32-bit.
    uint8_t cmap_format;    // 4 or 12.
    const struct font *fallback;  // Font tried for characters this one lacks (NULL = none).
} font_t;

// Pixel layout of a string relative to the origin: the top of the text, and the left edge,
// centre or right edge of its lines, depending on the alignment.
typedef struct font_box {
    int x;          // Coverage rectangle: the union of the layout box and all glyph ink.
    int y;          // y is <= 0, and so is x for left aligned text (ink may overhang the origin).
    int w;
    int h;
    int baseline;   // y of the first baseline relative to the origin.
} font_box_t;

// Widest outline, in pixels.
#define FONT_STROKE_MAX     (32)

// Which part of each line of text sits at the origin's x.
typedef enum font_align {
    FONT_ALIGN_LEFT = 0,    // Its left edge.
    FONT_ALIGN_CENTER,      // Its centre.
    FONT_ALIGN_RIGHT,       // Its right edge.
} font_align_t;

// How a string is laid out. Zero everywhere but scale is plain left aligned text.
typedef struct font_layout {
    float scale;            // Multiplies the font's size (1 = as loaded).
    font_align_t align;
    int wrap_width;         // Lines are broken to fit in this many pixels (0 = only at line breaks).
    int stroke;             // Pixels of outline around every glyph (0 = none), up to FONT_STROKE_MAX.
} font_layout_t;

// Line advance in pixels at the font's size; text is laid out with this rounded to a whole pixel.
static inline float font_line_height(const font_t *f) {
    return (f->ascender - f->descender + f->line_gap) * (f->px / f->units_per_em);
}

// Parse and validate a TrueType font held in memory, to be drawn at px pixels per em.
// Returns an error for an unsupported or corrupt font. The memory must outlive the font_t.
font_err_t font_load(font_t *f, const void *data, size_t size, float px);

// True if the font, or a font it falls back to, has a glyph for every character of a UTF-8
// string (line breaks and control characters aside), so that none would draw as a box.
bool font_has(const font_t *f, const char *str);

// Lay out a UTF-8 string without rasterizing. On error the box is zeroed.
font_err_t font_measure(const font_t *f, const char *str, const font_layout_t *layout, font_box_t *box);

// Rasterize a UTF-8 string into a GRAYSCALE coverage image of box->w x box->h pixels
// (box from font_measure with the same arguments). The image is cleared first.
//
// Without an outline a pixel is the text's coverage. With one, both the outline and the text
// are in the one image: 0-127 is the coverage of the outline where there is no text (the
// outer edge), and 128-255 is how much of a pixel that the outline covers fully is text
// (the inner edge and the text's interior). A color palette and an alpha palette indexed by
// the pixel then draw both colors in one pass, see imlib_draw_string().
font_err_t font_render(const font_t *f, const char *str, const font_layout_t *layout, const font_box_t *box,
                       struct image *out);

#endif // __FONT_H__
