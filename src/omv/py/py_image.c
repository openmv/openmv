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
#include "ff_wrapper.h"
#include "xalloc.h"
#include "fb_alloc.h"
#include "framebuffer.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"
#include "omv_boardconfig.h"
#include "py/runtime0.h"
#include "py/runtime.h"

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
    mp_printf(print, "size:%d threshold:%d normalized:%d", array_length(self->kpts), self->threshold, self->normalized);
}

mp_obj_t py_kp_unary_op(mp_uint_t op, mp_obj_t self_in) {
    py_kp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_LEN:
            return MP_OBJ_NEW_SMALL_INT(array_length(self->kpts));

        default:
            return MP_OBJ_NULL; // op not supported
    }
}

static mp_obj_t py_kp_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_kp_obj_t *self = self_in;
        int size = array_length(self->kpts);
        int i = mp_get_index(self->base.type, size, index, false);
        kp_t *kp = array_at(self->kpts, i);
        return mp_obj_new_tuple(5, (mp_obj_t []) {mp_obj_new_int(kp->x),
                                                  mp_obj_new_int(kp->y),
                                                  mp_obj_new_int(kp->score),
                                                  mp_obj_new_int(kp->octave),
                                                  mp_obj_new_int(kp->angle)});
    }

    return MP_OBJ_NULL; // op not supported
}

static const mp_obj_type_t py_kp_type = {
    { &mp_type_type },
    .name  = MP_QSTR_kp_desc,
    .print = py_kp_print,
    .subscr = py_kp_subscr,
    .unary_op = py_kp_unary_op,
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

// Keypoints match object /////////////////////////////////////////////////////
#define kptmatch_obj_size 9
typedef struct _py_kptmatch_obj_t {
    mp_obj_base_t base;
    mp_obj_t cx, cy;
    mp_obj_t x, y, w, h;
    mp_obj_t count;
    mp_obj_t theta;
    mp_obj_t match;
} py_kptmatch_obj_t;

static void py_kptmatch_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_kptmatch_obj_t *self = self_in;
    mp_printf(print, "<keypoints match object> {cx:%d, cy:%d, x:%d, y:%d, w:%d, h:%d, count:%d, theta:%d}",
              mp_obj_get_int(self->cx), mp_obj_get_int(self->cy), mp_obj_get_int(self->x), mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),  mp_obj_get_int(self->h),  mp_obj_get_int(self->count), mp_obj_get_int(self->theta));
}

static mp_obj_t py_kptmatch_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_kptmatch_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(kptmatch_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, kptmatch_obj_size, index, false)) {
            case 0: return self->cx;
            case 1: return self->cy;
            case 2: return self->x;
            case 3: return self->y;
            case 4: return self->w;
            case 5: return self->h;
            case 6: return self->count;
            case 7: return self->theta;
            case 8: return self->match;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_kptmatch_cx(mp_obj_t self_in)     { return ((py_kptmatch_obj_t *) self_in)->cx;    }
mp_obj_t py_kptmatch_cy(mp_obj_t self_in)     { return ((py_kptmatch_obj_t *) self_in)->cy;    }
mp_obj_t py_kptmatch_x (mp_obj_t self_in)     { return ((py_kptmatch_obj_t *) self_in)->x;     }
mp_obj_t py_kptmatch_y (mp_obj_t self_in)     { return ((py_kptmatch_obj_t *) self_in)->y;     }
mp_obj_t py_kptmatch_w (mp_obj_t self_in)     { return ((py_kptmatch_obj_t *) self_in)->w;     }
mp_obj_t py_kptmatch_h (mp_obj_t self_in)     { return ((py_kptmatch_obj_t *) self_in)->h;     }
mp_obj_t py_kptmatch_count(mp_obj_t self_in)  { return ((py_kptmatch_obj_t *) self_in)->count; }
mp_obj_t py_kptmatch_theta(mp_obj_t self_in)  { return ((py_kptmatch_obj_t *) self_in)->theta; }
mp_obj_t py_kptmatch_match(mp_obj_t self_in)  { return ((py_kptmatch_obj_t *) self_in)->match; }
mp_obj_t py_kptmatch_rect(mp_obj_t  self_in)  {
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_kptmatch_obj_t *) self_in)->x,
                                              ((py_kptmatch_obj_t *) self_in)->y,
                                              ((py_kptmatch_obj_t *) self_in)->w,
                                              ((py_kptmatch_obj_t *) self_in)->h});
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_cx_obj,    py_kptmatch_cx);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_cy_obj,    py_kptmatch_cy);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_x_obj,     py_kptmatch_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_y_obj,     py_kptmatch_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_w_obj,     py_kptmatch_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_h_obj,     py_kptmatch_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_count_obj, py_kptmatch_count);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_theta_obj, py_kptmatch_theta);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_match_obj, py_kptmatch_match);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_kptmatch_rect_obj,  py_kptmatch_rect);

STATIC const mp_rom_map_elem_t py_kptmatch_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_cx),      MP_ROM_PTR(&py_kptmatch_cx_obj)      },
    { MP_ROM_QSTR(MP_QSTR_cy),      MP_ROM_PTR(&py_kptmatch_cy_obj)      },
    { MP_ROM_QSTR(MP_QSTR_x),       MP_ROM_PTR(&py_kptmatch_x_obj)       },
    { MP_ROM_QSTR(MP_QSTR_y),       MP_ROM_PTR(&py_kptmatch_y_obj)       },
    { MP_ROM_QSTR(MP_QSTR_w),       MP_ROM_PTR(&py_kptmatch_w_obj)       },
    { MP_ROM_QSTR(MP_QSTR_h),       MP_ROM_PTR(&py_kptmatch_h_obj)       },
    { MP_ROM_QSTR(MP_QSTR_count),   MP_ROM_PTR(&py_kptmatch_count_obj)   },
    { MP_ROM_QSTR(MP_QSTR_theta),   MP_ROM_PTR(&py_kptmatch_theta_obj)   },
    { MP_ROM_QSTR(MP_QSTR_match),   MP_ROM_PTR(&py_kptmatch_match_obj)   },
    { MP_ROM_QSTR(MP_QSTR_rect),    MP_ROM_PTR(&py_kptmatch_rect_obj)    },
};

STATIC MP_DEFINE_CONST_DICT(py_kptmatch_locals_dict, py_kptmatch_locals_dict_table);

static const mp_obj_type_t py_kptmatch_type = {
    { &mp_type_type },
    .name   = MP_QSTR_kptmatch,
    .print  = py_kptmatch_print,
    .subscr = py_kptmatch_subscr,
    .locals_dict = (mp_obj_t) &py_kptmatch_locals_dict,
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
    switch(self->_cobj.bpp) {
        case IMAGE_BPP_BINARY: {
            mp_printf(print, "{w:%d, h:%d, type=\"binary\", size:%d}",
                      self->_cobj.w, self->_cobj.h,
                      ((self->_cobj.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * self->_cobj.h);
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            mp_printf(print, "{w:%d, h:%d, type=\"grayscale\", size:%d}",
                      self->_cobj.w, self->_cobj.h,
                      (self->_cobj.w * self->_cobj.h) * sizeof(uint8_t));
            break;
        }
        case IMAGE_BPP_RGB565: {
            mp_printf(print, "{w:%d, h:%d, type=\"rgb565\", size:%d}",
                      self->_cobj.w, self->_cobj.h,
                      (self->_cobj.w * self->_cobj.h) * sizeof(uint16_t));
            break;
        }
        default: {
            if((self->_cobj.data[0] == 0xFE) && (self->_cobj.data[self->_cobj.bpp-1] == 0xFE)) { // for ide
                print->print_strn(print->data, (const char *) self->_cobj.data, self->_cobj.bpp);
            } else { // not for ide
                mp_printf(print, "{w:%d, h:%d, type=\"jpeg\", size:%d}",
                          self->_cobj.w, self->_cobj.h,
                          self->_cobj.bpp);
            }
            break;
        }
    }
}

static mp_obj_t py_image_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    py_image_obj_t *self = self_in;
    if (value == MP_OBJ_NULL) { // delete
    } else if (value == MP_OBJ_SENTINEL) { // load
        switch (self->_cobj.bpp) {
            case IMAGE_BPP_BINARY: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.w * self->_cobj.h, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
                    for (mp_uint_t i = 0; i < result->len; i++) {
                        result->items[i] = mp_obj_new_int(IMAGE_GET_BINARY_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w));
                    }
                    return result;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.w * self->_cobj.h, index, false);
                return mp_obj_new_int(IMAGE_GET_BINARY_PIXEL(&(self->_cobj), i % self->_cobj.w, i / self->_cobj.w));
            }
            case IMAGE_BPP_BAYER:
            case IMAGE_BPP_GRAYSCALE: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.w * self->_cobj.h, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
                    for (mp_uint_t i = 0; i < result->len; i++) {
                        uint8_t p = IMAGE_GET_GRAYSCALE_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w);
                        result->items[i] = mp_obj_new_int(p);
                    }
                    return result;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.w * self->_cobj.h, index, false);
                uint8_t p = IMAGE_GET_GRAYSCALE_PIXEL(&(self->_cobj), i % self->_cobj.w, i / self->_cobj.w);
                return mp_obj_new_int(p);
            }
            case IMAGE_BPP_RGB565: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.w * self->_cobj.h, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
                    for (mp_uint_t i = 0; i < result->len; i++) {
                        uint16_t p = IMAGE_GET_RGB565_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w);
                        result->items[i] = mp_obj_new_tuple(3, (mp_obj_t []) {mp_obj_new_int(COLOR_RGB565_TO_R8(p)),
                                                                              mp_obj_new_int(COLOR_RGB565_TO_G8(p)),
                                                                              mp_obj_new_int(COLOR_RGB565_TO_B8(p))});
                    }
                    return result;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.w * self->_cobj.h, index, false);
                uint16_t p = IMAGE_GET_RGB565_PIXEL(&(self->_cobj), i % self->_cobj.w, i / self->_cobj.w);
                return mp_obj_new_tuple(3, (mp_obj_t []) {mp_obj_new_int(COLOR_RGB565_TO_R8(p)),
                                                          mp_obj_new_int(COLOR_RGB565_TO_G8(p)),
                                                          mp_obj_new_int(COLOR_RGB565_TO_B8(p))});
            }
            default: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.bpp, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
                    for (mp_uint_t i = 0; i < result->len; i++) {
                        result->items[i] = mp_obj_new_int(self->_cobj.data[slice.start + i]);
                    }
                    return result;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.bpp, index, false);
                return mp_obj_new_int(self->_cobj.data[i]);
            }
        }
    } else { // store
        switch (self->_cobj.bpp) {
            case IMAGE_BPP_BINARY: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.w * self->_cobj.h, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    if (MP_OBJ_IS_TYPE(value, &mp_type_list)) {
                        mp_uint_t value_l_len;
                        mp_obj_t *value_l;
                        mp_obj_get_array(value, &value_l_len, &value_l);
                        PY_ASSERT_TRUE_MSG(value_l_len == (slice.stop - slice.start), "cannot grow or shrink image");
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            IMAGE_PUT_BINARY_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w, mp_obj_get_int(value_l[i]));
                        }
                    } else {
                        mp_int_t v = mp_obj_get_int(value);
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            IMAGE_PUT_BINARY_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w, v);
                        }
                    }
                    return mp_const_none;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.w * self->_cobj.h, index, false);
                IMAGE_PUT_BINARY_PIXEL(&(self->_cobj), i % self->_cobj.w, i / self->_cobj.w, mp_obj_get_int(value));
                return mp_const_none;
            }
            case IMAGE_BPP_BAYER:
            case IMAGE_BPP_GRAYSCALE: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.w * self->_cobj.h, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    if (MP_OBJ_IS_TYPE(value, &mp_type_list)) {
                        mp_uint_t value_l_len;
                        mp_obj_t *value_l;
                        mp_obj_get_array(value, &value_l_len, &value_l);
                        PY_ASSERT_TRUE_MSG(value_l_len == (slice.stop - slice.start), "cannot grow or shrink image");
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            uint8_t p = mp_obj_get_int(value_l[i]);
                            IMAGE_PUT_GRAYSCALE_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w, p);
                        }
                    } else {
                        uint8_t p = mp_obj_get_int(value);
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            IMAGE_PUT_GRAYSCALE_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w, p);
                        }
                    }
                    return mp_const_none;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.w * self->_cobj.h, index, false);
                uint8_t p = mp_obj_get_int(value);
                IMAGE_PUT_GRAYSCALE_PIXEL(&(self->_cobj), i % self->_cobj.w, i / self->_cobj.w, p);
                return mp_const_none;
            }
            case IMAGE_BPP_RGB565: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.w * self->_cobj.h, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    if (MP_OBJ_IS_TYPE(value, &mp_type_list)) {
                        mp_uint_t value_l_len;
                        mp_obj_t *value_l;
                        mp_obj_get_array(value, &value_l_len, &value_l);
                        PY_ASSERT_TRUE_MSG(value_l_len == (slice.stop - slice.start), "cannot grow or shrink image");
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            mp_obj_t *value_2;
                            mp_obj_get_array_fixed_n(value_l[i], 3, &value_2);
                            uint16_t p = COLOR_R8_G8_B8_TO_RGB565(mp_obj_get_int(value_2[0]), mp_obj_get_int(value_2[1]), mp_obj_get_int(value_2[2]));
                            IMAGE_PUT_RGB565_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w, p);
                        }
                    } else {
                        mp_obj_t *value_2;
                        mp_obj_get_array_fixed_n(value, 3, &value_2);
                        uint16_t p = COLOR_R8_G8_B8_TO_RGB565(mp_obj_get_int(value_2[0]), mp_obj_get_int(value_2[1]), mp_obj_get_int(value_2[2]));
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            IMAGE_PUT_RGB565_PIXEL(&(self->_cobj), (slice.start + i) % self->_cobj.w, (slice.start + i) / self->_cobj.w, p);
                        }
                    }
                    return mp_const_none;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.w * self->_cobj.h, index, false);
                mp_obj_t *value_2;
                mp_obj_get_array_fixed_n(value, 3, &value_2);
                uint16_t p = COLOR_R8_G8_B8_TO_RGB565(mp_obj_get_int(value_2[0]), mp_obj_get_int(value_2[1]), mp_obj_get_int(value_2[2]));
                IMAGE_PUT_RGB565_PIXEL(&(self->_cobj), i % self->_cobj.w, i / self->_cobj.w, p);
                return mp_const_none;
            }
            default: {
                if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
                    mp_bound_slice_t slice;
                    if (!mp_seq_get_fast_slice_indexes(self->_cobj.bpp, index, &slice)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
                    }
                    if (MP_OBJ_IS_TYPE(value, &mp_type_list)) {
                        mp_uint_t value_l_len;
                        mp_obj_t *value_l;
                        mp_obj_get_array(value, &value_l_len, &value_l);
                        PY_ASSERT_TRUE_MSG(value_l_len == (slice.stop - slice.start), "cannot grow or shrink image");
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            self->_cobj.data[slice.start + i] = mp_obj_get_int(value_l[i]);
                        }
                    } else {
                        mp_int_t v = mp_obj_get_int(value);
                        for (mp_uint_t i = 0; i < (slice.stop - slice.start); i++) {
                            self->_cobj.data[slice.start + i] = v;
                        }
                    }
                    return mp_const_none;
                }
                mp_uint_t i = mp_get_index(self->base.type, self->_cobj.bpp, index, false);
                self->_cobj.data[i] = mp_obj_get_int(value);
                return mp_const_none;
            }
        }
    }
    return MP_OBJ_NULL; // op not supported
}

