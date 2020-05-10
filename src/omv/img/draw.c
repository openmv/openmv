/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Basic drawing functions.
 */
#include "font.h"
#include "imlib.h"

void* imlib_compute_row_ptr(const image_t *img, int y) {
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            return IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
        }
        case IMAGE_BPP_GRAYSCALE: {
            return IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
        }
        case IMAGE_BPP_RGB565: {
            return IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
        }
        default: {
            // This shouldn't happen, at least we return a valid memory block
            return img->data;
        }
    }
}

inline int imlib_get_pixel_fast(int img_bpp, const void *row_ptr, int x)
{
    switch(img_bpp) {
        case IMAGE_BPP_BINARY: {
            return IMAGE_GET_BINARY_PIXEL_FAST((uint32_t*)row_ptr, x);
        }
        case IMAGE_BPP_GRAYSCALE: {
            return IMAGE_GET_GRAYSCALE_PIXEL_FAST((uint8_t*)row_ptr, x);
        }
        case IMAGE_BPP_RGB565: {
            return IMAGE_GET_RGB565_PIXEL_FAST((uint16_t*)row_ptr, x);
        }
        default: {
            return -1;
        }
    }
}


// Set pixel (handles boundary check and image type check).
void imlib_set_pixel(image_t *img, int x, int y, int p)
{
    if ((0 <= x) && (x < img->w) && (0 <= y) && (y < img->h)) {
        switch(img->bpp) {
            case IMAGE_BPP_BINARY: {
                IMAGE_PUT_BINARY_PIXEL(img, x, y, p);
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                IMAGE_PUT_GRAYSCALE_PIXEL(img, x, y, p);
                break;
            }
            case IMAGE_BPP_RGB565: {
                IMAGE_PUT_RGB565_PIXEL(img, x, y, p);
                break;
            }
            default: {
                break;
            }
        }
    }
}

// https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
static void point_fill(image_t *img, int cx, int cy, int r0, int r1, int c)
{
    for (int y = r0; y <= r1; y++) {
        for (int x = r0; x <= r1; x++) {
            if (((x * x) + (y * y)) <= (r0 * r0)) {
                imlib_set_pixel(img, cx + x, cy + y, c);
            }
        }
    }
}

// https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c, int thickness)
{
    if (thickness > 0) {
        int thickness0 = (thickness - 0) / 2;
        int thickness1 = (thickness - 1) / 2;
        int dx = abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
        int dy = abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
        int err = ((dx > dy) ? dx : -dy) / 2;

        for (;;) {
            point_fill(img, x0, y0, -thickness0, thickness1, c);
            if ((x0 == x1) && (y0 == y1)) break;
            int e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 <  dy) { err += dx; y0 += sy; }
        }
    }
}

static void xLine(image_t *img, int x1, int x2, int y, int c)
{
    while (x1 <= x2) imlib_set_pixel(img, x1++, y, c);
}

static void yLine(image_t *img, int x, int y1, int y2, int c)
{
    while (y1 <= y2) imlib_set_pixel(img, x, y1++, c);
}

void imlib_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c, int thickness, bool fill)
{
    if (fill) {

        for (int y = ry, yy = ry + rh; y < yy; y++) {
            for (int x = rx, xx = rx + rw; x < xx; x++) {
                imlib_set_pixel(img, x, y, c);
            }
        }

    } else if (thickness > 0) {
        int thickness0 = (thickness - 0) / 2;
        int thickness1 = (thickness - 1) / 2;

        for (int i = rx - thickness0, j = rx + rw + thickness1, k = ry + rh - 1; i < j; i++) {
            yLine(img, i, ry - thickness0, ry + thickness1, c);
            yLine(img, i, k - thickness0, k + thickness1, c);
        }

        for (int i = ry - thickness0, j = ry + rh + thickness1, k = rx + rw - 1; i < j; i++) {
            xLine(img, rx - thickness0, rx + thickness1, i, c);
            xLine(img, k - thickness0, k + thickness1, i, c);
        }
    }
}

// https://stackoverflow.com/questions/27755514/circle-with-thickness-drawing-algorithm
void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c, int thickness, bool fill)
{
    if (fill) {
        point_fill(img, cx, cy, -r, r, c);
    } else if (thickness > 0) {
        int thickness0 = (thickness - 0) / 2;
        int thickness1 = (thickness - 1) / 2;

        int xo = r + thickness0;
        int xi = IM_MAX(r - thickness1, 0);
        int xi_tmp = xi;
        int y = 0;
        int erro = 1 - xo;
        int erri = 1 - xi;

        while(xo >= y) {
            xLine(img, cx + xi, cx + xo, cy + y,  c);
            yLine(img, cx + y,  cy + xi, cy + xo, c);
            xLine(img, cx - xo, cx - xi, cy + y,  c);
            yLine(img, cx - y,  cy + xi, cy + xo, c);
            xLine(img, cx - xo, cx - xi, cy - y,  c);
            yLine(img, cx - y,  cy - xo, cy - xi, c);
            xLine(img, cx + xi, cx + xo, cy - y,  c);
            yLine(img, cx + y,  cy - xo, cy - xi, c);

            y++;

            if (erro < 0) {
                erro += 2 * y + 1;
            } else {
                xo--;
                erro += 2 * (y - xo + 1);
            }

            if (y > xi_tmp) {
                xi = y;
            } else {
                if (erri < 0) {
                    erri += 2 * y + 1;
                } else {
                    xi--;
                    erri += 2 * (y - xi + 1);
                }
            }
        }
    }
}

// https://scratch.mit.edu/projects/50039326/
static void scratch_draw_pixel(image_t *img, int x0, int y0, int dx, int dy, float shear_dx, float shear_dy, int r0, int r1, int c)
{
    point_fill(img, x0 + dx, y0 + dy + fast_floorf((dx * shear_dy) / shear_dx), r0, r1, c);
}

