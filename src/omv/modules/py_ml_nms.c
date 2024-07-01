/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python NMS class.
 */
#include "imlib_config.h"

#ifdef IMLIB_ENABLE_TFLM
#include "py/runtime.h"
#include "py_helper.h"

// ML NMS Object.
typedef struct py_ml_nms_obj {
    mp_obj_base_t base;
    int window_w;
    int window_h;
    rectangle_t roi;
    list_t bounding_boxes;
} py_ml_nms_obj_t;

const mp_obj_type_t py_ml_nms_type;

// The use of mp_arg_parse_all() is deliberately avoided here to ensure this method remains fast.
static mp_obj_t py_ml_nms_add_bounding_box(uint n_args, const mp_obj_t *pos_args) {
    enum { ARG_self, ARG_xmin, ARG_ymin, ARG_xmax, ARG_ymax, ARG_score, ARG_label_index };
    py_ml_nms_obj_t *self_in = MP_OBJ_TO_PTR(pos_args[ARG_self]);

    bounding_box_lnk_data_t lnk_data;
    lnk_data.score = mp_obj_get_float(pos_args[ARG_score]);

    if ((lnk_data.score >= 0.0f) && (lnk_data.score <= 1.0f)) {
        float xmin = IM_CLAMP(mp_obj_get_float(pos_args[ARG_xmin]), 0.0f, ((float) self_in->window_w));
        float ymin = IM_CLAMP(mp_obj_get_float(pos_args[ARG_ymin]), 0.0f, ((float) self_in->window_h));
        float xmax = IM_CLAMP(mp_obj_get_float(pos_args[ARG_xmax]), 0.0f, ((float) self_in->window_w));
        float ymax = IM_CLAMP(mp_obj_get_float(pos_args[ARG_ymax]), 0.0f, ((float) self_in->window_h));

        lnk_data.rect.w = fast_floorf(xmax - xmin);
        lnk_data.rect.h = fast_floorf(ymax - ymin);

        if ((lnk_data.rect.w > 0) && (lnk_data.rect.h > 0)) {
            lnk_data.rect.x = fast_floorf(xmin);
            lnk_data.rect.y = fast_floorf(ymin);
            lnk_data.label_index = mp_obj_get_int(pos_args[ARG_label_index]);
            rectangle_nms_add_bounding_box(&self_in->bounding_boxes, &lnk_data);
        }
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_ml_nms_add_bounding_box_obj, 7, 7, py_ml_nms_add_bounding_box);

static mp_obj_t py_ml_nms_get_bounding_boxes(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_threshold, ARG_sigma };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_threshold,  MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE } },
        { MP_QSTR_sigma,  MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_ml_nms_obj_t *self_in = MP_OBJ_TO_PTR(pos_args[0]);
    float threshold = py_helper_arg_to_float(args[ARG_threshold].u_obj, 0.1f);
    float sigma = py_helper_arg_to_float(args[ARG_sigma].u_obj, 0.1f);
    int max_label = rectangle_nms_get_bounding_boxes(&self_in->bounding_boxes, threshold, sigma);
    rectangle_map_bounding_boxes(&self_in->bounding_boxes, self_in->window_w, self_in->window_h, &self_in->roi);

    // Create a list per class label.
    mp_obj_list_t *list = MP_OBJ_TO_PTR(mp_obj_new_list(max_label + 1, NULL));
    for (size_t i = 0; i <= max_label; i++) {
        list->items[i] = mp_obj_new_list(0, NULL);
    }

    list_for_each(it, (&self_in->bounding_boxes)) {
        bounding_box_lnk_data_t *lnk_data = (bounding_box_lnk_data_t *) it->data;
        mp_obj_t rect = mp_obj_new_tuple(4, (mp_obj_t []) {mp_obj_new_int(lnk_data->rect.x),
                                                           mp_obj_new_int(lnk_data->rect.y),
                                                           mp_obj_new_int(lnk_data->rect.w),
                                                           mp_obj_new_int(lnk_data->rect.h)});
        mp_obj_t o = mp_obj_new_tuple(2, (mp_obj_t []) {rect, mp_obj_new_float(lnk_data->score)});
        mp_obj_list_append(list->items[lnk_data->label_index], o);
    }

    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_ml_nms_get_bounding_boxes_obj, 1, py_ml_nms_get_bounding_boxes);

mp_obj_t py_ml_nms_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_window_w, ARG_window_h, ARG_roi };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_window_w, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0 } },
        { MP_QSTR_window_h, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0 } },
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_rom_obj = MP_ROM_NONE} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Extract the ROI manually as we do not have an image to validate against.
    mp_obj_t *roi_obj;
    mp_obj_get_array_fixed_n(args[ARG_roi].u_obj, 4, &roi_obj);

    rectangle_t roi = {
        .x = mp_obj_get_int(roi_obj[0]),
        .y = mp_obj_get_int(roi_obj[1]),
        .w = mp_obj_get_int(roi_obj[2]),
        .h = mp_obj_get_int(roi_obj[3])
    };

    if ((roi.w < 1) || (roi.h < 1)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid ROI dimensions!"));
    }

    py_ml_nms_obj_t *model = m_new_obj(py_ml_nms_obj_t);
    model->base.type = &py_ml_nms_type;
    model->window_w = args[ARG_window_w].u_int;
    model->window_h = args[ARG_window_h].u_int;
    model->roi = roi;
    list_init(&model->bounding_boxes, sizeof(bounding_box_lnk_data_t));
    return MP_OBJ_FROM_PTR(model);
}

static const mp_rom_map_elem_t py_ml_nms_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_add_bounding_box),    MP_ROM_PTR(&py_ml_nms_add_bounding_box_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_bounding_boxes),  MP_ROM_PTR(&py_ml_nms_get_bounding_boxes_obj) },
};

static MP_DEFINE_CONST_DICT(py_ml_nms_locals_dict, py_ml_nms_locals_table);

MP_DEFINE_CONST_OBJ_TYPE(
    py_ml_nms_type,
    MP_QSTR_ml_nms,
    MP_TYPE_FLAG_NONE,
    make_new, py_ml_nms_make_new,
    locals_dict, &py_ml_nms_locals_dict
    );
#endif // IMLIB_ENABLE_TFLM