static mp_int_t py_image_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags)
{
    py_image_obj_t *self = self_in;
    if (flags == MP_BUFFER_READ) {
        bufinfo->buf = self->_cobj.data;
        switch (self->_cobj.bpp) {
            case IMAGE_BPP_BINARY: {
                bufinfo->len = ((self->_cobj.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * self->_cobj.h;
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                bufinfo->len = (self->_cobj.w * self->_cobj.h) * sizeof(uint8_t);
                break;
            }
            case IMAGE_BPP_RGB565: {
                bufinfo->len = (self->_cobj.w * self->_cobj.h) * sizeof(uint16_t);
                break;
            }
            case IMAGE_BPP_BAYER: {
                bufinfo->len = self->_cobj.w * self->_cobj.h;
                break;
            }
            default: { // JPEG
                bufinfo->len = self->_cobj.bpp;
                break;
            }
        }
        bufinfo->typecode = 'b';
        return 0;
    } else {
        // can't write to an image
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

static mp_obj_t py_image_copy_to_fb(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    point_t arg_offs;
    image_t *arg_img = py_image_cobj(args[0]);
    py_helper_lookup_offset(kw_args, arg_img, &arg_offs);

    MAIN_FB()->w = arg_img->w;
    MAIN_FB()->h = arg_img->h;
    MAIN_FB()->bpp = arg_img->bpp;

    int yoffs = arg_offs.y;
    int xoffs = arg_offs.x * arg_img->bpp;
    int stride = arg_img->w * arg_img->bpp;

    for (int y=yoffs; y<arg_img->h; y++) {
        for (int x=xoffs; x<stride; x++) {
            MAIN_FB()->pixels[y*stride+x] = arg_img->pixels[y*stride+x];
        }
    }
    return mp_const_true;
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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");
    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    PY_ASSERT_TRUE_MSG((1 <= arg_q) && (arg_q <= 100), " 1 <= quality <= 100");

    uint32_t size;
    fb_alloc_mark();
    uint8_t *buffer = fb_alloc_all(&size);
    image_t out = { .w=arg_img->w, .h=arg_img->h, .bpp=size, .data=buffer };
    PY_ASSERT_FALSE_MSG(jpeg_compress(arg_img, &out, arg_q, false), "Out of Memory!");

    switch(arg_img->bpp) {
        case IMAGE_BPP_BINARY: {
            PY_ASSERT_TRUE_MSG(out.bpp <= (((arg_img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * arg_img->h), "Can't compress in place!");
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            PY_ASSERT_TRUE_MSG(out.bpp <= ((arg_img->w * arg_img->h) * sizeof(uint8_t)), "Can't compress in place!");
            break;
        }
        case IMAGE_BPP_RGB565: {
            PY_ASSERT_TRUE_MSG(out.bpp <= ((arg_img->w * arg_img->h) * sizeof(uint16_t)), "Can't compress in place!");
            break;
        }
        default: {
            break;
        }
    }

    memcpy(arg_img->data, out.data, out.bpp);
    arg_img->bpp = out.bpp;
    fb_free();
    fb_alloc_free_till_mark();

    if (MAIN_FB()->pixels == arg_img->data) {
        MAIN_FB()->bpp = arg_img->bpp;
    }

    return args[0];
}

static mp_obj_t py_image_compress_for_ide(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");
    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    PY_ASSERT_TRUE_MSG((1 <= arg_q) && (arg_q <= 100), " 1 <= quality <= 100");

    uint32_t size;
    fb_alloc_mark();
    uint8_t *buffer = fb_alloc_all(&size);
    image_t out = { .w=arg_img->w, .h=arg_img->h, .bpp=size, .data=buffer };
    PY_ASSERT_FALSE_MSG(jpeg_compress(arg_img, &out, arg_q, false), "Out of Memory!");

    switch(arg_img->bpp) {
        case IMAGE_BPP_BINARY: {
            PY_ASSERT_TRUE_MSG(((((out.bpp * 8) + 5) / 6) + 2) <=
                               (((arg_img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * arg_img->h),
                               "Can't compress in place!");
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            PY_ASSERT_TRUE_MSG(((((out.bpp * 8) + 5) / 6) + 2) <=
                               ((arg_img->w * arg_img->h) * sizeof(uint8_t)),
                               "Can't compress in place!");
            break;
        }
        case IMAGE_BPP_RGB565: {
            PY_ASSERT_TRUE_MSG(((((out.bpp * 8) + 5) / 6) + 2) <=
                               ((arg_img->w * arg_img->h) * sizeof(uint16_t)),
                               "Can't compress in place!");
            break;
        }
        default: {
            break;
        }
    }

    uint8_t *ptr = arg_img->data;

    *ptr++ = 0xFE;

    for(int i = 0, j = (out.bpp / 3) * 3; i < j; i += 3) {
        int x = 0;
        x |= out.data[i + 0] << 0;
        x |= out.data[i + 1] << 8;
        x |= out.data[i + 2] << 16;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
        *ptr++ = 0x80 | ((x >> 12) & 0x3F);
        *ptr++ = 0x80 | ((x >> 18) & 0x3F);
    }

    if((out.bpp % 3) == 2) { // 2 bytes -> 16-bits -> 24-bits sent
        int x = 0;
        x |= out.data[out.bpp - 2] << 0;
        x |= out.data[out.bpp - 1] << 8;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
        *ptr++ = 0x80 | ((x >> 12) & 0x3F);
    }

    if((out.bpp % 3) == 1) { // 1 byte -> 8-bits -> 16-bits sent
        int x = 0;
        x |= out.data[out.bpp - 1] << 0;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
    }

    *ptr++ = 0xFE;

    out.bpp = (((out.bpp * 8) + 5) / 6) + 2;
    arg_img->bpp = out.bpp;
    fb_free();
    fb_alloc_free_till_mark();

    if (MAIN_FB()->pixels == arg_img->data) {
        MAIN_FB()->bpp = arg_img->bpp;
    }

    return args[0];
}

static mp_obj_t py_image_compressed(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");
    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    PY_ASSERT_TRUE_MSG((1 <= arg_q) && (arg_q <= 100), " 1 <= quality <= 100");

    uint32_t size;
    fb_alloc_mark();
    uint8_t *buffer = fb_alloc_all(&size);
    image_t out = { .w=arg_img->w, .h=arg_img->h, .bpp=size, .data=buffer };
    PY_ASSERT_FALSE_MSG(jpeg_compress(arg_img, &out, arg_q, false), "Out of Memory!");
    uint8_t *temp = xalloc(out.bpp);
    memcpy(temp, out.data, out.bpp);
    out.data = temp;
    fb_free();
    fb_alloc_free_till_mark();

    return py_image_from_struct(&out);
}

static mp_obj_t py_image_compressed_for_ide(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");
    int arg_q = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 50);
    PY_ASSERT_TRUE_MSG((1 <= arg_q) && (arg_q <= 100), " 1 <= quality <= 100");

    uint32_t size;
    fb_alloc_mark();
    uint8_t *buffer = fb_alloc_all(&size);
    image_t out = { .w=arg_img->w, .h=arg_img->h, .bpp=size, .data=buffer };
    PY_ASSERT_FALSE_MSG(jpeg_compress(arg_img, &out, arg_q, false), "Out of Memory!");
    uint8_t *temp = xalloc((((out.bpp * 8) + 5) / 6) + 2);
    uint8_t *ptr = temp;

    *ptr++ = 0xFE;

    for(int i = 0, j = (out.bpp / 3) * 3; i < j; i += 3) {
        int x = 0;
        x |= out.data[i + 0] << 0;
        x |= out.data[i + 1] << 8;
        x |= out.data[i + 2] << 16;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
        *ptr++ = 0x80 | ((x >> 12) & 0x3F);
        *ptr++ = 0x80 | ((x >> 18) & 0x3F);
    }

    if((out.bpp % 3) == 2) { // 2 bytes -> 16-bits -> 24-bits sent
        int x = 0;
        x |= out.data[out.bpp - 2] << 0;
        x |= out.data[out.bpp - 1] << 8;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
        *ptr++ = 0x80 | ((x >> 12) & 0x0F);
    }

    if((out.bpp % 3) == 1) { // 1 byte -> 8-bits -> 16-bits sent
        int x = 0;
        x |= out.data[out.bpp - 1] << 0;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x03);
    }

    *ptr++ = 0xFE;

    out.bpp = (((out.bpp * 8) + 5) / 6) + 2;
    out.data = temp;
    fb_free();
    fb_alloc_free_till_mark();

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

static mp_obj_t py_image_clear(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    memset(arg_img->pixels, 0, arg_img->w * arg_img->h * arg_img->bpp);
    return img_obj;
}

static mp_obj_t py_image_get_pixel(mp_obj_t img_obj, mp_obj_t x_obj, mp_obj_t y_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(!IM_IS_JPEG(arg_img), "Image format is not supported.");

    int arg_x = mp_obj_get_int(x_obj);
    int arg_y = mp_obj_get_int(y_obj);
    if ((!IM_X_INSIDE(arg_img, arg_x)) || (!IM_Y_INSIDE(arg_img, arg_y))) {
        return mp_const_none;
    }

    switch (arg_img->bpp) {
        case 1: // GS
        case 3: // BAYER
            return mp_obj_new_int(IM_GET_GS_PIXEL(arg_img, arg_x, arg_y));
        case 2: { // RGB565
            uint16_t pixel = IM_GET_RGB565_PIXEL(arg_img, arg_x, arg_y);
            mp_obj_t pixel_tuple[3];
            pixel_tuple[0] = mp_obj_new_int(IM_R528(IM_R565(pixel)));
            pixel_tuple[1] = mp_obj_new_int(IM_G628(IM_G565(pixel)));
            pixel_tuple[2] = mp_obj_new_int(IM_B528(IM_B565(pixel)));
            return mp_obj_new_tuple(3, pixel_tuple);
        }
        default:
            return mp_const_none;
    }
}

static mp_obj_t py_image_set_pixel(uint n_args, const mp_obj_t *args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    py_kp_obj_t *kpts_obj = ((py_kp_obj_t*)args[1]);
    int arg_c = py_helper_lookup_color(kw_args, -1); // white
    int arg_s = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_size), arg_img->w*10/100);

    PY_ASSERT_TYPE(kpts_obj, &py_kp_type);
    for (int i=0; i<array_length(kpts_obj->kpts); i++) {
        kp_t *kp = array_at(kpts_obj->kpts, i);
        int cx = kp->x;
        int cy = kp->y;
        int size = arg_s/2;
        int si = (int) (sin_table[kp->angle] * size);
        int co = (int) (cos_table[kp->angle] * size);
        imlib_draw_line(arg_img, cx, cy, cx+co, cy+si, arg_c);
        imlib_draw_circle(arg_img, cx, cy, size, arg_c);
    }
    return mp_const_none;
}

static mp_obj_t py_image_binary(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    return args[0];
}

static mp_obj_t py_image_invert(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    imlib_invert(arg_img);
    return img_obj;
}

static mp_obj_t py_image_b_and(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_b_and(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_b_and(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_b_nand(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_b_nand(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_b_nand(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_b_or(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_b_or(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_b_or(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_b_nor(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_b_nor(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_b_nor(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_b_xor(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_b_xor(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_b_xor(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_b_xnor(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_b_xnor(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_b_xnor(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_erode(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");
    imlib_erode(arg_img, arg_ksize,
            py_helper_lookup_int(kw_args,
            MP_OBJ_NEW_QSTR(MP_QSTR_threshold), ((arg_ksize*2)+1)*((arg_ksize*2)+1)-1));
    return args[0];
}

static mp_obj_t py_image_dilate(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");
    imlib_dilate(arg_img, arg_ksize,
            py_helper_lookup_int(kw_args,
            MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 0));
    return args[0];
}

static mp_obj_t py_image_negate(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    imlib_negate(arg_img);
    return img_obj;
}

static mp_obj_t py_image_difference(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_difference(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_difference(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_replace(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        imlib_replace(arg_img, mp_obj_str_get_str(other_obj), NULL);
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        imlib_replace(arg_img, NULL, arg_other);
    }
    return img_obj;
}

static mp_obj_t py_image_blend(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int alpha = IM_MIN(IM_MAX(py_helper_lookup_int(kw_args,
        MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 128), 0), 256);

    if (MP_OBJ_IS_STR(args[1])) {
        imlib_blend(arg_img, mp_obj_str_get_str(args[1]), NULL, alpha);
    } else {
        image_t *arg_other = py_image_cobj(args[1]);
        imlib_blend(arg_img, NULL, arg_other, alpha);
    }
    return args[0];
}

static mp_obj_t py_image_morph(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

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
    return args[0];
}

static mp_obj_t py_image_midpoint(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    int arg_threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), false);
    int arg_offset = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_offset), 0);
    int arg_invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);

    int bias = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bias), 0.5) * 256;
    imlib_midpoint_filter(arg_img, arg_ksize, IM_MIN(IM_MAX(bias, 0), 256), arg_threshold, arg_offset, arg_invert);
    return args[0];
}

static mp_obj_t py_image_mean(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    int arg_threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), false);
    int arg_offset = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_offset), 0);
    int arg_invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);

    imlib_mean_filter(arg_img, arg_ksize, arg_threshold, arg_offset, arg_invert);
    return args[0];
}

static mp_obj_t py_image_mode(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");

    int arg_threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), false);
    int arg_offset = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_offset), 0);
    int arg_invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);

    imlib_mode_filter(arg_img, arg_ksize, arg_threshold, arg_offset, arg_invert);
    return args[0];
}

static mp_obj_t py_image_median(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(arg_ksize >= 0, "Kernel Size must be >= 0");
    PY_ASSERT_TRUE_MSG(arg_ksize <= 2, "Kernel Size must be <= 2");

    int arg_threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), false);
    int arg_offset = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_offset), 0);
    int arg_invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);

    int n = ((arg_ksize*2)+1)*((arg_ksize*2)+1);
    int percentile = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_percentile), 0.5) * n;
    imlib_median_filter(arg_img, arg_ksize, IM_MIN(IM_MAX(percentile, 0), n-1), arg_threshold, arg_offset, arg_invert);
    return args[0];
}

static mp_obj_t py_image_gaussian(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int arg_ksize = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG((arg_ksize == 3 || arg_ksize == 5), "Kernel Size must be 3 or 5");

    if (arg_ksize == 3) {
        imlib_morph(arg_img, 1, kernel_gauss_3, 1.0f/16.0f, 0.0f);
    } else if (arg_ksize == 5) {
        imlib_morph(arg_img, 2, kernel_gauss_5, 1.0f/256.0f, 0.0f);
    }

    return args[0];
}

static mp_obj_t py_image_max(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        fb_alloc_mark();
        imlib_max(arg_img, mp_obj_str_get_str(other_obj), NULL);
        fb_alloc_mark();
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        fb_alloc_mark();
        imlib_max(arg_img, NULL, arg_other);
        fb_alloc_free_till_mark();
    }

    return img_obj;
}

static mp_obj_t py_image_min(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    if (MP_OBJ_IS_STR(other_obj)) {
        fb_alloc_mark();
        imlib_min(arg_img, mp_obj_str_get_str(other_obj), NULL);
        fb_alloc_mark();
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        fb_alloc_mark();
        imlib_min(arg_img, NULL, arg_other);
        fb_alloc_free_till_mark();
    }

    return img_obj;
}

static mp_obj_t py_image_linpolar(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_RGB565(arg_img), "Image format is not supported.");
    bool reverse = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_reverse),
                                        (n_args > 1) ? mp_obj_get_int(args[1]) : false);

    fb_alloc_mark();
    imlib_logpolar(arg_img, true, reverse);
    fb_alloc_free_till_mark();
    return args[0];
}

static mp_obj_t py_image_logpolar(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_RGB565(arg_img), "Image format is not supported.");
    bool reverse = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_reverse),
                                        (n_args > 1) ? mp_obj_get_int(args[1]) : false);

    fb_alloc_mark();
    imlib_logpolar(arg_img, false, reverse);
    fb_alloc_free_till_mark();
    return args[0];
}

static mp_obj_t py_image_chrominvar(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_RGB565(arg_img), "Image format is not supported.");

    imlib_chrominvar(arg_img);
    return img_obj;
}

static mp_obj_t py_image_illuminvar(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_RGB565(arg_img), "Image format is not supported.");

    imlib_illuminvar(arg_img);
    return img_obj;
}

static mp_obj_t py_image_histeq(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    imlib_histeq(arg_img);
    return img_obj;
}

static mp_obj_t py_image_lens_corr(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    float strength = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_strength),
                                            (n_args > 1) ? mp_obj_get_float(args[1]) : 1.8);
    PY_ASSERT_TRUE_MSG(strength > 0.0, "strength must be > 0");

    float zoom = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_zoom),
                                        (n_args > 2) ? mp_obj_get_float(args[2]) : 1.0);
    PY_ASSERT_TRUE_MSG(zoom > 0.0, "zoom must be > 0");

    fb_alloc_mark();
    imlib_lens_corr(arg_img, strength, zoom);
    fb_alloc_free_till_mark();
    return args[0];
}

#ifdef OMV_ENABLE_ROTATION_CORR
static mp_obj_t py_image_rotation_corr(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    float x_rotation = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_rotation),
                                              (n_args > 1) ? mp_obj_get_float(args[1]) : 0.0);
    x_rotation = x_rotation * M_PI / 180;

    float y_rotation = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_rotation),
                                              (n_args > 2) ? mp_obj_get_float(args[2]) : 0.0);
    y_rotation = y_rotation * M_PI / 180;

    float z_rotation = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_z_rotation),
                                              (n_args > 3) ? mp_obj_get_float(args[3]) : 0.0);
    z_rotation = z_rotation * M_PI / 180;

    float x_translation = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_translation),
                                                 (n_args > 4) ? mp_obj_get_float(args[4]) : 0.0);

    float y_translation = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_translation),
                                                 (n_args > 5) ? mp_obj_get_float(args[5]) : 0.0);

    float zoom = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_zoom),
                                        (n_args > 6) ? mp_obj_get_float(args[6]) : 1.0);
    PY_ASSERT_TRUE_MSG(zoom > 0.0, "zoom must be > 0");

    fb_alloc_mark();
    imlib_rotation_corr(arg_img, x_rotation, y_rotation, z_rotation, x_translation, y_translation, zoom);
    fb_alloc_free_till_mark();
    return args[0];
}
#endif

static mp_obj_t py_image_mask_ellipse(mp_obj_t img_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    imlib_mask_ellipse(arg_img);
    return img_obj;
}

// Similarity Object //
#define py_similarity_obj_size 4
typedef struct py_similarity_obj {
    mp_obj_base_t base;
    mp_obj_t avg, std, min, max;
} py_similarity_obj_t;

static void py_similarity_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_similarity_obj_t *self = self_in;
    mp_printf(print,
              "{mean:%f, stdev:%f, min:%f, max:%f}",
              (double) mp_obj_get_float(self->avg),
              (double) mp_obj_get_float(self->std),
              (double) mp_obj_get_float(self->min),
              (double) mp_obj_get_float(self->max));
}

static mp_obj_t py_similarity_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_similarity_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_similarity_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->avg) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_similarity_obj_size, index, false)) {
            case 0: return self->avg;
            case 1: return self->std;
            case 2: return self->min;
            case 3: return self->max;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_similarity_mean(mp_obj_t self_in) { return ((py_similarity_obj_t *) self_in)->avg; }
mp_obj_t py_similarity_stdev(mp_obj_t self_in) { return ((py_similarity_obj_t *) self_in)->std; }
mp_obj_t py_similarity_min(mp_obj_t self_in) { return ((py_similarity_obj_t *) self_in)->min; }
mp_obj_t py_similarity_max(mp_obj_t self_in) { return ((py_similarity_obj_t *) self_in)->max; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_similarity_mean_obj, py_similarity_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_similarity_stdev_obj, py_similarity_stdev);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_similarity_min_obj, py_similarity_min);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_similarity_max_obj, py_similarity_max);

STATIC const mp_rom_map_elem_t py_similarity_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mean), MP_ROM_PTR(&py_similarity_mean_obj) },
    { MP_ROM_QSTR(MP_QSTR_stdev), MP_ROM_PTR(&py_similarity_stdev_obj) },
    { MP_ROM_QSTR(MP_QSTR_min), MP_ROM_PTR(&py_similarity_min_obj) },
    { MP_ROM_QSTR(MP_QSTR_max), MP_ROM_PTR(&py_similarity_max_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_similarity_locals_dict, py_similarity_locals_dict_table);

static const mp_obj_type_t py_similarity_type = {
    { &mp_type_type },
    .name  = MP_QSTR_similarity,
    .print = py_similarity_print,
    .subscr = py_similarity_subscr,
    .locals_dict = (mp_obj_t) &py_similarity_locals_dict,
};

static mp_obj_t py_image_get_similarity(mp_obj_t img_obj, mp_obj_t other_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    float avg, std, min, max;

    if (MP_OBJ_IS_STR(other_obj)) {
        fb_alloc_mark();
        imlib_get_similarity(arg_img, mp_obj_str_get_str(other_obj), NULL, &avg, &std, &min, &max);
        fb_alloc_mark();
    } else {
        image_t *arg_other = py_image_cobj(other_obj);
        fb_alloc_mark();
        imlib_get_similarity(arg_img, NULL, arg_other, &avg, &std, &min, &max);
        fb_alloc_free_till_mark();
    }

    py_similarity_obj_t *o = m_new_obj(py_similarity_obj_t);
    o->base.type = &py_similarity_type;
    o->avg = mp_obj_new_float(avg);
    o->std = mp_obj_new_float(std);
    o->min = mp_obj_new_float(min);
    o->max = mp_obj_new_float(max);
    return o;
}

// Statistics Object //
#define py_statistics_obj_size 24
typedef struct py_statistics_obj {
    mp_obj_base_t base;
    image_bpp_t bpp;
    mp_obj_t LMean, LMedian, LMode, LSTDev, LMin, LMax, LLQ, LUQ, AMean, AMedian, AMode, ASTDev, AMin, AMax, ALQ, AUQ, BMean, BMedian, BMode, BSTDev, BMin, BMax, BLQ, BUQ;
} py_statistics_obj_t;

static void py_statistics_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_statistics_obj_t *self = self_in;
    switch(self->bpp) {
        case IMAGE_BPP_BINARY: {
            mp_printf(print, "{mean:%d, median:%d, mode:%d, stdev:%d, min:%d, max:%d, lq:%d, uq:%d}",
                      mp_obj_get_int(self->LMean),
                      mp_obj_get_int(self->LMedian),
                      mp_obj_get_int(self->LMode),
                      mp_obj_get_int(self->LSTDev),
                      mp_obj_get_int(self->LMin),
                      mp_obj_get_int(self->LMax),
                      mp_obj_get_int(self->LLQ),
                      mp_obj_get_int(self->LUQ));
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            mp_printf(print, "{mean:%d, median:%d, mode:%d, stdev:%d, min:%d, max:%d, lq:%d, uq:%d}",
                      mp_obj_get_int(self->LMean),
                      mp_obj_get_int(self->LMedian),
                      mp_obj_get_int(self->LMode),
                      mp_obj_get_int(self->LSTDev),
                      mp_obj_get_int(self->LMin),
                      mp_obj_get_int(self->LMax),
                      mp_obj_get_int(self->LLQ),
                      mp_obj_get_int(self->LUQ));
            break;
        }
        case IMAGE_BPP_RGB565: {
            mp_printf(print, "{l_mean:%d, l_median:%d, l_mode:%d, l_stdev:%d, l_min:%d, l_max:%d, l_lq:%d, l_uq:%d,"
                             " a_mean:%d, a_median:%d, a_mode:%d, a_stdev:%d, a_min:%d, a_max:%d, a_lq:%d, a_uq:%d,"
                             " b_mean:%d, b_median:%d, b_mode:%d, b_stdev:%d, b_min:%d, b_max:%d, b_lq:%d, b_uq:%d}",
                      mp_obj_get_int(self->LMean),
                      mp_obj_get_int(self->LMedian),
                      mp_obj_get_int(self->LMode),
                      mp_obj_get_int(self->LSTDev),
                      mp_obj_get_int(self->LMin),
                      mp_obj_get_int(self->LMax),
                      mp_obj_get_int(self->LLQ),
                      mp_obj_get_int(self->LUQ),
                      mp_obj_get_int(self->AMean),
                      mp_obj_get_int(self->AMedian),
                      mp_obj_get_int(self->AMode),
                      mp_obj_get_int(self->ASTDev),
                      mp_obj_get_int(self->AMin),
                      mp_obj_get_int(self->AMax),
                      mp_obj_get_int(self->ALQ),
                      mp_obj_get_int(self->AUQ),
                      mp_obj_get_int(self->BMean),
                      mp_obj_get_int(self->BMedian),
                      mp_obj_get_int(self->BMode),
                      mp_obj_get_int(self->BSTDev),
                      mp_obj_get_int(self->BMin),
                      mp_obj_get_int(self->BMax),
                      mp_obj_get_int(self->BLQ),
                      mp_obj_get_int(self->BUQ));
            break;
        }
        default: {
            mp_printf(print, "{}");
            break;
        }
    }
}

