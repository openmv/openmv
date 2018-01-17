/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library.
 *
 */
#include <stdlib.h>
#include <mp.h>
#include "font.h"
#include "array.h"
#include "ff_wrapper.h"
#include "imlib.h"
#include "common.h"
#include "omv_boardconfig.h"

/////////////////
// Point Stuff //
/////////////////

void point_init(point_t *ptr, int x, int y)
{
    ptr->x = x;
    ptr->y = y;
}

void point_copy(point_t *dst, point_t *src)
{
    memcpy(dst, src, sizeof(point_t));
}

bool point_equal_fast(point_t *ptr0, point_t *ptr1)
{
    return !memcmp(ptr0, ptr1, sizeof(point_t));
}

int point_quadrance(point_t *ptr0, point_t *ptr1)
{
    int delta_x = ptr0->x - ptr1->x;
    int delta_y = ptr0->y - ptr1->y;
    return (delta_x * delta_x) + (delta_y * delta_y);
}

////////////////
// Line Stuff //
////////////////

// http://www.skytopia.com/project/articles/compsci/clipping.html
bool lb_clip_line(line_t *l, int x, int y, int w, int h) // line is drawn if this returns true
{
    int xdelta = l->x2 - l->x1, ydelta = l->y2 - l->y1, p[4], q[4];
    float umin = 0, umax = 1;

    p[0] = -(xdelta);
    p[1] = +(xdelta);
    p[2] = -(ydelta);
    p[3] = +(ydelta);

    q[0] = l->x1 - (x);
    q[1] = (x + w - 1) - l->x1;
    q[2] = l->y1 - (y);
    q[3] = (y + h - 1) - l->y1;

    for (int i = 0; i < 4; i++) {
        if (p[i]) {
            float u = ((float) q[i]) / ((float) p[i]);

            if (p[i] < 0) { // outside to inside
                if (u > umax) return false;
                if (u > umin) umin = u;
            }

            if (p[i] > 0) { // inside to outside
                if (u < umin) return false;
                if (u < umax) umax = u;
            }

        } else if (q[i] < 0) {
            return false;
        }
    }

    if (umax < umin) return false;

    int x1_c = l->x1 + (xdelta * umin);
    int y1_c = l->y1 + (ydelta * umin);
    int x2_c = l->x1 + (xdelta * umax);
    int y2_c = l->y1 + (ydelta * umax);
    l->x1 = x1_c;
    l->y1 = y1_c;
    l->x2 = x2_c;
    l->y2 = y2_c;

    return true;
}

/////////////////////
// Rectangle Stuff //
/////////////////////

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h)
{
    ptr->x = x;
    ptr->y = y;
    ptr->w = w;
    ptr->h = h;
}

void rectangle_copy(rectangle_t *dst, rectangle_t *src)
{
    memcpy(dst, src, sizeof(rectangle_t));
}

bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1)
{
    return !memcmp(ptr0, ptr1, sizeof(rectangle_t));
}

bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1)
{
    int x0 = ptr0->x;
    int y0 = ptr0->y;
    int w0 = ptr0->w;
    int h0 = ptr0->h;
    int x1 = ptr1->x;
    int y1 = ptr1->y;
    int w1 = ptr1->w;
    int h1 = ptr1->h;
    return (x0 < (x1 + w1)) && (y0 < (y1 + h1)) && (x1 < (x0 + w0)) && (y1 < (y0 + h0));
}

void rectangle_intersected(rectangle_t *dst, rectangle_t *src)
{
    int leftX = IM_MAX(dst->x, src->x);
    int topY = IM_MAX(dst->y, src->y);
    int rightX = IM_MIN(dst->x + dst->w, src->x + src->w);
    int bottomY = IM_MIN(dst->y + dst->h, src->y + src->h);
    dst->x = leftX;
    dst->y = topY;
    dst->w = rightX - leftX;
    dst->h = bottomY - topY;
}

void rectangle_united(rectangle_t *dst, rectangle_t *src)
{
    int leftX = IM_MIN(dst->x, src->x);
    int topY = IM_MIN(dst->y, src->y);
    int rightX = IM_MAX(dst->x + dst->w, src->x + src->w);
    int bottomY = IM_MAX(dst->y + dst->h, src->y + src->h);
    dst->x = leftX;
    dst->y = topY;
    dst->w = rightX - leftX;
    dst->h = bottomY - topY;
}

/////////////////
// Image Stuff //
/////////////////

void image_init(image_t *ptr, int w, int h, int bpp, void *data)
{
    ptr->w = w;
    ptr->h = h;
    ptr->bpp = bpp;
    ptr->data = data;
}

void image_copy(image_t *dst, image_t *src)
{
    memcpy(dst, src, sizeof(image_t));
}