// https://scratch.mit.edu/projects/50039326/
static void scratch_draw_line(image_t *img, int x0, int y0, int dx, int dy0, int dy1, float shear_dx, float shear_dy, int c)
{
    int y = y0 + fast_floorf((dx * shear_dy) / shear_dx);
    yLine(img, x0 + dx, y + dy0, y + dy1, c);
}

// https://scratch.mit.edu/projects/50039326/
static void scratch_draw_sheared_ellipse(image_t *img, int x0, int y0, int width, int height, bool filled, float shear_dx, float shear_dy, int c, int thickness)
{
    int thickness0 = (thickness - 0) / 2;
    int thickness1 = (thickness - 1) / 2;
    if (((thickness > 0) || filled) && (shear_dx != 0)) {
        int a_squared = width * width;
        int four_a_squared = a_squared * 4;
        int b_squared = height * height;
        int four_b_squared = b_squared * 4;

        int x = 0;
        int y = height;
        int sigma = (2 * b_squared) + (a_squared * (1 - (2 * height)));

        while ((b_squared * x) <= (a_squared * y)) {
            if (filled) {
                scratch_draw_line(img, x0, y0, x, -y, y, shear_dx, shear_dy, c);
                scratch_draw_line(img, x0, y0, -x, -y, y, shear_dx, shear_dy, c);
            } else {
                scratch_draw_pixel(img, x0, y0, x, y, shear_dx, shear_dy, -thickness0, thickness1, c);
                scratch_draw_pixel(img, x0, y0, -x, y, shear_dx, shear_dy, -thickness0, thickness1, c);
                scratch_draw_pixel(img, x0, y0, x, -y, shear_dx, shear_dy, -thickness0, thickness1, c);
                scratch_draw_pixel(img, x0, y0, -x, -y, shear_dx, shear_dy, -thickness0, thickness1, c);
            }

            if (sigma >= 0) {
                sigma += four_a_squared * (1 - y);
                y -= 1;
            }

            sigma += b_squared * ((4 * x) + 6);
            x += 1;
        }

        x = width;
        y = 0;
        sigma = (2 * a_squared) + (b_squared * (1 - (2 * width)));

        while ((a_squared * y) <= (b_squared * x)) {
            if (filled) {
                scratch_draw_line(img, x0, y0, x, -y, y, shear_dx, shear_dy, c);
                scratch_draw_line(img, x0, y0, -x, -y, y, shear_dx, shear_dy, c);
            } else {
                scratch_draw_pixel(img, x0, y0, x, y, shear_dx, shear_dy, -thickness0, thickness1, c);
                scratch_draw_pixel(img, x0, y0, -x, y, shear_dx, shear_dy, -thickness0, thickness1, c);
                scratch_draw_pixel(img, x0, y0, x, -y, shear_dx, shear_dy, -thickness0, thickness1, c);
                scratch_draw_pixel(img, x0, y0, -x, -y, shear_dx, shear_dy, -thickness0, thickness1, c);
            }

            if (sigma >= 0) {
                sigma += four_b_squared * (1 - x);
                x -= 1;
            }

            sigma += a_squared * ((4 * y) + 6);
            y += 1;
        }
    }
}

// https://scratch.mit.edu/projects/50039326/
static void scratch_draw_rotated_ellipse(image_t *img, int x, int y, int x_axis, int y_axis, int rotation, bool filled, int c, int thickness)
{
    if ((x_axis > 0) && (y_axis > 0)) {
        if ((x_axis == y_axis) || (rotation == 0)) {
            scratch_draw_sheared_ellipse(img, x, y, x_axis / 2, y_axis / 2, filled, 1, 0, c, thickness);
        } else if (rotation == 90) {
            scratch_draw_sheared_ellipse(img, x, y, y_axis / 2, x_axis / 2, filled, 1, 0, c, thickness);
        } else {

            // Avoid rotations above 90.
            if (rotation > 90) {
                rotation -= 90;
                int temp = x_axis;
                x_axis = y_axis;
                y_axis = temp;
            }

            // Avoid rotations above 45.
            if (rotation > 45) {
                rotation -= 90;
                int temp = x_axis;
                x_axis = y_axis;
                y_axis = temp;
            }

            float theta = fast_atanf(IM_DIV(y_axis, x_axis) * (-tanf(IM_DEG2RAD(rotation))));
            float shear_dx = (x_axis * cosf(theta) * cosf(IM_DEG2RAD(rotation))) - (y_axis * sinf(theta) * sinf(IM_DEG2RAD(rotation)));
            float shear_dy = (x_axis * cosf(theta) * sinf(IM_DEG2RAD(rotation))) + (y_axis * sinf(theta) * cosf(IM_DEG2RAD(rotation)));
            float shear_x_axis = fast_fabsf(shear_dx);
            float shear_y_axis = IM_DIV((y_axis * x_axis), shear_x_axis);
            scratch_draw_sheared_ellipse(img, x, y, fast_floorf(shear_x_axis / 2), fast_floorf(shear_y_axis / 2), filled, shear_dx, shear_dy, c, thickness);
        }
    }
}

void imlib_draw_ellipse(image_t *img, int cx, int cy, int rx, int ry, int rotation, int c, int thickness, bool fill)
{
    int r = rotation % 180;
    if (r < 0) r += 180;

    scratch_draw_rotated_ellipse(img, cx, cy, rx * 2, ry * 2, r, fill, c, thickness);
}