static mp_obj_t py_statistics_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_statistics_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_statistics_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->LMean) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_statistics_obj_size, index, false)) {
            case 0: return self->LMean;
            case 1: return self->LMedian;
            case 2: return self->LMode;
            case 3: return self->LSTDev;
            case 4: return self->LMin;
            case 5: return self->LMax;
            case 6: return self->LLQ;
            case 7: return self->LUQ;
            case 8: return self->AMean;
            case 9: return self->AMedian;
            case 10: return self->AMode;
            case 11: return self->ASTDev;
            case 12: return self->AMin;
            case 13: return self->AMax;
            case 14: return self->ALQ;
            case 15: return self->AUQ;
            case 16: return self->BMean;
            case 17: return self->BMedian;
            case 18: return self->BMode;
            case 19: return self->BSTDev;
            case 20: return self->BMin;
            case 21: return self->BMax;
            case 22: return self->BLQ;
            case 23: return self->BUQ;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_statistics_mean(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMean; }
mp_obj_t py_statistics_median(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMedian; }
mp_obj_t py_statistics_mode(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMode; }
mp_obj_t py_statistics_stdev(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LSTDev; }
mp_obj_t py_statistics_min(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMin; }
mp_obj_t py_statistics_max(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMax; }
mp_obj_t py_statistics_lq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LLQ; }
mp_obj_t py_statistics_uq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LUQ; }
mp_obj_t py_statistics_l_mean(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMean; }
mp_obj_t py_statistics_l_median(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMedian; }
mp_obj_t py_statistics_l_mode(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMode; }
mp_obj_t py_statistics_l_stdev(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LSTDev; }
mp_obj_t py_statistics_l_min(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMin; }
mp_obj_t py_statistics_l_max(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LMax; }
mp_obj_t py_statistics_l_lq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LLQ; }
mp_obj_t py_statistics_l_uq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->LUQ; }
mp_obj_t py_statistics_a_mean(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->AMean; }
mp_obj_t py_statistics_a_median(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->AMedian; }
mp_obj_t py_statistics_a_mode(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->AMode; }
mp_obj_t py_statistics_a_stdev(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->ASTDev; }
mp_obj_t py_statistics_a_min(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->AMin; }
mp_obj_t py_statistics_a_max(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->AMax; }
mp_obj_t py_statistics_a_lq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->ALQ; }
mp_obj_t py_statistics_a_uq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->AUQ; }
mp_obj_t py_statistics_b_mean(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BMean; }
mp_obj_t py_statistics_b_median(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BMedian; }
mp_obj_t py_statistics_b_mode(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BMode; }
mp_obj_t py_statistics_b_stdev(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BSTDev; }
mp_obj_t py_statistics_b_min(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BMin; }
mp_obj_t py_statistics_b_max(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BMax; }
mp_obj_t py_statistics_b_lq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BLQ; }
mp_obj_t py_statistics_b_uq(mp_obj_t self_in) { return ((py_statistics_obj_t *) self_in)->BUQ; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_mean_obj, py_statistics_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_median_obj, py_statistics_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_mode_obj, py_statistics_mode);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_stdev_obj, py_statistics_stdev);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_min_obj, py_statistics_min);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_max_obj, py_statistics_max);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_lq_obj, py_statistics_lq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_uq_obj, py_statistics_uq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_mean_obj, py_statistics_l_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_median_obj, py_statistics_l_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_mode_obj, py_statistics_l_mode);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_stdev_obj, py_statistics_l_stdev);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_min_obj, py_statistics_l_min);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_max_obj, py_statistics_l_max);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_lq_obj, py_statistics_l_lq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_l_uq_obj, py_statistics_l_uq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_mean_obj, py_statistics_a_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_median_obj, py_statistics_a_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_mode_obj, py_statistics_a_mode);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_stdev_obj, py_statistics_a_stdev);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_min_obj, py_statistics_a_min);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_max_obj, py_statistics_a_max);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_lq_obj, py_statistics_a_lq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_a_uq_obj, py_statistics_a_uq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_mean_obj, py_statistics_b_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_median_obj, py_statistics_b_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_mode_obj, py_statistics_b_mode);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_stdev_obj, py_statistics_b_stdev);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_min_obj, py_statistics_b_min);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_max_obj, py_statistics_b_max);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_lq_obj, py_statistics_b_lq);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_statistics_b_uq_obj, py_statistics_b_uq);

STATIC const mp_rom_map_elem_t py_statistics_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mean), MP_ROM_PTR(&py_statistics_mean_obj) },
    { MP_ROM_QSTR(MP_QSTR_median), MP_ROM_PTR(&py_statistics_median_obj) },
    { MP_ROM_QSTR(MP_QSTR_mode), MP_ROM_PTR(&py_statistics_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_stdev), MP_ROM_PTR(&py_statistics_stdev_obj) },
    { MP_ROM_QSTR(MP_QSTR_min), MP_ROM_PTR(&py_statistics_min_obj) },
    { MP_ROM_QSTR(MP_QSTR_max), MP_ROM_PTR(&py_statistics_max_obj) },
    { MP_ROM_QSTR(MP_QSTR_lq), MP_ROM_PTR(&py_statistics_lq_obj) },
    { MP_ROM_QSTR(MP_QSTR_uq), MP_ROM_PTR(&py_statistics_uq_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_mean), MP_ROM_PTR(&py_statistics_l_mean_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_median), MP_ROM_PTR(&py_statistics_l_median_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_mode), MP_ROM_PTR(&py_statistics_l_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_stdev), MP_ROM_PTR(&py_statistics_l_stdev_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_min), MP_ROM_PTR(&py_statistics_l_min_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_max), MP_ROM_PTR(&py_statistics_l_max_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_lq), MP_ROM_PTR(&py_statistics_l_lq_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_uq), MP_ROM_PTR(&py_statistics_l_uq_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_mean), MP_ROM_PTR(&py_statistics_a_mean_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_median), MP_ROM_PTR(&py_statistics_a_median_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_mode), MP_ROM_PTR(&py_statistics_a_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_stdev), MP_ROM_PTR(&py_statistics_a_stdev_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_min), MP_ROM_PTR(&py_statistics_a_min_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_max), MP_ROM_PTR(&py_statistics_a_max_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_lq), MP_ROM_PTR(&py_statistics_a_lq_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_uq), MP_ROM_PTR(&py_statistics_a_uq_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_mean), MP_ROM_PTR(&py_statistics_b_mean_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_median), MP_ROM_PTR(&py_statistics_b_median_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_mode), MP_ROM_PTR(&py_statistics_b_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_stdev), MP_ROM_PTR(&py_statistics_b_stdev_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_min), MP_ROM_PTR(&py_statistics_b_min_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_max), MP_ROM_PTR(&py_statistics_b_max_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_lq), MP_ROM_PTR(&py_statistics_b_lq_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_uq), MP_ROM_PTR(&py_statistics_b_uq_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_statistics_locals_dict, py_statistics_locals_dict_table);

static const mp_obj_type_t py_statistics_type = {
    { &mp_type_type },
    .name  = MP_QSTR_statistics,
    .print = py_statistics_print,
    .subscr = py_statistics_subscr,
    .locals_dict = (mp_obj_t) &py_statistics_locals_dict,
};

// Percentile Object //
#define py_percentile_obj_size 3
typedef struct py_percentile_obj {
    mp_obj_base_t base;
    image_bpp_t bpp;
    mp_obj_t LValue, AValue, BValue;
} py_percentile_obj_t;

static void py_percentile_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_percentile_obj_t *self = self_in;
    switch(self->bpp) {
        case IMAGE_BPP_BINARY: {
            mp_printf(print, "{value:%d}", mp_obj_get_int(self->LValue));
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            mp_printf(print, "{value:%d}", mp_obj_get_int(self->LValue));
            break;
        }
        case IMAGE_BPP_RGB565: {
            mp_printf(print, "{l_value:%d, a_value:%d, b_value:%d}", mp_obj_get_int(self->LValue), mp_obj_get_int(self->AValue), mp_obj_get_int(self->BValue));
            break;
        }
        default: {
            mp_printf(print, "{}");
            break;
        }
    }
}

static mp_obj_t py_percentile_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_percentile_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_percentile_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->LValue) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_percentile_obj_size, index, false)) {
            case 0: return self->LValue;
            case 1: return self->AValue;
            case 2: return self->BValue;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_percentile_value(mp_obj_t self_in) { return ((py_percentile_obj_t *) self_in)->LValue; }
mp_obj_t py_percentile_l_value(mp_obj_t self_in) { return ((py_percentile_obj_t *) self_in)->LValue; }
mp_obj_t py_percentile_a_value(mp_obj_t self_in) { return ((py_percentile_obj_t *) self_in)->AValue; }
mp_obj_t py_percentile_b_value(mp_obj_t self_in) { return ((py_percentile_obj_t *) self_in)->BValue; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_percentile_value_obj, py_percentile_value);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_percentile_l_value_obj, py_percentile_l_value);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_percentile_a_value_obj, py_percentile_a_value);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_percentile_b_value_obj, py_percentile_b_value);

STATIC const mp_rom_map_elem_t py_percentile_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&py_percentile_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_value), MP_ROM_PTR(&py_percentile_l_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_value), MP_ROM_PTR(&py_percentile_a_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_value), MP_ROM_PTR(&py_percentile_b_value_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_percentile_locals_dict, py_percentile_locals_dict_table);

static const mp_obj_type_t py_percentile_type = {
    { &mp_type_type },
    .name  = MP_QSTR_percentile,
    .print = py_percentile_print,
    .subscr = py_percentile_subscr,
    .locals_dict = (mp_obj_t) &py_percentile_locals_dict,
};

// Histogram Object //
#define py_histogram_obj_size 3
typedef struct py_histogram_obj {
    mp_obj_base_t base;
    image_bpp_t bpp;
    mp_obj_t LBins, ABins, BBins;
} py_histogram_obj_t;

static void py_histogram_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_histogram_obj_t *self = self_in;
    switch(self->bpp) {
        case IMAGE_BPP_BINARY: {
            mp_printf(print, "{bins:");
            mp_obj_print_helper(print, self->LBins, kind);
            mp_printf(print, "}");
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            mp_printf(print, "{bins:");
            mp_obj_print_helper(print, self->LBins, kind);
            mp_printf(print, "}");
            break;
        }
        case IMAGE_BPP_RGB565: {
            mp_printf(print, "{l_bins:");
            mp_obj_print_helper(print, self->LBins, kind);
            mp_printf(print, ", a_bins:");
            mp_obj_print_helper(print, self->ABins, kind);
            mp_printf(print, ", b_bins:");
            mp_obj_print_helper(print, self->BBins, kind);
            mp_printf(print, "}");
            break;
        }
        default: {
            mp_printf(print, "{}");
            break;
        }
    }
}

static mp_obj_t py_histogram_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_histogram_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_histogram_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->LBins) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_histogram_obj_size, index, false)) {
            case 0: return self->LBins;
            case 1: return self->ABins;
            case 2: return self->BBins;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_histogram_bins(mp_obj_t self_in) { return ((py_histogram_obj_t *) self_in)->LBins; }
mp_obj_t py_histogram_l_bins(mp_obj_t self_in) { return ((py_histogram_obj_t *) self_in)->LBins; }
mp_obj_t py_histogram_a_bins(mp_obj_t self_in) { return ((py_histogram_obj_t *) self_in)->ABins; }
mp_obj_t py_histogram_b_bins(mp_obj_t self_in) { return ((py_histogram_obj_t *) self_in)->BBins; }

mp_obj_t py_histogram_get_percentile(mp_obj_t self_in, mp_obj_t percentile)
{
    histogram_t hist;
    hist.LBinCount = ((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->LBins)->len;
    hist.ABinCount = ((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->ABins)->len;
    hist.BBinCount = ((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->BBins)->len;
    fb_alloc_mark();
    hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
    hist.ABins = fb_alloc(hist.ABinCount * sizeof(float));
    hist.BBins = fb_alloc(hist.BBinCount * sizeof(float));

    for (int i = 0; i < hist.LBinCount; i++) {
        hist.LBins[i] = mp_obj_get_float(((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->LBins)->items[i]);
    }

    for (int i = 0; i < hist.ABinCount; i++) {
        hist.ABins[i] = mp_obj_get_float(((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->ABins)->items[i]);
    }

    for (int i = 0; i < hist.BBinCount; i++) {
        hist.BBins[i] = mp_obj_get_float(((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->BBins)->items[i]);
    }

    percentile_t p;
    imlib_get_percentile(&p, ((py_histogram_obj_t *) self_in)->bpp, &hist, mp_obj_get_float(percentile));
    if (hist.BBinCount) fb_free();
    if (hist.ABinCount) fb_free();
    if (hist.LBinCount) fb_free();
    fb_alloc_free_till_mark();

    py_percentile_obj_t *o = m_new_obj(py_percentile_obj_t);
    o->base.type = &py_percentile_type;
    o->bpp = ((py_histogram_obj_t *) self_in)->bpp;

    o->LValue = mp_obj_new_int(p.LValue);
    o->AValue = mp_obj_new_int(p.AValue);
    o->BValue = mp_obj_new_int(p.BValue);

    return o;
}

mp_obj_t py_histogram_get_statistics(mp_obj_t self_in)
{
    histogram_t hist;
    hist.LBinCount = ((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->LBins)->len;
    hist.ABinCount = ((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->ABins)->len;
    hist.BBinCount = ((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->BBins)->len;
    fb_alloc_mark();
    hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
    hist.ABins = fb_alloc(hist.ABinCount * sizeof(float));
    hist.BBins = fb_alloc(hist.BBinCount * sizeof(float));

    for (int i = 0; i < hist.LBinCount; i++) {
        hist.LBins[i] = mp_obj_get_float(((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->LBins)->items[i]);
    }

    for (int i = 0; i < hist.ABinCount; i++) {
        hist.ABins[i] = mp_obj_get_float(((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->ABins)->items[i]);
    }

    for (int i = 0; i < hist.BBinCount; i++) {
        hist.BBins[i] = mp_obj_get_float(((mp_obj_list_t *) ((py_histogram_obj_t *) self_in)->BBins)->items[i]);
    }

    statistics_t stats;
    imlib_get_statistics(&stats, ((py_histogram_obj_t *) self_in)->bpp, &hist);
    if (hist.BBinCount) fb_free();
    if (hist.ABinCount) fb_free();
    if (hist.LBinCount) fb_free();
    fb_alloc_free_till_mark();

    py_statistics_obj_t *o = m_new_obj(py_statistics_obj_t);
    o->base.type = &py_statistics_type;
    o->bpp = ((py_histogram_obj_t *) self_in)->bpp;

    o->LMean = mp_obj_new_int(stats.LMean);
    o->LMedian = mp_obj_new_int(stats.LMedian);
    o->LMode= mp_obj_new_int(stats.LMode);
    o->LSTDev = mp_obj_new_int(stats.LSTDev);
    o->LMin = mp_obj_new_int(stats.LMin);
    o->LMax = mp_obj_new_int(stats.LMax);
    o->LLQ = mp_obj_new_int(stats.LLQ);
    o->LUQ = mp_obj_new_int(stats.LUQ);
    o->AMean = mp_obj_new_int(stats.AMean);
    o->AMedian = mp_obj_new_int(stats.AMedian);
    o->AMode= mp_obj_new_int(stats.AMode);
    o->ASTDev = mp_obj_new_int(stats.ASTDev);
    o->AMin = mp_obj_new_int(stats.AMin);
    o->AMax = mp_obj_new_int(stats.AMax);
    o->ALQ = mp_obj_new_int(stats.ALQ);
    o->AUQ = mp_obj_new_int(stats.AUQ);
    o->BMean = mp_obj_new_int(stats.BMean);
    o->BMedian = mp_obj_new_int(stats.BMedian);
    o->BMode= mp_obj_new_int(stats.BMode);
    o->BSTDev = mp_obj_new_int(stats.BSTDev);
    o->BMin = mp_obj_new_int(stats.BMin);
    o->BMax = mp_obj_new_int(stats.BMax);
    o->BLQ = mp_obj_new_int(stats.BLQ);
    o->BUQ = mp_obj_new_int(stats.BUQ);

    return o;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_bins_obj, py_histogram_bins);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_l_bins_obj, py_histogram_l_bins);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_a_bins_obj, py_histogram_a_bins);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_b_bins_obj, py_histogram_b_bins);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_histogram_get_percentile_obj, py_histogram_get_percentile);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_get_statistics_obj, py_histogram_get_statistics);

STATIC const mp_rom_map_elem_t py_histogram_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_bins), MP_ROM_PTR(&py_histogram_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_bins), MP_ROM_PTR(&py_histogram_l_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_bins), MP_ROM_PTR(&py_histogram_a_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_bins), MP_ROM_PTR(&py_histogram_b_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_percentile), MP_ROM_PTR(&py_histogram_get_percentile_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_stats), MP_ROM_PTR(&py_histogram_get_statistics_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_statistics), MP_ROM_PTR(&py_histogram_get_statistics_obj) },
    { MP_ROM_QSTR(MP_QSTR_statistics), MP_ROM_PTR(&py_histogram_get_statistics_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_histogram_locals_dict, py_histogram_locals_dict_table);

static const mp_obj_type_t py_histogram_type = {
    { &mp_type_type },
    .name  = MP_QSTR_histogram,
    .print = py_histogram_print,
    .subscr = py_histogram_subscr,
    .locals_dict = (mp_obj_t) &py_histogram_locals_dict,
};

static mp_obj_t py_image_get_histogram(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    histogram_t hist;
    switch(arg_img->bpp) {
        case IMAGE_BPP_BINARY: {
            int bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_BINARY_MAX-COLOR_BINARY_MIN+1));
            PY_ASSERT_TRUE_MSG(bins >= 2, "bins must be >= 2");
            hist.LBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_l_bins), bins);
            PY_ASSERT_TRUE_MSG(hist.LBinCount >= 2, "l_bins must be >= 2");
            hist.ABinCount = 0;
            hist.BBinCount = 0;
            fb_alloc_mark();
            hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
            hist.ABins = NULL;
            hist.BBins = NULL;
            imlib_get_histogram(&hist, arg_img, &roi);
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            int bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1));
            PY_ASSERT_TRUE_MSG(bins >= 2, "bins must be >= 2");
            hist.LBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_l_bins), bins);
            PY_ASSERT_TRUE_MSG(hist.LBinCount >= 2, "l_bins must be >= 2");
            hist.ABinCount = 0;
            hist.BBinCount = 0;
            fb_alloc_mark();
            hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
            hist.ABins = NULL;
            hist.BBins = NULL;
            imlib_get_histogram(&hist, arg_img, &roi);
            break;
        }
        case IMAGE_BPP_RGB565: {
            int l_bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_L_MAX-COLOR_L_MIN+1));
            PY_ASSERT_TRUE_MSG(l_bins >= 2, "bins must be >= 2");
            hist.LBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_l_bins), l_bins);
            PY_ASSERT_TRUE_MSG(hist.LBinCount >= 2, "l_bins must be >= 2");
            int a_bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_A_MAX-COLOR_A_MIN+1));
            PY_ASSERT_TRUE_MSG(a_bins >= 2, "bins must be >= 2");
            hist.ABinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_a_bins), a_bins);
            PY_ASSERT_TRUE_MSG(hist.ABinCount >= 2, "a_bins must be >= 2");
            int b_bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_B_MAX-COLOR_B_MIN+1));
            PY_ASSERT_TRUE_MSG(b_bins >= 2, "bins must be >= 2");
            hist.BBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_b_bins), b_bins);
            PY_ASSERT_TRUE_MSG(hist.BBinCount >= 2, "b_bins must be >= 2");
            fb_alloc_mark();
            hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
            hist.ABins = fb_alloc(hist.ABinCount * sizeof(float));
            hist.BBins = fb_alloc(hist.BBinCount * sizeof(float));
            imlib_get_histogram(&hist, arg_img, &roi);
            break;
        }
        default: {
            return MP_OBJ_NULL;
        }
    }

    py_histogram_obj_t *o = m_new_obj(py_histogram_obj_t);
    o->base.type = &py_histogram_type;
    o->bpp = arg_img->bpp;

    o->LBins = mp_obj_new_list(hist.LBinCount, NULL);
    o->ABins = mp_obj_new_list(hist.ABinCount, NULL);
    o->BBins = mp_obj_new_list(hist.BBinCount, NULL);

    for (int i = 0; i < hist.LBinCount; i++) {
        ((mp_obj_list_t *) o->LBins)->items[i] = mp_obj_new_float(hist.LBins[i]);
    }

    for (int i = 0; i < hist.ABinCount; i++) {
        ((mp_obj_list_t *) o->ABins)->items[i] = mp_obj_new_float(hist.ABins[i]);
    }

    for (int i = 0; i < hist.BBinCount; i++) {
        ((mp_obj_list_t *) o->BBins)->items[i] = mp_obj_new_float(hist.BBins[i]);
    }

    if (hist.BBinCount) fb_free();
    if (hist.ABinCount) fb_free();
    if (hist.LBinCount) fb_free();
    fb_alloc_free_till_mark();

    return o;
}