uint32_t image_size(image_t *ptr)
{
    switch (ptr->bpp) {
        case IMAGE_BPP_BINARY: {
            return ((ptr->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * ptr->h;
        }
        case IMAGE_BPP_GRAYSCALE: {
            return (ptr->w * ptr->h) * sizeof(uint8_t);
        }
        case IMAGE_BPP_RGB565: {
            return (ptr->w * ptr->h) * sizeof(uint16_t);
        }
        case IMAGE_BPP_BAYER: {
            return ptr->w * ptr->h;
        }
        default: { // JPEG
            return ptr->bpp;
        }
    }
}

// Gamma uncompress
extern const float xyz_table[256];

const int8_t kernel_gauss_3[3*3] = {
     1, 2, 1,
     2, 4, 2,
     1, 2, 1,
};

const int8_t kernel_gauss_5[5*5] = {
    1,  4,  6,  4, 1,
    4, 16, 24, 16, 4,
    6, 24, 36, 24, 6,
    4, 16, 24, 16, 4,
    1,  4,  6,  4, 1
};

const int8_t kernel_laplacian_3[3*3] = {
     -1, -1, -1,
     -1,  8, -1,
     -1, -1, -1
};

const int8_t kernel_high_pass_3[3*3] = {
    -1, -1, -1,
    -1, +8, -1,
    -1, -1, -1
};

// USE THE LUT FOR RGB->LAB CONVERSION - NOT THIS FUNCTION!
void imlib_rgb_to_lab(simple_color_t *rgb, simple_color_t *lab)
{
    // https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation
    // https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions

    float r_lin = xyz_table[rgb->red];
    float g_lin = xyz_table[rgb->green];
    float b_lin = xyz_table[rgb->blue];

    float x = ((r_lin * 0.4124f) + (g_lin * 0.3576f) + (b_lin * 0.1805f)) / 095.047f;
    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) / 100.000f;
    float z = ((r_lin * 0.0193f) + (g_lin * 0.1192f) + (b_lin * 0.9505f)) / 108.883f;

    x = (x>0.008856f) ? fast_cbrtf(x) : ((x * 7.787037f) + 0.137931f);
    y = (y>0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);
    z = (z>0.008856f) ? fast_cbrtf(z) : ((z * 7.787037f) + 0.137931f);

    lab->L = ((int8_t) fast_roundf(116 * y)) - 16;
    lab->A = ((int8_t) fast_roundf(500 * (x-y)));
    lab->B = ((int8_t) fast_roundf(200 * (y-z)));
}

void imlib_lab_to_rgb(simple_color_t *lab, simple_color_t *rgb)
{
    // https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions
    // https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation

    float x = ((lab->L + 16) * 0.008621f) + (lab->A * 0.002f);
    float y = ((lab->L + 16) * 0.008621f);
    float z = ((lab->L + 16) * 0.008621f) - (lab->B * 0.005f);

    x = ((x>0.206897f) ? (x*x*x) : ((0.128419f * x) - 0.017713f)) * 095.047f;
    y = ((y>0.206897f) ? (y*y*y) : ((0.128419f * y) - 0.017713f)) * 100.000f;
    z = ((z>0.206897f) ? (z*z*z) : ((0.128419f * z) - 0.017713f)) * 108.883f;

    float r_lin = ((x * +3.2406f) + (y * -1.5372f) + (z * -0.4986f)) / 100.0f;
    float g_lin = ((x * -0.9689f) + (y * +1.8758f) + (z * +0.0415f)) / 100.0f;
    float b_lin = ((x * +0.0557f) + (y * -0.2040f) + (z * +1.0570f)) / 100.0f;

    r_lin = (r_lin>0.0031308f) ? ((1.055f*powf(r_lin, 0.416666f))-0.055f) : (r_lin*12.92f);
    g_lin = (g_lin>0.0031308f) ? ((1.055f*powf(g_lin, 0.416666f))-0.055f) : (g_lin*12.92f);
    b_lin = (b_lin>0.0031308f) ? ((1.055f*powf(b_lin, 0.416666f))-0.055f) : (b_lin*12.92f);

    rgb->red   = IM_MAX(IM_MIN(fast_roundf(r_lin * 255), 255), 0);
    rgb->green = IM_MAX(IM_MIN(fast_roundf(g_lin * 255), 255), 0);
    rgb->blue  = IM_MAX(IM_MIN(fast_roundf(b_lin * 255), 255), 0);
}

void imlib_rgb_to_grayscale(simple_color_t *rgb, simple_color_t *grayscale)
{
    float r_lin = xyz_table[rgb->red];
    float g_lin = xyz_table[rgb->green];
    float b_lin = xyz_table[rgb->blue];
    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) / 100.0f;
    y = (y>0.0031308f) ? ((1.055f*powf(y, 0.416666f))-0.055f) : (y*12.92f);
    grayscale->G = IM_MAX(IM_MIN(fast_roundf(y * 255), 255), 0);
}

// Just copy settings back.
void imlib_grayscale_to_rgb(simple_color_t *grayscale, simple_color_t *rgb)
{
    rgb->red   = grayscale->G;
    rgb->green = grayscale->G;
    rgb->blue  = grayscale->G;
}

ALWAYS_INLINE uint16_t imlib_yuv_to_rgb(uint8_t y, int8_t u, int8_t v)
{
    uint32_t r = IM_MAX(IM_MIN(y + ((91881*v)>>16), 255), 0);
    uint32_t g = IM_MAX(IM_MIN(y - (((22554*u)+(46802*v))>>16), 255), 0);
    uint32_t b = IM_MAX(IM_MIN(y + ((116130*u)>>16), 255), 0);
    return IM_RGB565(IM_R825(r), IM_G826(g), IM_B825(b));
}

////////////////////////////////////////////////////////////////////////////////

