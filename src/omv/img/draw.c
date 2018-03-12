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
static void point_fill(image_t *img, int cx, int cy, int r, int c)
{
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if ((x * x) + (y * y) <= (r * r)) {
                imlib_set_pixel(img, cx + x, cy + y, c);
            }
        }
    }
}

// https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c, int thickness)
{
    int dx = abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    int dy = abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int err = ((dx > dy) ? dx : -dy) / 2;

    for (;;) {
        point_fill(img, x0, y0, thickness - 1, c);
        if ((x0 == x1) && (y0 == y1)) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
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

        for (int i = rx - ((thickness - 0) / 2), j = rx + rw + ((thickness - 1) / 2),
            k = ry + rh - 1; i < j; i++) {
            yLine(img, i, ry - ((thickness - 0) / 2), ry + ((thickness - 1) / 2), c);
            yLine(img, i, k - ((thickness - 0) / 2), k + ((thickness - 1) / 2), c);
        }

        for (int i = ry - ((thickness - 0) / 2), j = ry + rh + ((thickness - 1) / 2),
            k = rx + rw - 1; i < j; i++) {
            xLine(img, rx - ((thickness - 0) / 2), rx + ((thickness - 1) / 2), i, c);
            xLine(img, k - ((thickness - 0) / 2), k + ((thickness - 1) / 2), i, c);
        }
    }
}

// https://stackoverflow.com/questions/27755514/circle-with-thickness-drawing-algorithm
void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c, int thickness, bool fill)
{
    if (fill) {
        point_fill(img, cx, cy, r, c);
    } else if (thickness > 0) {
        thickness = IM_MIN(thickness, r);

        int xo = r + ((thickness - 0) / 2);
        int xi = r - ((thickness - 1) / 2);
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

            if (y > (r - ((thickness - 1) / 2))) {
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

void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c, int scale)
{
    const int anchor = x_off;

    for(char ch, last = '\0'; (ch = *str); str++, last = ch) {

        if ((last == '\r') && (ch == '\n')) { // handle "\r\n" strings
            continue;
        }

        if ((ch == '\n') || (ch == '\r')) { // handle '\n' or '\r' strings
            x_off = anchor;
            y_off += font[0].h * scale; // newline height == space height
            continue;
        }

        if ((ch < ' ') || (ch > '~')) { // handle unknown characters
            imlib_draw_rectangle(img,
                x_off + ((scale * 3) / 2),
                y_off + ((scale * 3) / 2),
                (font[0].w * scale) - (((scale * 3) / 2) * 2),
                (font[0].h * scale) - (((scale * 3) / 2) * 2),
                c, scale, false);
            continue;
        }

        const glyph_t *g = &font[ch - ' '];

        for (int y = 0, yy = g->h * scale; y < yy; y++) {
            for (int x = 0, xx = g->w * scale; x < xx; x++) {
                if (g->data[y / scale] & (1 << (g->w - (x / scale)))) {
                    imlib_set_pixel(img, (x_off + x), (y_off + y), c);
                }
            }
        }

        x_off += g->w * scale;
    }
}