static mp_obj_t py_image_get_statistics(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    histogram_t hist;
    switch(arg_img->bpp) {
        case IMAGE_BPP_BINARY: {
            int bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_BINARY_MAX-COLOR_BINARY_MIN+1));
            PY_ASSERT_TRUE_MSG(bins >= 2, "bins must be >= 2");
            hist.LBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_l_bins), bins);
            PY_ASSERT_TRUE_MSG(hist.LBinCount >= 2, "l_bins must be >= 2");
            hist.ABinCount = 0;
            hist.BBinCount = 0;
            fb_alloc_mark();
            hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
            hist.ABins = NULL;
            hist.BBins = NULL;
            imlib_get_histogram(&hist, arg_img, &roi);
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            int bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1));
            PY_ASSERT_TRUE_MSG(bins >= 2, "bins must be >= 2");
            hist.LBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_l_bins), bins);
            PY_ASSERT_TRUE_MSG(hist.LBinCount >= 2, "l_bins must be >= 2");
            hist.ABinCount = 0;
            hist.BBinCount = 0;
            fb_alloc_mark();
            hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
            hist.ABins = NULL;
            hist.BBins = NULL;
            imlib_get_histogram(&hist, arg_img, &roi);
            break;
        }
        case IMAGE_BPP_RGB565: {
            int l_bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_L_MAX-COLOR_L_MIN+1));
            PY_ASSERT_TRUE_MSG(l_bins >= 2, "bins must be >= 2");
            hist.LBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_l_bins), l_bins);
            PY_ASSERT_TRUE_MSG(hist.LBinCount >= 2, "l_bins must be >= 2");
            int a_bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_A_MAX-COLOR_A_MIN+1));
            PY_ASSERT_TRUE_MSG(a_bins >= 2, "bins must be >= 2");
            hist.ABinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_a_bins), a_bins);
            PY_ASSERT_TRUE_MSG(hist.ABinCount >= 2, "a_bins must be >= 2");
            int b_bins = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bins), (COLOR_B_MAX-COLOR_B_MIN+1));
            PY_ASSERT_TRUE_MSG(b_bins >= 2, "bins must be >= 2");
            hist.BBinCount = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_b_bins), b_bins);
            PY_ASSERT_TRUE_MSG(hist.BBinCount >= 2, "b_bins must be >= 2");
            fb_alloc_mark();
            hist.LBins = fb_alloc(hist.LBinCount * sizeof(float));
            hist.ABins = fb_alloc(hist.ABinCount * sizeof(float));
            hist.BBins = fb_alloc(hist.BBinCount * sizeof(float));
            imlib_get_histogram(&hist, arg_img, &roi);
            break;
        }
        default: {
            return MP_OBJ_NULL;
        }
    }

    statistics_t stats;
    imlib_get_statistics(&stats, arg_img->bpp, &hist);
    if (hist.BBinCount) fb_free();
    if (hist.ABinCount) fb_free();
    if (hist.LBinCount) fb_free();
    fb_alloc_free_till_mark();

    py_statistics_obj_t *o = m_new_obj(py_statistics_obj_t);
    o->base.type = &py_statistics_type;
    o->bpp = arg_img->bpp;

    o->LMean = mp_obj_new_int(stats.LMean);
    o->LMedian = mp_obj_new_int(stats.LMedian);
    o->LMode= mp_obj_new_int(stats.LMode);
    o->LSTDev = mp_obj_new_int(stats.LSTDev);
    o->LMin = mp_obj_new_int(stats.LMin);
    o->LMax = mp_obj_new_int(stats.LMax);
    o->LLQ = mp_obj_new_int(stats.LLQ);
    o->LUQ = mp_obj_new_int(stats.LUQ);
    o->AMean = mp_obj_new_int(stats.AMean);
    o->AMedian = mp_obj_new_int(stats.AMedian);
    o->AMode= mp_obj_new_int(stats.AMode);
    o->ASTDev = mp_obj_new_int(stats.ASTDev);
    o->AMin = mp_obj_new_int(stats.AMin);
    o->AMax = mp_obj_new_int(stats.AMax);
    o->ALQ = mp_obj_new_int(stats.ALQ);
    o->AUQ = mp_obj_new_int(stats.AUQ);
    o->BMean = mp_obj_new_int(stats.BMean);
    o->BMedian = mp_obj_new_int(stats.BMedian);
    o->BMode= mp_obj_new_int(stats.BMode);
    o->BSTDev = mp_obj_new_int(stats.BSTDev);
    o->BMin = mp_obj_new_int(stats.BMin);
    o->BMax = mp_obj_new_int(stats.BMax);
    o->BLQ = mp_obj_new_int(stats.BLQ);
    o->BUQ = mp_obj_new_int(stats.BUQ);

    return o;
}

// Line Object //
#define py_line_obj_size 8
typedef struct py_line_obj {
    mp_obj_base_t base;
    mp_obj_t x1, y1, x2, y2, length, magnitude, theta, rho;
} py_line_obj_t;

static void py_line_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_line_obj_t *self = self_in;
    mp_printf(print,
              "{x1:%d, y1:%d, x2:%d, y2:%d, length:%d, magnitude:%d, theta:%d, rho:%d}",
              mp_obj_get_int(self->x1),
              mp_obj_get_int(self->y1),
              mp_obj_get_int(self->x2),
              mp_obj_get_int(self->y2),
              mp_obj_get_int(self->length),
              mp_obj_get_int(self->magnitude),
              mp_obj_get_int(self->theta),
              mp_obj_get_int(self->rho));
}

static mp_obj_t py_line_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_line_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_line_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x1) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_line_obj_size, index, false)) {
            case 0: return self->x1;
            case 1: return self->y1;
            case 2: return self->x2;
            case 3: return self->y2;
            case 4: return self->length;
            case 5: return self->magnitude;
            case 6: return self->theta;
            case 7: return self->rho;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_line_line(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_line_obj_t *) self_in)->x1,
                                              ((py_line_obj_t *) self_in)->y1,
                                              ((py_line_obj_t *) self_in)->x2,
                                              ((py_line_obj_t *) self_in)->y2});
}

mp_obj_t py_line_x1(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->x1; }
mp_obj_t py_line_y1(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->y1; }
mp_obj_t py_line_x2(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->x2; }
mp_obj_t py_line_y2(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->y2; }
mp_obj_t py_line_length(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->length; }
mp_obj_t py_line_magnitude(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->magnitude; }
mp_obj_t py_line_theta(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->theta; }
mp_obj_t py_line_rho(mp_obj_t self_in) { return ((py_line_obj_t *) self_in)->rho; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_line_obj, py_line_line);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_x1_obj, py_line_x1);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_y1_obj, py_line_y1);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_x2_obj, py_line_x2);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_y2_obj, py_line_y2);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_length_obj, py_line_length);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_magnitude_obj, py_line_magnitude);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_theta_obj, py_line_theta);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_line_rho_obj, py_line_rho);

STATIC const mp_rom_map_elem_t py_line_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&py_line_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_x1), MP_ROM_PTR(&py_line_x1_obj) },
    { MP_ROM_QSTR(MP_QSTR_y1), MP_ROM_PTR(&py_line_y1_obj) },
    { MP_ROM_QSTR(MP_QSTR_x2), MP_ROM_PTR(&py_line_x2_obj) },
    { MP_ROM_QSTR(MP_QSTR_y2), MP_ROM_PTR(&py_line_y2_obj) },
    { MP_ROM_QSTR(MP_QSTR_length), MP_ROM_PTR(&py_line_length_obj) },
    { MP_ROM_QSTR(MP_QSTR_magnitude), MP_ROM_PTR(&py_line_magnitude_obj) },
    { MP_ROM_QSTR(MP_QSTR_theta), MP_ROM_PTR(&py_line_theta_obj) },
    { MP_ROM_QSTR(MP_QSTR_rho), MP_ROM_PTR(&py_line_rho_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_line_locals_dict, py_line_locals_dict_table);

static const mp_obj_type_t py_line_type = {
    { &mp_type_type },
    .name  = MP_QSTR_line,
    .print = py_line_print,
    .subscr = py_line_subscr,
    .locals_dict = (mp_obj_t) &py_line_locals_dict,
};

static mp_obj_t py_image_get_regression(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    mp_uint_t arg_thresholds_len;
    mp_obj_t *arg_thresholds;
    mp_obj_get_array(args[1], &arg_thresholds_len, &arg_thresholds);
    if (!arg_thresholds_len) return mp_const_none;

    list_t thresholds;
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));

    for(mp_uint_t i = 0; i < arg_thresholds_len; i++) {
        mp_uint_t arg_threshold_len;
        mp_obj_t *arg_threshold;
        mp_obj_get_array(arg_thresholds[i], &arg_threshold_len, &arg_threshold);
        if (arg_threshold_len) {
            color_thresholds_list_lnk_data_t lnk_data;
            lnk_data.LMin = (arg_threshold_len > 0) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[0]),
                        IM_MAX(COLOR_L_MAX, COLOR_GRAYSCALE_MAX)), IM_MIN(COLOR_L_MIN, COLOR_GRAYSCALE_MIN)) : IM_MIN(COLOR_L_MIN, COLOR_GRAYSCALE_MIN);
            lnk_data.LMax = (arg_threshold_len > 1) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[1]),
                        IM_MAX(COLOR_L_MAX, COLOR_GRAYSCALE_MAX)), IM_MIN(COLOR_L_MIN, COLOR_GRAYSCALE_MIN)) : IM_MAX(COLOR_L_MAX, COLOR_GRAYSCALE_MAX);
            lnk_data.AMin = (arg_threshold_len > 2) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[2]), COLOR_A_MAX), COLOR_A_MIN) : COLOR_A_MIN;
            lnk_data.AMax = (arg_threshold_len > 3) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[3]), COLOR_A_MAX), COLOR_A_MIN) : COLOR_A_MAX;
            lnk_data.BMin = (arg_threshold_len > 4) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[4]), COLOR_B_MAX), COLOR_B_MIN) : COLOR_B_MIN;
            lnk_data.BMax = (arg_threshold_len > 5) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[5]), COLOR_B_MAX), COLOR_B_MIN) : COLOR_B_MAX;
            color_thresholds_list_lnk_data_t lnk_data_tmp;
            memcpy(&lnk_data_tmp, &lnk_data, sizeof(color_thresholds_list_lnk_data_t));
            lnk_data.LMin = IM_MIN(lnk_data_tmp.LMin, lnk_data_tmp.LMax);
            lnk_data.LMax = IM_MAX(lnk_data_tmp.LMin, lnk_data_tmp.LMax);
            lnk_data.AMin = IM_MIN(lnk_data_tmp.AMin, lnk_data_tmp.AMax);
            lnk_data.AMax = IM_MAX(lnk_data_tmp.AMin, lnk_data_tmp.AMax);
            lnk_data.BMin = IM_MIN(lnk_data_tmp.BMin, lnk_data_tmp.BMax);
            lnk_data.BMax = IM_MAX(lnk_data_tmp.BMin, lnk_data_tmp.BMax);
            list_push_back(&thresholds, &lnk_data);
        }
    }

    unsigned int x_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_stride), 2);
    PY_ASSERT_TRUE_MSG(x_stride > 0, "x_stride must not be zero.");
    unsigned int y_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_stride), 1);
    PY_ASSERT_TRUE_MSG(y_stride > 0, "y_stride must not be zero.");
    bool invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);
    bool robust = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_robust), false);

    find_lines_list_lnk_data_t out;
    fb_alloc_mark();
    bool result = imlib_get_regression(&out, arg_img, &roi, x_stride, y_stride, &thresholds, invert, robust);
    fb_alloc_free_till_mark();
    list_free(&thresholds);
    if (!result) {
        return mp_const_none;
    }

    py_line_obj_t *o = m_new_obj(py_line_obj_t);
    o->base.type = &py_line_type;
    o->x1 = mp_obj_new_int(out.line.x1);
    o->y1 = mp_obj_new_int(out.line.y1);
    o->x2 = mp_obj_new_int(out.line.x2);
    o->y2 = mp_obj_new_int(out.line.y2);
    int x_diff = out.line.x2 - out.line.x1;
    int y_diff = out.line.y2 - out.line.y1;
    o->length = mp_obj_new_int(fast_roundf(fast_sqrtf((x_diff * x_diff) + (y_diff * y_diff))));
    o->magnitude = mp_obj_new_int(out.magnitude);
    o->theta = mp_obj_new_int(out.theta);
    o->rho = mp_obj_new_int(out.rho);

    return o;
}

// Blob Object //
#define py_blob_obj_size 10
typedef struct py_blob_obj {
    mp_obj_base_t base;
    mp_obj_t x, y, w, h, pixels, cx, cy, rotation, code, count;
} py_blob_obj_t;

static void py_blob_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_blob_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, w:%d, h:%d, pixels:%d, cx:%d, cy:%d, rotation:%f, code:%d, count:%d}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_get_int(self->pixels),
              mp_obj_get_int(self->cx),
              mp_obj_get_int(self->cy),
              (double) mp_obj_get_float(self->rotation),
              mp_obj_get_int(self->code),
              mp_obj_get_int(self->count));
}

static mp_obj_t py_blob_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_blob_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_blob_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_blob_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->pixels;
            case 5: return self->cx;
            case 6: return self->cy;
            case 7: return self->rotation;
            case 8: return self->code;
            case 9: return self->count;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_blob_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_blob_obj_t *) self_in)->x,
                                              ((py_blob_obj_t *) self_in)->y,
                                              ((py_blob_obj_t *) self_in)->w,
                                              ((py_blob_obj_t *) self_in)->h});
}

mp_obj_t py_blob_x(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->x; }
mp_obj_t py_blob_y(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->y; }
mp_obj_t py_blob_w(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->w; }
mp_obj_t py_blob_h(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->h; }
mp_obj_t py_blob_pixels(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->pixels; }
mp_obj_t py_blob_cx(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->cx; }
mp_obj_t py_blob_cy(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->cy; }
mp_obj_t py_blob_rotation(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->rotation; }
mp_obj_t py_blob_code(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->code; }
mp_obj_t py_blob_count(mp_obj_t self_in) { return ((py_blob_obj_t *) self_in)->count; }
mp_obj_t py_blob_area(mp_obj_t self_in) {
    return mp_obj_new_int(mp_obj_get_int(((py_blob_obj_t *) self_in)->w) * mp_obj_get_int(((py_blob_obj_t *) self_in)->h));
}
mp_obj_t py_blob_density(mp_obj_t self_in) {
    int area = mp_obj_get_int(((py_blob_obj_t *) self_in)->w) * mp_obj_get_int(((py_blob_obj_t *) self_in)->h);
    if (area) return mp_obj_new_float(mp_obj_get_int(((py_blob_obj_t *) self_in)->pixels) / (float) area);
    return mp_obj_new_float(0.0f);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_rect_obj, py_blob_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_x_obj, py_blob_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_y_obj, py_blob_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_w_obj, py_blob_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_h_obj, py_blob_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_pixels_obj, py_blob_pixels);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_cx_obj, py_blob_cx);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_cy_obj, py_blob_cy);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_rotation_obj, py_blob_rotation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_code_obj, py_blob_code);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_count_obj, py_blob_count);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_area_obj, py_blob_area);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_blob_density_obj, py_blob_density);