static save_image_format_t imblib_parse_extension(image_t *img, const char *path)
{
    size_t l = strlen(path);
    const char *p = path + l;
    if (l >= 5) {
               if (((p[-1] == 'g') || (p[-1] == 'G'))
               &&  ((p[-2] == 'e') || (p[-2] == 'E'))
               &&  ((p[-3] == 'p') || (p[-3] == 'P'))
               &&  ((p[-4] == 'j') || (p[-4] == 'J'))
               &&  ((p[-5] == '.') || (p[-5] == '.'))) {
                    // Will convert to JPG if not.
                    return FORMAT_JPG;
        }
    }
    if (l >= 4) {
               if (((p[-1] == 'g') || (p[-1] == 'G'))
               &&  ((p[-2] == 'p') || (p[-2] == 'P'))
               &&  ((p[-3] == 'j') || (p[-3] == 'J'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    // Will convert to JPG if not.
                    return FORMAT_JPG;
        } else if (((p[-1] == 'p') || (p[-1] == 'P'))
               &&  ((p[-2] == 'm') || (p[-2] == 'M'))
               &&  ((p[-3] == 'b') || (p[-3] == 'B'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (IM_IS_JPEG(img) || IM_IS_BAYER(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not BMP!"));
                    }
                    return FORMAT_BMP;
        } else if (((p[-1] == 'm') || (p[-1] == 'M'))
               &&  ((p[-2] == 'p') || (p[-2] == 'P'))
               &&  ((p[-3] == 'p') || (p[-3] == 'P'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (!IM_IS_RGB565(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not PPM!"));
                    }
                    return FORMAT_PNM;
        } else if (((p[-1] == 'm') || (p[-1] == 'M'))
               &&  ((p[-2] == 'g') || (p[-2] == 'G'))
               &&  ((p[-3] == 'p') || (p[-3] == 'P'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (!IM_IS_GS(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not PGM!"));
                    }
                    return FORMAT_PNM;
        } else if (((p[-1] == 'w') || (p[-1] == 'W'))
               &&  ((p[-2] == 'a') || (p[-2] == 'A'))
               &&  ((p[-3] == 'r') || (p[-3] == 'R'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (!IM_IS_BAYER(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not BAYER!"));
                    }
                    return FORMAT_RAW;
        }

    }
    return FORMAT_DONT_CARE;
}

bool imlib_read_geometry(FIL *fp, image_t *img, const char *path, img_read_settings_t *rs)
{
    file_read_open(fp, path);
    char magic[2];
    read_data(fp, &magic, 2);
    file_close(fp);

    bool vflipped = false;
    if ((magic[0]=='P')
    && ((magic[1]=='2') || (magic[1]=='3')
    ||  (magic[1]=='5') || (magic[1]=='6'))) { // PPM
        rs->format = FORMAT_PNM;
        file_read_open(fp, path);
        file_buffer_on(fp); // REMEMBER TO TURN THIS OFF LATER!
        ppm_read_geometry(fp, img, path, &rs->ppm_rs);
    } else if ((magic[0]=='B') && (magic[1]=='M')) { // BMP
        rs->format = FORMAT_BMP;
        file_read_open(fp, path);
        file_buffer_on(fp); // REMEMBER TO TURN THIS OFF LATER!
        vflipped = bmp_read_geometry(fp, img, path, &rs->bmp_rs);
    } else {
        ff_unsupported_format(NULL);
    }
    imblib_parse_extension(img, path); // Enforce extension!
    return vflipped;
}

static void imlib_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, img_read_settings_t *rs)
{
    switch (rs->format) {
        case FORMAT_BMP:
            bmp_read_pixels(fp, img, line_start, line_end, &rs->bmp_rs);
            break;
        case FORMAT_PNM:
            ppm_read_pixels(fp, img, line_start, line_end, &rs->ppm_rs);
            break;
        default: // won't happen
            break;
    }
}

void imlib_image_operation(image_t *img, const char *path, image_t *other, line_op_t op, void *data)
{
    if (path) {
        uint32_t size = fb_avail() / 2;
        void *alloc = fb_alloc(size); // We have to do this before the read.
        // This code reads a window of an image in at a time and then executes
        // the line operation on each line in that window before moving to the
        // next window. The vflipped part is here because BMP files can be saved
        // vertically flipped resulting in us reading the image backwards.
        FIL fp;
        image_t temp;
        img_read_settings_t rs;
        bool vflipped = imlib_read_geometry(&fp, &temp, path, &rs);
        if (!IM_EQUAL(img, &temp)) {
            ff_not_equal(&fp);
        }
        // When processing vertically flipped images the read function will fill
        // the window up from the bottom. The read function assumes that the
        // window is equal to an image in size. However, since this is not the
        // case we shrink the window size to how many lines we're buffering.
        temp.pixels = alloc;
        temp.h = (size / (temp.w * temp.bpp)); // round down
        // This should never happen unless someone forgot to free.
        if ((!temp.pixels) || (!temp.h)) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError,
                                               "Not enough memory available!"));
        }
        for (int i=0; i<img->h; i+=temp.h) { // goes past end
            int can_do = IM_MIN(temp.h, img->h-i);
            imlib_read_pixels(&fp, &temp, 0, can_do, &rs);
            for (int j=0; j<can_do; j++) {
                if (!vflipped) {
                    op(img, i+j, temp.pixels+(temp.w*temp.bpp*j), data);
                } else {
                    op(img, (img->h-i-can_do)+j, temp.pixels+(temp.w*temp.bpp*j), data);
                }
            }
        }
        file_buffer_off(&fp);
        file_close(&fp);
        fb_free();
    } else {
        if (!IM_EQUAL(img, other)) {
            ff_not_equal(NULL);
        }
        for (int i=0; i<img->h; i++) {
            op(img, i, other->pixels + (img->w * img->bpp * i), data);
        }
    }
}

void imlib_load_image(image_t *img, const char *path)
{
    FIL fp;
    file_read_open(&fp, path);
    char magic[2];
    read_data(&fp, &magic, 2);
    file_close(&fp);

    if ((magic[0]=='P')
    && ((magic[1]=='2') || (magic[1]=='3')
    ||  (magic[1]=='5') || (magic[1]=='6'))) { // PPM
        ppm_read(img, path);
    } else if ((magic[0]=='B') && (magic[1]=='M')) { // BMP
        bmp_read(img, path);
    } else if ((magic[0]==0xFF) && (magic[1]==0xD8)) { // JPEG
        jpeg_read(img, path);
    } else {
        ff_unsupported_format(NULL);
    }
    imblib_parse_extension(img, path); // Enforce extension!
}

void imlib_save_image(image_t *img, const char *path, rectangle_t *roi, int quality)
{
    switch (imblib_parse_extension(img, path)) {
        case FORMAT_BMP:
            bmp_write_subimg(img, path, roi);
            break;
        case FORMAT_PNM:
            ppm_write_subimg(img, path, roi);
            break;
        case FORMAT_RAW: {
            FIL fp;
            file_write_open(&fp, path);
            write_data(&fp, img->pixels, img->w * img->h);
            break;
        }
        case FORMAT_JPG:
        case FORMAT_DONT_CARE:
            if (IM_IS_JPEG(img) || IM_IS_BAYER(img)) {
                char *new_path = strcat(strcpy(fb_alloc(strlen(path)+5), path), ".jpg");
                jpeg_write(img, new_path, quality);
                fb_free();
            } else {
                char *new_path = strcat(strcpy(fb_alloc(strlen(path)+5), path), ".bmp");
                bmp_write_subimg(img, new_path, roi);
                fb_free();
            }
            break;
    }
}

void imlib_copy_image(image_t *dst, image_t *src, rectangle_t *roi)
{
    if (IM_IS_JPEG(src)) {
        dst->w = src->w;
        dst->h = src->h;
        dst->bpp = src->bpp;
        dst->pixels = xalloc(src->bpp);
        memcpy(dst->pixels, src->pixels, src->bpp);
    } else {
        rectangle_t rect;
        if (!rectangle_subimg(src, roi, &rect)) ff_no_intersection(NULL);
        dst->w = rect.w;
        dst->h = rect.h;
        dst->bpp = src->bpp;
        dst->pixels = xalloc(rect.w * rect.h * src->bpp);
        uint8_t *dst_pointer = dst->pixels;
        for (int i = rect.y; i < (rect.y + rect.h); i++) {
            int length = rect.w * src->bpp;
            memcpy(dst_pointer,
                   src->pixels + (rect.x * src->bpp) + (i * src->w * src->bpp),
                   length);
            dst_pointer += length;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

// Get pixel (handles boundary check and image type check).
int imlib_get_pixel(image_t *img, int x, int y)
{
    return (IM_X_INSIDE(img, x) && IM_Y_INSIDE(img, y)) ?
        ( IM_IS_GS(img)
        ? IM_GET_GS_PIXEL(img, x, y)
        : IM_GET_RGB565_PIXEL(img, x, y) )
    : 0;
}

// Set pixel (handles boundary check and image type check).
void imlib_set_pixel(image_t *img, int x, int y, int p)
{
    if (IM_X_INSIDE(img, x) && IM_Y_INSIDE(img, y)) {
        if (IM_IS_GS(img)) {
            IM_SET_GS_PIXEL(img, x, y, p);
        } else {
            IM_SET_RGB565_PIXEL(img, x, y, p);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c)
{
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = x0<x1 ? 1 : -1;
    int sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2;
    for (;;) {
        imlib_set_pixel(img, x0, y0, c);
        if (x0==x1 && y0==y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void imlib_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c)
{
    if (rw<=0 || rh<=0) {
        return;
    }
    for (int i=rx, j=rx+rw, k=ry+rh-1; i<j; i++) {
        imlib_set_pixel(img, i, ry, c);
        imlib_set_pixel(img, i, k, c);
    }
    for (int i=ry+1, j=ry+rh-1, k=rx+rw-1; i<j; i++) {
        imlib_set_pixel(img, rx, i, c);
        imlib_set_pixel(img, k, i, c);
    }
}

void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c)
{
    int x = r, y = 0, radiusError = 1-x;
    while (x>=y) {
        imlib_set_pixel(img,  x + cx,  y + cy, c);
        imlib_set_pixel(img,  y + cx,  x + cy, c);
        imlib_set_pixel(img, -x + cx,  y + cy, c);
        imlib_set_pixel(img, -y + cx,  x + cy, c);
        imlib_set_pixel(img, -x + cx, -y + cy, c);
        imlib_set_pixel(img, -y + cx, -x + cy, c);
        imlib_set_pixel(img,  x + cx, -y + cy, c);
        imlib_set_pixel(img,  y + cx, -x + cy, c);
        y++;
        if (radiusError<0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c)
{
    const int anchor = x_off;
    for(char ch, last='\0'; (ch=*str); str++, last=ch) {
        if (last=='\r' && ch=='\n') { // handle "\r\n" strings
            continue;
        }
        if (ch=='\n' || ch=='\r') { // handle '\n' or '\r' strings
            x_off = anchor;
            y_off += font[0].h; // newline height == space height
            continue;
        }
        if (ch<' ' || ch>'~') {
            imlib_draw_rectangle(img,(x_off+1),(y_off+1),font[0].w-2,font[0].h-2,c);
            continue;
        }
        const glyph_t *g = &font[ch-' '];
        for (int y=0; y<g->h; y++) {
            for (int x=0; x<g->w; x++) {
                if (g->data[y] & (1<<(g->w-x))) {
                    imlib_set_pixel(img, (x_off+x), (y_off+y), c);
                }
            }
        }
        x_off += g->w;
    }
}

////////////////////////////////////////////////////////////////////////////////

void imlib_binary(image_t *img,
                  int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds,
                  bool invert)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            bool in = false;
            for (int k=0; k<num_thresholds; k++) {
                in |= invert ^
                      ((l_thresholds[k].G <= pixels[i])
                   && (pixels[i] <= h_thresholds[k].G));
            }
            pixels[i] = in ? 0xFF : 0;
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            const int pixel = pixels[i];
            const int lab_l = IM_RGB5652L(pixel);
            const int lab_a = IM_RGB5652A(pixel);
            const int lab_b = IM_RGB5652B(pixel);
            bool in = false;
            for (int k=0; k<num_thresholds; k++) {
                in |= invert ^
                     (((l_thresholds[k].L <= lab_l)
                   && (lab_l <= h_thresholds[k].L))
                   && ((l_thresholds[k].A <= lab_a)
                   && (lab_a <= h_thresholds[k].A))
                   && ((l_thresholds[k].B <= lab_b)
                   && (lab_b <= h_thresholds[k].B)));
            }
            pixels[i] = in ? 0xFFFF : 0;
        }
    }
}

void imlib_invert(image_t *img)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            pixels[i] = ~pixels[i];
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            pixels[i] = ~pixels[i];
        }
    }
}

static void imlib_b_and_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] &= other[i];
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] &= ((uint16_t *) other)[i];
        }
    }
}

void imlib_b_and(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_b_and_line_op, NULL);
}

static void imlib_b_nand_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = ~(pixels[i] & other[i]);
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = ~(pixels[i] & ((uint16_t *) other)[i]);
        }
    }
}

void imlib_b_nand(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_b_nand_line_op, NULL);
}

static void imlib_b_or_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] |= other[i];
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] |= ((uint16_t *) other)[i];
        }
    }
}

