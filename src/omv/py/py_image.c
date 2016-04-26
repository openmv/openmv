/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image Python module.
 *
 */
#include <arm_math.h>
#include <mp.h>
#include "imlib.h"
#include "array.h"
#include "sensor.h"
#include "ff.h"
#include "xalloc.h"
#include "fb_alloc.h"
#include "framebuffer.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"

static const mp_obj_type_t py_cascade_type;
static const mp_obj_type_t py_image_type;

extern const char *ffs_strerror(FRESULT res);

// Haar Cascade ///////////////////////////////////////////////////////////////

typedef struct _py_cascade_obj_t {
    mp_obj_base_t base;
    struct cascade _cobj;
} py_cascade_obj_t;

void *py_cascade_cobj(mp_obj_t cascade)
{
    PY_ASSERT_TYPE(cascade, &py_cascade_type);
    return &((py_cascade_obj_t *)cascade)->_cobj;
}

static void py_cascade_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_cascade_obj_t *self = self_in;
    mp_printf(print, "width:%d height:%d n_stages:%d n_features:%d n_rectangles:%d\n",
            self->_cobj.window.w, self->_cobj.window.h, self->_cobj.n_stages,
            self->_cobj.n_features, self->_cobj.n_rectangles);
}

static const mp_obj_type_t py_cascade_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Cascade,
    .print = py_cascade_print,
};

// Keypoints object ///////////////////////////////////////////////////////////

typedef struct _py_kp_obj_t {
    mp_obj_base_t base;
    array_t *kpts;
    int threshold;
    bool normalized;
} py_kp_obj_t;

static void py_kp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_kp_obj_t *self = self_in;
    mp_printf(print, "size:%d threshold:%d normalized:%d\n", array_length(self->kpts), self->threshold, self->normalized);
}

static const mp_obj_type_t py_kp_type = {
    { &mp_type_type },
    .name  = MP_QSTR_kp_desc,
    .print = py_kp_print,
};

// LBP descriptor /////////////////////////////////////////////////////////////

typedef struct _py_lbp_obj_t {
    mp_obj_base_t base;
    uint8_t *hist;
} py_lbp_obj_t;

static void py_lbp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    mp_printf(print, "<lbp descriptor>");
}

static const mp_obj_type_t py_lbp_type = {
    { &mp_type_type },
    .name  = MP_QSTR_lbp_desc,
    .print = py_lbp_print,
};

// Image //////////////////////////////////////////////////////////////////////

typedef struct _py_image_obj_t {
    mp_obj_base_t base;
    image_t _cobj;
} py_image_obj_t;

void *py_image_cobj(mp_obj_t img_obj)
{
    PY_ASSERT_TYPE(img_obj, &py_image_type);
    return &((py_image_obj_t *)img_obj)->_cobj;
}

static void py_image_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_image_obj_t *self = self_in;
    mp_printf(print, "<image width:%d height:%d bpp:%d>", self->_cobj.w, self->_cobj.h, self->_cobj.bpp);
}

static mp_obj_t py_image_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    py_image_obj_t *o = self_in;
    image_t *arg_img = py_image_cobj(self_in);
    if (value == MP_OBJ_NULL) {
        // delete
        // not supported
        return MP_OBJ_NULL;
    } else if (value == MP_OBJ_SENTINEL) {
        // load
        if (IM_IS_GS(arg_img)) {
            int i = mp_get_index(o->base.type, arg_img->w*arg_img->h, index, false);
            int x = (i % arg_img->w);
            int y = (i / arg_img->w);
            return mp_obj_new_int(IM_GET_GS_PIXEL(arg_img, x, y));
        } else if (IM_IS_RGB565(arg_img)) {
            int i = mp_get_index(o->base.type, arg_img->w*arg_img->h, index, false);
            int x = (i % arg_img->w);
            int y = (i / arg_img->w);
            return mp_obj_new_int(IM_GET_RGB565_PIXEL(arg_img, x, y));
        } else {
            int i = mp_get_index(o->base.type, arg_img->bpp, index, false);
            return mp_obj_new_int(arg_img->pixels[i]); // JPEG
        }
    } else {
        // store
        if (IM_IS_GS(arg_img)) {
            int i = mp_get_index(o->base.type, arg_img->w*arg_img->h, index, false);
            int x = (i % arg_img->w);
            int y = (i / arg_img->w);
            IM_SET_GS_PIXEL(arg_img, x, y, mp_obj_get_int(value));
        } else if (IM_IS_RGB565(arg_img)) {
            int i = mp_get_index(o->base.type, arg_img->w*arg_img->h, index, false);
            int x = (i % arg_img->w);
            int y = (i / arg_img->w);
            IM_SET_RGB565_PIXEL(arg_img, x, y, mp_obj_get_int(value));
        } else {
            int i = mp_get_index(o->base.type, arg_img->bpp, index, false);
            arg_img->pixels[i] = mp_obj_get_int(value); // JPEG
        }
    }
    return mp_const_none;
}

static mp_int_t py_image_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags)
{
    image_t *arg_img = py_image_cobj(self_in);
    if (flags == MP_BUFFER_READ) {
        bufinfo->buf = arg_img->pixels;
        if (IM_IS_JPEG(arg_img)) {
            bufinfo->len = arg_img->bpp;
        } else {
            bufinfo->len = arg_img->w*arg_img->h*arg_img->bpp;
        }
        bufinfo->typecode = 'b';
        return 0;
    } else {
        // not supported
        bufinfo->buf = NULL;
        bufinfo->len = 0;
        bufinfo->typecode = -1;
        return 1;
    }
}

static mp_obj_t py_image_copy(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    mp_obj_t img_obj = py_image(0, 0, 0, 0);
    image_t *img = py_image_cobj(img_obj);

    imlib_copy_image(img, arg_img, &roi);
    return img_obj;
}

static mp_obj_t py_image_save(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    const char *path = mp_obj_str_get_str(args[1]);

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    arg_q = IM_MIN(IM_MAX(arg_q, 1), 100);

    imlib_save_image(arg_img, path, &roi, arg_q);
    return mp_const_none;
}