STATIC const mp_rom_map_elem_t py_blob_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_blob_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_blob_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_blob_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_blob_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_blob_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixels), MP_ROM_PTR(&py_blob_pixels_obj) },
    { MP_ROM_QSTR(MP_QSTR_cx), MP_ROM_PTR(&py_blob_cx_obj) },
    { MP_ROM_QSTR(MP_QSTR_cy), MP_ROM_PTR(&py_blob_cy_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotation), MP_ROM_PTR(&py_blob_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_code), MP_ROM_PTR(&py_blob_code_obj) },
    { MP_ROM_QSTR(MP_QSTR_count), MP_ROM_PTR(&py_blob_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_area), MP_ROM_PTR(&py_blob_area_obj) } ,
    { MP_ROM_QSTR(MP_QSTR_density), MP_ROM_PTR(&py_blob_density_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_blob_locals_dict, py_blob_locals_dict_table);

static const mp_obj_type_t py_blob_type = {
    { &mp_type_type },
    .name  = MP_QSTR_blob,
    .print = py_blob_print,
    .subscr = py_blob_subscr,
    .locals_dict = (mp_obj_t) &py_blob_locals_dict,
};

static bool py_image_find_blobs_threshold_cb(void *fun_obj, find_blobs_list_lnk_data_t *blob)
{
    py_blob_obj_t *o = m_new_obj(py_blob_obj_t);
    o->base.type = &py_blob_type;
    o->x = mp_obj_new_int(blob->rect.x);
    o->y = mp_obj_new_int(blob->rect.y);
    o->w = mp_obj_new_int(blob->rect.w);
    o->h = mp_obj_new_int(blob->rect.h);
    o->pixels = mp_obj_new_int(blob->pixels);
    o->cx = mp_obj_new_int(blob->centroid.x);
    o->cy = mp_obj_new_int(blob->centroid.y);
    o->rotation = mp_obj_new_float(blob->rotation);
    o->code = mp_obj_new_int(blob->code);
    o->count = mp_obj_new_int(blob->count);

    return mp_obj_is_true(mp_call_function_1(fun_obj, o));
}

static bool py_image_find_blobs_merge_cb(void *fun_obj, find_blobs_list_lnk_data_t *blob0, find_blobs_list_lnk_data_t *blob1)
{
    py_blob_obj_t *o0 = m_new_obj(py_blob_obj_t);
    o0->base.type = &py_blob_type;
    o0->x = mp_obj_new_int(blob0->rect.x);
    o0->y = mp_obj_new_int(blob0->rect.y);
    o0->w = mp_obj_new_int(blob0->rect.w);
    o0->h = mp_obj_new_int(blob0->rect.h);
    o0->pixels = mp_obj_new_int(blob0->pixels);
    o0->cx = mp_obj_new_int(blob0->centroid.x);
    o0->cy = mp_obj_new_int(blob0->centroid.y);
    o0->rotation = mp_obj_new_float(blob0->rotation);
    o0->code = mp_obj_new_int(blob0->code);
    o0->count = mp_obj_new_int(blob0->count);

    py_blob_obj_t *o1 = m_new_obj(py_blob_obj_t);
    o1->base.type = &py_blob_type;
    o1->x = mp_obj_new_int(blob1->rect.x);
    o1->y = mp_obj_new_int(blob1->rect.y);
    o1->w = mp_obj_new_int(blob1->rect.w);
    o1->h = mp_obj_new_int(blob1->rect.h);
    o1->pixels = mp_obj_new_int(blob1->pixels);
    o1->cx = mp_obj_new_int(blob1->centroid.x);
    o1->cy = mp_obj_new_int(blob1->centroid.y);
    o1->rotation = mp_obj_new_float(blob1->rotation);
    o1->code = mp_obj_new_int(blob1->code);
    o1->count = mp_obj_new_int(blob1->count);

    return mp_obj_is_true(mp_call_function_2(fun_obj, o0, o1));
}

static mp_obj_t py_image_find_blobs(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    mp_uint_t arg_thresholds_len;
    mp_obj_t *arg_thresholds;
    mp_obj_get_array(args[1], &arg_thresholds_len, &arg_thresholds);
    if (!arg_thresholds_len) return mp_obj_new_list(0, NULL);

    list_t thresholds;
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));

    for(mp_uint_t i = 0; i < arg_thresholds_len; i++) {
        mp_uint_t arg_threshold_len;
        mp_obj_t *arg_threshold;
        mp_obj_get_array(arg_thresholds[i], &arg_threshold_len, &arg_threshold);
        if (arg_threshold_len) {
            color_thresholds_list_lnk_data_t lnk_data;
            lnk_data.LMin = (arg_threshold_len > 0) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[0]),
                        IM_MAX(COLOR_L_MAX, COLOR_GRAYSCALE_MAX)), IM_MIN(COLOR_L_MIN, COLOR_GRAYSCALE_MIN)) : IM_MIN(COLOR_L_MIN, COLOR_GRAYSCALE_MIN);
            lnk_data.LMax = (arg_threshold_len > 1) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[1]),
                        IM_MAX(COLOR_L_MAX, COLOR_GRAYSCALE_MAX)), IM_MIN(COLOR_L_MIN, COLOR_GRAYSCALE_MIN)) : IM_MAX(COLOR_L_MAX, COLOR_GRAYSCALE_MAX);
            lnk_data.AMin = (arg_threshold_len > 2) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[2]), COLOR_A_MAX), COLOR_A_MIN) : COLOR_A_MIN;
            lnk_data.AMax = (arg_threshold_len > 3) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[3]), COLOR_A_MAX), COLOR_A_MIN) : COLOR_A_MAX;
            lnk_data.BMin = (arg_threshold_len > 4) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[4]), COLOR_B_MAX), COLOR_B_MIN) : COLOR_B_MIN;
            lnk_data.BMax = (arg_threshold_len > 5) ? IM_MAX(IM_MIN(mp_obj_get_int(arg_threshold[5]), COLOR_B_MAX), COLOR_B_MIN) : COLOR_B_MAX;
            color_thresholds_list_lnk_data_t lnk_data_tmp;
            memcpy(&lnk_data_tmp, &lnk_data, sizeof(color_thresholds_list_lnk_data_t));
            lnk_data.LMin = IM_MIN(lnk_data_tmp.LMin, lnk_data_tmp.LMax);
            lnk_data.LMax = IM_MAX(lnk_data_tmp.LMin, lnk_data_tmp.LMax);
            lnk_data.AMin = IM_MIN(lnk_data_tmp.AMin, lnk_data_tmp.AMax);
            lnk_data.AMax = IM_MAX(lnk_data_tmp.AMin, lnk_data_tmp.AMax);
            lnk_data.BMin = IM_MIN(lnk_data_tmp.BMin, lnk_data_tmp.BMax);
            lnk_data.BMax = IM_MAX(lnk_data_tmp.BMin, lnk_data_tmp.BMax);
            list_push_back(&thresholds, &lnk_data);
        }
    }

    unsigned int x_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_stride), 2);
    PY_ASSERT_TRUE_MSG(x_stride > 0, "x_stride must not be zero.");
    unsigned int y_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_stride), 1);
    PY_ASSERT_TRUE_MSG(y_stride > 0, "y_stride must not be zero.");
    bool invert = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);
    unsigned int area_threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_area_threshold), 10);
    unsigned int pixels_threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_pixels_threshold), 10);
    bool merge = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_merge), false);
    int margin = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_margin), 0);

    mp_map_elem_t *threshold_cb_kw_arg = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold_cb), MP_MAP_LOOKUP);
    mp_obj_t threshold_cb_kw_val = (threshold_cb_kw_arg != NULL) ? threshold_cb_kw_arg->value : NULL;

    mp_map_elem_t *merge_cb_kw_arg = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_merge_cb), MP_MAP_LOOKUP);
    mp_obj_t merge_cb_kw_val = (merge_cb_kw_arg != NULL) ? merge_cb_kw_arg->value : NULL;

    list_t out;
    fb_alloc_mark();
    imlib_find_blobs(&out, arg_img, &roi, x_stride, y_stride, &thresholds, invert, area_threshold,
            pixels_threshold, merge, margin, py_image_find_blobs_threshold_cb, threshold_cb_kw_val,
            py_image_find_blobs_merge_cb, merge_cb_kw_val);
    fb_alloc_free_till_mark();
    list_free(&thresholds);

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_blobs_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_blob_obj_t *o = m_new_obj(py_blob_obj_t);
        o->base.type = &py_blob_type;
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->pixels = mp_obj_new_int(lnk_data.pixels);
        o->cx = mp_obj_new_int(lnk_data.centroid.x);
        o->cy = mp_obj_new_int(lnk_data.centroid.y);
        o->rotation = mp_obj_new_float(lnk_data.rotation);
        o->code = mp_obj_new_int(lnk_data.code);
        o->count = mp_obj_new_int(lnk_data.count);

        objects_list->items[i] = o;
    }

    return objects_list;
}

static mp_obj_t py_image_find_lines(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    unsigned int x_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_stride), 2);
    PY_ASSERT_TRUE_MSG(x_stride > 0, "x_stride must not be zero.");
    unsigned int y_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_stride), 1);
    PY_ASSERT_TRUE_MSG(y_stride > 0, "y_stride must not be zero.");

    list_t out;
    fb_alloc_mark();
    imlib_find_lines(&out, arg_img, &roi, x_stride, y_stride, py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 1000),
                                                              py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_theta_margin), 25),
                                                              py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_rho_margin), 25));
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_lines_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_line_obj_t *o = m_new_obj(py_line_obj_t);
        o->base.type = &py_line_type;
        o->x1 = mp_obj_new_int(lnk_data.line.x1);
        o->y1 = mp_obj_new_int(lnk_data.line.y1);
        o->x2 = mp_obj_new_int(lnk_data.line.x2);
        o->y2 = mp_obj_new_int(lnk_data.line.y2);
        int x_diff = lnk_data.line.x2 - lnk_data.line.x1;
        int y_diff = lnk_data.line.y2 - lnk_data.line.y1;
        o->length = mp_obj_new_int(fast_roundf(fast_sqrtf((x_diff * x_diff) + (y_diff * y_diff))));
        o->magnitude = mp_obj_new_int(lnk_data.magnitude);
        o->theta = mp_obj_new_int(lnk_data.theta);
        o->rho = mp_obj_new_int(lnk_data.rho);

        objects_list->items[i] = o;
    }

    return objects_list;
}

#ifdef OMV_ENABLE_FIND_LINE_SEGMENTS
static mp_obj_t py_image_find_line_segments(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    list_t out;
    fb_alloc_mark();
    imlib_lsd_find_line_segments(&out, arg_img, &roi,
                                 py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_merge_distance), 0),
                                 py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_max_theta_diff), 15));
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_lines_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_line_obj_t *o = m_new_obj(py_line_obj_t);
        o->base.type = &py_line_type;
        o->x1 = mp_obj_new_int(lnk_data.line.x1);
        o->y1 = mp_obj_new_int(lnk_data.line.y1);
        o->x2 = mp_obj_new_int(lnk_data.line.x2);
        o->y2 = mp_obj_new_int(lnk_data.line.y2);
        int x_diff = lnk_data.line.x2 - lnk_data.line.x1;
        int y_diff = lnk_data.line.y2 - lnk_data.line.y1;
        o->length = mp_obj_new_int(fast_roundf(fast_sqrtf((x_diff * x_diff) + (y_diff * y_diff))));
        o->magnitude = mp_obj_new_int(lnk_data.magnitude);
        o->theta = mp_obj_new_int(lnk_data.theta);
        o->rho = mp_obj_new_int(lnk_data.rho);

        objects_list->items[i] = o;
    }

    return objects_list;
}
#endif

#ifdef OMV_ENABLE_FIND_CIRCLES
// Circle Object //
#define py_circle_obj_size 4
typedef struct py_circle_obj {
    mp_obj_base_t base;
    mp_obj_t x, y, r, magnitude;
} py_circle_obj_t;

static void py_circle_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_circle_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, r:%d, magnitude:%d}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->r),
              mp_obj_get_int(self->magnitude));
}

static mp_obj_t py_circle_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_circle_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_circle_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_circle_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->r;
            case 3: return self->magnitude;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_circle_circle(mp_obj_t self_in)
{
    return mp_obj_new_tuple(3, (mp_obj_t []) {((py_circle_obj_t *) self_in)->x,
                                              ((py_circle_obj_t *) self_in)->y,
                                              ((py_circle_obj_t *) self_in)->r});
}

mp_obj_t py_circle_x(mp_obj_t self_in) { return ((py_circle_obj_t *) self_in)->x; }
mp_obj_t py_circle_y(mp_obj_t self_in) { return ((py_circle_obj_t *) self_in)->y; }
mp_obj_t py_circle_r(mp_obj_t self_in) { return ((py_circle_obj_t *) self_in)->r; }
mp_obj_t py_circle_magnitude(mp_obj_t self_in) { return ((py_circle_obj_t *) self_in)->magnitude; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_circle_circle_obj, py_circle_circle);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_circle_x_obj, py_circle_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_circle_y_obj, py_circle_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_circle_r_obj, py_circle_r);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_circle_magnitude_obj, py_circle_magnitude);

STATIC const mp_rom_map_elem_t py_circle_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&py_circle_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_circle_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_circle_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_r), MP_ROM_PTR(&py_circle_r_obj) },
    { MP_ROM_QSTR(MP_QSTR_magnitude), MP_ROM_PTR(&py_circle_magnitude_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_circle_locals_dict, py_circle_locals_dict_table);

static const mp_obj_type_t py_circle_type = {
    { &mp_type_type },
    .name  = MP_QSTR_circle,
    .print = py_circle_print,
    .subscr = py_circle_subscr,
    .locals_dict = (mp_obj_t) &py_circle_locals_dict,
};

static mp_obj_t py_image_find_circles(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    unsigned int x_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_stride), 2);
    PY_ASSERT_TRUE_MSG(x_stride > 0, "x_stride must not be zero.");
    unsigned int y_stride = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_stride), 1);
    PY_ASSERT_TRUE_MSG(y_stride > 0, "y_stride must not be zero.");

    list_t out;
    fb_alloc_mark();
    imlib_find_circles(&out, arg_img, &roi, x_stride, y_stride, py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 2000),
                                                                py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_margin), 10),
                                                                py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_margin), 10),
                                                                py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_r_margin), 10));
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_circles_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_circle_obj_t *o = m_new_obj(py_circle_obj_t);
        o->base.type = &py_circle_type;
        o->x = mp_obj_new_int(lnk_data.p.x);
        o->y = mp_obj_new_int(lnk_data.p.y);
        o->r = mp_obj_new_int(lnk_data.r);
        o->magnitude = mp_obj_new_int(lnk_data.magnitude);

        objects_list->items[i] = o;
    }

    return objects_list;
}
#endif // OMV_ENABLE_FIND_CIRCLES

#ifdef OMV_ENABLE_FIND_RECTS
// Rect Object //
#define py_rect_obj_size 5
typedef struct py_rect_obj {
    mp_obj_base_t base;
    mp_obj_t corners;
    mp_obj_t x, y, w, h, magnitude;
} py_rect_obj_t;

static void py_rect_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_rect_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, w:%d, h:%d, magnitude:%d}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_get_int(self->magnitude));
}

static mp_obj_t py_rect_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_rect_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_rect_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_rect_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->magnitude;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_rect_corners(mp_obj_t self_in) { return ((py_rect_obj_t *) self_in)->corners; }
mp_obj_t py_rect_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_rect_obj_t *) self_in)->x,
                                              ((py_rect_obj_t *) self_in)->y,
                                              ((py_rect_obj_t *) self_in)->w,
                                              ((py_rect_obj_t *) self_in)->h});
}

mp_obj_t py_rect_x(mp_obj_t self_in) { return ((py_rect_obj_t *) self_in)->x; }
mp_obj_t py_rect_y(mp_obj_t self_in) { return ((py_rect_obj_t *) self_in)->y; }
mp_obj_t py_rect_w(mp_obj_t self_in) { return ((py_rect_obj_t *) self_in)->w; }
mp_obj_t py_rect_h(mp_obj_t self_in) { return ((py_rect_obj_t *) self_in)->h; }
mp_obj_t py_rect_magnitude(mp_obj_t self_in) { return ((py_rect_obj_t *) self_in)->magnitude; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_corners_obj, py_rect_corners);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_rect_obj, py_rect_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_x_obj, py_rect_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_y_obj, py_rect_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_w_obj, py_rect_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_h_obj, py_rect_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_rect_magnitude_obj, py_rect_magnitude);

STATIC const mp_rom_map_elem_t py_rect_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_corners), MP_ROM_PTR(&py_rect_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_rect_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_rect_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_rect_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_rect_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_rect_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_magnitude), MP_ROM_PTR(&py_rect_magnitude_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_rect_locals_dict, py_rect_locals_dict_table);

static const mp_obj_type_t py_rect_type = {
    { &mp_type_type },
    .name  = MP_QSTR_rect,
    .print = py_rect_print,
    .subscr = py_rect_subscr,
    .locals_dict = (mp_obj_t) &py_rect_locals_dict,
};

static mp_obj_t py_image_find_rects(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    list_t out;
    fb_alloc_mark();
    imlib_find_rects(&out, arg_img, &roi, py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 1000));
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_rects_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_rect_obj_t *o = m_new_obj(py_rect_obj_t);
        o->base.type = &py_rect_type;
        o->corners = mp_obj_new_tuple(4, (mp_obj_t [])
            {mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[0].x), mp_obj_new_int(lnk_data.corners[0].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[1].x), mp_obj_new_int(lnk_data.corners[1].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[2].x), mp_obj_new_int(lnk_data.corners[2].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[3].x), mp_obj_new_int(lnk_data.corners[3].y)})});
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->magnitude = mp_obj_new_int(lnk_data.magnitude);

        objects_list->items[i] = o;
    }

    return objects_list;
}
#endif

#ifdef OMV_ENABLE_QRCODES
// QRCode Object //
#define py_qrcode_obj_size 10
typedef struct py_qrcode_obj {
    mp_obj_base_t base;
    mp_obj_t corners;
    mp_obj_t x, y, w, h, payload, version, ecc_level, mask, data_type, eci;
} py_qrcode_obj_t;

static void py_qrcode_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_qrcode_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, w:%d, h:%d, payload:\"%s\", version:%d, ecc_level:%d, mask:%d, data_type:%d, eci:%d}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_str_get_str(self->payload),
              mp_obj_get_int(self->version),
              mp_obj_get_int(self->ecc_level),
              mp_obj_get_int(self->mask),
              mp_obj_get_int(self->data_type),
              mp_obj_get_int(self->eci));
}

static mp_obj_t py_qrcode_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_qrcode_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_qrcode_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_qrcode_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->payload;
            case 5: return self->version;
            case 6: return self->ecc_level;
            case 7: return self->mask;
            case 8: return self->data_type;
            case 9: return self->eci;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_qrcode_corners(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->corners; }
mp_obj_t py_qrcode_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_qrcode_obj_t *) self_in)->x,
                                              ((py_qrcode_obj_t *) self_in)->y,
                                              ((py_qrcode_obj_t *) self_in)->w,
                                              ((py_qrcode_obj_t *) self_in)->h});
}

mp_obj_t py_qrcode_x(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->x; }
mp_obj_t py_qrcode_y(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->y; }
mp_obj_t py_qrcode_w(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->w; }
mp_obj_t py_qrcode_h(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->h; }
mp_obj_t py_qrcode_payload(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->payload; }
mp_obj_t py_qrcode_version(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->version; }
mp_obj_t py_qrcode_ecc_level(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->ecc_level; }
mp_obj_t py_qrcode_mask(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->mask; }
mp_obj_t py_qrcode_data_type(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->data_type; }
mp_obj_t py_qrcode_eci(mp_obj_t self_in) { return ((py_qrcode_obj_t *) self_in)->eci; }
mp_obj_t py_qrcode_is_numeric(mp_obj_t self_in) { return mp_obj_new_bool(mp_obj_get_int(((py_qrcode_obj_t *) self_in)->data_type) == 1); }
mp_obj_t py_qrcode_is_alphanumeric(mp_obj_t self_in) { return mp_obj_new_bool(mp_obj_get_int(((py_qrcode_obj_t *) self_in)->data_type) == 2); }
mp_obj_t py_qrcode_is_binary(mp_obj_t self_in) { return mp_obj_new_bool(mp_obj_get_int(((py_qrcode_obj_t *) self_in)->data_type) == 4); }
mp_obj_t py_qrcode_is_kanji(mp_obj_t self_in) { return mp_obj_new_bool(mp_obj_get_int(((py_qrcode_obj_t *) self_in)->data_type) == 8); }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_corners_obj, py_qrcode_corners);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_rect_obj, py_qrcode_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_x_obj, py_qrcode_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_y_obj, py_qrcode_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_w_obj, py_qrcode_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_h_obj, py_qrcode_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_payload_obj, py_qrcode_payload);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_version_obj, py_qrcode_version);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_ecc_level_obj, py_qrcode_ecc_level);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_mask_obj, py_qrcode_mask);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_data_type_obj, py_qrcode_data_type);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_eci_obj, py_qrcode_eci);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_is_numeric_obj, py_qrcode_is_numeric);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_is_alphanumeric_obj, py_qrcode_is_alphanumeric);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_is_binary_obj, py_qrcode_is_binary);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_qrcode_is_kanji_obj, py_qrcode_is_kanji);

STATIC const mp_rom_map_elem_t py_qrcode_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_corners), MP_ROM_PTR(&py_qrcode_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_qrcode_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_qrcode_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_qrcode_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_qrcode_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_qrcode_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_payload), MP_ROM_PTR(&py_qrcode_payload_obj) },
    { MP_ROM_QSTR(MP_QSTR_version), MP_ROM_PTR(&py_qrcode_version_obj) },
    { MP_ROM_QSTR(MP_QSTR_ecc_level), MP_ROM_PTR(&py_qrcode_ecc_level_obj) },
    { MP_ROM_QSTR(MP_QSTR_mask), MP_ROM_PTR(&py_qrcode_mask_obj) },
    { MP_ROM_QSTR(MP_QSTR_data_type), MP_ROM_PTR(&py_qrcode_data_type_obj) },
    { MP_ROM_QSTR(MP_QSTR_eci), MP_ROM_PTR(&py_qrcode_eci_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_numeric), MP_ROM_PTR(&py_qrcode_is_numeric_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_alphanumeric), MP_ROM_PTR(&py_qrcode_is_alphanumeric_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_binary), MP_ROM_PTR(&py_qrcode_is_binary_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_kanji), MP_ROM_PTR(&py_qrcode_is_kanji_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_qrcode_locals_dict, py_qrcode_locals_dict_table);

static const mp_obj_type_t py_qrcode_type = {
    { &mp_type_type },
    .name  = MP_QSTR_qrcode,
    .print = py_qrcode_print,
    .subscr = py_qrcode_subscr,
    .locals_dict = (mp_obj_t) &py_qrcode_locals_dict,
};

static mp_obj_t py_image_find_qrcodes(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    list_t out;
    fb_alloc_mark();
    imlib_find_qrcodes(&out, arg_img, &roi);
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_qrcodes_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_qrcode_obj_t *o = m_new_obj(py_qrcode_obj_t);
        o->base.type = &py_qrcode_type;
        o->corners = mp_obj_new_tuple(4, (mp_obj_t [])
            {mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[0].x), mp_obj_new_int(lnk_data.corners[0].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[1].x), mp_obj_new_int(lnk_data.corners[1].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[2].x), mp_obj_new_int(lnk_data.corners[2].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[3].x), mp_obj_new_int(lnk_data.corners[3].y)})});
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->payload = mp_obj_new_str(lnk_data.payload, lnk_data.payload_len, false);
        o->version = mp_obj_new_int(lnk_data.version);
        o->ecc_level = mp_obj_new_int(lnk_data.ecc_level);
        o->mask = mp_obj_new_int(lnk_data.mask);
        o->data_type = mp_obj_new_int(lnk_data.data_type);
        o->eci = mp_obj_new_int(lnk_data.eci);

        objects_list->items[i] = o;
        xfree(lnk_data.payload);
    }

    return objects_list;
}
#endif