void imlib_b_or(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_b_or_line_op, NULL);
}

static void imlib_b_nor_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = ~(pixels[i] | other[i]);
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = ~(pixels[i] | ((uint16_t *) other)[i]);
        }
    }
}

void imlib_b_nor(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_b_nor_line_op, NULL);
}

static void imlib_b_xor_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] ^= other[i];
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] ^= ((uint16_t *) other)[i];
        }
    }
}

void imlib_b_xor(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_b_xor_line_op, NULL);
}

static void imlib_b_xnor_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = ~(pixels[i] ^ other[i]);
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = ~(pixels[i] ^ ((uint16_t *) other)[i]);
        }
    }
}

void imlib_b_xnor(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_b_xnor_line_op, NULL);
}

static void imlib_erode_dilate(image_t *img, int ksize, int threshold, int e_or_d)
{
    int brows = ksize + 1;
    uint8_t *buffer = fb_alloc(img->w * brows * img->bpp);
    if (IM_IS_GS(img)) {
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                // We're writing into the buffer like if it were a window.
                int buffer_idx = ((y%brows)*img->w)+x;
                buffer[buffer_idx] = IM_GET_GS_PIXEL(img, x, y);
                if ((!!buffer[buffer_idx]) == e_or_d) {
                    continue; // short circuit (makes this very fast - usually)
                }
                int acc = e_or_d ? 0 : -1; // don't count center pixel...
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            acc += !!IM_GET_GS_PIXEL(img, x+k, y+j);
                        } else { // outer pixels should not affect result.
                            acc += e_or_d ? 0 : 1;
                            // 1 for erode prevents acc from being lower.
                            // 0 for dilate prevents acc from being higher.
                        }
                    }
                }
                if (!e_or_d) {
                    // Preserve original pixel value...
                    if (acc < threshold) buffer[buffer_idx] = 0; // clear
                } else {
                    // Preserve original pixel value...
                    if (acc > threshold) buffer[buffer_idx] = -1; // set
                }
            }
            if (y>=ksize) {
                memcpy(img->pixels+((y-ksize)*img->w),
                       buffer+(((y-ksize)%brows)*img->w),
                       img->w * sizeof(uint8_t));
            }
        }
        for (int y=img->h-ksize; y<img->h; y++) {
            memcpy(img->pixels+(y*img->w),
                   buffer+((y%brows)*img->w),
                   img->w * sizeof(uint8_t));
        }
    } else {
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                // We're writing into the buffer like if it were a window.
                int buffer_idx = ((y%brows)*img->w)+x;
                ((uint16_t *) buffer)[buffer_idx] = IM_GET_RGB565_PIXEL(img, x, y);
                if ((!!((uint16_t *) buffer)[buffer_idx]) == e_or_d) {
                    continue; // short circuit (makes this very fast - usually)
                }
                int acc = e_or_d ? 0 : -1; // don't count center pixel...
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            acc += !!IM_GET_RGB565_PIXEL(img, x+k, y+j);
                        } else { // outer pixels should not affect result.
                            acc += e_or_d ? 0 : 1;
                            // 1 for erode prevents acc from being lower.
                            // 0 for dilate prevents acc from being higher.
                        }
                    }
                }
                if (!e_or_d) {
                    // Preserve original pixel value...
                    if (acc < threshold) ((uint16_t *) buffer)[buffer_idx] = 0; // clear
                } else {
                    // Preserve original pixel value...
                    if (acc > threshold) ((uint16_t *) buffer)[buffer_idx] = -1; // set
                }
            }
            if (y>=ksize) {
                memcpy(((uint16_t *) img->pixels)+((y-ksize)*img->w),
                       ((uint16_t *) buffer)+(((y-ksize)%brows)*img->w),
                       img->w * sizeof(uint16_t));
            }
        }
        for (int y=img->h-ksize; y<img->h; y++) {
            memcpy(((uint16_t *) img->pixels)+(y*img->w),
                   ((uint16_t *) buffer)+((y%brows)*img->w),
                   img->w * sizeof(uint16_t));
        }
    }
    fb_free();
}