static mp_obj_t py_image_compress(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    arg_q = IM_MIN(IM_MAX(arg_q, 1), 100);

    // Check if this image is the one in the frame buffer...
    if ((fb->pixels+FB_JPEG_OFFS_SIZE) == arg_img->pixels) {
        // We do not allow shallow copies so this is okay...
        image_t src = {.w=fb->w, .h=fb->h, .bpp=fb->bpp,  .pixels=fb->pixels+FB_JPEG_OFFS_SIZE};
        image_t dst = {.w=fb->w, .h=fb->h, .bpp=128*1024, .pixels=fb->pixels};
        jpeg_compress(&src, &dst, arg_q);
        fb->bpp = dst.bpp;
        arg_img->bpp = dst.bpp;
        arg_img->pixels = dst.pixels;
    } else {
        // Use fb_alloc to compress and then copy image over into old image...
        uint32_t size;
        uint8_t *buffer = fb_alloc_all(&size);
        image_t out = { .w=arg_img->w, .h=arg_img->h, .bpp=size, .pixels=buffer };
        // When jpeg_compress needs more memory than in currently allocated it
        // will try to realloc. MP will detect that the pointer is outside of
        // the heap and return NULL which will cause an out of memory error.
        jpeg_compress(arg_img, &out, arg_q);
        if (out.bpp <= (arg_img->w * arg_img->h * arg_img->bpp)) {
            memcpy(arg_img->pixels, out.pixels, out.bpp);
            arg_img->bpp = out.bpp;
        } else {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Won't fit!"));
        }
        fb_free();
        // Double check this was not the fb.
        // (happens in non-JPEG mode)...
        if (fb->pixels == arg_img->pixels) {
            fb->bpp = arg_img->bpp;
        }
    }

    return mp_const_none;
}

static mp_obj_t py_image_compressed(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    arg_q = IM_MIN(IM_MAX(arg_q, 1), 100);

    // We use fb_alloc here versus xalloc to avoid huge memory hole issues while
    // JPEG compression is running. We don't want to try to compress using the
    // heap because of the massive memory requirement until we are done...

    uint32_t size;
    uint8_t *buffer = fb_alloc_all(&size);
    image_t out = { .w=arg_img->w, .h=arg_img->h, .bpp=size, .pixels=buffer };
    // When jpeg_compress needs more memory than in currently allocated it
    // will try to realloc. MP will detect that the pointer is outside of
    // the heap and return NULL which will cause an out of memory error.
    jpeg_compress(arg_img, &out, arg_q);
    uint8_t *temp = xalloc(out.bpp);
    memcpy(temp, out.pixels, out.bpp);
    out.pixels = temp;
    fb_free();

    return py_image_from_struct(&out);
}

static mp_obj_t py_image_width(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    return mp_obj_new_int(arg_img->w);
}

static mp_obj_t py_image_height(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    return mp_obj_new_int(arg_img->h);
}

static mp_obj_t py_image_format(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    if (IM_IS_GS(arg_img)) {
        return mp_obj_new_int(PIXFORMAT_GRAYSCALE);
    } else if (IM_IS_RGB565(arg_img)) {
        return mp_obj_new_int(PIXFORMAT_RGB565);
    } else {
        return mp_obj_new_int(PIXFORMAT_JPEG);
    }
}

static mp_obj_t py_image_size(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    if (IM_IS_JPEG(arg_img)) {
        return mp_obj_new_int(arg_img->bpp);
    } else {
        return mp_obj_new_int(arg_img->w * arg_img->h * arg_img->bpp);
    }
}

static mp_obj_t py_image_get_pixel(mp_obj_t img_obj, mp_obj_t x_obj, mp_obj_t y_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_x = mp_obj_get_int(x_obj);
    int arg_y = mp_obj_get_int(y_obj);
    if ((!IM_X_INSIDE(arg_img, arg_x)) || (!IM_Y_INSIDE(arg_img, arg_y))) {
        return mp_const_none;
    }

    if (IM_IS_GS(arg_img)) {
        return mp_obj_new_int(IM_GET_GS_PIXEL(arg_img, arg_x, arg_y));
    } else {
        uint16_t pixel = IM_GET_RGB565_PIXEL(arg_img, arg_x, arg_y);
        mp_obj_t pixel_tuple[3];
        pixel_tuple[0] = mp_obj_new_int(IM_R528(IM_R565(pixel)));
        pixel_tuple[1] = mp_obj_new_int(IM_G628(IM_G565(pixel)));
        pixel_tuple[2] = mp_obj_new_int(IM_B528(IM_B565(pixel)));
        return mp_obj_new_tuple(3, pixel_tuple);
    }
}

static mp_obj_t py_image_set_pixel(uint n_args, const mp_obj_t *args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_x = mp_obj_get_int(args[1]);
    int arg_y = mp_obj_get_int(args[2]);
    if ((!IM_X_INSIDE(arg_img, arg_x)) || (!IM_Y_INSIDE(arg_img, arg_y))) {
        return mp_const_none;
    }

    if (IM_IS_GS(arg_img)) {
        IM_SET_GS_PIXEL(arg_img, arg_x, arg_y, mp_obj_get_int(args[3]));
    } else {
        mp_obj_t *arg_color;
        mp_obj_get_array_fixed_n(args[3], 3, &arg_color);
        int red = IM_R825(mp_obj_get_int(arg_color[0]));
        int green = IM_G826(mp_obj_get_int(arg_color[1]));
        int blue = IM_B825(mp_obj_get_int(arg_color[2]));
        IM_SET_RGB565_PIXEL(arg_img, arg_x, arg_y, IM_RGB565(red, green, blue));
    }
    return mp_const_none;
}

static mp_obj_t py_image_draw_line(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    mp_obj_t *arg_vec;
    mp_obj_get_array_fixed_n(args[1], 4, &arg_vec);

    int arg_x0 = mp_obj_get_int(arg_vec[0]);
    int arg_y0 = mp_obj_get_int(arg_vec[1]);
    int arg_x1 = mp_obj_get_int(arg_vec[2]);
    int arg_y1 = mp_obj_get_int(arg_vec[3]);
    int arg_c  = py_helper_lookup_color(kw_args, -1); // white

    imlib_draw_line(arg_img, arg_x0, arg_y0, arg_x1, arg_y1, arg_c);
    return mp_const_none;
}

static mp_obj_t py_image_draw_rectangle(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    mp_obj_t *arg_vec;
    mp_obj_get_array_fixed_n(args[1], 4, &arg_vec);

    int arg_rx = mp_obj_get_int(arg_vec[0]);
    int arg_ry = mp_obj_get_int(arg_vec[1]);
    int arg_rw = mp_obj_get_int(arg_vec[2]);
    int arg_rh = mp_obj_get_int(arg_vec[3]);
    int arg_c  = py_helper_lookup_color(kw_args, -1); // white

    imlib_draw_rectangle(arg_img, arg_rx, arg_ry, arg_rw, arg_rh, arg_c);
    return mp_const_none;
}