#ifdef OMV_ENABLE_APRILTAGS
// AprilTag Object //
#define py_apriltag_obj_size 18
typedef struct py_apriltag_obj {
    mp_obj_base_t base;
    mp_obj_t corners;
    mp_obj_t x, y, w, h, id, family, cx, cy, rotation, decision_margin, hamming, goodness;
    mp_obj_t x_translation, y_translation, z_translation;
    mp_obj_t x_rotation, y_rotation, z_rotation;
} py_apriltag_obj_t;

static void py_apriltag_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_apriltag_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, w:%d, h:%d, id:%d, family:%d, cx:%d, cy:%d, rotation:%f, decision_margin:%f, hamming:%d, goodness:%f,"
              " x_translation:%f, y_translation:%f, z_translation:%f,"
              " x_rotation:%f, y_rotation:%f, z_rotation:%f}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_get_int(self->id),
              mp_obj_get_int(self->family),
              mp_obj_get_int(self->cx),
              mp_obj_get_int(self->cy),
              (double) mp_obj_get_float(self->rotation),
              (double) mp_obj_get_float(self->decision_margin),
              mp_obj_get_int(self->hamming),
              (double) mp_obj_get_float(self->goodness),
              (double) mp_obj_get_float(self->x_translation),
              (double) mp_obj_get_float(self->y_translation),
              (double) mp_obj_get_float(self->z_translation),
              (double) mp_obj_get_float(self->x_rotation),
              (double) mp_obj_get_float(self->y_rotation),
              (double) mp_obj_get_float(self->z_rotation));
}

static mp_obj_t py_apriltag_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_apriltag_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_apriltag_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_apriltag_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->id;
            case 5: return self->family;
            case 6: return self->cx;
            case 7: return self->cy;
            case 8: return self->rotation;
            case 9: return self->decision_margin;
            case 10: return self->hamming;
            case 11: return self->goodness;
            case 12: return self->x_translation;
            case 13: return self->y_translation;
            case 14: return self->z_translation;
            case 15: return self->x_rotation;
            case 16: return self->y_rotation;
            case 17: return self->z_rotation;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_apriltag_corners(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->corners; }
mp_obj_t py_apriltag_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_apriltag_obj_t *) self_in)->x,
                                              ((py_apriltag_obj_t *) self_in)->y,
                                              ((py_apriltag_obj_t *) self_in)->w,
                                              ((py_apriltag_obj_t *) self_in)->h});
}

mp_obj_t py_apriltag_x(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->x; }
mp_obj_t py_apriltag_y(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->y; }
mp_obj_t py_apriltag_w(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->w; }
mp_obj_t py_apriltag_h(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->h; }
mp_obj_t py_apriltag_id(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->id; }
mp_obj_t py_apriltag_family(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->family; }
mp_obj_t py_apriltag_cx(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->cx; }
mp_obj_t py_apriltag_cy(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->cy; }
mp_obj_t py_apriltag_rotation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->rotation; }
mp_obj_t py_apriltag_decision_margin(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->decision_margin; }
mp_obj_t py_apriltag_hamming(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->hamming; }
mp_obj_t py_apriltag_goodness(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->goodness; }
mp_obj_t py_apriltag_x_translation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->x_translation; }
mp_obj_t py_apriltag_y_translation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->y_translation; }
mp_obj_t py_apriltag_z_translation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->z_translation; }
mp_obj_t py_apriltag_x_rotation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->x_rotation; }
mp_obj_t py_apriltag_y_rotation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->y_rotation; }
mp_obj_t py_apriltag_z_rotation(mp_obj_t self_in) { return ((py_apriltag_obj_t *) self_in)->z_rotation; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_corners_obj, py_apriltag_corners);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_rect_obj, py_apriltag_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_x_obj, py_apriltag_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_y_obj, py_apriltag_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_w_obj, py_apriltag_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_h_obj, py_apriltag_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_id_obj, py_apriltag_id);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_family_obj, py_apriltag_family);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_cx_obj, py_apriltag_cx);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_cy_obj, py_apriltag_cy);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_rotation_obj, py_apriltag_rotation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_decision_margin_obj, py_apriltag_decision_margin);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_hamming_obj, py_apriltag_hamming);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_goodness_obj, py_apriltag_goodness);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_x_translation_obj, py_apriltag_x_translation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_y_translation_obj, py_apriltag_y_translation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_z_translation_obj, py_apriltag_z_translation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_x_rotation_obj, py_apriltag_x_rotation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_y_rotation_obj, py_apriltag_y_rotation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_apriltag_z_rotation_obj, py_apriltag_z_rotation);

STATIC const mp_rom_map_elem_t py_apriltag_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_corners), MP_ROM_PTR(&py_apriltag_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_apriltag_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_apriltag_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_apriltag_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_apriltag_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_apriltag_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_id), MP_ROM_PTR(&py_apriltag_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_family), MP_ROM_PTR(&py_apriltag_family_obj) },
    { MP_ROM_QSTR(MP_QSTR_cx), MP_ROM_PTR(&py_apriltag_cx_obj) },
    { MP_ROM_QSTR(MP_QSTR_cy), MP_ROM_PTR(&py_apriltag_cy_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotation), MP_ROM_PTR(&py_apriltag_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_decision_margin), MP_ROM_PTR(&py_apriltag_decision_margin_obj) },
    { MP_ROM_QSTR(MP_QSTR_hamming), MP_ROM_PTR(&py_apriltag_hamming_obj) },
    { MP_ROM_QSTR(MP_QSTR_goodness), MP_ROM_PTR(&py_apriltag_goodness_obj) },
    { MP_ROM_QSTR(MP_QSTR_x_translation), MP_ROM_PTR(&py_apriltag_x_translation_obj) },
    { MP_ROM_QSTR(MP_QSTR_y_translation), MP_ROM_PTR(&py_apriltag_y_translation_obj) },
    { MP_ROM_QSTR(MP_QSTR_z_translation), MP_ROM_PTR(&py_apriltag_z_translation_obj) },
    { MP_ROM_QSTR(MP_QSTR_x_rotation), MP_ROM_PTR(&py_apriltag_x_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_y_rotation), MP_ROM_PTR(&py_apriltag_y_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_z_rotation), MP_ROM_PTR(&py_apriltag_z_rotation_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_apriltag_locals_dict, py_apriltag_locals_dict_table);

static const mp_obj_type_t py_apriltag_type = {
    { &mp_type_type },
    .name  = MP_QSTR_apriltag,
    .print = py_apriltag_print,
    .subscr = py_apriltag_subscr,
    .locals_dict = (mp_obj_t) &py_apriltag_locals_dict,
};

static mp_obj_t py_image_find_apriltags(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    apriltag_families_t families = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_families), TAG36H11);
    // 2.8mm Focal Length w/ OV7725 sensor for reference.
    float fx = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_fx), (2.8 / 3.984) * arg_img->w);
    // 2.8mm Focal Length w/ OV7725 sensor for reference.
    float fy = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_fy), (2.8 / 2.952) * arg_img->h);
    // Use the image versus the roi here since the image should be projected from the camera center.
    float cx = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_cx), arg_img->w * 0.5);
    // Use the image versus the roi here since the image should be projected from the camera center.
    float cy = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_cy), arg_img->h * 0.5);

    list_t out;
    fb_alloc_mark();
    imlib_find_apriltags(&out, arg_img, &roi, families, fx, fy, cx, cy);
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_apriltags_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_apriltag_obj_t *o = m_new_obj(py_apriltag_obj_t);
        o->base.type = &py_apriltag_type;
        o->corners = mp_obj_new_tuple(4, (mp_obj_t [])
            {mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[0].x), mp_obj_new_int(lnk_data.corners[0].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[1].x), mp_obj_new_int(lnk_data.corners[1].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[2].x), mp_obj_new_int(lnk_data.corners[2].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[3].x), mp_obj_new_int(lnk_data.corners[3].y)})});
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->id = mp_obj_new_int(lnk_data.id);
        o->family = mp_obj_new_int(lnk_data.family);
        o->cx = mp_obj_new_int(lnk_data.centroid.x);
        o->cy = mp_obj_new_int(lnk_data.centroid.y);
        o->rotation = mp_obj_new_float(lnk_data.z_rotation);
        o->decision_margin = mp_obj_new_float(lnk_data.decision_margin);
        o->hamming = mp_obj_new_int(lnk_data.hamming);
        o->goodness = mp_obj_new_float(lnk_data.goodness);
        o->x_translation = mp_obj_new_float(lnk_data.x_translation);
        o->y_translation = mp_obj_new_float(lnk_data.y_translation);
        o->z_translation = mp_obj_new_float(lnk_data.z_translation);
        o->x_rotation = mp_obj_new_float(lnk_data.x_rotation);
        o->y_rotation = mp_obj_new_float(lnk_data.y_rotation);
        o->z_rotation = mp_obj_new_float(lnk_data.z_rotation);

        objects_list->items[i] = o;
    }

    return objects_list;
}
#endif // OMV_ENABLE_APRILTAGS

#ifdef OMV_ENABLE_DATAMATRICES
// DataMatrix Object //
#define py_datamatrix_obj_size 10
typedef struct py_datamatrix_obj {
    mp_obj_base_t base;
    mp_obj_t corners;
    mp_obj_t x, y, w, h, payload, rotation, rows, columns, capacity, padding;
} py_datamatrix_obj_t;

static void py_datamatrix_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_datamatrix_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, w:%d, h:%d, payload:\"%s\", rotation:%f, rows:%d, columns:%d, capacity:%d, padding:%d}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_str_get_str(self->payload),
              (double) mp_obj_get_float(self->rotation),
              mp_obj_get_int(self->rows),
              mp_obj_get_int(self->columns),
              mp_obj_get_int(self->capacity),
              mp_obj_get_int(self->padding));
}

static mp_obj_t py_datamatrix_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_datamatrix_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_datamatrix_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_datamatrix_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->payload;
            case 5: return self->rotation;
            case 6: return self->rows;
            case 7: return self->columns;
            case 8: return self->capacity;
            case 9: return self->padding;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_datamatrix_corners(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->corners; }
mp_obj_t py_datamatrix_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_datamatrix_obj_t *) self_in)->x,
                                              ((py_datamatrix_obj_t *) self_in)->y,
                                              ((py_datamatrix_obj_t *) self_in)->w,
                                              ((py_datamatrix_obj_t *) self_in)->h});
}

mp_obj_t py_datamatrix_x(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->x; }
mp_obj_t py_datamatrix_y(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->y; }
mp_obj_t py_datamatrix_w(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->w; }
mp_obj_t py_datamatrix_h(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->h; }
mp_obj_t py_datamatrix_payload(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->payload; }
mp_obj_t py_datamatrix_rotation(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->rotation; }
mp_obj_t py_datamatrix_rows(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->rows; }
mp_obj_t py_datamatrix_columns(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->columns; }
mp_obj_t py_datamatrix_capacity(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->capacity; }
mp_obj_t py_datamatrix_padding(mp_obj_t self_in) { return ((py_datamatrix_obj_t *) self_in)->padding; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_corners_obj, py_datamatrix_corners);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_rect_obj, py_datamatrix_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_x_obj, py_datamatrix_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_y_obj, py_datamatrix_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_w_obj, py_datamatrix_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_h_obj, py_datamatrix_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_payload_obj, py_datamatrix_payload);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_rotation_obj, py_datamatrix_rotation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_rows_obj, py_datamatrix_rows);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_columns_obj, py_datamatrix_columns);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_capacity_obj, py_datamatrix_capacity);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_datamatrix_padding_obj, py_datamatrix_padding);

STATIC const mp_rom_map_elem_t py_datamatrix_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_corners), MP_ROM_PTR(&py_datamatrix_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_datamatrix_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_datamatrix_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_datamatrix_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_datamatrix_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_datamatrix_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_payload), MP_ROM_PTR(&py_datamatrix_payload_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotation), MP_ROM_PTR(&py_datamatrix_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_rows), MP_ROM_PTR(&py_datamatrix_rows_obj) },
    { MP_ROM_QSTR(MP_QSTR_columns), MP_ROM_PTR(&py_datamatrix_columns_obj) },
    { MP_ROM_QSTR(MP_QSTR_capacity), MP_ROM_PTR(&py_datamatrix_capacity_obj) },
    { MP_ROM_QSTR(MP_QSTR_padding), MP_ROM_PTR(&py_datamatrix_padding_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_datamatrix_locals_dict, py_datamatrix_locals_dict_table);

static const mp_obj_type_t py_datamatrix_type = {
    { &mp_type_type },
    .name  = MP_QSTR_datamatrix,
    .print = py_datamatrix_print,
    .subscr = py_datamatrix_subscr,
    .locals_dict = (mp_obj_t) &py_datamatrix_locals_dict,
};

static mp_obj_t py_image_find_datamatrices(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    list_t out;
    fb_alloc_mark();
    imlib_find_datamatrices(&out, arg_img, &roi, py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_effort), 200));
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_datamatrices_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_datamatrix_obj_t *o = m_new_obj(py_datamatrix_obj_t);
        o->base.type = &py_datamatrix_type;
        o->corners = mp_obj_new_tuple(4, (mp_obj_t [])
            {mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[0].x), mp_obj_new_int(lnk_data.corners[0].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[1].x), mp_obj_new_int(lnk_data.corners[1].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[2].x), mp_obj_new_int(lnk_data.corners[2].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[3].x), mp_obj_new_int(lnk_data.corners[3].y)})});
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->payload = mp_obj_new_str(lnk_data.payload, lnk_data.payload_len, false);
        o->rotation = mp_obj_new_float((lnk_data.rotation * PI) / 180);
        o->rows = mp_obj_new_int(lnk_data.rows);
        o->columns = mp_obj_new_int(lnk_data.columns);
        o->capacity = mp_obj_new_int(lnk_data.capacity);
        o->padding = mp_obj_new_int(lnk_data.padding);

        objects_list->items[i] = o;
        xfree(lnk_data.payload);
    }

    return objects_list;
}
#endif // OMV_ENABLE_DATAMATRICES

#ifdef OMV_ENABLE_BARCODES
// BarCode Object //
#define py_barcode_obj_size 8
typedef struct py_barcode_obj {
    mp_obj_base_t base;
    mp_obj_t corners;
    mp_obj_t x, y, w, h, payload, type, rotation, quality;
} py_barcode_obj_t;

static void py_barcode_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_barcode_obj_t *self = self_in;
    mp_printf(print,
              "{x:%d, y:%d, w:%d, h:%d, payload:\"%s\", type:%d, rotation:%f, quality:%d}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_str_get_str(self->payload),
              mp_obj_get_int(self->type),
              (double) mp_obj_get_float(self->rotation),
              mp_obj_get_int(self->quality));
}

static mp_obj_t py_barcode_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_barcode_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_barcode_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_barcode_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->payload;
            case 5: return self->type;
            case 6: return self->rotation;
            case 7: return self->quality;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_barcode_corners(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->corners; }
mp_obj_t py_barcode_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_barcode_obj_t *) self_in)->x,
                                              ((py_barcode_obj_t *) self_in)->y,
                                              ((py_barcode_obj_t *) self_in)->w,
                                              ((py_barcode_obj_t *) self_in)->h});
}

mp_obj_t py_barcode_x(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->x; }
mp_obj_t py_barcode_y(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->y; }
mp_obj_t py_barcode_w(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->w; }
mp_obj_t py_barcode_h(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->h; }
mp_obj_t py_barcode_payload_fun(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->payload; }
mp_obj_t py_barcode_type_fun(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->type; }
mp_obj_t py_barcode_rotation_fun(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->rotation; }
mp_obj_t py_barcode_quality_fun(mp_obj_t self_in) { return ((py_barcode_obj_t *) self_in)->quality; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_corners_obj, py_barcode_corners);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_rect_obj, py_barcode_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_x_obj, py_barcode_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_y_obj, py_barcode_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_w_obj, py_barcode_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_h_obj, py_barcode_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_payload_fun_obj, py_barcode_payload_fun);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_type_fun_obj, py_barcode_type_fun);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_rotation_fun_obj, py_barcode_rotation_fun);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_barcode_quality_fun_obj, py_barcode_quality_fun);

STATIC const mp_rom_map_elem_t py_barcode_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_corners), MP_ROM_PTR(&py_barcode_corners_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_barcode_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_barcode_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_barcode_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_barcode_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_barcode_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_payload), MP_ROM_PTR(&py_barcode_payload_fun_obj) },
    { MP_ROM_QSTR(MP_QSTR_type), MP_ROM_PTR(&py_barcode_type_fun_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotation), MP_ROM_PTR(&py_barcode_rotation_fun_obj) },
    { MP_ROM_QSTR(MP_QSTR_quality), MP_ROM_PTR(&py_barcode_quality_fun_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_barcode_locals_dict, py_barcode_locals_dict_table);

static const mp_obj_type_t py_barcode_type = {
    { &mp_type_type },
    .name  = MP_QSTR_barcode,
    .print = py_barcode_print,
    .subscr = py_barcode_subscr,
    .locals_dict = (mp_obj_t) &py_barcode_locals_dict,
};

static mp_obj_t py_image_find_barcodes(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    list_t out;
    fb_alloc_mark();
    imlib_find_barcodes(&out, arg_img, &roi);
    fb_alloc_free_till_mark();

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
    for (size_t i = 0; list_size(&out); i++) {
        find_barcodes_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_barcode_obj_t *o = m_new_obj(py_barcode_obj_t);
        o->base.type = &py_barcode_type;
        o->corners = mp_obj_new_tuple(4, (mp_obj_t [])
            {mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[0].x), mp_obj_new_int(lnk_data.corners[0].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[1].x), mp_obj_new_int(lnk_data.corners[1].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[2].x), mp_obj_new_int(lnk_data.corners[2].y)}),
             mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_int(lnk_data.corners[3].x), mp_obj_new_int(lnk_data.corners[3].y)})});
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->payload = mp_obj_new_str(lnk_data.payload, lnk_data.payload_len, false);
        o->type = mp_obj_new_int(lnk_data.type);
        o->rotation = mp_obj_new_float((lnk_data.rotation * PI) / 180);
        o->quality = mp_obj_new_int(lnk_data.quality);

        objects_list->items[i] = o;
        xfree(lnk_data.payload);
    }

    return objects_list;
}
#endif // OMV_ENABLE_BARCODES

#ifdef OMV_ENABLE_PHASE_CORRELATION
// Displacement Object //
#define py_displacement_obj_size 5
typedef struct py_displacement_obj {
    mp_obj_base_t base;
    mp_obj_t x_translation, y_translation, rotation, scale, response;
} py_displacement_obj_t;

static void py_displacement_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_displacement_obj_t *self = self_in;
    mp_printf(print,
              "{x_translation:%f, y_translation:%f, rotation:%f, scale:%f, response:%f}",
              (double) mp_obj_get_float(self->x_translation),
              (double) mp_obj_get_float(self->y_translation),
              (double) mp_obj_get_float(self->rotation),
              (double) mp_obj_get_float(self->scale),
              (double) mp_obj_get_float(self->response));
}