void imlib_erode(image_t *img, int ksize, int threshold)
{
    // Threshold should be equal to ((ksize*2)+1)*((ksize*2)+1)-1
    // for normal operation. E.g. for ksize==3 -> threshold==8
    // Basically you're adjusting the number of pixels that
    // must be set in the kernel (besides the center) for the output to be 1.
    // Erode normally requires all pixels to be 1.
    imlib_erode_dilate(img, ksize, threshold, 0);
}

void imlib_dilate(image_t *img, int ksize, int threshold)
{
    // Threshold should be equal to 0
    // for normal operation. E.g. for ksize==3 -> threshold==0
    // Basically you're adjusting the number of pixels that
    // must be set in the kernel (besides the center) for the output to be 1.
    // Dilate normally requires one pixel to be 1.
    imlib_erode_dilate(img, ksize, threshold, 1);
}

////////////////////////////////////////////////////////////////////////////////

void imlib_negate(image_t *img)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            pixels[i] = IM_MAX_GS - pixels[i];
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            const int pixel = pixels[i];
            const int r = IM_MAX_R5 - IM_R565(pixel);
            const int g = IM_MAX_G6 - IM_G565(pixel);
            const int b = IM_MAX_B5 - IM_B565(pixel);
            pixels[i] = IM_RGB565(r, g, b);
        }
    }
}

