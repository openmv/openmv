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

void* imlib_compute_row_ptr(image_t *img, int y) {
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

inline int imlib_get_pixel_fast(int img_bpp, void *row_ptr, int x)
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

void imlib_draw_image(image_t *img, image_t *other, int x_off, int y_off, float x_scale, float y_scale, int alpha, image_t *mask, const uint16_t *color_palette)
{
    // Scaler to convert from img scale to other scale
    const float over_xscale = IM_DIV(1.0, x_scale), over_yscale = IM_DIV(1.0f, y_scale);

    // Packaed alpha for SMUAD calls
    const uint32_t va = (alpha << 16) + (256 - alpha);

    const int img_bpp = img->bpp;
    const int other_bpp = other->bpp;

    // Left or top of other is out of bounds
    int other_x_start = (x_off < 0) ? -x_off : 0;
    int other_y_start = (y_off < 0) ? -y_off : 0;
    
    // Scaled other size
    const int other_width_scaled = fast_floorf(abs(other->w * x_scale));
    const int other_height_scaled = fast_floorf(abs(other->h * y_scale));

    // Right or bottom of image is out of bounds
    int other_x_end = (x_off + other_width_scaled >= img->w) ? img->w - x_off : other_width_scaled;
    int other_y_end = (y_off + other_height_scaled >= img->h) ? img->h - y_off : other_height_scaled;

    // Check bounds are within img
    if (other_x_start + x_off >= img->w || other_y_start + y_off >= img->h) return;
    if (other_x_end + x_off <= 0 || other_y_end + y_off <= 0) return;

    // If scaling is negative we essentially flip the other coordinates so they work from bottom left instead of bottom right
    if (over_xscale < 0) {
        other_x_start -= other_width_scaled;
        other_x_end -= other_width_scaled;
        x_off += other_width_scaled;
    }

    if (over_yscale < 0) {
        other_y_start -= other_height_scaled;
        other_y_end -= other_height_scaled;
        y_off += other_height_scaled;
    }

    switch(img_bpp) {
        case IMAGE_BPP_BINARY: {
            // Iterate the img area to be updated
            for (int y = other_y_start; y < other_y_end; y++) {
                uint32_t *img_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y_off + y);

                const int other_y = fast_floorf(y * over_yscale);
                void *other_row_ptr = imlib_compute_row_ptr(other, other_y);

                for (int x = other_x_start; x < other_x_end; x++) {
                    const int other_x = fast_floorf(x * over_xscale);

                    if ((!mask) || image_get_mask_pixel(mask, other_x, other_y)) {
                        const uint32_t other_pixel = safe_map_pixel(IMAGE_BPP_BINARY, other_bpp, imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x));
                        
                        uint32_t result_pixel;
                        if (alpha==256) {
                            result_pixel = other_pixel;
                        }
                        else {
                            const uint32_t img_pixel = IMAGE_GET_BINARY_PIXEL_FAST(img_row_ptr, x_off + x);
                            
                            const uint32_t vgs = (other_pixel << 16) + img_pixel;
                            result_pixel = __SMUAD(va, vgs)>>8;
                        }

                        IMAGE_PUT_BINARY_PIXEL_FAST(img_row_ptr, x_off + x, result_pixel);
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            // Iterate the img area to be updated
            for (int y = other_y_start; y < other_y_end; y++) {
                // Pre-add x_off here to save adding it inside the central loop
                uint8_t *img_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y_off + y) + x_off;

                const int other_y = fast_floorf(y * over_yscale);
                void *other_row_ptr = imlib_compute_row_ptr(other, other_y);

                for (int x = other_x_start; x < other_x_end; x++) {
                    const int other_x = fast_floorf(x * over_xscale);

                    if ((!mask) || image_get_mask_pixel(mask, other_x, other_y)) {
                        const uint8_t other_pixel = safe_map_pixel(IMAGE_BPP_GRAYSCALE, other_bpp, imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x));

                        uint8_t result_pixel;
                        if (alpha==256) {
                            result_pixel = other_pixel;
                        }
                        else {
                            const uint8_t img_pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(img_row_ptr,  x);
                            
                            const uint32_t vgs = (other_pixel << 16) + img_pixel;
                            result_pixel = __SMUAD(va, vgs)>>8;
                        }

                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(img_row_ptr, x, result_pixel);
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            // Iterate the img area to be updated
            for (int y = other_y_start; y < other_y_end; y++) {
                // Pre-add x_off here to save adding it inside the central loop
                uint16_t *img_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y_off + y) + x_off;
                
                const int other_y = fast_floorf(y * over_yscale);
                void *other_row_ptr = imlib_compute_row_ptr(other, other_y);

                for (int x = other_x_start; x < other_x_end; x++) {
                    const int other_x = fast_floorf(x * over_xscale);

                    if ((!mask) || image_get_mask_pixel(mask, other_x, other_y)) {
                        uint32_t other_pixel = imlib_get_pixel_fast(other_bpp, other_row_ptr, other_x);
                        other_pixel = color_palette
                            ? color_palette[other_pixel]
                            : safe_map_pixel(IMAGE_BPP_RGB565, other_bpp, other_pixel);
                        
                        uint16_t result_pixel;
                        if (alpha==256) {
                            result_pixel = other_pixel;
                        }
                        else {
                            const uint16_t img_pixel = IMAGE_GET_RGB565_PIXEL_FAST(img_row_ptr, x);
                            
                            const uint32_t vr = (COLOR_RGB565_TO_R5(other_pixel) << 16) + COLOR_RGB565_TO_R5(img_pixel);
                            const uint32_t vg = (COLOR_RGB565_TO_G6(other_pixel) << 16) + COLOR_RGB565_TO_G6(img_pixel);
                            const uint32_t vb = (COLOR_RGB565_TO_B5(other_pixel) << 16) + COLOR_RGB565_TO_B5(img_pixel);
                            const uint8_t r = __SMUAD(va, vr)>>8;
                            const uint8_t g = __SMUAD(va, vg)>>8;
                            const uint8_t b = __SMUAD(va, vb)>>8;

                            result_pixel = COLOR_R5_G6_B5_TO_RGB565(r, g, b);
                        }
                        IMAGE_PUT_RGB565_PIXEL_FAST(img_row_ptr, x, result_pixel);
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