static mp_obj_t py_displacement_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_displacement_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_displacement_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x_translation) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_displacement_obj_size, index, false)) {
            case 0: return self->x_translation;
            case 1: return self->y_translation;
            case 2: return self->rotation;
            case 3: return self->scale;
            case 4: return self->response;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_displacement_x_translation(mp_obj_t self_in) { return ((py_displacement_obj_t *) self_in)->x_translation; }
mp_obj_t py_displacement_y_translation(mp_obj_t self_in) { return ((py_displacement_obj_t *) self_in)->y_translation; }
mp_obj_t py_displacement_rotation(mp_obj_t self_in) { return ((py_displacement_obj_t *) self_in)->rotation; }
mp_obj_t py_displacement_scale(mp_obj_t self_in) { return ((py_displacement_obj_t *) self_in)->scale; }
mp_obj_t py_displacement_response(mp_obj_t self_in) { return ((py_displacement_obj_t *) self_in)->response; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_displacement_x_translation_obj, py_displacement_x_translation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_displacement_y_translation_obj, py_displacement_y_translation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_displacement_rotation_obj, py_displacement_rotation);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_displacement_scale_obj, py_displacement_scale);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_displacement_response_obj, py_displacement_response);

STATIC const mp_rom_map_elem_t py_displacement_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_x_translation), MP_ROM_PTR(&py_displacement_x_translation_obj) },
    { MP_ROM_QSTR(MP_QSTR_y_translation), MP_ROM_PTR(&py_displacement_y_translation_obj) },
    { MP_ROM_QSTR(MP_QSTR_rotation), MP_ROM_PTR(&py_displacement_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_scale), MP_ROM_PTR(&py_displacement_scale_obj) },
    { MP_ROM_QSTR(MP_QSTR_response), MP_ROM_PTR(&py_displacement_response_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_displacement_locals_dict, py_displacement_locals_dict_table);

static const mp_obj_type_t py_displacement_type = {
    { &mp_type_type },
    .name  = MP_QSTR_displacement,
    .print = py_displacement_print,
    .subscr = py_displacement_subscr,
    .locals_dict = (mp_obj_t) &py_displacement_locals_dict,
};

static mp_obj_t py_image_find_displacement(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    image_t *arg_template_img = py_image_cobj(args[1]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_template_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    rectangle_t template_roi;
    py_helper_lookup_rectangle_2(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_template_roi), arg_template_img, &template_roi);

    PY_ASSERT_FALSE_MSG((roi.w != template_roi.w) || (roi.h != template_roi.h), "ROI(w,h) != TEMPLATE_ROI(w,h)");

    bool logpolar = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_logpolar), false);
    bool fix_rotation_scale = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_fix_rotation_scale), false);

    float x, y, r, s, response;
    fb_alloc_mark();
    imlib_phasecorrelate(arg_img, arg_template_img, &roi, &template_roi, logpolar, fix_rotation_scale, &x, &y, &r, &s, &response);
    fb_alloc_free_till_mark();

    py_displacement_obj_t *o = m_new_obj(py_displacement_obj_t);
    o->base.type = &py_displacement_type;
    o->x_translation = mp_obj_new_float(x);
    o->y_translation = mp_obj_new_float(y);
    o->rotation = mp_obj_new_float(r);
    o->scale = mp_obj_new_float(s);
    o->response = mp_obj_new_float(response);

    return o;
}
#endif // OMV_ENABLE_PHASE_CORRELATION

#ifdef OMV_ENABLE_LENET
static mp_obj_t py_image_find_number(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    // Make sure ROI is bigger than or equal to template size
    PY_ASSERT_TRUE_MSG((roi.w == LENET_INPUT_W && roi.h == LENET_INPUT_H),
            "Region of interest must be 28x28!");

    // Make sure ROI is smaller than or equal to image size
    PY_ASSERT_TRUE_MSG(((roi.x + roi.w) <= arg_img->w && (roi.y + roi.h) <= arg_img->h),
            "Region of interest is bigger than image!");

    int r = 0;
    float c = 0.0f;
    lenet5_t *lenet = (lenet5_t*) lenet_model_num;

    r = lenet_predict(lenet, arg_img, &roi, &c);
    mp_obj_t ret_obj[2] = {
        mp_obj_new_int(r),
        mp_obj_new_float(c),
    };

    return mp_obj_new_tuple(2, ret_obj);
}
#endif //OMV_ENABLE_LENET

static mp_obj_t py_image_midpoint_pool(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int x_div = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(x_div >= 1, "Width divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(x_div <= arg_img->w, "Width divisor must be less than <= img width");
    int y_div = mp_obj_get_int(args[2]);
    PY_ASSERT_TRUE_MSG(y_div >= 1, "Height divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(y_div <= arg_img->h, "Height divisor must be less than <= img height");

    image_t out_img;
    out_img.w = arg_img->w / x_div;
    out_img.h = arg_img->h / y_div;
    out_img.bpp = arg_img->bpp;
    out_img.pixels = arg_img->pixels;

    int bias = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bias), 0.5) * 256;
    imlib_midpoint_pool(arg_img, &out_img, x_div, y_div, IM_MIN(IM_MAX(bias, 0), 256));
    arg_img->w = out_img.w;
    arg_img->h = out_img.h;
    return mp_const_none;
}

static mp_obj_t py_image_midpoint_pooled(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int x_div = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(x_div >= 1, "Width divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(x_div <= arg_img->w, "Width divisor must be less than <= img width");
    int y_div = mp_obj_get_int(args[2]);
    PY_ASSERT_TRUE_MSG(y_div >= 1, "Height divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(y_div <= arg_img->h, "Height divisor must be less than <= img height");

    image_t out_img;
    out_img.w = arg_img->w / x_div;
    out_img.h = arg_img->h / y_div;
    out_img.bpp = arg_img->bpp;
    out_img.pixels = xalloc(out_img.w * out_img.h * out_img.bpp);

    int bias = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_bias), 0.5) * 256;
    imlib_midpoint_pool(arg_img, &out_img, x_div, y_div, IM_MIN(IM_MAX(bias, 0), 256));
    return py_image_from_struct(&out_img);
}

static mp_obj_t py_image_mean_pool(mp_obj_t img_obj, mp_obj_t x_div_obj, mp_obj_t y_div_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int x_div = mp_obj_get_int(x_div_obj);
    PY_ASSERT_TRUE_MSG(x_div >= 1, "Width divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(x_div <= arg_img->w, "Width divisor must be less than <= img width");
    int y_div = mp_obj_get_int(y_div_obj);
    PY_ASSERT_TRUE_MSG(y_div >= 1, "Height divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(y_div <= arg_img->h, "Height divisor must be less than <= img height");

    image_t out_img;
    out_img.w = arg_img->w / x_div;
    out_img.h = arg_img->h / y_div;
    out_img.bpp = arg_img->bpp;
    out_img.pixels = arg_img->pixels;

    imlib_mean_pool(arg_img, &out_img, x_div, y_div);
    arg_img->w = out_img.w;
    arg_img->h = out_img.h;
    // Check if this image is the one in the frame buffer...
    if ((MAIN_FB()->pixels == arg_img->pixels) || (MAIN_FB()->pixels == arg_img->pixels)) {
        MAIN_FB()->w = out_img.w;
        MAIN_FB()->h = out_img.h;
    }

    return img_obj;
}

static mp_obj_t py_image_mean_pooled(mp_obj_t img_obj, mp_obj_t x_div_obj, mp_obj_t y_div_obj)
{
    image_t *arg_img = py_image_cobj(img_obj);
    PY_ASSERT_TRUE_MSG(IM_IS_MUTABLE(arg_img), "Image format is not supported.");

    int x_div = mp_obj_get_int(x_div_obj);
    PY_ASSERT_TRUE_MSG(x_div >= 1, "Width divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(x_div <= arg_img->w, "Width divisor must be less than <= img width");
    int y_div = mp_obj_get_int(y_div_obj);
    PY_ASSERT_TRUE_MSG(y_div >= 1, "Height divisor must be greater than >= 1");
    PY_ASSERT_TRUE_MSG(y_div <= arg_img->h, "Height divisor must be less than <= img height");

    image_t out_img;
    out_img.w = arg_img->w / x_div;
    out_img.h = arg_img->h / y_div;
    out_img.bpp = arg_img->bpp;
    out_img.pixels = xalloc(out_img.w * out_img.h * out_img.bpp);

    imlib_mean_pool(arg_img, &out_img, x_div, y_div);
    return py_image_from_struct(&out_img);
}

static mp_obj_t py_image_find_template(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    image_t *arg_template = py_image_cobj(args[1]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_template),
            "This function is only supported on GRAYSCALE images");

    float arg_thresh = mp_obj_get_float(args[2]);

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);

    // Make sure ROI is bigger than or equal to template size
    PY_ASSERT_TRUE_MSG((roi.w >= arg_template->w && roi.h >= arg_template->h),
            "Region of interest is smaller than template!");

    // Make sure ROI is smaller than or equal to image size
    PY_ASSERT_TRUE_MSG(((roi.x + roi.w) <= arg_img->w && (roi.y + roi.h) <= arg_img->h),
            "Region of interest is bigger than image!");

    int step = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_step), 2);
    int search = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_search), SEARCH_EX);

    // Find template
    rectangle_t r;
    float corr;
    if (search == SEARCH_DS) {
        corr = imlib_template_match_ds(arg_img, arg_template, &r);
    } else {
        corr = imlib_template_match_ex(arg_img, arg_template, &roi, step, &r);
    }

    if (corr > arg_thresh) {
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

static mp_obj_t py_image_find_features(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img),
            "This function is only supported on GRAYSCALE images");

    cascade_t *cascade = py_cascade_cobj(args[1]);
    cascade->threshold = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 0.5f);
    cascade->scale_factor = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale_factor), 1.5f);

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
    lbp_obj->hist = imlib_lbp_desc(arg_img, &roi);
    return lbp_obj;
}

static mp_obj_t py_image_find_keypoints(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img), "This function is only supported on GRAYSCALE images");

    // Lookup args
    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, arg_img, &roi);
    int threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 20);
    bool normalized = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_normalized), false);
    float scale_factor = py_helper_lookup_float(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale_factor), 1.5f);
    int max_keypoints = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_max_keypoints), 100);
    corner_detector_t corner_detector =  py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_corner_detector), CORNER_AGAST);

    // Find keypoints
    array_t *kpts = orb_find_keypoints(arg_img, normalized, threshold, scale_factor, max_keypoints, corner_detector, &roi);

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

static mp_obj_t py_image_find_edges(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *img = py_image_cobj(args[0]);
    edge_detector_t edge_type = mp_obj_get_int(args[1]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(img), "This function is only supported on GRAYSCALE images");

    rectangle_t roi;
    py_helper_lookup_rectangle(kw_args, img, &roi);

    int thresh[2] = {100, 200};
    py_helper_lookup_int_array(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), thresh, 2);

    switch (edge_type) {
        case EDGE_SIMPLE: {
            imlib_edge_simple(img, &roi, thresh[0], thresh[1]);
            break;
        }
        case EDGE_CANNY: {
            imlib_edge_canny(img, &roi, thresh[0], thresh[1]);
            break;
        }

    }

    return mp_const_true;
}

static mp_obj_t py_image_find_hog(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    image_t *arg_img = py_image_cobj(args[0]);
    PY_ASSERT_TRUE_MSG(IM_IS_GS(arg_img), "This function is only supported on GRAYSCALE images");

    rectangle_t arg_r;
    py_helper_lookup_rectangle(kw_args, arg_img, &arg_r);

    rectangle_t rect;
    if (!rectangle_subimg(arg_img, &arg_r, &rect)) {
        return mp_const_none;
    }

    int size = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_size), 8);
    imlib_find_hog(arg_img, &rect, size);

    return mp_const_none;
}

/* Image file functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_copy_obj, 1, py_image_copy);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_copy_to_fb_obj, 1, py_image_copy_to_fb);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_obj, 2, py_image_save);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_compress_obj, 1, py_image_compress);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_compress_for_ide_obj, 1, py_image_compress_for_ide);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_compressed_obj, 1, py_image_compressed);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_compressed_for_ide_obj, 1, py_image_compressed_for_ide);
/* Basic image functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_width_obj, py_image_width);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_height_obj, py_image_height);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_format_obj, py_image_format);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_size_obj, py_image_size);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_clear_obj, py_image_clear);
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
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_b_and_obj, py_image_b_and);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_b_nand_obj, py_image_b_nand);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_b_or_obj, py_image_b_or);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_b_nor_obj, py_image_b_nor);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_b_xor_obj, py_image_b_xor);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_b_xnor_obj, py_image_b_xnor);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_erode_obj, 2, py_image_erode);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_dilate_obj, 2, py_image_dilate);
/* Background Subtraction (Frame Differencing) functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_negate_obj, py_image_negate);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_difference_obj, py_image_difference);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_replace_obj, py_image_replace);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_blend_obj, 2, py_image_blend);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_max_obj, py_image_max);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_min_obj, py_image_min);
/* Image Morphing */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_morph_obj, 3, py_image_morph);
/* Image Filtering */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_midpoint_obj, 2, py_image_midpoint);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_mean_obj, 2, py_image_mean);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_mode_obj, 2, py_image_mode);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_median_obj, 2, py_image_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_gaussian_obj, 1, py_image_gaussian);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_linpolar_obj, 1, py_image_linpolar);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_logpolar_obj, 1, py_image_logpolar);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_chrominvar_obj, py_image_chrominvar);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_illuminvar_obj, py_image_illuminvar);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_histeq_obj, py_image_histeq);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_lens_corr_obj, 1, py_image_lens_corr);
#ifdef OMV_ENABLE_ROTATION_CORR
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_rotation_corr_obj, 1, py_image_rotation_corr);
#endif
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_mask_ellipse_obj, py_image_mask_ellipse);
/* Image Statistics */
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_get_similarity_obj, py_image_get_similarity);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_get_histogram_obj, 1, py_image_get_histogram);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_get_statistics_obj, 1, py_image_get_statistics);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_get_regression_obj, 2, py_image_get_regression);
/* Color Tracking */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_blobs_obj, 2, py_image_find_blobs);
/* Shape Detection */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_lines_obj, 1, py_image_find_lines);
#ifdef OMV_ENABLE_FIND_LINE_SEGMENTS
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_line_segments_obj, 1, py_image_find_line_segments);
#endif
#ifdef OMV_ENABLE_FIND_CIRCLES
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_circles_obj, 1, py_image_find_circles);
#endif
#ifdef OMV_ENABLE_FIND_RECTS
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_rects_obj, 1, py_image_find_rects);
#endif
/* Code Detection */
#ifdef OMV_ENABLE_QRCODES
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_qrcodes_obj, 1, py_image_find_qrcodes);
#endif
#ifdef OMV_ENABLE_APRILTAGS
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_apriltags_obj, 1, py_image_find_apriltags);
#endif
#ifdef OMV_ENABLE_DATAMATRICES
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_datamatrices_obj, 1, py_image_find_datamatrices);
#endif
#ifdef OMV_ENABLE_BARCODES
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_barcodes_obj, 1, py_image_find_barcodes);
#endif
/* Template Matching */
#ifdef OMV_ENABLE_PHASE_CORRELATION
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_displacement_obj, 2, py_image_find_displacement);
#endif
#ifdef OMV_ENABLE_LENET
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_number_obj, 1, py_image_find_number);
#endif
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_midpoint_pool_obj, 3, py_image_midpoint_pool);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_midpoint_pooled_obj, 3, py_image_midpoint_pooled);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_mean_pool_obj, py_image_mean_pool);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_mean_pooled_obj, py_image_mean_pooled);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_template_obj, 3, py_image_find_template);
/* Feature Detection */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_features_obj, 2, py_image_find_features);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_eye_obj, py_image_find_eye);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_lbp_obj, py_image_find_lbp);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_keypoints_obj, 1, py_image_find_keypoints);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_edges_obj, 2, py_image_find_edges);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_hog_obj, 1, py_image_find_hog);
static const mp_map_elem_t locals_dict_table[] = {
    /* Image file functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_copy),                (mp_obj_t)&py_image_copy_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_copy_to_fb),          (mp_obj_t)&py_image_copy_to_fb_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_save),                (mp_obj_t)&py_image_save_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compress),            (mp_obj_t)&py_image_compress_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compress_for_ide),    (mp_obj_t)&py_image_compress_for_ide_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compressed),          (mp_obj_t)&py_image_compressed_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compressed_for_ide),  (mp_obj_t)&py_image_compressed_for_ide_obj},
    /* Basic image functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_width),               (mp_obj_t)&py_image_width_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_height),              (mp_obj_t)&py_image_height_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_format),              (mp_obj_t)&py_image_format_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_size),                (mp_obj_t)&py_image_size_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_clear),               (mp_obj_t)&py_image_clear_obj},
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
    {MP_OBJ_NEW_QSTR(MP_QSTR_and),                 (mp_obj_t)&py_image_b_and_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_b_and),               (mp_obj_t)&py_image_b_and_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_nand),                (mp_obj_t)&py_image_b_nand_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_b_nand),              (mp_obj_t)&py_image_b_nand_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_or),                  (mp_obj_t)&py_image_b_or_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_b_or),                (mp_obj_t)&py_image_b_or_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_nor),                 (mp_obj_t)&py_image_b_nor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_b_nor),               (mp_obj_t)&py_image_b_nor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_xor),                 (mp_obj_t)&py_image_b_xor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_b_xor),               (mp_obj_t)&py_image_b_xor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_xnor),                (mp_obj_t)&py_image_b_xnor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_b_xnor),              (mp_obj_t)&py_image_b_xnor_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_erode),               (mp_obj_t)&py_image_erode_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_dilate),              (mp_obj_t)&py_image_dilate_obj},
    /* Background Subtraction (Frame Differencing) functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_negate),              (mp_obj_t)&py_image_negate_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_difference),          (mp_obj_t)&py_image_difference_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_replace),             (mp_obj_t)&py_image_replace_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blend),               (mp_obj_t)&py_image_blend_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_max),                 (mp_obj_t)&py_image_max_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_min),                 (mp_obj_t)&py_image_min_obj},
    /* Image Morphing */
    {MP_OBJ_NEW_QSTR(MP_QSTR_morph),               (mp_obj_t)&py_image_morph_obj},
    /* Image Filtering */
    {MP_OBJ_NEW_QSTR(MP_QSTR_midpoint),            (mp_obj_t)&py_image_midpoint_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_mean),                (mp_obj_t)&py_image_mean_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_mode),                (mp_obj_t)&py_image_mode_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_median),              (mp_obj_t)&py_image_median_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_gaussian),            (mp_obj_t)&py_image_gaussian_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_linpolar),            (mp_obj_t)&py_image_linpolar_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_logpolar),            (mp_obj_t)&py_image_logpolar_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_chrominvar),          (mp_obj_t)&py_image_chrominvar_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_illuminvar),          (mp_obj_t)&py_image_illuminvar_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_histeq),              (mp_obj_t)&py_image_histeq_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_lens_corr),           (mp_obj_t)&py_image_lens_corr_obj},
#ifdef OMV_ENABLE_ROTATION_CORR
    {MP_OBJ_NEW_QSTR(MP_QSTR_rotation_corr),       (mp_obj_t)&py_image_rotation_corr_obj},
#endif
    {MP_OBJ_NEW_QSTR(MP_QSTR_mask_ellipse),        (mp_obj_t)&py_image_mask_ellipse_obj},
    /* Image Statistics */
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_similarity),      (mp_obj_t)&py_image_get_similarity_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_hist),            (mp_obj_t)&py_image_get_histogram_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_histogram),       (mp_obj_t)&py_image_get_histogram_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_histogram),           (mp_obj_t)&py_image_get_histogram_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_stats),           (mp_obj_t)&py_image_get_statistics_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_statistics),      (mp_obj_t)&py_image_get_statistics_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_statistics),          (mp_obj_t)&py_image_get_statistics_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_get_regression),      (mp_obj_t)&py_image_get_regression_obj},
    /* Color Tracking */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_blobs),          (mp_obj_t)&py_image_find_blobs_obj},
    /* Shape Detection */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_lines),          (mp_obj_t)&py_image_find_lines_obj},