static void imlib_difference_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = abs(pixels[i] - other[i]);
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            const int pixel = pixels[i], other_pixel = ((uint16_t *) other)[i];
            const int r = abs(IM_R565(pixel) - IM_R565(other_pixel));
            const int g = abs(IM_G565(pixel) - IM_G565(other_pixel));
            const int b = abs(IM_B565(pixel) - IM_B565(other_pixel));
            pixels[i] = IM_RGB565(r, g, b);
        }
    }
}

void imlib_difference(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_difference_line_op, NULL);
}

static void imlib_replace_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        memcpy(pixels, other, img->w * sizeof(uint8_t));
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        memcpy(pixels, other, img->w * sizeof(uint16_t));
    }
}

void imlib_replace(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_replace_line_op, NULL);
}

static void imlib_blend_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    uint32_t alpha = (uint32_t) data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = __SMUAD(alpha,__PKHBT(pixels[i],other[i],16))>>8;
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            const int pixel = pixels[i], other_pixel = ((uint16_t *) other)[i];
            uint32_t vr = __PKHBT(IM_R565(pixel), IM_R565(other_pixel), 16);
            uint32_t vg = __PKHBT(IM_G565(pixel), IM_G565(other_pixel), 16);
            uint32_t vb = __PKHBT(IM_B565(pixel), IM_B565(other_pixel), 16);
            uint32_t r = __SMUAD(alpha, vr)>>8;
            uint32_t g = __SMUAD(alpha, vg)>>8;
            uint32_t b = __SMUAD(alpha, vb)>>8;
            pixels[i] = IM_RGB565(r, g, b);
        }
    }
}

void imlib_blend(image_t *img, const char *path, image_t *other, int alpha)
{
    imlib_image_operation(img, path, other, imlib_blend_line_op, (void *) __PKHBT((256-alpha), alpha, 16));
}

static void imlib_max_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = IM_MAX(pixels[i], other[i]);
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            const int pixel = pixels[i], other_pixel = ((uint16_t *) other)[i];
            const int r = IM_MAX(IM_R565(pixel), IM_R565(other_pixel));
            const int g = IM_MAX(IM_G565(pixel), IM_G565(other_pixel));
            const int b = IM_MAX(IM_B565(pixel), IM_B565(other_pixel));
            pixels[i] = IM_RGB565(r, g, b);
        }
    }
}

void imlib_max(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_max_line_op, NULL);
}

static void imlib_min_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    data = data;

    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels + (img->w * line);
        for (int i=0; i<img->w; i++) {
            pixels[i] = IM_MIN(pixels[i], other[i]);
        }
    } else {
        uint16_t *pixels = ((uint16_t *) img->pixels) + (img->w * line);
        for (int i=0; i<img->w; i++) {
            const int pixel = pixels[i], other_pixel = ((uint16_t *) other)[i];
            const int r = IM_MIN(IM_R565(pixel), IM_R565(other_pixel));
            const int g = IM_MIN(IM_G565(pixel), IM_G565(other_pixel));
            const int b = IM_MIN(IM_B565(pixel), IM_B565(other_pixel));
            pixels[i] = IM_RGB565(r, g, b);
        }
    }
}

void imlib_min(image_t *img, const char *path, image_t *other)
{
    imlib_image_operation(img, path, other, imlib_min_line_op, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void imlib_chrominvar(image_t *img)
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
                    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
                    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

                    float lin_sum = r_lin + g_lin + b_lin;

                    float r_lin_div = 0.0f;
                    float g_lin_div = 0.0f;
                    float b_lin_div = 0.0f;

                    if (lin_sum > 0.0f) {
                        lin_sum = 1.0f / lin_sum;
                        r_lin_div = r_lin * lin_sum;
                        g_lin_div = g_lin * lin_sum;
                        b_lin_div = b_lin * lin_sum;
                    }

                    int r_lin_div_int = IM_MAX(IM_MIN(r_lin_div * 255.0f, COLOR_R8_MAX), COLOR_R8_MIN);
                    int g_lin_div_int = IM_MAX(IM_MIN(g_lin_div * 255.0f, COLOR_G8_MAX), COLOR_G8_MIN);
                    int b_lin_div_int = IM_MAX(IM_MIN(b_lin_div * 255.0f, COLOR_B8_MAX), COLOR_B8_MIN);

                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, COLOR_R8_G8_B8_TO_RGB565(r_lin_div_int, g_lin_div_int, b_lin_div_int));
                }
            }

            break;
        }
        default: {
            break;
        }
    }
}

extern const uint16_t invariant_table[65536];

void imlib_illuminvar(image_t *img) // http://ai.stanford.edu/~alireza/publication/cic15.pdf
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
#ifdef OMV_HAVE_INVARIANT_TABLE
                    int rgb565 = invariant_table[pixel];