static mp_obj_t py_image_draw_circle(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_cx = mp_obj_get_int(args[1]);
    int arg_cy = mp_obj_get_int(args[2]);
    int arg_r  = mp_obj_get_int(args[3]);
    int arg_c  = py_helper_lookup_color(kw_args, -1); // white

    imlib_draw_circle(arg_img, arg_cx, arg_cy, arg_r, arg_c);
    return mp_const_none;
}

static mp_obj_t py_image_draw_string(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_x_off       = mp_obj_get_int(args[1]);
    int arg_y_off       = mp_obj_get_int(args[2]);
    const char *arg_str = mp_obj_str_get_str(args[3]);
    int arg_c           = py_helper_lookup_color(kw_args, -1); // white

    imlib_draw_string(arg_img, arg_x_off, arg_y_off, arg_str, arg_c);
    return mp_const_none;
}

static mp_obj_t py_image_draw_cross(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_x = mp_obj_get_int(args[1]);
    int arg_y = mp_obj_get_int(args[2]);
    int arg_c = py_helper_lookup_color(kw_args, -1); // white
    int arg_s = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_size), 5);

    imlib_draw_line(arg_img, arg_x-arg_s, arg_y      , arg_x+arg_s, arg_y      , arg_c);
    imlib_draw_line(arg_img, arg_x      , arg_y-arg_s, arg_x      , arg_y+arg_s, arg_c);
    return mp_const_none;
}

static mp_obj_t py_image_draw_keypoints(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_c = py_helper_lookup_color(kw_args, -1); // white
    int arg_s = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_size), 10);

    if (MP_OBJ_IS_TYPE(args[1],&mp_type_tuple)||(MP_OBJ_IS_TYPE(args[1],&mp_type_list))) {
        mp_uint_t arg_vec_len;
        mp_obj_t *arg_vec;
        mp_obj_get_array(args[1], &arg_vec_len, &arg_vec);
        if (!arg_vec_len) return mp_const_none;
        for (int i=0; i<arg_vec_len; i++) {
            mp_obj_t *arg_keypoint;
            mp_obj_get_array_fixed_n(arg_vec[i], 3, &arg_keypoint);
            int x = mp_obj_get_int(arg_keypoint[0]);
            int y = mp_obj_get_int(arg_keypoint[1]);
            float angle = mp_obj_get_float(arg_keypoint[2]);
            float co = arm_cos_f32(angle);
            float si = arm_sin_f32(angle);
            imlib_draw_line(arg_img, x, y, x+(co*arg_s), y+(si*arg_s), arg_c);
            imlib_draw_circle(arg_img, x, y, (arg_s-2)/2, arg_c);
        }
    } else {
        py_kp_obj_t *kpts_obj = ((py_kp_obj_t*)args[1]);
        PY_ASSERT_TYPE(kpts_obj, &py_kp_type);
        for (int i=0; i<array_length(kpts_obj->kpts); i++) {
            kp_t *kp = array_at(kpts_obj->kpts, i);
            imlib_draw_circle(arg_img, kp->x, kp->y, (arg_s-2)/2, arg_c);
        }
    }
    return mp_const_none;
}

static mp_obj_t py_image_binary(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    mp_uint_t arg_t_len;
    mp_obj_t *arg_t;
    mp_obj_get_array(args[1], &arg_t_len, &arg_t);
    if (!arg_t_len) return mp_const_none;

    simple_color_t l_t[arg_t_len], u_t[arg_t_len];
    if (IM_IS_GS(arg_img)) {
        for (int i=0; i<arg_t_len; i++) {
            mp_obj_t *temp;
            mp_obj_get_array_fixed_n(arg_t[i], 2, &temp);
            int lo = mp_obj_get_int(temp[0]);
            int hi = mp_obj_get_int(temp[1]);
            // Swap ranges if they are wrong.
            l_t[i].G = IM_MIN(lo, hi);
            u_t[i].G = IM_MAX(lo, hi);
        }
    } else {
        for (int i=0; i<arg_t_len; i++) {
            mp_obj_t *temp;
            mp_obj_get_array_fixed_n(arg_t[i], 6, &temp);
            int l_lo = mp_obj_get_int(temp[0]);
            int l_hi = mp_obj_get_int(temp[1]);
            int a_lo = mp_obj_get_int(temp[2]);
            int a_hi = mp_obj_get_int(temp[3]);
            int b_lo = mp_obj_get_int(temp[4]);
            int b_hi = mp_obj_get_int(temp[5]);
            // Swap ranges if they are wrong.
            l_t[i].L = IM_MIN(l_lo, l_hi);
            u_t[i].L = IM_MAX(l_lo, l_hi);
            l_t[i].A = IM_MIN(a_lo, a_hi);
            u_t[i].A = IM_MAX(a_lo, a_hi);
            l_t[i].B = IM_MIN(b_lo, b_hi);
            u_t[i].B = IM_MAX(b_lo, b_hi);
        }
    }

    int arg_invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), 0);
    imlib_binary(arg_img, arg_t_len, l_t, u_t, arg_invert ? 1 : 0);
    return mp_const_none;
}

static mp_obj_t py_image_invert(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    imlib_invert(arg_img);
    return mp_const_none;
}