// char rotation == 0, 90, 180, 360, etc.
// string rotation == 0, 90, 180, 360, etc.
void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c, float scale, int x_spacing, int y_spacing, bool mono_space,
                       int char_rotation, bool char_hmirror, bool char_vflip, int string_rotation, bool string_hmirror, bool string_vflip)
{
    char_rotation %= 360;
    if (char_rotation < 0) char_rotation += 360;
    char_rotation = (char_rotation / 90) * 90;

    string_rotation %= 360;
    if (string_rotation < 0) string_rotation += 360;
    string_rotation = (string_rotation / 90) * 90;

    bool char_swap_w_h = (char_rotation == 90) || (char_rotation == 270);
    bool char_upsidedown = (char_rotation == 180) || (char_rotation == 270);

    if (string_hmirror) x_off -= fast_floorf(font[0].w * scale) - 1;
    if (string_vflip) y_off -= fast_floorf(font[0].h * scale) - 1;

    int org_x_off = x_off;
    int org_y_off = y_off;
    const int anchor = x_off;

    for(char ch, last = '\0'; (ch = *str); str++, last = ch) {

        if ((last == '\r') && (ch == '\n')) { // handle "\r\n" strings
            continue;
        }

        if ((ch == '\n') || (ch == '\r')) { // handle '\n' or '\r' strings
            x_off = anchor;
            y_off += (string_vflip ? -1 : +1) * (fast_floorf((char_swap_w_h ? font[0].w : font[0].h) * scale) + y_spacing); // newline height == space height
            continue;
        }

        if ((ch < ' ') || (ch > '~')) { // handle unknown characters
            continue;
        }

        const glyph_t *g = &font[ch - ' '];

        if (!mono_space) {
            // Find the first pixel set and offset to that.
            bool exit = false;

            if (!char_swap_w_h) {
                for (int x = 0, xx = g->w; x < xx; x++) {
                    for (int y = 0, yy = g->h; y < yy; y++) {
                        if (g->data[(char_upsidedown ^ char_vflip) ? (g->h - 1 - y) : y] &
                            (1 << ((char_upsidedown ^ char_hmirror ^ string_hmirror) ? x : (g->w - 1 - x)))) {
                            x_off += (string_hmirror ? +1 : -1) * fast_floorf(x * scale);
                            exit = true;
                            break;
                        }
                    }

                    if (exit) break;
                }
            } else {
                for (int y = g->h - 1; y >= 0; y--) {
                    for (int x = 0, xx = g->w; x < xx; x++) {
                        if (g->data[(char_upsidedown ^ char_vflip) ? (g->h - 1 - y) : y] &
                            (1 << ((char_upsidedown ^ char_hmirror ^ string_hmirror) ? x : (g->w - 1 - x)))) {
                            x_off += (string_hmirror ? +1 : -1) * fast_floorf((g->h - 1 - y) * scale);
                            exit = true;
                            break;
                        }
                    }

                    if (exit) break;
                }
            }
        }

        for (int y = 0, yy = fast_floorf(g->h * scale); y < yy; y++) {
            for (int x = 0, xx = fast_floorf(g->w * scale); x < xx; x++) {
                if (g->data[fast_floorf(y / scale)] & (1 << (g->w - 1 - fast_floorf(x / scale)))) {
                    int16_t x_tmp = x_off + (char_hmirror ? (xx - x - 1) : x), y_tmp = y_off + (char_vflip ? (yy - y - 1) : y);
                    point_rotate(x_tmp, y_tmp, IM_DEG2RAD(char_rotation), x_off + (xx / 2), y_off + (yy / 2), &x_tmp, &y_tmp);
                    point_rotate(x_tmp, y_tmp, IM_DEG2RAD(string_rotation), org_x_off, org_y_off, &x_tmp, &y_tmp);
                    imlib_set_pixel(img, x_tmp, y_tmp, c);
                }
            }
        }

        if (mono_space) {
            x_off += (string_hmirror ? -1 : +1) * (fast_floorf((char_swap_w_h ? g->h : g->w) * scale) + x_spacing);
        } else {
            // Find the last pixel set and offset to that.
            bool exit = false;

            if (!char_swap_w_h) {
                for (int x = g->w - 1; x >= 0; x--) {
                    for (int y = g->h - 1; y >= 0; y--) {
                        if (g->data[(char_upsidedown ^ char_vflip) ? (g->h - 1 - y) : y] &
                            (1 << ((char_upsidedown ^ char_hmirror ^ string_hmirror) ? x : (g->w - 1 - x)))) {
                            x_off += (string_hmirror ? -1 : +1) * (fast_floorf((x + 2) * scale) + x_spacing);
                            exit = true;
                            break;
                        }
                    }

                    if (exit) break;
                }
            } else {
                for (int y = 0, yy = g->h; y < yy; y++) {
                    for (int x = g->w - 1; x >= 0; x--) {
                        if (g->data[(char_upsidedown ^ char_vflip) ? (g->h - 1 - y) : y] &
                            (1 << ((char_upsidedown ^ char_hmirror ^ string_hmirror) ? x : (g->w - 1 - x)))) {
                            x_off += (string_hmirror ? -1 : +1) * (fast_floorf(((g->h - 1 - y) + 2) * scale) + x_spacing);
                            exit = true;
                            break;
                        }
                    }

                    if (exit) break;
                }
            }

            if (!exit) x_off += (string_hmirror ? -1 : +1) * fast_floorf(scale * 3); // space char
        }
    }
}