#else
                    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)] + 1.0;
                    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)] + 1.0;
                    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)] + 1.0;

                    float r_lin_sharp = (r_lin *  0.9968f) + (g_lin *  0.0228f) + (b_lin * 0.0015f);
                    float g_lin_sharp = (r_lin * -0.0071f) + (g_lin *  0.9933f) + (b_lin * 0.0146f);
                    float b_lin_sharp = (r_lin *  0.0103f) + (g_lin * -0.0161f) + (b_lin * 0.9839f);

                    float lin_sharp_avg = r_lin_sharp * g_lin_sharp * b_lin_sharp;
                    lin_sharp_avg = (lin_sharp_avg > 0.0f) ? fast_cbrtf(lin_sharp_avg) : 0.0f;

                    float r_lin_sharp_div = 0.0f;
                    float g_lin_sharp_div = 0.0f;
                    float b_lin_sharp_div = 0.0f;

                    if (lin_sharp_avg > 0.0f) {
                        lin_sharp_avg = 1.0f / lin_sharp_avg;
                        r_lin_sharp_div = r_lin_sharp * lin_sharp_avg;
                        g_lin_sharp_div = g_lin_sharp * lin_sharp_avg;
                        b_lin_sharp_div = b_lin_sharp * lin_sharp_avg;
                    }

                    float r_lin_sharp_div_log = (r_lin_sharp_div > 0.0f) ? fast_log(r_lin_sharp_div) : 0.0f;
                    float g_lin_sharp_div_log = (g_lin_sharp_div > 0.0f) ? fast_log(g_lin_sharp_div) : 0.0f;
                    float b_lin_sharp_div_log = (b_lin_sharp_div > 0.0f) ? fast_log(b_lin_sharp_div) : 0.0f;

                    float chi_x = (r_lin_sharp_div_log * 0.7071f) + (g_lin_sharp_div_log * -0.7071f) + (b_lin_sharp_div_log *  0.0000f);
                    float chi_y = (r_lin_sharp_div_log * 0.4082f) + (g_lin_sharp_div_log *  0.4082f) + (b_lin_sharp_div_log * -0.8164f);

                    float e_t_x =  0.9326f;
                    float e_t_y = -0.3609f;

                    float p_th_00 = e_t_x * e_t_x;
                    float p_th_01 = e_t_x * e_t_y;
                    float p_th_10 = e_t_y * e_t_x;
                    float p_th_11 = e_t_y * e_t_y;

                    float x_th_x = (p_th_00 * chi_x) + (p_th_01 * chi_y);
                    float x_th_y = (p_th_10 * chi_x) + (p_th_11 * chi_y);

                    float r_chi = (x_th_x *  0.7071f) + (x_th_y *  0.4082f);
                    float g_chi = (x_th_x * -0.7071f) + (x_th_y *  0.4082f);
                    float b_chi = (x_th_x *  0.0000f) + (x_th_y * -0.8164f);

                    float r_chi_invariant = fast_expf(r_chi);
                    float g_chi_invariant = fast_expf(g_chi);
                    float b_chi_invariant = fast_expf(b_chi);

                    float chi_invariant_sum = r_chi_invariant + g_chi_invariant + b_chi_invariant;

                    float r_chi_invariant_m = 0.0f;
                    float g_chi_invariant_m = 0.0f;
                    float b_chi_invariant_m = 0.0f;

                    if (chi_invariant_sum > 0.0f) {
                        chi_invariant_sum = 1.0f / chi_invariant_sum;
                        r_chi_invariant_m = r_chi_invariant * chi_invariant_sum;
                        g_chi_invariant_m = g_chi_invariant * chi_invariant_sum;
                        b_chi_invariant_m = b_chi_invariant * chi_invariant_sum;
                    }

                    int r_chi_invariant_m_int = IM_MAX(IM_MIN(r_chi_invariant_m * 255.0f, COLOR_R8_MAX), COLOR_R8_MIN);
                    int g_chi_invariant_m_int = IM_MAX(IM_MIN(g_chi_invariant_m * 255.0f, COLOR_G8_MAX), COLOR_G8_MIN);
                    int b_chi_invariant_m_int = IM_MAX(IM_MIN(b_chi_invariant_m * 255.0f, COLOR_B8_MAX), COLOR_B8_MIN);

                    int rgb565 = COLOR_R8_G8_B8_TO_RGB565(r_chi_invariant_m_int, g_chi_invariant_m_int, b_chi_invariant_m_int);
#endif
                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, rgb565);
                }
            }

            break;
        }
        default: {
            break;
        }
    }
}

void imlib_histeq(image_t *img)
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            int a = img->w * img->h;
            float s = (COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1)*sizeof(uint32_t));
            uint8_t *pixels = (uint8_t *) img->pixels;

            // Compute the image histogram
            for (int i=0; i<a; i++) {
                hist[pixels[i]-COLOR_GRAYSCALE_MIN] += 1;
            }

            // Compute the CDF
            for (int i=0, sum=0; i<(COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1); i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int i=0; i<a; i++) {
                int pixel = pixels[i];
                pixels[i] = (s * hist[pixel-COLOR_GRAYSCALE_MIN]) + COLOR_GRAYSCALE_MIN;
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            int a = img->w * img->h;
            float s = (COLOR_Y_MAX-COLOR_Y_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_Y_MAX-COLOR_Y_MIN+1)*sizeof(uint32_t));
            uint16_t *pixels = (uint16_t *) img->pixels;

            // Compute image histogram
            for (int i=0; i<a; i++) {
                hist[COLOR_RGB565_TO_Y(pixels[i])-COLOR_Y_MIN] += 1;
            }

            // Compute the CDF
            for (int i=0, sum=0; i<(COLOR_Y_MAX-COLOR_Y_MIN+1); i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int i=0; i<a; i++) {
                int pixel = pixels[i];
                pixels[i] = imlib_yuv_to_rgb((s * hist[COLOR_RGB565_TO_Y(pixel)-COLOR_Y_MIN]),
                                             COLOR_RGB565_TO_U(pixel),
                                             COLOR_RGB565_TO_V(pixel));
            }

            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