static mp_obj_t py_image_and(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_and(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_and(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_nand(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_nand(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_nand(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_or(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_or(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_or(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_nor(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_nor(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_nor(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_xor(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_xor(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_xor(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_xnor(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_xnor(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_xnor(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_erode(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");
    imlib_erode(arg_img, arg_ksize,
            py_helper_lookup_int(kw_args,
            MP_OBJ_NEW_QSTR(MP_QSTR_threshold), ((arg_ksize*2)+1)*((arg_ksize*2)+1)-1));
    return mp_const_none;
}

static mp_obj_t py_image_dilate(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");
    imlib_dilate(arg_img, arg_ksize,
            py_helper_lookup_int(kw_args,
            MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 0));
    return mp_const_none;
}

static mp_obj_t py_image_negate(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    imlib_negate(arg_img);
    return mp_const_none;
}

static mp_obj_t py_image_difference(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_difference(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_difference(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_replace(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_replace(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_replace(arg_img, NULL, arg_other);
    }
    return mp_const_none;
}

static mp_obj_t py_image_blend(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int alpha = IM_MIN(IM_MAX(py_helper_lookup_int(kw_args,
        MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 128), 0), 256);

    if (MP_OBJ_IS_STR(args[1])) {
        imlib_blend(arg_img, mp_obj_str_get_str(args[1]), NULL, alpha);
    } else {
        image_t *arg_other = py_image_cobj(args[1]);
        imlib_blend(arg_img, NULL, arg_other, alpha);
    }
    return mp_const_none;
}

static mp_obj_t py_image_morph(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    int array_size = ((arg_ksize*2)+1)*((arg_ksize*2)+1);
    mp_obj_t *krn;
    mp_obj_get_array_fixed_n(args[2], array_size, &krn);

    int8_t arg_krn[array_size];
    int arg_m = 0;
    for (int i = 0; i < array_size; i++) {
        int value = mp_obj_get_int(krn[i]);
        PY_ASSERT_FALSE_MSG((value < -128) || (127 < value),
                "Kernel Values must be between [-128:127] inclusive");
        arg_krn[i] = value;
        arg_m += arg_krn[i];
    }

    if (arg_m == 0) {
        arg_m = 1;
    }

    imlib_morph(arg_img, arg_ksize, arg_krn,
            py_helper_lookup_float(kw_args,
                    MP_OBJ_NEW_QSTR(MP_QSTR_mul), 1.0 / ((float) arg_m)),
            py_helper_lookup_int(kw_args,
                    MP_OBJ_NEW_QSTR(MP_QSTR_add), 0));
    return mp_const_none;
}

static mp_obj_t py_image_statistics(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    rectangle_t arg_r;
    py_helper_lookup_rectangle(kw_args, arg_img, &arg_r);
    statistics_t out;
    imlib_statistics(arg_img, &arg_r, &out);

    if (IM_IS_GS(arg_img)) {
        return mp_obj_new_tuple(8, (mp_obj_t[8])
                {mp_obj_new_int(out.g_mean), mp_obj_new_int(out.g_median),
                 mp_obj_new_int(out.g_mode), mp_obj_new_int(out.g_st_dev),
                 mp_obj_new_int(out.g_min), mp_obj_new_int(out.g_max),
                 mp_obj_new_int(out.g_lower_q), mp_obj_new_int(out.g_upper_q)});
    } else {
        return mp_obj_new_tuple(24, (mp_obj_t[24])
                {mp_obj_new_int(out.l_mean), mp_obj_new_int(out.l_median),
                 mp_obj_new_int(out.l_mode), mp_obj_new_int(out.l_st_dev),
                 mp_obj_new_int(out.l_min), mp_obj_new_int(out.l_max),
                 mp_obj_new_int(out.l_lower_q), mp_obj_new_int(out.l_upper_q),
                 mp_obj_new_int(out.a_mean), mp_obj_new_int(out.a_median),
                 mp_obj_new_int(out.a_mode), mp_obj_new_int(out.a_st_dev),
                 mp_obj_new_int(out.a_min), mp_obj_new_int(out.a_max),
                 mp_obj_new_int(out.a_lower_q), mp_obj_new_int(out.a_upper_q),
                 mp_obj_new_int(out.b_mean), mp_obj_new_int(out.b_median),
                 mp_obj_new_int(out.b_mode), mp_obj_new_int(out.b_st_dev),
                 mp_obj_new_int(out.b_min), mp_obj_new_int(out.b_max),
                 mp_obj_new_int(out.b_lower_q), mp_obj_new_int(out.b_upper_q)});
    }
}

static mp_obj_t py_image_midpoint(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    int bias = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bias), 0.5) * 256;
    imlib_midpoint_filter(arg_img, arg_ksize, IM_MIN(IM_MAX(bias, 0), 256));
    return mp_const_none;
}

static mp_obj_t py_image_mean(mp_obj_t img_obj, mp_obj_t k_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(k_obj);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    imlib_mean_filter(arg_img, arg_ksize);
    return mp_const_none;
}

static mp_obj_t py_image_mode(mp_obj_t img_obj, mp_obj_t k_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(k_obj);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    imlib_mode_filter(arg_img, arg_ksize);
    return mp_const_none;
}

static mp_obj_t py_image_median(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");
    PY_ASSERT_TRUE_MSG(arg_ksize <= 2, "Kernel Size must be <= 2");

    int n = ((arg_ksize*2)+1)*((arg_ksize*2)+1);
    int percentile = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_percentile), 0.5) * n;
    imlib_median_filter(arg_img, arg_ksize, IM_MIN(IM_MAX(percentile, 0), n-1));
    return mp_const_none;
}

static mp_obj_t py_image_histeq(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    imlib_histeq(arg_img);
    return mp_const_none;
}

static bool py_image_find_blobs_f_fun(void *fun_obj, void *img_obj, color_blob_t *cb)
{
    mp_obj_t blob_obj[10] = {
        mp_obj_new_int(cb->x),
        mp_obj_new_int(cb->y),
        mp_obj_new_int(cb->w),
        mp_obj_new_int(cb->h),
        mp_obj_new_int(cb->pixels),
        mp_obj_new_int(cb->cx),
        mp_obj_new_int(cb->cy),
        mp_obj_new_float(cb->rotation),
        mp_obj_new_int(cb->code),
        mp_obj_new_int(cb->count)
    };
    return mp_obj_is_true(mp_call_function_2(fun_obj, img_obj, mp_obj_new_tuple(10, blob_obj)));
}

static mp_obj_t py_image_find_blobs(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    mp_uint_t arg_t_len;
    mp_obj_t *arg_t;
    mp_obj_get_array(args[1], &arg_t_len, &arg_t);
    if (!arg_t_len) return mp_obj_new_list(0, NULL); // return an empty array to be iteratable

    simple_color_t l_t[arg_t_len], u_t[arg_t_len];
    if (IM_IS_GS(arg_img)) {
        for (int i=0; i<arg_t_len; i++) {
            mp_obj_t *temp;
            mp_obj_get_array_fixed_n(arg_t[i], 2, &temp);
            int lo = mp_obj_get_int(temp[0]);
            int hi = mp_obj_get_int(temp[1]);
            // Swap ranges if they are wrong.
            l_t[i].G = IM_MIN(lo, hi);
            u_t[i].G = IM_MAX(lo, hi);
        }
    } else {
        for (int i=0; i<arg_t_len; i++) {
            mp_obj_t *temp;
            mp_obj_get_array_fixed_n(arg_t[i], 6, &temp);
            int l_lo = mp_obj_get_int(temp[0]);
            int l_hi = mp_obj_get_int(temp[1]);
            int a_lo = mp_obj_get_int(temp[2]);
            int a_hi = mp_obj_get_int(temp[3]);
            int b_lo = mp_obj_get_int(temp[4]);
            int b_hi = mp_obj_get_int(temp[5]);
            // Swap ranges if they are wrong.
            l_t[i].L = IM_MIN(l_lo, l_hi);
            u_t[i].L = IM_MAX(l_lo, l_hi);
            l_t[i].A = IM_MIN(a_lo, a_hi);
            u_t[i].A = IM_MAX(a_lo, a_hi);
            l_t[i].B = IM_MIN(b_lo, b_hi);
            u_t[i].B = IM_MAX(b_lo, b_hi);
        }
    }

    rectangle_t arg_r;
    py_helper_lookup_rectangle(kw_args, arg_img, &arg_r);

    mp_map_elem_t *kw_arg = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_feature_filter), MP_MAP_LOOKUP);
    mp_obj_t kw_val = (kw_arg != NULL) ? kw_arg->value : NULL;

    int arg_invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), 0);
    array_t *blobs_list = imlib_find_blobs(arg_img, arg_t_len, l_t, u_t, arg_invert ? 1 : 0, &arg_r,
                                           py_image_find_blobs_f_fun, kw_val, args[0]);
    if (blobs_list == NULL) {
        return mp_obj_new_list(0, NULL); // return an empty array to be iteratable
    }
    mp_obj_t objects_list = mp_obj_new_list(0, NULL);
    for (int i=0, j=array_length(blobs_list); i<j; i++) {
        color_blob_t *cb = array_at(blobs_list, i);
        mp_obj_t blob_obj[10] = {
            mp_obj_new_int(cb->x),
            mp_obj_new_int(cb->y),
            mp_obj_new_int(cb->w),
            mp_obj_new_int(cb->h),
            mp_obj_new_int(cb->pixels),
            mp_obj_new_int(cb->cx),
            mp_obj_new_int(cb->cy),
            mp_obj_new_float(cb->rotation),
            mp_obj_new_int(cb->code),
            mp_obj_new_int(cb->count)
        };
        mp_obj_list_append(objects_list, mp_obj_new_tuple(10, blob_obj));
    }
    array_free(blobs_list);
    return objects_list;
}

static bool py_image_find_markers_f_fun(void *fun_obj, void *img_obj, color_blob_t *cb)
{
    mp_obj_t blob_obj[10] = {
        mp_obj_new_int(cb->x),
        mp_obj_new_int(cb->y),
        mp_obj_new_int(cb->w),
        mp_obj_new_int(cb->h),
        mp_obj_new_int(cb->pixels),
        mp_obj_new_int(cb->cx),
        mp_obj_new_int(cb->cy),
        mp_obj_new_float(cb->rotation),
        mp_obj_new_int(cb->code),
        mp_obj_new_int(cb->count)
    };
    return mp_obj_is_true(mp_call_function_2(fun_obj, img_obj, mp_obj_new_tuple(10, blob_obj)));
}

static mp_obj_t py_image_find_markers(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_FALSE_MSG(IM_IS_JPEG(arg_img),
            "Operation not supported on JPEG");

    int margin = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_margin), 2);

    mp_map_elem_t *kw_arg = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_feature_filter), MP_MAP_LOOKUP);
    mp_obj_t kw_val = (kw_arg != NULL) ? kw_arg->value : NULL;

    mp_uint_t arg_t_len;
    mp_obj_t *arg_t;
    mp_obj_get_array(args[1], &arg_t_len, &arg_t);
    if (!arg_t_len) return mp_obj_new_list(0, NULL); // return an empty array to be iteratable

    array_t *blobs_list;
    array_alloc_init(&blobs_list, xfree, arg_t_len);
    for (int i=0; i<arg_t_len; i++) {
        mp_obj_t *temp;
        mp_obj_get_array_fixed_n(arg_t[i], 10, &temp);
        color_blob_t *cb = xalloc(sizeof(color_blob_t));
        cb->x = mp_obj_get_int(temp[0]);
        cb->y = mp_obj_get_int(temp[1]);
        cb->w = mp_obj_get_int(temp[2]);
        cb->h = mp_obj_get_int(temp[3]);
        cb->pixels = mp_obj_get_int(temp[4]);
        cb->cx = mp_obj_get_int(temp[5]);
        cb->cy = mp_obj_get_int(temp[6]);
        cb->rotation = mp_obj_get_float(temp[7]);
        cb->code = mp_obj_get_int(temp[8]);
        cb->count = mp_obj_get_int(temp[9]);
        array_push_back(blobs_list, cb);
    }
    array_t *blobs_list_ret = imlib_find_markers(blobs_list, margin,
                                                 py_image_find_markers_f_fun, kw_val, args[0]);
    if (blobs_list_ret == NULL) {
        return mp_obj_new_list(0, NULL); // return an empty array to be iteratable
    }
    array_free(blobs_list);
    mp_obj_t objects_list = mp_obj_new_list(0, NULL);
    for (int i=0, j=array_length(blobs_list_ret); i<j; i++) {
        color_blob_t *cb = array_at(blobs_list_ret, i);
        mp_obj_t blob_obj[10] = {
            mp_obj_new_int(cb->x),
            mp_obj_new_int(cb->y),
            mp_obj_new_int(cb->w),
            mp_obj_new_int(cb->h),
            mp_obj_new_int(cb->pixels),
            mp_obj_new_int(cb->cx),
            mp_obj_new_int(cb->cy),
            mp_obj_new_float(cb->rotation),
            mp_obj_new_int(cb->code),
            mp_obj_new_int(cb->count)
        };
        mp_obj_list_append(objects_list, mp_obj_new_tuple(10, blob_obj));
    }
    array_free(blobs_list_ret);
    return objects_list;
}

static mp_obj_t py_image_find_features(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    cascade_t *cascade = py_cascade_cobj(args[1]);
    cascade->threshold = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 0.5f);
    cascade->scale_factor = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale), 1.5f);

    rectangle_t arg_r;
    py_helper_lookup_rectangle(kw_args, arg_img, &arg_r);

    rectangle_t rect;
    if (!rectangle_subimg(arg_img, &arg_r, &rect)) {
        return mp_obj_new_list(0, NULL);
    }

    // Make sure ROI is bigger than feature size
    PY_ASSERT_TRUE_MSG((rect.w > cascade->window.w && rect.h > cascade->window.h),
            "Region of interest is smaller than detector window!");

    // Detect objects
    array_t *objects_array = imlib_detect_objects(arg_img, cascade, &rect);

    // Add detected objects to a new Python list...
    mp_obj_t objects_list = mp_obj_new_list(0, NULL);
    for (int i=0; i<array_length(objects_array); i++) {
        rectangle_t *r = array_at(objects_array, i);
        mp_obj_t rec_obj[4] = {
            mp_obj_new_int(r->x),
            mp_obj_new_int(r->y),
            mp_obj_new_int(r->w),
            mp_obj_new_int(r->h),
        };
        mp_obj_list_append(objects_list, mp_obj_new_tuple(4, rec_obj));
    }
    array_free(objects_array);
    return objects_list;
}

static mp_obj_t py_image_find_eye(mp_obj_t img_obj, mp_obj_t roi_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    mp_obj_t *array;
    mp_obj_get_array_fixed_n(roi_obj, 4, &array);

    rectangle_t arg_r = {
        mp_obj_get_int(array[0]),
        mp_obj_get_int(array[1]),
        mp_obj_get_int(array[2]),
        mp_obj_get_int(array[3]),
    };

    rectangle_t rect;
    if (!rectangle_subimg(arg_img, &arg_r, &rect)) {
        return mp_const_none;
    }

    point_t iris;
    imlib_find_iris(arg_img, &iris, &rect);

    mp_obj_t eye_obj[2] = {
        mp_obj_new_int(iris.x),
        mp_obj_new_int(iris.y),
    };

    return mp_obj_new_tuple(2, eye_obj);
}

static mp_obj_t py_image_find_template(mp_obj_t img_obj, mp_obj_t template_obj, mp_obj_t threshold)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    image_t *arg_template = py_image_cobj(template_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_template),
            "This function is only supported on GRAYSCALE images");

    float t = mp_obj_get_float(threshold);

    rectangle_t r;
    float corr = imlib_template_match(arg_img, arg_template, &r);

    if (corr > t) {
        mp_obj_t rec_obj[4] = {
            mp_obj_new_int(r.x),
            mp_obj_new_int(r.y),
            mp_obj_new_int(r.w),
            mp_obj_new_int(r.h)
        };
        return mp_obj_new_tuple(4, rec_obj);
    }
    return mp_const_none;
}

static mp_obj_t py_image_find_lbp(mp_obj_t img_obj, mp_obj_t roi_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    mp_obj_t *array;
    mp_obj_get_array_fixed_n(roi_obj, 4, &array);

    rectangle_t roi = {
        mp_obj_get_int(array[0]),
        mp_obj_get_int(array[1]),
        mp_obj_get_int(array[2]),
        mp_obj_get_int(array[3]),
    };

    py_lbp_obj_t *lbp_obj = m_new_obj(py_lbp_obj_t);
    lbp_obj->base.type = &py_lbp_type;
    lbp_obj->hist = imlib_lbp_cascade(arg_img, &roi);
    return lbp_obj;
}

static mp_obj_t py_image_find_keypoints(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    rectangle_t arg_r;
    py_helper_lookup_rectangle(kw_args, arg_img, &arg_r);

    rectangle_t rect;
    if (!rectangle_subimg(arg_img, &arg_r, &rect)) {
        return mp_const_none;
    }

    int threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 32);
    bool normalized = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_normalized), false);

    array_t *kpts = freak_find_keypoints(arg_img, normalized, threshold, &rect);

    if (array_length(kpts)) {
        py_kp_obj_t *kp_obj = m_new_obj(py_kp_obj_t);
        kp_obj->base.type = &py_kp_type;
        kp_obj->kpts = kpts;
        kp_obj->threshold = threshold;
        kp_obj->normalized = normalized;
        return kp_obj;
    }
    return mp_const_none;
}

/* Image file functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_copy_obj, 1, py_image_copy);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_obj, 2, py_image_save);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_compress_obj, 1, py_image_compress);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_compressed_obj, 1, py_image_compressed);
/* Basic image functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_width_obj, py_image_width);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_height_obj, py_image_height);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_format_obj, py_image_format);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_size_obj, py_image_size);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_get_pixel_obj, py_image_get_pixel);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_image_set_pixel_obj, 4, 4, py_image_set_pixel);
/* Drawing functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_draw_line_obj, 2, py_image_draw_line);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_draw_rectangle_obj, 2, py_image_draw_rectangle);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_draw_circle_obj, 4, py_image_draw_circle);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_draw_string_obj, 4, py_image_draw_string);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_draw_cross_obj, 3, py_image_draw_cross);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_draw_keypoints_obj, 2, py_image_draw_keypoints);
/* Binary functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_binary_obj, 2, py_image_binary);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_invert_obj, py_image_invert);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_and_obj, py_image_and);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_nand_obj, py_image_nand);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_or_obj, py_image_or);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_nor_obj, py_image_nor);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_xor_obj, py_image_xor);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_xnor_obj, py_image_xnor);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_erode_obj, 2, py_image_erode);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_dilate_obj, 2, py_image_dilate);
/* Background Subtraction (Frame Differencing) functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_negate_obj, py_image_negate);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_difference_obj, py_image_difference);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_replace_obj, py_image_replace);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_blend_obj, 2, py_image_blend);
/* Image Morphing */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_morph_obj, 3, py_image_morph);
/* Image Statistics */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_statistics_obj, 1, py_image_statistics);
/* Image Filtering */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_midpoint_obj, 2, py_image_midpoint);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_mean_obj, py_image_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_mode_obj, py_image_mode);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_median_obj, 2, py_image_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_histeq_obj, py_image_histeq);
/* Color Tracking */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_blobs_obj, 2, py_image_find_blobs);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_markers_obj, 2, py_image_find_markers);
/* Feature Detection */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_features_obj, 2, py_image_find_features);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_eye_obj, py_image_find_eye);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_find_template_obj, py_image_find_template);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_lbp_obj, py_image_find_lbp);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_keypoints_obj, 1, py_image_find_keypoints);
static const mp_map_elem_t locals_dict_table[] = {
    /* Image file functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_copy),                (mp_obj_t)&py_image_copy_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_save),                (mp_obj_t)&py_image_save_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compress),            (mp_obj_t)&py_image_compress_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compressed),          (mp_obj_t)&py_image_compressed_obj},
    /* Basic image functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_width),               (mp_obj_t)&py_image_width_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_height),              (mp_obj_t)&py_image_height_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_format),              (mp_obj_t)&py_image_format_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_size),                (mp_obj_t)&py_image_size_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel),           (mp_obj_t)&py_image_get_pixel_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel),           (mp_obj_t)&py_image_set_pixel_obj},
    /* Drawing functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_line),           (mp_obj_t)&py_image_draw_line_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_rectangle),      (mp_obj_t)&py_image_draw_rectangle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_circle),         (mp_obj_t)&py_image_draw_circle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_string),         (mp_obj_t)&py_image_draw_string_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_cross),          (mp_obj_t)&py_image_draw_cross_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_keypoints),      (mp_obj_t)&py_image_draw_keypoints_obj},
    /* Binary functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_binary),              (mp_obj_t)&py_image_binary_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_invert),              (mp_obj_t)&py_image_invert_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_and),                 (mp_obj_t)&py_image_and_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_nand),                (mp_obj_t)&py_image_nand_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_or),                  (mp_obj_t)&py_image_or_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_nor),                 (mp_obj_t)&py_image_nor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_xor),                 (mp_obj_t)&py_image_xor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_xnor),                (mp_obj_t)&py_image_xnor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_erode),               (mp_obj_t)&py_image_erode_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_dilate),              (mp_obj_t)&py_image_dilate_obj},
    /* Background Subtraction (Frame Differencing) functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_negate),              (mp_obj_t)&py_image_negate_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_difference),          (mp_obj_t)&py_image_difference_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_replace),             (mp_obj_t)&py_image_replace_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blend),               (mp_obj_t)&py_image_blend_obj},
    /* Image Morphing */
    {MP_OBJ_NEW_QSTR(MP_QSTR_morph),               (mp_obj_t)&py_image_morph_obj},
    /* Image Statistics */
    {MP_OBJ_NEW_QSTR(MP_QSTR_statistics),          (mp_obj_t)&py_image_statistics_obj},
    /* Image Filtering */
    {MP_OBJ_NEW_QSTR(MP_QSTR_midpoint),            (mp_obj_t)&py_image_midpoint_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_mean),                (mp_obj_t)&py_image_mean_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_mode),                (mp_obj_t)&py_image_mode_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_median),              (mp_obj_t)&py_image_median_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_histeq),              (mp_obj_t)&py_image_histeq_obj},
    /* Color Tracking */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_blobs),          (mp_obj_t)&py_image_find_blobs_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_markers),        (mp_obj_t)&py_image_find_markers_obj},
    /* Feature Detection */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_features),       (mp_obj_t)&py_image_find_features_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_eye),            (mp_obj_t)&py_image_find_eye_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_template),       (mp_obj_t)&py_image_find_template_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_lbp),            (mp_obj_t)&py_image_find_lbp_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_keypoints),      (mp_obj_t)&py_image_find_keypoints_obj},
    { NULL, NULL },
};
STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_image_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Image,
    .print = py_image_print,
    .buffer_p = { .get_buffer = py_image_get_buffer },
    .subscr = py_image_subscr,
    .locals_dict = (mp_obj_t)&locals_dict,
};

