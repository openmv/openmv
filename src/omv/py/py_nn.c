/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NN Python module.
 */
#include <mp.h>
#include "nn.h"
#include "py_helper.h"
#include "py_image.h"
#include "omv_boardconfig.h"

#ifdef IMLIB_ENABLE_CNN

static const mp_obj_type_t py_net_type;

typedef struct _py_net_obj_t {
    mp_obj_base_t base;
    nn_t _cobj;
} py_net_obj_t;

void *py_net_cobj(mp_obj_t net_obj)
{
    PY_ASSERT_TYPE(net_obj, &py_net_type);
    return &((py_net_obj_t *)net_obj)->_cobj;
}

STATIC void py_net_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_net_obj_t *self = self_in;
    nn_dump_network(py_net_cobj(self));
}

STATIC mp_obj_t py_net_forward(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    nn_t *net = py_net_cobj(args[0]);
    image_t *img = py_helper_arg_to_image_mutable(args[1]);

    rectangle_t roi;
    py_helper_keyword_rectangle_roi(img, n_args, args, 2, kw_args, &roi);

    bool softmax = py_helper_keyword_int(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_softmax), false);
    bool dry_run = py_helper_keyword_int(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_dry_run), false);

    mp_obj_t output_list = mp_obj_new_list(0, NULL);
    if (dry_run == false) {
        nn_run_network(net, img, &roi, softmax);
    } else {
        nn_dry_run_network(net, img, softmax);
    }

    for (int i=0; i<net->output_size; i++) {
        mp_obj_list_append(output_list, mp_obj_new_float(((float) (net->output_data[i] + 128)) / 255));
    }
    return output_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_net_forward_obj, 2, py_net_forward);

// NN Class Object
#define py_nn_class_obj_size 6
typedef struct py_nn_class_obj {
    mp_obj_base_t base;
    mp_obj_t x, y, w, h, index, value;
} py_nn_class_obj_t;

static void py_nn_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_nn_class_obj_t *self = self_in;
    mp_printf(print,
              "{\"x\":%d, \"y\":%d, \"w\":%d, \"h\":%d, \"index\":%d, \"value\":%f}",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h),
              mp_obj_get_int(self->index),
              (double) mp_obj_get_float(self->value));
}

static mp_obj_t py_nn_class_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value)
{
    if (value == MP_OBJ_SENTINEL) { // load
        py_nn_class_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_nn_class_obj_size, index, &slice)) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_nn_class_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->index;
            case 5: return self->value;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_nn_class_rect(mp_obj_t self_in)
{
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_nn_class_obj_t *) self_in)->x,
                                              ((py_nn_class_obj_t *) self_in)->y,
                                              ((py_nn_class_obj_t *) self_in)->w,
                                              ((py_nn_class_obj_t *) self_in)->h});
}

mp_obj_t py_nn_class_x(mp_obj_t self_in) { return ((py_nn_class_obj_t *) self_in)->x; }
mp_obj_t py_nn_class_y(mp_obj_t self_in) { return ((py_nn_class_obj_t *) self_in)->y; }
mp_obj_t py_nn_class_w(mp_obj_t self_in) { return ((py_nn_class_obj_t *) self_in)->w; }
mp_obj_t py_nn_class_h(mp_obj_t self_in) { return ((py_nn_class_obj_t *) self_in)->h; }
mp_obj_t py_nn_class_index(mp_obj_t self_in) { return ((py_nn_class_obj_t *) self_in)->index; }
mp_obj_t py_nn_class_value(mp_obj_t self_in) { return ((py_nn_class_obj_t *) self_in)->value; }

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_rect_obj, py_nn_class_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_x_obj, py_nn_class_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_y_obj, py_nn_class_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_w_obj, py_nn_class_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_h_obj, py_nn_class_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_index_obj, py_nn_class_index);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_class_value_obj, py_nn_class_value);

STATIC const mp_rom_map_elem_t py_nn_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_nn_class_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_nn_class_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_nn_class_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_nn_class_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_nn_class_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_index), MP_ROM_PTR(&py_nn_class_index_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&py_nn_class_value_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_nn_class_locals_dict, py_nn_class_locals_dict_table);

static const mp_obj_type_t py_nn_class_type = {
    { &mp_type_type },
    .name  = MP_QSTR_nn_class,
    .print = py_nn_class_print,
    .subscr = py_nn_class_subscr,
    .locals_dict = (mp_obj_t) &py_nn_class_locals_dict
};

typedef struct py_nn_class_obj_list_lnk_data {
    rectangle_t rect;
    int index;
    float value;
    int merge_number;
} py_nn_class_obj_list_lnk_data_t;

