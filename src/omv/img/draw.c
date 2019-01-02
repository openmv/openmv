/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "font.h"
#include "imlib.h"

// Get pixel (handles boundary check and image type check).
int imlib_get_pixel(image_t *img, int x, int y)
{
    if ((0 <= x) && (x < img->w) && (0 <= y) && (y < img->h)) {
        switch(img->bpp) {
            case IMAGE_BPP_BINARY: {
                return IMAGE_GET_BINARY_PIXEL(img, x, y);
            }
            case IMAGE_BPP_GRAYSCALE: {
                return IMAGE_GET_GRAYSCALE_PIXEL(img, x, y);
            }
            case IMAGE_BPP_RGB565: {
                return IMAGE_GET_RGB565_PIXEL(img, x, y);
            }
            default: {
                return -1;
            }
        }
    }

    return -1;
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
        if ((x_axis == y_axis) || ((rotation % 180) == 0)) {
            scratch_draw_sheared_ellipse(img, x, y, x_axis / 2, y_axis / 2, filled, 1, 0, c, thickness);
        } else if ((rotation % 180) == 90) {
            scratch_draw_sheared_ellipse(img, x, y, y_axis / 2, x_axis / 2, filled, 1, 0, c, thickness);
        } else {
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
    scratch_draw_rotated_ellipse(img, cx, cy, rx * 2, ry * 2, rotation, fill, c, thickness);
}

void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c, float scale, int x_spacing, int y_spacing, bool mono_space)
{
    const int anchor = x_off;

    for(char ch, last = '\0'; (ch = *str); str++, last = ch) {

        if ((last == '\r') && (ch == '\n')) { // handle "\r\n" strings
            continue;
        }

        if ((ch == '\n') || (ch == '\r')) { // handle '\n' or '\r' strings
            x_off = anchor;
            y_off += fast_roundf(font[0].h * scale) + y_spacing; // newline height == space height
            continue;
        }

        if ((ch < ' ') || (ch > '~')) { // handle unknown characters
            imlib_draw_rectangle(img,
                x_off + (fast_roundf(scale * 3) / 2),
                y_off + (fast_roundf(scale * 3) / 2),
                fast_roundf(font[0].w * scale) - ((fast_roundf(scale * 3) / 2) * 2),
                fast_roundf(font[0].h * scale) - ((fast_roundf(scale * 3) / 2) * 2),
                c, fast_roundf(scale), false);
            continue;
        }

        const glyph_t *g = &font[ch - ' '];

        if (!mono_space) {
            // Find the first pixel set and offset to that.
            bool exit = false;

            for (int x = 0, xx = g->w; x < xx; x++) {
                for (int y = 0, yy = g->h; y < yy; y++) {
                    if (g->data[y] & (1 << (g->w - 1 - x))) {
                        x_off -= fast_roundf(x * scale);
                        exit = true;
                        break;
                    }
                }

                if (exit) break;
            }
        }

        for (int y = 0, yy = fast_roundf(g->h * scale); y < yy; y++) {
            for (int x = 0, xx = fast_roundf(g->w * scale); x < xx; x++) {
                if (g->data[fast_roundf(y / scale)] & (1 << (g->w - 1 - fast_roundf(x / scale)))) {
                    imlib_set_pixel(img, (x_off + x), (y_off + y), c);
                }
            }
        }

        if (mono_space) {
            x_off += fast_roundf(g->w * scale) + x_spacing;
        } else {
            // Find the last pixel set and offset to that.
            bool exit = false;

            for (int x = g->w - 1; x >= 0; x--) {
                for (int y = g->h - 1; y >= 0; y--) {
                    if (g->data[y] & (1 << (g->w - 1 - x))) {
                        x_off += fast_roundf((x + 2) * scale) + x_spacing;
                        exit = true;
                        break;
                    }
                }

                if (exit) break;
            }

            if (!exit) x_off += fast_roundf(scale * 3); // space char
        }
    }
}

static int safe_map_pixel(image_t *dst, image_t *src, int pixel)
{
    switch (dst->bpp) {
        case IMAGE_BPP_BINARY: {
            switch (src->bpp) {
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
            switch (src->bpp) {
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
            switch (src->bpp) {
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

void imlib_draw_image(image_t *img, image_t *other, int x_off, int y_off, float x_scale, float y_scale, float alpha, image_t *mask)
{
    float over_xscale = IM_DIV(1.0, x_scale), over_yscale = IM_DIV(1.0f, y_scale), beta = 1 - alpha;

    for (int y = 0, yy = fast_roundf(other->h * y_scale); y < yy; y++) {
        int other_y = fast_roundf(y * over_yscale);

        for (int x = 0, xx = fast_roundf(other->w * x_scale); x < xx; x++) {
            int other_x = fast_roundf(x * over_xscale);

            if ((!mask) || image_get_mask_pixel(mask, other_x, other_y)) {
                int pixel = safe_map_pixel(img, other, imlib_get_pixel(other, other_x, other_y));
                imlib_set_pixel(img, x_off + x, y_off + y, (alpha == 1) ? pixel :
                                (pixel * alpha) + (imlib_get_pixel(img, x_off + x, y_off + y) * beta));
            }
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
        out.data = fb_alloc0(image_size(&out));

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
                color_seed_threshold = fast_roundf(seed_threshold * COLOR_BINARY_MAX);
                color_floating_threshold = fast_roundf(floating_threshold * COLOR_BINARY_MAX);
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                color_seed_threshold = fast_roundf(seed_threshold * COLOR_GRAYSCALE_MAX);
                color_floating_threshold = fast_roundf(floating_threshold * COLOR_GRAYSCALE_MAX);
                break;
            }
            case IMAGE_BPP_RGB565: {
                color_seed_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_roundf(seed_threshold * COLOR_R5_MAX),
                                                                fast_roundf(seed_threshold * COLOR_G6_MAX),
                                                                fast_roundf(seed_threshold * COLOR_B5_MAX));
                color_floating_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_roundf(floating_threshold * COLOR_R5_MAX),
                                                                    fast_roundf(floating_threshold * COLOR_G6_MAX),
                                                                    fast_roundf(floating_threshold * COLOR_B5_MAX));
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