mp_obj_t py_image(int w, int h, int bpp, void *pixels)
{
    py_image_obj_t *o = m_new_obj(py_image_obj_t);
    o->base.type = &py_image_type;
    o->_cobj.w = w;
    o->_cobj.h = h;
    o->_cobj.bpp = bpp;
    o->_cobj.pixels = pixels;
    return o;
}

mp_obj_t py_image_from_struct(image_t *img)
{
    py_image_obj_t *o = m_new_obj(py_image_obj_t);
    o->base.type = &py_image_type;
    o->_cobj = *img;
    return o;
}

mp_obj_t py_image_rgb_to_lab(mp_obj_t tuple)
{
    mp_obj_t *rgb;
    mp_obj_get_array_fixed_n(tuple, 3, &rgb);

    simple_color_t rgb_color, lab_color;
    rgb_color.red = mp_obj_get_int(rgb[0]);
    rgb_color.green = mp_obj_get_int(rgb[1]);
    rgb_color.blue = mp_obj_get_int(rgb[2]);
    imlib_rgb_to_lab(&rgb_color, &lab_color);

    return mp_obj_new_tuple(3, (mp_obj_t[3])
            {mp_obj_new_int(lab_color.L),
             mp_obj_new_int(lab_color.A),
             mp_obj_new_int(lab_color.B)});
}