static int safe_map_pixel(int dst_bpp, int src_bpp, int pixel)
{
    switch (dst_bpp) {
        case IMAGE_BPP_BINARY: {
            switch (src_bpp) {
                case IMAGE_BPP_BINARY: {
                    return pixel;
                }
                case IMAGE_BPP_GRAYSCALE: {
                    return COLOR_GRAYSCALE_TO_BINARY(pixel);
                }
                case IMAGE_BPP_RGB565: {
                    return COLOR_RGB565_TO_BINARY(pixel);
                }
                default: {
                    return 0;
                }
            }
        }
        case IMAGE_BPP_GRAYSCALE: {
            switch (src_bpp) {
                case IMAGE_BPP_BINARY: {
                    return COLOR_BINARY_TO_GRAYSCALE(pixel);
                }
                case IMAGE_BPP_GRAYSCALE: {
                    return pixel;
                }
                case IMAGE_BPP_RGB565: {
                    return COLOR_RGB565_TO_GRAYSCALE(pixel);
                }
                default: {
                    return 0;
                }
            }
        }
        case IMAGE_BPP_RGB565: {
            switch (src_bpp) {
                case IMAGE_BPP_BINARY: {
                    return COLOR_BINARY_TO_RGB565(pixel);
                }
                case IMAGE_BPP_GRAYSCALE: {
                    return COLOR_GRAYSCALE_TO_RGB565(pixel);
                }
                case IMAGE_BPP_RGB565: {
                    return pixel;
                }
                default: {
                    return 0;
                }
            }
        }
        default: {
            return 0;
        }
    }
}

/**
 * Blend two RGB888 format pixels using alpha.
 * NOTE:
 *   Interpolating RGB is not a good way of blending colors as it can generate colors that aren't in the original image.
 *   It's better to blend by transforming to another color space then interpolate, but that may slow things down.
 *   We could implement a better blend at a later date using a hint like image.BLEND_USING_HSV.
 *
 * @param background_pixel Background pixel value in RGB888
 * @param foreground_pixel Foreground pixel value in RGB888
 * @param alpha Foreground alpha 0->128
 * @param alpha_complement 128 minues Foreground alpha
 * @return Blended pixel in RGB888 format
 */
uint32_t draw_blendop_rgb888(uint32_t background_pixel, uint32_t foreground_pixel, uint32_t alpha, uint32_t alpha_complement)
{
    // rrrrrrrrggggggggbbbbbbbb
    uint32_t frb = foreground_pixel & 0xFF00FF;
    // rrrrrrrr........bbbbbbbb
    uint32_t fg = (foreground_pixel >> 8) & 255;
    // ................gggggggg
    uint32_t brb = background_pixel & 0xFF00FF;
    // rrrrrrrr........bbbbbbbb
    uint32_t bg = (background_pixel >> 8) & 255;
    // ................gggggggg

    uint32_t rb = (frb * alpha + brb * alpha_complement) >> 7;
    uint32_t g = (fg * alpha + bg * alpha_complement) >> 7;

    return (rb & 0xFF00FF) + (g << 8);
}

/**
 * Scale an RGB565 format pixel returning an RGB888 result.
 *
 * @param pixel RGB565 pixel to scale.
 * @param scale Amount to scale 0->128
 * @return Scaled pixel as RGB888
 */
uint32_t draw_scaleop_RGB565_to_RGB888(uint32_t pixel, uint32_t scale)
{
    uint32_t vr = COLOR_RGB565_TO_R8(pixel);
    uint32_t vg = COLOR_RGB565_TO_G8(pixel);
    uint32_t vb = COLOR_RGB565_TO_B8(pixel);

    // Scale is 0->128 so we shift right 7
    uint32_t r = (vr * scale) >> 7;
    uint32_t g = (vg * scale) >> 7;
    uint32_t b = (vb * scale) >> 7;
    
    return (r << 16) + (g << 8) + b;
}

/**
 * Convert a pixel to binary.
 * Used by interpolation cache line methods to convert mask to bitmap.
 *
 * @param bpp Bits per pixel of pixel.
 * @param pixel Pixel value.
 * @return pixel in binary format.
 */
inline bool pixel_to_binary(int bpp, uint32_t pixel) {
    switch (bpp) {
        case IMAGE_BPP_BINARY: {
            return pixel;
        }
        case IMAGE_BPP_GRAYSCALE: {
            return COLOR_GRAYSCALE_TO_BINARY(pixel);
        }
        case IMAGE_BPP_RGB565: {
            return COLOR_RGB565_TO_BINARY(pixel);
        }
        default: {
            return false;
        }
    }
}

/**
 * Used by IMAGE_HINT_BILINEAR to generate a grayscale linear interpolated row.
 * The drawing algorithm will later apply the vertical interpolation between two cached lines.
 *
 * @param cache_line Where to write the line
 * @param alpha 0->128, alpha blending value for other image.
 * @param other_row_ptr Other source image row pointer.
 * @param other_bpp Other image bits per pixel.
 * @param mask_row_ptr Mask image row pointer.
 * @param mask_bpp Mask image bits per pixel.
 * @param other_x_start Start x pixel location in source/mask image.
 * @param other_x_end End x pixel (exclusive) location in source/mask image.
 * @param over_x_scale Scale from other scale to image scale.
 */
static void int_generate_cache_line_grayscale(uint16_t *cache_line, int alpha, uint8_t *other_row_ptr, int other_bpp, void *mask_row_ptr, int mask_bpp, int other_x_start, int other_x_end, float over_xscale, const uint8_t *alpha_palette)
{
    for (int i = 0, x = other_x_start; x < other_x_end; x++, i++) {
        float other_x_float = (x + 0.5) * over_xscale;
        uint32_t other_x = fast_floorf(other_x_float);
        uint32_t weight_x = fast_floorf((other_x_float - other_x) * alpha);
        bool mask1 = true, mask2 = true;

        if (mask_row_ptr) {
            mask1 = pixel_to_binary(mask_bpp, imlib_get_pixel_fast(mask_bpp, mask_row_ptr, other_x));
            mask2 = pixel_to_binary(mask_bpp, imlib_get_pixel_fast(mask_bpp, mask_row_ptr, other_x + 1));
        }

        uint32_t alpha1 = mask1 ? (alpha - weight_x) : 0;
        uint32_t alpha2 = mask2 ? weight_x : 0;
        uint32_t other_pixel1 = safe_map_pixel(IMAGE_BPP_GRAYSCALE, other_bpp, imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x));
        uint32_t other_pixel2 = safe_map_pixel(IMAGE_BPP_GRAYSCALE, other_bpp, imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x + 1));

        if (alpha_palette) {
            alpha1 = alpha1 * alpha_palette[other_pixel1] >> 8;
            alpha2 = alpha2 * alpha_palette[other_pixel2] >> 8;
        }

        other_pixel1 *= alpha1;
        other_pixel2 *= alpha2;
        
        // Image alpha is the remaining alpha after applying other alpha
        uint32_t img_alpha = 256 - (alpha1 + alpha2);

        // Note img_alpha is now 0->128 to fit into a byte
        cache_line[i] = ((other_pixel1 + other_pixel2) & 0xFF00) + (img_alpha >> 1);
    }
}