#ifdef OMV_ENABLE_FIND_LINE_SEGMENTS
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_line_segments),  (mp_obj_t)&py_image_find_line_segments_obj},
#endif
#ifdef OMV_ENABLE_FIND_CIRCLES
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_circles),        (mp_obj_t)&py_image_find_circles_obj},
#endif
#ifdef OMV_ENABLE_FIND_RECTS
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_rects),          (mp_obj_t)&py_image_find_rects_obj},
#endif
    /* Code Detection */
#ifdef OMV_ENABLE_QRCODES
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_qrcodes),        (mp_obj_t)&py_image_find_qrcodes_obj},
#endif
#ifdef OMV_ENABLE_APRILTAGS
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_apriltags),      (mp_obj_t)&py_image_find_apriltags_obj},
#endif
#ifdef OMV_ENABLE_DATAMATRICES
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_datamatrices),   (mp_obj_t)&py_image_find_datamatrices_obj},
#endif
#ifdef OMV_ENABLE_BARCODES
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_barcodes),       (mp_obj_t)&py_image_find_barcodes_obj},
#endif
    /* Template Matching */
#ifdef OMV_ENABLE_PHASE_CORRELATION
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_displacement),   (mp_obj_t)&py_image_find_displacement_obj},
#endif
#ifdef OMV_ENABLE_LENET
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_number),         (mp_obj_t)&py_image_find_number_obj},
#endif
    {MP_OBJ_NEW_QSTR(MP_QSTR_midpoint_pool),       (mp_obj_t)&py_image_midpoint_pool_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_midpoint_pooled),     (mp_obj_t)&py_image_midpoint_pooled_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_mean_pool),           (mp_obj_t)&py_image_mean_pool_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_mean_pooled),         (mp_obj_t)&py_image_mean_pooled_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_template),       (mp_obj_t)&py_image_find_template_obj},
    /* Feature Detection */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_features),       (mp_obj_t)&py_image_find_features_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_eye),            (mp_obj_t)&py_image_find_eye_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_lbp),            (mp_obj_t)&py_image_find_lbp_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_keypoints),      (mp_obj_t)&py_image_find_keypoints_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_edges),          (mp_obj_t)&py_image_find_edges_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_hog),            (mp_obj_t)&py_image_find_hog_obj},
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

// ImageWriter Object //
typedef struct py_imagewriter_obj {
    mp_obj_base_t base;
    FIL fp;
    uint32_t ms;
} py_imagewriter_obj_t;

static void py_imagewriter_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_imagewriter_obj_t *self = self_in;
    mp_printf(print, "{size:%d}", f_size(&self->fp));
}

mp_obj_t py_imagewriter_size(mp_obj_t self_in)
{
    return mp_obj_new_int(f_size(&((py_imagewriter_obj_t *) self_in)->fp));
}

mp_obj_t py_imagewriter_add_frame(mp_obj_t self_in, mp_obj_t img_obj)
{
    // Don't use the file buffer here...

    FIL *fp = &((py_imagewriter_obj_t *) self_in)->fp;
    PY_ASSERT_TYPE(img_obj, &py_image_type);
    image_t *arg_img = &((py_image_obj_t *) img_obj)->_cobj;

    uint32_t ms = systick_current_millis(); // Write out elapsed ms.
    write_long(fp, ms - ((py_imagewriter_obj_t *) self_in)->ms);
    ((py_imagewriter_obj_t *) self_in)->ms = ms;

    write_long(fp, arg_img->w);
    write_long(fp, arg_img->h);
    write_long(fp, arg_img->bpp);

    uint32_t size = image_size(arg_img);

    write_data(fp, arg_img->data, size);
    if (size % 16) write_data(fp, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16 - (size % 16)); // Pad to multiple of 16 bytes.
    return mp_const_none;
}

mp_obj_t py_imagewriter_close(mp_obj_t self_in)
{
    file_close(&((py_imagewriter_obj_t *) self_in)->fp);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_imagewriter_size_obj, py_imagewriter_size);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_imagewriter_add_frame_obj, py_imagewriter_add_frame);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_imagewriter_close_obj, py_imagewriter_close);

STATIC const mp_rom_map_elem_t py_imagewriter_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_size), MP_ROM_PTR(&py_imagewriter_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_add_frame), MP_ROM_PTR(&py_imagewriter_add_frame_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&py_imagewriter_close_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_imagewriter_locals_dict, py_imagewriter_locals_dict_table);

static const mp_obj_type_t py_imagewriter_type = {
    { &mp_type_type },
    .name  = MP_QSTR_imagewriter,
    .print = py_imagewriter_print,
    .locals_dict = (mp_obj_t) &py_imagewriter_locals_dict,
};

mp_obj_t py_image_imagewriter(mp_obj_t path)
{
    py_imagewriter_obj_t *obj = m_new_obj(py_imagewriter_obj_t);
    obj->base.type = &py_imagewriter_type;
    file_write_open(&obj->fp, mp_obj_str_get_str(path));

    write_long(&obj->fp, *((uint32_t *) "OMV ")); // OpenMV
    write_long(&obj->fp, *((uint32_t *) "IMG ")); // Image
    write_long(&obj->fp, *((uint32_t *) "STR ")); // Stream
    write_long(&obj->fp, *((uint32_t *) "V1.0")); // v1.0

    obj->ms = systick_current_millis();
    return obj;
}

// ImageReader Object //
typedef struct py_imagereader_obj {
    mp_obj_base_t base;
    FIL fp;
    uint32_t ms;
} py_imagereader_obj_t;

static void py_imagereader_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_imagereader_obj_t *self = self_in;
    mp_printf(print, "{size:%d}", f_size(&self->fp));
}

mp_obj_t py_imagereader_size(mp_obj_t self_in)
{
    return mp_obj_new_int(f_size(&((py_imagereader_obj_t *) self_in)->fp));
}

mp_obj_t py_imagereader_next_frame(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    // Don't use the file buffer here...

    bool copy_to_fb = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_copy_to_fb), true);
    if (copy_to_fb) fb_update_jpeg_buffer();

    FIL *fp = &((py_imagereader_obj_t *) args[0])->fp;
    image_t image = {0};

    if (f_eof(fp)) {
        if (!py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_loop), true)) {
            return mp_const_none;
        }

        file_seek(fp, 16); // skip past the header

        if (f_eof(fp)) { // empty file
            return mp_const_none;
        }
    }

    uint32_t ms_tmp;
    read_long(fp, &ms_tmp);

    uint32_t ms; // Wait for elapsed ms.
    for (ms = systick_current_millis();
         ((ms - ((py_imagewriter_obj_t *) args[0])->ms) < ms_tmp);
         ms = systick_current_millis()) {
        __WFI();
    }

    ((py_imagewriter_obj_t *) args[0])->ms = ms;

    read_long(fp, (uint32_t *) &image.w);
    read_long(fp, (uint32_t *) &image.h);
    read_long(fp, (uint32_t *) &image.bpp);

    uint32_t size = image_size(&image);

    if (copy_to_fb) {
        PY_ASSERT_TRUE_MSG((size <= OMV_RAW_BUF_SIZE), "FB Overflow!");
        image.data = MAIN_FB()->pixels;
        MAIN_FB()->w = image.w;
        MAIN_FB()->h = image.h;
        MAIN_FB()->bpp = image.bpp;
    } else {
        image.data = xalloc(size);
    }

    char ignore[15];
    read_data(fp, image.data, size);
    if (size % 16) read_data(fp, ignore, 16 - (size % 16)); // Read in to multiple of 16 bytes.

    return py_image_from_struct(&image);
}

mp_obj_t py_imagereader_close(mp_obj_t self_in)
{
    file_close(&((py_imagereader_obj_t *) self_in)->fp);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_imagereader_size_obj, py_imagereader_size);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_imagereader_next_frame_obj, 1, py_imagereader_next_frame);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_imagereader_close_obj, py_imagereader_close);

STATIC const mp_rom_map_elem_t py_imagereader_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_size), MP_ROM_PTR(&py_imagereader_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_next_frame), MP_ROM_PTR(&py_imagereader_next_frame_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&py_imagereader_close_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_imagereader_locals_dict, py_imagereader_locals_dict_table);

static const mp_obj_type_t py_imagereader_type = {
    { &mp_type_type },
    .name  = MP_QSTR_imagereader,
    .print = py_imagereader_print,
    .locals_dict = (mp_obj_t) &py_imagereader_locals_dict,
};

mp_obj_t py_image_imagereader(mp_obj_t path)
{
    py_imagereader_obj_t *obj = m_new_obj(py_imagereader_obj_t);
    obj->base.type = &py_imagereader_type;
    file_read_open(&obj->fp, mp_obj_str_get_str(path));

    read_long_expect(&obj->fp, *((uint32_t *) "OMV ")); // OpenMV
    read_long_expect(&obj->fp, *((uint32_t *) "IMG ")); // Image
    read_long_expect(&obj->fp, *((uint32_t *) "STR ")); // Stream
    read_long_expect(&obj->fp, *((uint32_t *) "V1.0")); // v1.0

    obj->ms = systick_current_millis();
    return obj;
}

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

mp_obj_t py_image_load_image(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    bool copy_to_fb = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_copy_to_fb), false);
    if (copy_to_fb) fb_update_jpeg_buffer();

    const char *path = mp_obj_str_get_str(args[0]);
    image_t image = {0};

    if (copy_to_fb) {
       MAIN_FB()->w = 0;
       MAIN_FB()->h = 0;
       MAIN_FB()->bpp = 0;

       FIL fp;
       img_read_settings_t rs;
       imlib_read_geometry(&fp, &image, path, &rs);
       file_buffer_off(&fp);
       file_close(&fp);

       uint32_t size = image_size(&image);

       PY_ASSERT_TRUE_MSG((size <= OMV_RAW_BUF_SIZE), "FB Overflow!");
       image.pixels = MAIN_FB()->pixels;
       MAIN_FB()->w = image.w;
       MAIN_FB()->h = image.h;
       MAIN_FB()->bpp = image.bpp;
    }

    imlib_load_image(&image, path);
    return py_image_from_struct(&image);
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
    UINT bytes;
    FRESULT res;

    uint32_t desc_type;
    mp_obj_t desc = mp_const_none;
    const char *path = mp_obj_str_get_str(args[0]);

    if ((res = f_open_helper(&fp, path, FA_READ|FA_OPEN_EXISTING)) == FR_OK) {
        // Read descriptor type
        res = f_read(&fp, &desc_type, sizeof(desc_type), &bytes);
        if (res != FR_OK || bytes  != sizeof(desc_type)) {
            goto error;
        }

        // Load descriptor
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

            case DESC_ORB: {
                array_t *kpts = NULL;
                array_alloc(&kpts, xfree);

                res = orb_load_descriptor(&fp, kpts);
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

error:
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
    UINT bytes;
    FRESULT res;

    uint32_t desc_type;
    const char *path = mp_obj_str_get_str(args[1]);

    if ((res = f_open_helper(&fp, path, FA_WRITE|FA_CREATE_ALWAYS)) == FR_OK) {
        // Find descriptor type
        mp_obj_type_t *desc_obj_type = mp_obj_get_type(args[0]);
        if (desc_obj_type ==  &py_lbp_type) {
            desc_type = DESC_LBP;
        } else if (desc_obj_type ==  &py_kp_type) {
            desc_type = DESC_ORB;
        }

        // Write descriptor type
        res = f_write(&fp, &desc_type, sizeof(desc_type), &bytes);
        if (res != FR_OK || bytes  !=  sizeof(desc_type)) {
            goto error;
        }

        // Write descriptor
        switch (desc_type) {
            case DESC_LBP: {
                py_lbp_obj_t *lbp = ((py_lbp_obj_t*)args[0]);
                res = imlib_lbp_desc_save(&fp, lbp->hist);
                break;
            }

            case DESC_ORB: {
                py_kp_obj_t *kpts = ((py_kp_obj_t*)args[0]);
                res = orb_save_descriptor(&fp, kpts->kpts);
                break;
            }
        }
        // ignore unsupported descriptors when saving
        f_close(&fp);
    }

error:
    // File open or read error
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    return mp_const_true;
}

static mp_obj_t py_image_match_descriptor(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    mp_obj_t match_obj = mp_const_none;
    mp_obj_type_t *desc1_type = mp_obj_get_type(args[0]);
    mp_obj_type_t *desc2_type = mp_obj_get_type(args[1]);
    PY_ASSERT_TRUE_MSG((desc1_type == desc2_type), "Descriptors have different types!");

    if (desc1_type ==  &py_lbp_type) {
        py_lbp_obj_t *lbp1 = ((py_lbp_obj_t*)args[0]);
        py_lbp_obj_t *lbp2 = ((py_lbp_obj_t*)args[1]);

        // Sanity checks
        PY_ASSERT_TYPE(lbp1, &py_lbp_type);
        PY_ASSERT_TYPE(lbp2, &py_lbp_type);

        // Match descriptors
        match_obj = mp_obj_new_int(imlib_lbp_desc_distance(lbp1->hist, lbp2->hist));
    } else if (desc1_type == &py_kp_type) {
        py_kp_obj_t *kpts1 = ((py_kp_obj_t*)args[0]);
        py_kp_obj_t *kpts2 = ((py_kp_obj_t*)args[1]);
        int threshold = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 85);
        int filter_outliers = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_filter_outliers), false);

        // Sanity checks
        PY_ASSERT_TYPE(kpts1, &py_kp_type);
        PY_ASSERT_TYPE(kpts2, &py_kp_type);
        PY_ASSERT_TRUE_MSG((threshold >=0 && threshold <= 100), "Expected threshold between 0 and 100");

        int theta = 0;          // Estimated angle of rotation
        int count = 0;          // Number of matches
        point_t c = {0};        // Centroid
        rectangle_t r = {0};    // Bounding rectangle
        // List of matching keypoints indices
        mp_obj_t match_list = mp_obj_new_list(0, NULL);

        if (array_length(kpts1->kpts) && array_length(kpts1->kpts)) {
            int *match = fb_alloc(array_length(kpts1->kpts) * sizeof(int) * 2);

            // Match the two keypoint sets
            count = orb_match_keypoints(kpts1->kpts, kpts2->kpts, match, threshold, &r, &c, &theta);

            // Add matching keypoints to Python list.
            for (int i=0; i<count; i+=2) {
                mp_obj_t index_obj[2] = {
                    mp_obj_new_int(match[i+0]),
                    mp_obj_new_int(match[i+1]),
                };
                mp_obj_list_append(match_list, mp_obj_new_tuple(2, index_obj));
            }

            // Free match list
            fb_free();

            if (filter_outliers == true) {
                count = orb_filter_keypoints(kpts2->kpts, &r, &c);
            }
        }

        py_kptmatch_obj_t *o = m_new_obj(py_kptmatch_obj_t);
        o->base.type = &py_kptmatch_type;
        o->cx    = mp_obj_new_int(c.x);
        o->cy    = mp_obj_new_int(c.y);
        o->x     = mp_obj_new_int(r.x);
        o->y     = mp_obj_new_int(r.y);
        o->w     = mp_obj_new_int(r.w);
        o->h     = mp_obj_new_int(r.h);
        o->count = mp_obj_new_int(count);
        o->theta = mp_obj_new_int(theta);
        o->match = match_list;
        match_obj = o;
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Descriptor type is not supported"));
    }

    return match_obj;
}

int py_image_descriptor_from_roi(image_t *img, const char *path, rectangle_t *roi)
{
    FIL fp;
    FRESULT res = FR_OK;

    printf("Save Descriptor: ROI(%d %d %d %d)\n", roi->x, roi->y, roi->w, roi->h);
    array_t *kpts = orb_find_keypoints(img, false, 20, 1.5f, 100, CORNER_AGAST, roi);
    printf("Save Descriptor: KPTS(%d)\n", array_length(kpts));

    if (array_length(kpts)) {
        if ((res = f_open_helper(&fp, path, FA_WRITE|FA_CREATE_ALWAYS)) == FR_OK) {
            res = orb_save_descriptor(&fp, kpts);
            f_close(&fp);
        }
        // File open/write error
        if (res != FR_OK) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
        }
    }
    return 0;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_imagewriter_obj, py_image_imagewriter);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_imagereader_obj, py_image_imagereader);
/* Color space functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_rgb_to_lab_obj, py_image_rgb_to_lab);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_lab_to_rgb_obj, py_image_lab_to_rgb);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_rgb_to_grayscale_obj, py_image_rgb_to_grayscale);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_grayscale_to_rgb_obj, py_image_grayscale_to_rgb);
/* Image Module Functions */
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_image_obj, 1, py_image_load_image);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_cascade_obj, 1, py_image_load_cascade);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_descriptor_obj, 1, py_image_load_descriptor);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_descriptor_obj, 2, py_image_save_descriptor);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_match_descriptor_obj, 2, py_image_match_descriptor);
static const mp_map_elem_t globals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR___name__),            MP_OBJ_NEW_QSTR(MP_QSTR_image)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_SEARCH_EX),           MP_OBJ_NEW_SMALL_INT(SEARCH_EX)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_SEARCH_DS),           MP_OBJ_NEW_SMALL_INT(SEARCH_DS)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_EDGE_CANNY),          MP_OBJ_NEW_SMALL_INT(EDGE_CANNY)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_EDGE_SIMPLE),         MP_OBJ_NEW_SMALL_INT(EDGE_SIMPLE)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_CORNER_FAST),         MP_OBJ_NEW_SMALL_INT(CORNER_FAST)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_CORNER_AGAST),        MP_OBJ_NEW_SMALL_INT(CORNER_AGAST)},
#ifdef OMV_ENABLE_APRILTAGS
    {MP_OBJ_NEW_QSTR(MP_QSTR_TAG16H5),             MP_OBJ_NEW_SMALL_INT(TAG16H5)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_TAG25H7),             MP_OBJ_NEW_SMALL_INT(TAG25H7)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_TAG25H9),             MP_OBJ_NEW_SMALL_INT(TAG25H9)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_TAG36H10),            MP_OBJ_NEW_SMALL_INT(TAG36H10)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_TAG36H11),            MP_OBJ_NEW_SMALL_INT(TAG36H11)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_ARTOOLKIT),           MP_OBJ_NEW_SMALL_INT(ARTOOLKIT)},
#endif
#ifdef OMV_ENABLE_BARCODES
    {MP_OBJ_NEW_QSTR(MP_QSTR_EAN2),                MP_OBJ_NEW_SMALL_INT(BARCODE_EAN2)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_EAN5),                MP_OBJ_NEW_SMALL_INT(BARCODE_EAN5)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_EAN8),                MP_OBJ_NEW_SMALL_INT(BARCODE_EAN8)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_UPCE),                MP_OBJ_NEW_SMALL_INT(BARCODE_UPCE)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_ISBN10),              MP_OBJ_NEW_SMALL_INT(BARCODE_ISBN10)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_UPCA),                MP_OBJ_NEW_SMALL_INT(BARCODE_UPCA)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_EAN13),               MP_OBJ_NEW_SMALL_INT(BARCODE_EAN13)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_ISBN13),              MP_OBJ_NEW_SMALL_INT(BARCODE_ISBN13)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_I25),                 MP_OBJ_NEW_SMALL_INT(BARCODE_I25)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_DATABAR),             MP_OBJ_NEW_SMALL_INT(BARCODE_DATABAR)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_DATABAR_EXP),         MP_OBJ_NEW_SMALL_INT(BARCODE_DATABAR_EXP)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_CODABAR),             MP_OBJ_NEW_SMALL_INT(BARCODE_CODABAR)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_CODE39),              MP_OBJ_NEW_SMALL_INT(BARCODE_CODE39)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_PDF417),              MP_OBJ_NEW_SMALL_INT(BARCODE_PDF417)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_CODE93),              MP_OBJ_NEW_SMALL_INT(BARCODE_CODE93)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_CODE128),             MP_OBJ_NEW_SMALL_INT(BARCODE_CODE128)},
#endif
    {MP_OBJ_NEW_QSTR(MP_QSTR_ImageWriter),         (mp_obj_t)&py_image_imagewriter_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_ImageReader),         (mp_obj_t)&py_image_imagereader_obj},
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
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t image_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};