mp_obj_t py_image_lab_to_rgb(mp_obj_t tuple)
{
    mp_obj_t *lab;
    mp_obj_get_array_fixed_n(tuple, 3, &lab);

    simple_color_t lab_color, rgb_color;
    lab_color.L = mp_obj_get_int(lab[0]);
    lab_color.A = mp_obj_get_int(lab[1]);
    lab_color.B = mp_obj_get_int(lab[2]);
    imlib_lab_to_rgb(&lab_color, &rgb_color);

    return mp_obj_new_tuple(3, (mp_obj_t[3])
            {mp_obj_new_int(rgb_color.red),
             mp_obj_new_int(rgb_color.green),
             mp_obj_new_int(rgb_color.blue)});
}

mp_obj_t py_image_rgb_to_grayscale(mp_obj_t tuple)
{
    mp_obj_t *rgb;
    mp_obj_get_array_fixed_n(tuple, 3, &rgb);

    simple_color_t rgb_color, grayscale_color;
    rgb_color.red = mp_obj_get_int(rgb[0]);
    rgb_color.green = mp_obj_get_int(rgb[1]);
    rgb_color.blue = mp_obj_get_int(rgb[2]);
    imlib_rgb_to_grayscale(&rgb_color, &grayscale_color);

    return mp_obj_new_int(grayscale_color.G);
}