/**
 * Used by IMAGE_HINT_BILINEAR to generate a RGB888 linear interpolated row.
 * The drawing algorithm will later apply the vertical interpolation between two cached lines.
 *
 * @param cache_line Where to write the line
 * @param alpha 0->128, alpha blending value for other image.
 * @param other_row_ptr Other source image row pointer.
 * @param other_bpp Other image bits per pixel.
 * @param mask_row_ptr Mask image row pointer.
 * @param mask_bpp Mask image bits per pixel.
 * @param other_x_start Start x pixel location in source/mask image.
 * @param other_x_end End x pixel (exclusive) location in source/mask image.
 * @param over_x_scale Scale from other scale to image scale.
 */
static void int_generate_cache_line_rgb565(uint32_t *cache_line, int alpha, const uint16_t *other_row_ptr, int other_bpp, const void *mask_row_ptr, int mask_bpp, int other_x_start, int other_x_end, float over_xscale, const uint16_t *color_palette, const uint8_t *alpha_palette)
{
    // generate line
    for (int i = 0, x = other_x_start; x < other_x_end; x++, i++) {
        float other_x_float = (x + 0.5) * over_xscale;
        uint32_t other_x = fast_floorf(other_x_float);
        uint32_t weight_x = fast_floorf((other_x_float - other_x) * alpha);
        bool mask1 = true, mask2 = true;

        if (mask_row_ptr) {
            mask1 = pixel_to_binary(mask_bpp, imlib_get_pixel_fast(mask_bpp, mask_row_ptr, other_x));
            mask2 = pixel_to_binary(mask_bpp, imlib_get_pixel_fast(mask_bpp, mask_row_ptr, other_x + 1));
        }

        uint32_t alpha1 = mask1 ? (alpha - weight_x) : 0;
        uint32_t alpha2 = mask2 ? weight_x : 0;
        uint32_t other_pixel1 = imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x);
        uint32_t other_pixel2 = imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x + 1);

        if (alpha_palette) {
            alpha1 = alpha1 * alpha_palette[other_pixel1] >> 8;
            alpha2 = alpha2 * alpha_palette[other_pixel2] >> 8;
        }

        other_pixel1 = color_palette ? color_palette[other_pixel1] : safe_map_pixel(IMAGE_BPP_RGB565, other_bpp, other_pixel1);
        other_pixel1 = draw_scaleop_RGB565_to_RGB888(other_pixel1, alpha1);
        other_pixel2 = color_palette ? color_palette[other_pixel2] : safe_map_pixel(IMAGE_BPP_RGB565, other_bpp, other_pixel2);
        other_pixel2 = draw_scaleop_RGB565_to_RGB888(other_pixel2, alpha2);
        
        // Image alpha is the remaining alpha after applying other alpha
        uint32_t img_alpha = 128 - (alpha1 + alpha2);

        cache_line[i] = ((other_pixel1 + other_pixel2) << 8) + img_alpha; // RGBA8888
    }
}

/**
 * Draw an image onto another image converting format if necessary.
 * 
 * @param img The image to draw onto.
 * @param other The image to draw.
 * @param x_off X offset in destination.
 * @param y_off Y offset in destination.
 * @param x_scale X scale.
 * @param y_scale Y scale.
 * @param alpha Alpha, between 0 and 256 inclusive.
 * @param mask Mask image, if interpolating must be the same size as the other image.
 * @param color_palette Color palette for transforming grayscale images to RGB565.
 * @param alpha_palette Alpha palette for masking grayscale images.
 * @param hint Rendering hint.  e.g. IMAGE_HINT_BILINEAR, IMAGE_HINT_CENTER
 */