STATIC mp_obj_t py_net_search(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    nn_t *arg_net = py_net_cobj(args[0]);
    image_t *arg_img = py_helper_arg_to_image_mutable(args[1]);

    rectangle_t roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 2, kw_args, &roi);

    float arg_threshold = py_helper_keyword_float(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 0.6);
    PY_ASSERT_TRUE_MSG((0 <= arg_threshold) && (arg_threshold <= 1), "0 <= threshold <= 1");

    float arg_min_scale = py_helper_keyword_float(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_min_scale), 1.0);
    PY_ASSERT_TRUE_MSG((0 < arg_min_scale) && (arg_min_scale <= 1), "0 < min_scale <= 1");

    float arg_scale_mul = py_helper_keyword_float(n_args, args, 5, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale_mul), 0.5);
    PY_ASSERT_TRUE_MSG((0 <= arg_scale_mul) && (arg_scale_mul < 1), "0 <= scale_mul < 1");

    float arg_x_overlap = py_helper_keyword_float(n_args, args, 6, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_overlap), 0);
    PY_ASSERT_TRUE_MSG(((0 <= arg_x_overlap) && (arg_x_overlap < 1)) || (arg_x_overlap == -1), "0 <= x_overlap < 1");

    float arg_y_overlap = py_helper_keyword_float(n_args, args, 7, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_overlap), 0);
    PY_ASSERT_TRUE_MSG(((0 <= arg_y_overlap) && (arg_y_overlap < 1)) || (arg_y_overlap == -1), "0 <= y_overlap < 1");

    float arg_contrast_threshold = py_helper_keyword_float(n_args, args, 8, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_contrast_threshold), 1);
    PY_ASSERT_TRUE_MSG(0 <= arg_contrast_threshold, "0 <= contrast_threshold");

    bool softmax = py_helper_keyword_int(n_args, args, 9, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_softmax), false);

    list_t out;
    list_init(&out, sizeof(py_nn_class_obj_list_lnk_data_t));

    for (float scale = 1; scale >= arg_min_scale; scale *= arg_scale_mul) {
        // Either provide a subtle offset to center multiple detection windows or center the only detection window.
        for (int y = roi.y + ((arg_y_overlap != -1) ? (fmodf(roi.h, (roi.h * scale)) / 2) : ((roi.h - (roi.h * scale)) / 2));
                // Finish when the detection window is outside of the ROI.
                (y + (roi.h * scale)) <= (roi.y + roi.h);
                // Step by an overlap amount accounting for scale or just terminate after one iteration.
                y += ((arg_y_overlap != -1) ? (roi.h * scale * (1 - arg_y_overlap)) : roi.h)) {
            // Either provide a subtle offset to center multiple detection windows or center the only detection window.
            for (int x = roi.x + ((arg_x_overlap != -1) ? (fmodf(roi.w, (roi.w * scale)) / 2) : ((roi.w - (roi.w * scale)) / 2));
                 // Finish when the detection window is outside of the ROI.
                 (x + (roi.w * scale)) <= (roi.x + roi.w);
                 // Step by an overlap amount accounting for scale or just terminate after one iteration.
                 x += ((arg_x_overlap != -1) ? (roi.w * scale * (1 - arg_x_overlap)) : roi.w)) {
                rectangle_t new_roi;
                rectangle_init(&new_roi, x, y, roi.w * scale, roi.h * scale);
                if (rectangle_overlap(&roi, &new_roi)) {

                    int sum = 0;
                    int sum_2 = 0;
                    for (int b = new_roi.y, bb = new_roi.y + new_roi.h, bbb = fast_sqrtf(new_roi.h); b < bb; b += bbb) {
                        for (int a = new_roi.x, aa = new_roi.x + new_roi.w, aaa = fast_sqrtf(new_roi.w); a < aa; a += aaa) {
                            switch(arg_img->bpp) {
                                case IMAGE_BPP_BINARY: {
                                    int pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL(arg_img, a, b));
                                    sum += pixel;
                                    sum_2 += pixel * pixel;
                                    break;
                                }
                                case IMAGE_BPP_GRAYSCALE: {
                                    int pixel = IMAGE_GET_GRAYSCALE_PIXEL(arg_img, a, b);
                                    sum += pixel;
                                    sum_2 += pixel * pixel;
                                    break;
                                }
                                case IMAGE_BPP_RGB565: {
                                    int pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL(arg_img, a, b));
                                    sum += pixel;
                                    sum_2 += pixel * pixel;
                                    break;
                                }
                            }
                        }
                    }

                    int area = new_roi.w * new_roi.h;
                    int mean = sum / area;
                    int variance = (sum_2 / area) - (mean * mean);

                    if (fast_sqrtf(variance) >= arg_contrast_threshold) { // Skip flat regions...
                        nn_run_network(arg_net, arg_img, &new_roi, softmax);

                        int max_index = -1;
                        float max_value = -1;
                        for (int i=0; i<arg_net->output_size; i++) {
                            float value = ((float) (arg_net->output_data[i] + 128)) / 255;
                            if ((value >= arg_threshold) && (value > max_value)) {
                                max_index = i;
                                max_value = value;
                            }
                        }

                        if (max_index != -1) {
                            py_nn_class_obj_list_lnk_data_t lnk_data;
                            lnk_data.rect.x = new_roi.x;
                            lnk_data.rect.y = new_roi.y;
                            lnk_data.rect.w = new_roi.w;
                            lnk_data.rect.h = new_roi.h;
                            lnk_data.index = max_index;
                            lnk_data.value = max_value;
                            lnk_data.merge_number = 1;
                            list_push_back(&out, &lnk_data);
                        }
                    }
                }
            }
        }
    }

    // Merge all overlapping and same detections and average them.

    for (;;) {
        bool merge_occured = false;

        list_t out_temp;
        list_init(&out_temp, sizeof(py_nn_class_obj_list_lnk_data_t));

        while (list_size(&out)) {
            py_nn_class_obj_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            for (size_t k = 0, l = list_size(&out); k < l; k++) {
                py_nn_class_obj_list_lnk_data_t tmp_data;
                list_pop_front(&out, &tmp_data);

                if ((lnk_data.index == tmp_data.index)
                && rectangle_overlap(&(lnk_data.rect), &(tmp_data.rect))) {
                    lnk_data.rect.x = ((lnk_data.rect.x * lnk_data.merge_number) + tmp_data.rect.x) / (lnk_data.merge_number + 1);
                    lnk_data.rect.y = ((lnk_data.rect.y * lnk_data.merge_number) + tmp_data.rect.y) / (lnk_data.merge_number + 1);
                    lnk_data.rect.w = ((lnk_data.rect.w * lnk_data.merge_number) + tmp_data.rect.w) / (lnk_data.merge_number + 1);
                    lnk_data.rect.h = ((lnk_data.rect.h * lnk_data.merge_number) + tmp_data.rect.h) / (lnk_data.merge_number + 1);
                    lnk_data.value = ((lnk_data.value * lnk_data.merge_number) + tmp_data.value) / (lnk_data.merge_number + 1);
                    lnk_data.merge_number += 1;
                    merge_occured = true;
                } else {
                    list_push_back(&out, &tmp_data);
                }
            }

            list_push_back(&out_temp, &lnk_data);
        }

        list_copy(&out, &out_temp);

        if (!merge_occured) {
            break;
        }
    }

    // Determine the winner between overlapping different class detections.

    for (;;) {
        bool merge_occured = false;

        list_t out_temp;
        list_init(&out_temp, sizeof(py_nn_class_obj_list_lnk_data_t));

        while (list_size(&out)) {
            py_nn_class_obj_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            for (size_t k = 0, l = list_size(&out); k < l; k++) {
                py_nn_class_obj_list_lnk_data_t tmp_data;
                list_pop_front(&out, &tmp_data);

                if ((lnk_data.index != tmp_data.index)
                && rectangle_overlap(&(lnk_data.rect), &(tmp_data.rect))) {
                    if (tmp_data.value > lnk_data.value) {
                        memcpy(&lnk_data, &tmp_data, sizeof(py_nn_class_obj_list_lnk_data_t));
                    }

                    merge_occured = true;
                } else {
                    list_push_back(&out, &tmp_data);
                }
            }

            list_push_back(&out_temp, &lnk_data);
        }

        list_copy(&out, &out_temp);

        if (!merge_occured) {
            break;
        }
    }

    mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);

    for (size_t i = 0; list_size(&out); i++) {
        py_nn_class_obj_list_lnk_data_t lnk_data;
        list_pop_front(&out, &lnk_data);

        py_nn_class_obj_t *o = m_new_obj(py_nn_class_obj_t);
        o->base.type = &py_nn_class_type;
        o->x = mp_obj_new_int(lnk_data.rect.x);
        o->y = mp_obj_new_int(lnk_data.rect.y);
        o->w = mp_obj_new_int(lnk_data.rect.w);
        o->h = mp_obj_new_int(lnk_data.rect.h);
        o->index = mp_obj_new_int(lnk_data.index);
        o->value = mp_obj_new_float(lnk_data.value);

        objects_list->items[i] = o;
    }

    return objects_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_net_search_obj, 2, py_net_search);

STATIC const mp_rom_map_elem_t locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_forward), MP_ROM_PTR(&py_net_forward_obj) },
    { MP_ROM_QSTR(MP_QSTR_search), MP_ROM_PTR(&py_net_search_obj) }
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_net_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Net,
    .print = py_net_print,
    .locals_dict = (mp_obj_t) &locals_dict
};

static mp_obj_t py_nn_load(mp_obj_t path_obj)
{
    const char *path = mp_obj_str_get_str(path_obj);
    py_net_obj_t *net = m_new_obj(py_net_obj_t);
    net->base.type = &py_net_type;
    nn_load_network(py_net_cobj(net), path);
    return net;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_load_obj, py_nn_load);

#endif // IMLIB_ENABLE_CNN

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_nn) },
#ifdef IMLIB_ENABLE_CNN
    { MP_ROM_QSTR(MP_QSTR_load),     MP_ROM_PTR(&py_nn_load_obj) },
#else
    { MP_ROM_QSTR(MP_QSTR_load),     MP_ROM_PTR(&py_func_unavailable_obj) }
#endif // IMLIB_ENABLE_CNN
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t nn_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict
};