// A simple algorithm for correcting lens distortion.
// See http://www.tannerhelland.com/4743/simple-algorithm-correcting-lens-distortion/
void imlib_lens_corr(image_t *img, float strength, float zoom)
{
    zoom = 1 / zoom;
    int halfWidth = img->w / 2;
    int halfHeight = img->h / 2;
    float lens_corr_radius = strength / fast_sqrtf((img->w * img->w) + (img->h * img->h));

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Create a temp copy of the image to pull pixels from.
            uint32_t *tmp = fb_alloc(((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * img->h);
            memcpy(tmp, img->data, ((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * img->h);
            memset(img->data, 0, ((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * img->h);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;
                float zoomedY = newY * zoom;

                for (int x = 0, xx = img->w; x < xx; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float zoomedX = newX * zoom;

                    float r = lens_corr_radius * fast_sqrtf(newX2 + newY2);
                    float theta = (r < 0.0000001f) ? 1.0f : (fast_atanf(r) / r);
                    int sourceX = halfWidth + fast_roundf(theta * zoomedX);
                    int sourceY = halfHeight + fast_roundf(theta * zoomedY);

                    if ((0 <= sourceX) && (sourceX < img->w) && (0 <= sourceY) && (sourceY < img->h)) {
                        uint32_t *ptr = tmp + (((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sourceY);
                        int pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX);
                        IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, pixel);
                    }
                }
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            // Create a temp copy of the image to pull pixels from.
            uint8_t *tmp = fb_alloc(img->w * img->h * sizeof(uint8_t));
            memcpy(tmp, img->data, img->w * img->h * sizeof(uint8_t));
            memset(img->data, 0, img->w * img->h * sizeof(uint8_t));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;
                float zoomedY = newY * zoom;

                for (int x = 0, xx = img->w; x < xx; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float zoomedX = newX * zoom;

                    float r = lens_corr_radius * fast_sqrtf(newX2 + newY2);
                    float theta = (r < 0.0000001f) ? 1.0f : (fast_atanf(r) / r);
                    int sourceX = halfWidth + fast_roundf(theta * zoomedX);
                    int sourceY = halfHeight + fast_roundf(theta * zoomedY);

                    if ((0 <= sourceX) && (sourceX < img->w) && (0 <= sourceY) && (sourceY < img->h)) {
                        uint8_t *ptr = tmp + (img->w * sourceY);
                        int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(ptr, sourceX);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, pixel);
                    }
                }
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            // Create a temp copy of the image to pull pixels from.
            uint16_t *tmp = fb_alloc(img->w * img->h * sizeof(uint16_t));
            memcpy(tmp, img->data, img->w * img->h * sizeof(uint16_t));
            memset(img->data, 0, img->w * img->h * sizeof(uint16_t));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;
                float zoomedY = newY * zoom;

                for (int x = 0, xx = img->w; x < xx; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float zoomedX = newX * zoom;

                    float r = lens_corr_radius * fast_sqrtf(newX2 + newY2);
                    float theta = (r < 0.0000001f) ? 1.0f : (fast_atanf(r) / r);
                    int sourceX = halfWidth + fast_roundf(theta * zoomedX);
                    int sourceY = halfHeight + fast_roundf(theta * zoomedY);

                    if ((0 <= sourceX) && (sourceX < img->w) && (0 <= sourceY) && (sourceY < img->h)) {
                        uint16_t *ptr = tmp + (img->w * sourceY);
                        int pixel = IMAGE_GET_RGB565_PIXEL_FAST(ptr, sourceX);
                        IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, pixel);
                    }
                }
            }

            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_mask_ellipse(image_t *img)
{
    int h = img->w/2;
    int v = img->h/2;
    int a = h * h;
    int b = v * v;
    uint8_t *pixels = img->pixels;

    for (int y=0; y<img->h; y++) {
        for (int x=0; x<img->w; x++) {
            if ((((x-h)*(x-h)*100) / a + ((y-v)*(y-v)*100) / b) > 100) {
                pixels[y*img->w+x] = 0;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

int imlib_image_mean(image_t *src)
{
    int s=0;
    int n=src->w*src->h;

    for (int i=0; i<n; i++) {
        s += src->pixels[i];
    }

    /* mean */
    return s/n;
}

// One pass standard deviation.
int imlib_image_std(image_t *src)
{
    int w=src->w;
    int h=src->h;
    int n=w*h;
    uint8_t *data=src->pixels;

    uint32_t s=0, sq=0;
    for (int i=0; i<n; i+=2) {
        s += data[i+0]+data[i+1];
        uint32_t tmp = __PKHBT(data[i+0], data[i+1], 16);
        sq = __SMLAD(tmp, tmp, sq);
    }

    if (n%2) {
        s += data[n-1];
        sq += data[n-1]*data[n-1];
    }

    /* mean */
    int m = s/n;

    /* variance */
    uint32_t v = sq/n-(m*m);

    /* std */
    return fast_sqrtf(v);
}

void imlib_sepconv3(image_t *img, const int8_t *krn, const float m, const int b)
{
    int ksize = 3;
    // TODO: Support RGB
    int *buffer = fb_alloc(img->w * 2 * sizeof(*buffer));

    // NOTE: This doesn't deal with borders right now. Adding if
    // statements in the inner loop will slow it down significantly.
    for (int y=0; y<img->h-ksize; y++) {
        for (int x=0; x<img->w-ksize; x+=ksize) {
            for (int k=0; k<ksize; k++) {
                int acc=0;
                //if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j))
                acc = __SMLAD(krn[0], IM_GET_GS_PIXEL(img, x+k, y+0), acc);
                acc = __SMLAD(krn[1], IM_GET_GS_PIXEL(img, x+k, y+1), acc);
                acc = __SMLAD(krn[2], IM_GET_GS_PIXEL(img, x+k, y+2), acc);
                buffer[((y%2)*img->w) + x+k] = acc;
            }
        }
        if (y > 0) {
            // flush buffer
            for (int x=0; x<img->w-ksize; x++) {
                int acc = 0;
                acc = __SMLAD(krn[0], buffer[((y-1)%2) * img->w + x + 0], acc);
                acc = __SMLAD(krn[1], buffer[((y-1)%2) * img->w + x + 1], acc);
                acc = __SMLAD(krn[2], buffer[((y-1)%2) * img->w + x + 2], acc);
                acc = (acc * m) + b; // scale, offset, and clamp
                acc = IM_MAX(IM_MIN(acc, IM_MAX_GS), 0);
                IM_SET_GS_PIXEL(img, (x+1), (y), acc);
            }
        }
    }
    fb_free();
}