mp_obj_t py_image_grayscale_to_rgb(mp_obj_t not_tuple)
{
    simple_color_t grayscale_color, rgb_color;
    grayscale_color.G = mp_obj_get_int(not_tuple);
    imlib_grayscale_to_rgb(&grayscale_color, &rgb_color);

    return mp_obj_new_tuple(3, (mp_obj_t[3])
            {mp_obj_new_int(rgb_color.red),
             mp_obj_new_int(rgb_color.green),
             mp_obj_new_int(rgb_color.blue)});
}

mp_obj_t py_image_load_image(mp_obj_t path_obj)
{
    mp_obj_t image_obj = py_image(0, 0, 0, 0);
    imlib_load_image(py_image_cobj(image_obj), mp_obj_str_get_str(path_obj));
    return image_obj;
}

mp_obj_t py_image_load_cascade(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    cascade_t cascade;
    const char *path = mp_obj_str_get_str(args[0]);

    // Load cascade from file or flash
    int res = imlib_load_cascade(&cascade, path);
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    // Read the number of stages
    int stages = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("stages")), cascade.n_stages);
    // Check the number of stages
    if (stages > 0 && stages < cascade.n_stages) {
        cascade.n_stages = stages;
    }

    // Return micropython cascade object
    py_cascade_obj_t *o = m_new_obj(py_cascade_obj_t);
    o->base.type = &py_cascade_type;
    o->_cobj = cascade;
    return o;
}