void imlib_draw_image(image_t *img, image_t *other, int x_off, int y_off, float x_scale, float y_scale, int alpha, image_t *mask, const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint)
{
    // If alpha is 0 then nothing changes
    if (alpha == 0) return;

    if (hint & IMAGE_HINT_BILINEAR) {
        // Cannot interpolate a 1x1 pixel.
        if (other->w <= 1 || other->h <= 1) hint &= ~IMAGE_HINT_BILINEAR;
    }

    // Scaled other size
    int other_width_scaled = fast_floorf(x_scale * other->w);
    int other_height_scaled = fast_floorf(y_scale * other->h);

    // Center other if hint is set
    if (hint & IMAGE_HINT_CENTER) {
        x_off -= other_width_scaled >> 1;
        y_off -= other_height_scaled >> 1;
    }

    // Scaler to convert from img scale to other scale
    float over_xscale = IM_DIV(1.0f, x_scale), over_yscale = IM_DIV(1.0f, y_scale);

    // Left or top of other is out of bounds
    int other_x_start = (x_off < 0) ? -x_off : 0;
    int other_y_start = (y_off < 0) ? -y_off : 0;

    // Right or bottom of image is out of bounds
    int other_x_end = (x_off + other_width_scaled >= img->w) ? img->w - x_off : other_width_scaled;
    int other_y_end = (y_off + other_height_scaled >= img->h) ? img->h - y_off : other_height_scaled;

    // Check bounds are within img
    if (other_x_start + x_off >= img->w || other_y_start + y_off >= img->h) return;
    if (other_x_end + x_off <= 0 || other_y_end + y_off <= 0) return;

    // If we're linear interpolating the last pixel will overflow if we land on it, we want to land just before it.
    if (hint & IMAGE_HINT_BILINEAR) {
        over_xscale *= (float)(other->w - 1) / other->w;
        over_yscale *= (float)(other->h - 1) / other->h;
    }

    const int img_bpp = img->bpp;
    const int other_bpp = other->bpp;
    const int mask_bpp = mask ? mask->bpp : 0;

    switch(img_bpp) {
        case IMAGE_BPP_BINARY: {
            // If alpha is less that 128 on a bitmap we're just copying the image back to the image, so do nothing
            if (alpha >= 128) {
                // Iterate the img area to be updated
                for (int y = other_y_start; y < other_y_end; y++) {
                    uint32_t *img_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y_off + y);
                    const int other_y = fast_floorf(y * over_yscale);
                    void *other_row_ptr = imlib_compute_row_ptr(other, other_y);

                    for (int x = other_x_start; x < other_x_end; x++) {
                        const int other_x = fast_floorf(x * over_xscale);

                        if (!mask || image_get_mask_pixel(mask, other_x, other_y)) {
                            uint32_t result_pixel = safe_map_pixel(IMAGE_BPP_BINARY, other_bpp, imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x));
                            
                            IMAGE_PUT_BINARY_PIXEL_FAST(img_row_ptr, x_off + x, result_pixel);
                        }
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            if (hint & IMAGE_HINT_BILINEAR) {
                fb_alloc_mark();

                // Allocate cache lines
                int bytes_per_img_line = img->w * sizeof(uint8_t) * 2; // (1 byte graysclae + 1 byte alpha) = * 2
                uint16_t *cache_line_1 = fb_alloc(bytes_per_img_line, FB_ALLOC_NO_HINT);
                uint16_t *cache_line_2 = fb_alloc(bytes_per_img_line, FB_ALLOC_NO_HINT);
                uint16_t *cache_line_top = cache_line_2;
                uint16_t *cache_line_bottom = cache_line_1;

                // Pre-fill cache for first drawn line
                int temp_other_y = fast_floorf(other_y_start * over_yscale);
                uint8_t *other_row_ptr = imlib_compute_row_ptr(other, temp_other_y);
                void *mask_row_ptr = mask ? imlib_compute_row_ptr(mask, temp_other_y) : NULL;

                int_generate_cache_line_grayscale(cache_line_bottom, alpha, other_row_ptr, other_bpp, mask_row_ptr, mask_bpp, other_x_start, other_x_end, over_xscale, alpha_palette);

                // Used to detect when other starts rendering from the next line
                int last_other_y = -1;

                // Iterate the img area to be updated
                for (int y = other_y_start; y < other_y_end; y++) {
                    // Pre-add x_off here to save adding it inside the central loop
                    uint8_t *img_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y_off + y) + x_off;

                    // calculate y offset in other
                    float other_y_float = (y + 0.5) * over_yscale;

                    // Calculate weighting between top and bottom pixel
                    int other_y = fast_floorf(other_y_float);
                    int weight_y = fast_floorf((other_y_float - other_y) * 256);
                    uint32_t y_interpolate = (weight_y << 16) + (256 - weight_y);

                    // If we've moved to the next line in the other image then generate the new cache line
                    if (last_other_y != other_y) {
                        last_other_y = other_y;
                        
                        // Move to next line.  Swap y+1 cache line to y
                        uint16_t *cache_line_temp = cache_line_top;
                        cache_line_top = cache_line_bottom;
                        cache_line_bottom = cache_line_temp;

                        // And generate a new y+1
                        other_row_ptr = imlib_compute_row_ptr(other, other_y + 1);
                        mask_row_ptr = mask ? imlib_compute_row_ptr(mask, other_y + 1) : NULL;
                        int_generate_cache_line_grayscale(cache_line_bottom, alpha, other_row_ptr, other_bpp, mask_row_ptr, mask_bpp, other_x_start, other_x_end, over_xscale, alpha_palette);
                    }

                    // Draw the line to img
                    for (int i = 0, x = other_x_start; x < other_x_end; x++, i++) {
                        // Pack pixel data for SMUADS
                        uint32_t pixel_data = (cache_line_bottom[i] << 16) | cache_line_top[i];

                        // Extract alpha for top + bottom pixel for SMUAD
                        uint32_t img_alpha_pixels = pixel_data & 0xFF00FF;

                        // Calculate the alpha weighting for the img pixel, don't unshift so we have more accuracy when combining
                        uint32_t img_alpha_15bits = __SMUAD(y_interpolate, img_alpha_pixels); // 8 bits x 7 bits = 15 bits

                        // Extract other pixels for SMUAD
                        uint32_t other_pixels = (pixel_data >> 8) & 0xFF00FF;

                        // Apply alpha, but don't unshift for more accuracy in combining
                        uint32_t pixel_16bits = __SMUAD(y_interpolate, other_pixels);

                        // Get img pixel.
                        uint8_t img_pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(img_row_ptr,  x);

                        // Combine img_pixel with other pixel, and shift fixed component
                        uint32_t pixel = (((img_pixel * img_alpha_15bits) >> 7) + pixel_16bits) >> 8;

                        // Store pixel
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(img_row_ptr, x, pixel);
                    }
                }
                // De-allocate cache lines
                fb_alloc_free_till_mark();
            } else {
                // 00000000otheralph00000000imgalpha
                uint32_t packed_alpha = (alpha << 16) + (256 - alpha);

                // Iterate the img area to be updated
                for (int y = other_y_start; y < other_y_end; y++) {
                    // Pre-add x_off here to save adding it inside the central loop
                    uint8_t *img_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y_off + y) + x_off;
                    int other_y = fast_floorf(y * over_yscale);
                    uint16_t *other_row_ptr = imlib_compute_row_ptr(other, other_y);

                    for (int x = other_x_start; x < other_x_end; x++) {
                        int other_x = fast_floorf(x * over_xscale);

                        if (!mask || image_get_mask_pixel(mask, other_x, other_y)) {
                            uint8_t result_pixel = safe_map_pixel(IMAGE_BPP_GRAYSCALE, other_bpp, imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x));

                            if (alpha_palette) {
                                uint32_t temp_alpha = (alpha * alpha_palette[result_pixel]) >> 8;
                                
                                packed_alpha = (temp_alpha << 16) + (256 - temp_alpha);
                            }

                            if (packed_alpha & 0x1ff) {
                                uint8_t img_pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(img_row_ptr,  x);
                                uint32_t vgs = (result_pixel << 16) + img_pixel;

                                result_pixel = __SMUAD(packed_alpha, vgs) >> 8;
                            }

                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(img_row_ptr, x, result_pixel);
                        }
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            // Alpha is 0->128
            alpha >>= 1;

            if (hint & IMAGE_HINT_BILINEAR) {
                fb_alloc_mark();

                // Allocate cache lines
                int bytes_per_img_line = img->w * 4; // (3 bytes RGB888 + 1 byte alpha) = * 4
                uint32_t *cache_line_1 = fb_alloc(bytes_per_img_line, FB_ALLOC_NO_HINT);
                uint32_t *cache_line_2 = fb_alloc(bytes_per_img_line, FB_ALLOC_NO_HINT);
                uint32_t *cache_line_top = cache_line_2;
                uint32_t *cache_line_bottom = cache_line_1;

                // Pre-fill cache for first drawn line
                int temp_other_y = fast_floorf(other_y_start * over_yscale);
                uint16_t *other_row_ptr = imlib_compute_row_ptr(other, temp_other_y);
                void *mask_row_ptr = mask ? imlib_compute_row_ptr(mask, temp_other_y) : NULL;

                int_generate_cache_line_rgb565(cache_line_bottom, alpha, other_row_ptr, other_bpp, mask_row_ptr, mask_bpp, other_x_start, other_x_end, over_xscale, color_palette, alpha_palette);

                // Used to detect when other starts rendering from the next line
                int last_other_y = -1;

                // Iterate the img area to be updated
                for (int y = other_y_start; y < other_y_end; y++) {
                    // Pre-add x_off here to save adding it inside the central loop
                    uint16_t *img_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y_off + y) + x_off;

                    // calculate y offset in other
                    float other_y_float = (y + 0.5) * over_yscale;

                    // Calculate weighting between top and bottom pixel
                    int other_y = fast_floorf(other_y_float);
                    int weight_y = fast_floorf((other_y_float - other_y) * 256);
                    uint32_t y_interpolate = ((256 - weight_y) << 16) + weight_y;

                    // Weighting is 0->128 for blendops to prevent overflow
                    weight_y >>= 1;

                    // If the scale is negative the pixels come in reverse order, so reverse the remainder weighting
                    int weight_y_complement = 128 - weight_y;
                    
                    if (last_other_y != other_y) {      
                        uint32_t *cache_line_temp = cache_line_top;
                        cache_line_top = cache_line_bottom;
                        cache_line_bottom = cache_line_temp;
                        
                        other_row_ptr = imlib_compute_row_ptr(other, other_y + 1);
                        mask_row_ptr = mask ? imlib_compute_row_ptr(mask, other_y + 1) : NULL;
                        int_generate_cache_line_rgb565(cache_line_bottom, alpha, other_row_ptr, other_bpp, mask_row_ptr, mask_bpp, other_x_start, other_x_end, over_xscale, color_palette, alpha_palette);

                        last_other_y = other_y;
                    }

                    for (int i = 0, x = other_x_start; x < other_x_end; x++, i++) {
                        uint32_t top = cache_line_top[i];
                        uint32_t bottom = cache_line_bottom[i];
                        uint32_t result_pixel = draw_blendop_rgb888(top >> 8, bottom >> 8, weight_y, weight_y_complement);

                        // Pack top and bottom img alpha for SMUAD
                        uint32_t img_alpha_top_bottom = ((top & 255) << 16) | (bottom & 255);

                        // Blend img alphas
                        uint32_t img_alpha = __SMUAD(y_interpolate, img_alpha_top_bottom) >> 8;

                        // if img alpha will have an effect
                        if (img_alpha) {
                            // Get img pixel
                            uint32_t img_pixel = IMAGE_GET_RGB565_PIXEL_FAST(img_row_ptr, x);

                            // Apply alpha to img pixel
                            img_pixel = draw_scaleop_RGB565_to_RGB888(img_pixel, img_alpha);

                            // Add to other pixel (which already had alpha applied in generate_line_cache)
                            result_pixel = img_pixel + result_pixel;
                        }

                        // Convert back to RGB565
                        result_pixel = COLOR_R5_G6_B5_TO_RGB565(result_pixel >> 19, (result_pixel >> 10) & 63, (result_pixel >> 3) & 63);
                        
                        // Store pixel
                        IMAGE_PUT_RGB565_PIXEL_FAST(img_row_ptr, x, result_pixel);
                    }
                }
                
                // De-allocate cache lines
                fb_alloc_free_till_mark();
            } else {
                uint32_t va = __PKHBT((128 - alpha), alpha, 16);

                // Iterate the img area to be updated
                for (int y = other_y_start; y < other_y_end; y++) {
                    // Pre-add x_off here to save adding it inside the central loop
                    uint16_t *img_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y_off + y) + x_off;
                    float other_y_float = y * over_yscale;
                    int other_y = fast_floorf(other_y_float);
                    uint16_t *other_row_ptr = imlib_compute_row_ptr(other, other_y);
                    
                    for (int x = other_x_start; x < other_x_end; x++) {
                        int other_x = fast_floorf(x * over_xscale);

                        if (!mask || image_get_mask_pixel(mask, other_x, other_y)) {
                            uint32_t result_pixel = imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x);
                            
                            if (alpha_palette) {
                                uint32_t temp_alpha = (alpha * alpha_palette[result_pixel]) >> 8;
                                
                                va = __PKHBT((128 - temp_alpha), temp_alpha, 16);
                            }
                            result_pixel = color_palette ? color_palette[result_pixel] : safe_map_pixel(IMAGE_BPP_RGB565, other_bpp, result_pixel);
                            
                            if (va & 0x1ff) {
                                // Blend img to other pixel
                                uint16_t img_pixel = IMAGE_GET_RGB565_PIXEL_FAST(img_row_ptr, x);
                                uint32_t r_ta = COLOR_RGB565_TO_R5(result_pixel);
                                uint32_t g_ta = COLOR_RGB565_TO_G6(result_pixel);
                                uint32_t b_ta = COLOR_RGB565_TO_B5(result_pixel);
                                uint32_t vr = __PKHBT(COLOR_RGB565_TO_R5(img_pixel), r_ta, 16);
                                uint32_t vg = __PKHBT(COLOR_RGB565_TO_G6(img_pixel), g_ta, 16);
                                uint32_t vb = __PKHBT(COLOR_RGB565_TO_B5(img_pixel), b_ta, 16);
                                uint32_t r = __SMUAD(va, vr) >> 7;
                                uint32_t g = __SMUAD(va, vg) >> 7;
                                uint32_t b = __SMUAD(va, vb) >> 7;

                                result_pixel = COLOR_R5_G6_B5_TO_RGB565(r, g, b);
                            }
                            IMAGE_PUT_RGB565_PIXEL_FAST(img_row_ptr, x, result_pixel);
                        }
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

#ifdef IMLIB_ENABLE_FLOOD_FILL
void imlib_flood_fill(image_t *img, int x, int y,
                      float seed_threshold, float floating_threshold,
                      int c, bool invert, bool clear_background, image_t *mask)
{
    if ((0 <= x) && (x < img->w) && (0 <= y) && (y < img->h)) {
        image_t out;
        out.w = img->w;
        out.h = img->h;
        out.bpp = IMAGE_BPP_BINARY;
        out.data = fb_alloc0(image_size(&out), FB_ALLOC_NO_HINT);

        if (mask) {
            for (int y = 0, yy = out.h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&out, y);
                for (int x = 0, xx = out.w; x < xx; x++) {
                    if (image_get_mask_pixel(mask, x, y)) IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x);
                }
            }
        }

        int color_seed_threshold = 0;
        int color_floating_threshold = 0;

        switch(img->bpp) {
            case IMAGE_BPP_BINARY: {
                color_seed_threshold = fast_floorf(seed_threshold * COLOR_BINARY_MAX);
                color_floating_threshold = fast_floorf(floating_threshold * COLOR_BINARY_MAX);
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                color_seed_threshold = fast_floorf(seed_threshold * COLOR_GRAYSCALE_MAX);
                color_floating_threshold = fast_floorf(floating_threshold * COLOR_GRAYSCALE_MAX);
                break;
            }
            case IMAGE_BPP_RGB565: {
                color_seed_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_floorf(seed_threshold * COLOR_R5_MAX),
                                                                fast_floorf(seed_threshold * COLOR_G6_MAX),
                                                                fast_floorf(seed_threshold * COLOR_B5_MAX));
                color_floating_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_floorf(floating_threshold * COLOR_R5_MAX),
                                                                    fast_floorf(floating_threshold * COLOR_G6_MAX),
                                                                    fast_floorf(floating_threshold * COLOR_B5_MAX));
                break;
            }
            default: {
                break;
            }
        }

        imlib_flood_fill_int(&out, img, x, y, color_seed_threshold, color_floating_threshold, NULL, NULL);

        switch(img->bpp) {
            case IMAGE_BPP_BINARY: {
                for (int y = 0, yy = out.h; y < yy; y++) {
                    uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                    uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&out, y);
                    for (int x = 0, xx = out.w; x < xx; x++) {
                        if (IMAGE_GET_BINARY_PIXEL_FAST(out_row_ptr, x) ^ invert) {
                            IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, c);
                        } else if (clear_background) {
                            IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, 0);
                        }
                    }
                }
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                for (int y = 0, yy = out.h; y < yy; y++) {
                    uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                    uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&out, y);
                    for (int x = 0, xx = out.w; x < xx; x++) {
                        if (IMAGE_GET_BINARY_PIXEL_FAST(out_row_ptr, x) ^ invert) {
                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, c);
                        } else if (clear_background) {
                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, 0);
                        }
                    }
                }
                break;
            }
            case IMAGE_BPP_RGB565: {
                for (int y = 0, yy = out.h; y < yy; y++) {
                    uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                    uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&out, y);
                    for (int x = 0, xx = out.w; x < xx; x++) {
                        if (IMAGE_GET_BINARY_PIXEL_FAST(out_row_ptr, x) ^ invert) {
                            IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, c);
                        } else if (clear_background) {
                            IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, 0);
                        }
                    }
                }
                break;
            }
            default: {
                break;
            }
        }

        fb_free();
    }
}
#endif // IMLIB_ENABLE_FLOOD_FILL