mp_obj_t py_image_load_descriptor(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    FIL fp;
    FRESULT res;

    mp_obj_t desc = mp_const_none;
    descriptor_t desc_type = mp_obj_get_int(args[0]);
    const char *path = mp_obj_str_get_str(args[1]);

    if ((res = f_open(&fp, path, FA_READ|FA_OPEN_EXISTING)) == FR_OK) {
        switch (desc_type) {
            case DESC_LBP: {
                py_lbp_obj_t *lbp = m_new_obj(py_lbp_obj_t);
                lbp->base.type = &py_lbp_type;

                res = imlib_lbp_desc_load(&fp, &lbp->hist);
                if (res == FR_OK) {
                    desc = lbp;
                }
                break;
            }

            case DESC_FREAK: {
                array_t *kpts = NULL;
                array_alloc(&kpts, xfree);

                res = freak_load_descriptor(&fp, kpts);
                if (res == FR_OK) {
                    // Return keypoints MP object
                    py_kp_obj_t *kp_obj = m_new_obj(py_kp_obj_t);
                    kp_obj->base.type = &py_kp_type;
                    kp_obj->kpts = kpts;
                    kp_obj->threshold = 10;
                    kp_obj->normalized = false;
                    desc = kp_obj;
                }
                break;
            }
        }

        f_close(&fp);
    }

    // File open or write error
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    // If no file error and descriptor is still none, then it's not supported.
    if (desc == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Descriptor type is not supported"));
    }
    return desc;
}

mp_obj_t py_image_save_descriptor(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    FIL fp;
    FRESULT res;

    descriptor_t desc_type = mp_obj_get_int(args[0]);
    const char *path = mp_obj_str_get_str(args[1]);

    if ((res = f_open(&fp, path, FA_WRITE|FA_CREATE_ALWAYS)) == FR_OK) {
        switch (desc_type) {
            case DESC_LBP: {
                py_lbp_obj_t *lbp = ((py_lbp_obj_t*)args[2]);
                res = imlib_lbp_desc_save(&fp, lbp->hist);
                break;
            }

            case DESC_FREAK: {
                py_kp_obj_t *kpts = ((py_kp_obj_t*)args[2]);
                res = freak_save_descriptor(&fp, kpts->kpts);
                break;
            }
        }
        // ignore unsupported descriptors when saving

        f_close(&fp);
    }

    // File open or read error
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    return mp_const_true;
}

static mp_obj_t py_image_match_descriptor(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    mp_obj_t match_obj = mp_const_none;
    descriptor_t desc_type = mp_obj_get_int(args[0]);

    switch (desc_type) {
        case DESC_LBP: {
            py_lbp_obj_t *lbp1 = ((py_lbp_obj_t*)args[1]);
            py_lbp_obj_t *lbp2 = ((py_lbp_obj_t*)args[2]);

            // Sanity checks
            PY_ASSERT_TYPE(lbp1, &py_lbp_type);
            PY_ASSERT_TYPE(lbp2, &py_lbp_type);

            match_obj = mp_obj_new_int(imlib_lbp_desc_distance(lbp1->hist, lbp2->hist));
            break;
        }

        case DESC_FREAK: {
            py_kp_obj_t *kpts1 = ((py_kp_obj_t*)args[1]);
            py_kp_obj_t *kpts2 = ((py_kp_obj_t*)args[2]);
            int threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 60);

            // Sanity checks
            PY_ASSERT_TYPE(kpts1, &py_kp_type);
            PY_ASSERT_TYPE(kpts2, &py_kp_type);
            PY_ASSERT_TRUE_MSG((threshold >=0 && threshold <= 100), "Expected threshold between 0 and 100");

            int match=0, cx=0, cy=0;
            // Match the two keypoint sets
            // Returns the number of matches
            match = freak_match_keypoints(kpts1->kpts, kpts2->kpts, threshold);

            if (match) {
                for (int i=0; i<array_length(kpts1->kpts); i++) {
                    kp_t *kp = array_at(kpts1->kpts, i);
                    if (kp->match != NULL) {
                        cx += kp->match->x;
                        cy += kp->match->y;
                    }
                }
            }

            mp_obj_t rec_obj[3] = {
                mp_obj_new_int(cx/match),
                mp_obj_new_int(cy/match),
                mp_obj_new_int(match*100/IM_MAX(array_length(kpts1->kpts), 1))
            };
            match_obj = mp_obj_new_tuple(3, rec_obj);
        }
    }

    return match_obj;
}

int py_image_descriptor_from_roi(image_t *img, const char *path, rectangle_t *roi)
{
    FIL fp;
    FRESULT res = FR_OK;

    printf("Save Descriptor: ROI(%d %d %d %d)\n", roi->x, roi->y, roi->w, roi->h);
    array_t *kpts = freak_find_keypoints(img, false, 10, roi);
    printf("Save Descriptor: KPTS(%d)\n", array_length(kpts));

    if (array_length(kpts)) {
        if ((res = f_open(&fp, path, FA_WRITE|FA_CREATE_ALWAYS)) == FR_OK) {
            res = freak_save_descriptor(&fp, kpts);
            f_close(&fp);
        }
        // File open/write error
        if (res != FR_OK) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
        }
    }
    return 0;
}

/* Color space functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_rgb_to_lab_obj, py_image_rgb_to_lab);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_lab_to_rgb_obj, py_image_lab_to_rgb);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_rgb_to_grayscale_obj, py_image_rgb_to_grayscale);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_grayscale_to_rgb_obj, py_image_grayscale_to_rgb);
/* Image Module Functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_image_obj, py_image_load_image);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_cascade_obj, 1, py_image_load_cascade);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_descriptor_obj, 2, py_image_load_descriptor);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_descriptor_obj, 3, py_image_save_descriptor);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_match_descriptor_obj, 3, py_image_match_descriptor);
static const mp_map_elem_t globals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR___name__),            MP_OBJ_NEW_QSTR(MP_QSTR_image)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_LBP),                 MP_OBJ_NEW_SMALL_INT(DESC_LBP)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_FREAK),               MP_OBJ_NEW_SMALL_INT(DESC_FREAK)},
    /* Color space functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_rgb_to_lab),          (mp_obj_t)&py_image_rgb_to_lab_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_lab_to_rgb),          (mp_obj_t)&py_image_lab_to_rgb_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_rgb_to_grayscale),    (mp_obj_t)&py_image_rgb_to_grayscale_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_grayscale_to_rgb),    (mp_obj_t)&py_image_grayscale_to_rgb_obj},
    /* Image Module Functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_Image),               (mp_obj_t)&py_image_load_image_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_HaarCascade),         (mp_obj_t)&py_image_load_cascade_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_load_descriptor),     (mp_obj_t)&py_image_load_descriptor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_save_descriptor),     (mp_obj_t)&py_image_save_descriptor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_match_descriptor),    (mp_obj_t)&py_image_match_descriptor_obj},
    { NULL, NULL }
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t image_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_image,
    .globals = (mp_obj_t)&globals_dict,
};
