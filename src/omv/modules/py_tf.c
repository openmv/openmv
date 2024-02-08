/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python Tensorflow library wrapper.
 */
#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/binary.h"

#include "py_helper.h"
#include "imlib_config.h"

#ifdef IMLIB_ENABLE_TF
#include "py_image.h"
#include "file_utils.h"
#include "py_tf.h"
#include "libtf_builtin_models.h"
#define GRAYSCALE_RANGE    ((COLOR_GRAYSCALE_MAX) -(COLOR_GRAYSCALE_MIN))
#define GRAYSCALE_MID      (((GRAYSCALE_RANGE) +1) / 2)

void py_tf_alloc_putchar_buffer() {
    py_tf_putchar_buffer = (char *) fb_alloc0(PY_TF_PUTCHAR_BUFFER_LEN + 1, FB_ALLOC_NO_HINT);
    py_tf_putchar_buffer_index = 0;
    py_tf_putchar_buffer_len = PY_TF_PUTCHAR_BUFFER_LEN;
}

STATIC const char *py_tf_map_datatype(libtf_datatype_t datatype) {
    if (datatype == LIBTF_DATATYPE_UINT8) {
        return "uint8";
    } else if (datatype == LIBTF_DATATYPE_INT8) {
        return "int8";
    } else {
        return "float";
    }
}

STATIC void py_tf_model_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_tf_model_obj_t *self = self_in;
    mp_printf(print,
              "{\"len\":%d, \"ram\":%d, "
              "\"input_height\":%d, \"input_width\":%d, \"input_channels\":%d, \"input_datatype\":\"%s\", "
              "\"input_scale\":%f, \"input_zero_point\":%d, "
              "\"output_height\":%d, \"output_width\":%d, \"output_channels\":%d, \"output_datatype\":\"%s\", "
              "\"output_scale\":%f, \"output_zero_point\":%d}",
              self->model_data_len, self->params.tensor_arena_size,
              self->params.input_height, self->params.input_width, self->params.input_channels,
              py_tf_map_datatype(self->params.input_datatype),
              (double) self->params.input_scale, self->params.input_zero_point,
              self->params.output_height, self->params.output_width, self->params.output_channels,
              py_tf_map_datatype(self->params.output_datatype),
              (double) self->params.output_scale, self->params.output_zero_point);
}

// TF Classification Object
#define py_tf_classification_obj_size    5
typedef struct py_tf_classification_obj {
    mp_obj_base_t base;
    mp_obj_t x, y, w, h, output;
} py_tf_classification_obj_t;

STATIC void py_tf_classification_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_tf_classification_obj_t *self = self_in;
    mp_printf(print,
              "{\"x\":%d, \"y\":%d, \"w\":%d, \"h\":%d, \"output\":",
              mp_obj_get_int(self->x),
              mp_obj_get_int(self->y),
              mp_obj_get_int(self->w),
              mp_obj_get_int(self->h));
    mp_obj_print_helper(print, self->output, kind);
    mp_printf(print, "}");
}

STATIC mp_obj_t py_tf_classification_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    if (value == MP_OBJ_SENTINEL) {
        // load
        py_tf_classification_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_tf_classification_obj_size, index, &slice)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->x) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_tf_classification_obj_size, index, false)) {
            case 0: return self->x;
            case 1: return self->y;
            case 2: return self->w;
            case 3: return self->h;
            case 4: return self->output;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_tf_classification_rect(mp_obj_t self_in) {
    return mp_obj_new_tuple(4, (mp_obj_t []) {((py_tf_classification_obj_t *) self_in)->x,
                                              ((py_tf_classification_obj_t *) self_in)->y,
                                              ((py_tf_classification_obj_t *) self_in)->w,
                                              ((py_tf_classification_obj_t *) self_in)->h});
}

mp_obj_t py_tf_classification_x(mp_obj_t self_in) {
    return ((py_tf_classification_obj_t *) self_in)->x;
}
mp_obj_t py_tf_classification_y(mp_obj_t self_in) {
    return ((py_tf_classification_obj_t *) self_in)->y;
}
mp_obj_t py_tf_classification_w(mp_obj_t self_in) {
    return ((py_tf_classification_obj_t *) self_in)->w;
}
mp_obj_t py_tf_classification_h(mp_obj_t self_in) {
    return ((py_tf_classification_obj_t *) self_in)->h;
}
mp_obj_t py_tf_classification_output(mp_obj_t self_in) {
    return ((py_tf_classification_obj_t *) self_in)->output;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_classification_rect_obj, py_tf_classification_rect);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_classification_x_obj, py_tf_classification_x);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_classification_y_obj, py_tf_classification_y);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_classification_w_obj, py_tf_classification_w);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_classification_h_obj, py_tf_classification_h);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_classification_output_obj, py_tf_classification_output);

STATIC const mp_rom_map_elem_t py_tf_classification_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&py_tf_classification_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&py_tf_classification_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&py_tf_classification_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_w), MP_ROM_PTR(&py_tf_classification_w_obj) },
    { MP_ROM_QSTR(MP_QSTR_h), MP_ROM_PTR(&py_tf_classification_h_obj) },
    { MP_ROM_QSTR(MP_QSTR_output), MP_ROM_PTR(&py_tf_classification_output_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_tf_classification_locals_dict, py_tf_classification_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    py_tf_classification_type,
    MP_QSTR_tf_classification,
    MP_TYPE_FLAG_NONE,
    print, py_tf_classification_print,
    subscr, py_tf_classification_subscr,
    locals_dict, &py_tf_classification_locals_dict
    );

static const mp_obj_type_t py_tf_model_type;

STATIC mp_obj_t int_py_tf_load(mp_obj_t path_obj, bool alloc_mode, bool helper_mode) {
    if (!helper_mode) {
        fb_alloc_mark();
    }

    const char *path = mp_obj_str_get_str(path_obj);
    py_tf_model_obj_t *tf_model = m_new_obj(py_tf_model_obj_t);
    tf_model->base.type = &py_tf_model_type;
    tf_model->model_data = NULL;

    for (int i = 0; i < MP_ARRAY_SIZE(libtf_builtin_models); i++) {
        const libtf_builtin_model_t *model = &libtf_builtin_models[i];
        if (!strcmp(path, model->name)) {
            tf_model->model_data = (unsigned char *) model->data;
            tf_model->model_data_len = model->size;
        }
    }

    if (tf_model->model_data == NULL) {
        #if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
        FIL fp;
        file_open(&fp, path, false, FA_READ | FA_OPEN_EXISTING);
        tf_model->model_data_len = f_size(&fp);
        tf_model->model_data = alloc_mode
            ? fb_alloc(tf_model->model_data_len, FB_ALLOC_PREFER_SIZE)
            : xalloc(tf_model->model_data_len);
        file_read(&fp, tf_model->model_data, tf_model->model_data_len);
        file_close(&fp);
        #else
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image I/O is not supported"));
        #endif
    }

    if (!helper_mode) {
        py_tf_alloc_putchar_buffer();
    }

    uint32_t tensor_arena_size;
    uint8_t *tensor_arena = fb_alloc_all(&tensor_arena_size, FB_ALLOC_PREFER_SIZE);

    if (libtf_get_parameters(tf_model->model_data, tensor_arena, tensor_arena_size, &tf_model->params) != 0) {
        // Note can't use MP_ERROR_TEXT here...
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
    }

    fb_free(); // free fb_alloc_all()

    if (!helper_mode) {
        fb_free(); // free py_tf_alloc_putchar_buffer()
    }

    // In this mode we leave the model allocated on the frame buffer.
    // py_tf_free_from_fb() must be called to free the model allocated on the frame buffer.
    // On error everything is cleaned because of fb_alloc_mark().

    if ((!helper_mode) && (!alloc_mode)) {
        fb_alloc_free_till_mark();
    } else if ((!helper_mode) && alloc_mode) {
        fb_alloc_mark_permanent(); // tf_model->model_data will not be popped on exception.
    }

    return tf_model;
}

STATIC mp_obj_t py_tf_load(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_load_to_fb };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_load_to_fb, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_bool = false } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    return int_py_tf_load(pos_args[0], args[ARG_load_to_fb].u_int, false);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_load_obj, 1, py_tf_load);

STATIC mp_obj_t py_tf_load_builtin_model(mp_obj_t path_obj) {
    mp_obj_t net = int_py_tf_load(path_obj, false, false);
    const char *path = mp_obj_str_get_str(path_obj);
    mp_obj_t labels = mp_obj_new_list(0, NULL);

    for (int i = 0; i < MP_ARRAY_SIZE(libtf_builtin_models); i++) {
        const libtf_builtin_model_t *model = &libtf_builtin_models[i];
        if (!strcmp(path, model->name)) {
            for (int l = 0; l < model->n_labels; l++) {
                const char *label = model->labels[l];
                mp_obj_list_append(labels, mp_obj_new_str(label, strlen(label)));
            }
            break;
        }
    }
    return mp_obj_new_tuple(2, (mp_obj_t []) {labels, net});
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_load_builtin_model_obj, py_tf_load_builtin_model);

STATIC mp_obj_t py_tf_free_from_fb() {
    fb_alloc_free_till_mark_past_mark_permanent();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tf_free_from_fb_obj, py_tf_free_from_fb);

STATIC py_tf_model_obj_t *py_tf_load_alloc(mp_obj_t path_obj) {
    if (MP_OBJ_IS_TYPE(path_obj, &py_tf_model_type)) {
        return (py_tf_model_obj_t *) path_obj;
    } else {
        return (py_tf_model_obj_t *) int_py_tf_load(path_obj, true, true);
    }
}

typedef struct py_tf_input_callback_data {
    image_t *img;
    rectangle_t *roi;
    float fscale;
} py_tf_input_callback_data_t;

STATIC void py_tf_input_callback(void *callback_data,
                                 void *model_input,
                                 libtf_parameters_t *params) {
    py_tf_input_callback_data_t *arg = (py_tf_input_callback_data_t *) callback_data;

    // Disable checking input scaling and zero-point. Nets can be all over the place on the input
    // scaling and zero-point but still work with the code below.

    // if (params->input_datatype == LIBTF_DATATYPE_UINT8) {
    //     if (fast_roundf(params->input_scale * GRAYSCALE_RANGE) != 1) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model input scale to be 1/255!"));
    //     }

    //     if (params->input_zero_point != 0) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model input zero point to be 0!"));
    //     }
    // }

    // if (params->input_datatype == LIBTF_DATATYPE_INT8) {
    //     if (fast_roundf(params->input_scale * GRAYSCALE_RANGE) != 1) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model input scale to be 1/255!"));
    //     }

    //     if (params->input_zero_point != -GRAYSCALE_MID) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model input zero point to be -128!"));
    //     }
    // }

    int shift = (params->input_datatype == LIBTF_DATATYPE_INT8) ? GRAYSCALE_MID : 0;

    image_t dst_img;
    dst_img.w = params->input_width;
    dst_img.h = params->input_height;
    dst_img.data = (uint8_t *) model_input;

    if (params->input_channels == 1) {
        dst_img.pixfmt = PIXFORMAT_GRAYSCALE;
    } else if (params->input_channels == 3) {
        dst_img.pixfmt = PIXFORMAT_RGB565;
    } else {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model input channels to be 1 or 3!"));
    }

    imlib_draw_image(&dst_img, arg->img, 0, 0, 1.0f, 1.0f, arg->roi,
                     -1, 256, NULL, NULL, IMAGE_HINT_BILINEAR | IMAGE_HINT_CENTER |
                     IMAGE_HINT_SCALE_ASPECT_EXPAND | IMAGE_HINT_BLACK_BACKGROUND, NULL, NULL, NULL);

    int size = (params->input_width * params->input_height) - 1; // must be int per countdown loop

    if (params->input_channels == 1) {
        // GRAYSCALE
        if (params->input_datatype == LIBTF_DATATYPE_FLOAT) {
            // convert u8 -> f32
            uint8_t *model_input_u8 = (uint8_t *) model_input;
            float *model_input_f32 = (float *) model_input;

            for (; size >= 0; size -= 1) {
                model_input_f32[size] = model_input_u8[size] * arg->fscale;
            }
        } else {
            if (shift) {
                // convert u8 -> s8
                uint8_t *model_input_8 = (uint8_t *) model_input;

                #if (__ARM_ARCH > 6)
                for (; size >= 3; size -= 4) {
                    *((uint32_t *) (model_input_8 + size - 3)) ^= 0x80808080;
                }
                #endif

                for (; size >= 0; size -= 1) {
                    model_input_8[size] ^= GRAYSCALE_MID;
                }
            }
        }
    } else if (params->input_channels == 3) {
        // RGB888
        int rgb_size = size * 3; // must be int per countdown loop

        if (params->input_datatype == LIBTF_DATATYPE_FLOAT) {
            uint16_t *model_input_u16 = (uint16_t *) model_input;
            float *model_input_f32 = (float *) model_input;

            for (; size >= 0; size -= 1, rgb_size -= 3) {
                int pixel = model_input_u16[size];
                model_input_f32[rgb_size] = COLOR_RGB565_TO_R8(pixel) * arg->fscale;
                model_input_f32[rgb_size + 1] = COLOR_RGB565_TO_G8(pixel) * arg->fscale;
                model_input_f32[rgb_size + 2] = COLOR_RGB565_TO_B8(pixel) * arg->fscale;
            }
        } else {
            uint16_t *model_input_u16 = (uint16_t *) model_input;
            uint8_t *model_input_8 = (uint8_t *) model_input;

            for (; size >= 0; size -= 1, rgb_size -= 3) {
                int pixel = model_input_u16[size];
                model_input_8[rgb_size] = COLOR_RGB565_TO_R8(pixel) ^ shift;
                model_input_8[rgb_size + 1] = COLOR_RGB565_TO_G8(pixel) ^ shift;
                model_input_8[rgb_size + 2] = COLOR_RGB565_TO_B8(pixel) ^ shift;
            }
        }
    }
}

STATIC void py_tf_classify_output_callback(void *callback_data,
                                           void *model_output,
                                           libtf_parameters_t *params) {
    mp_obj_t *arg = (mp_obj_t *) callback_data;

    if (params->output_height != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output height to be 1!"));
    }

    if (params->output_width != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output width to be 1!"));
    }

    *arg = mp_obj_new_list(params->output_channels, NULL);

    if (params->output_datatype == LIBTF_DATATYPE_FLOAT) {
        for (int i = 0, ii = params->output_channels; i < ii; i++) {
            ((mp_obj_list_t *) *arg)->items[i] =
                mp_obj_new_float(((float *) model_output)[i]);
        }
    } else if (params->output_datatype == LIBTF_DATATYPE_INT8) {
        for (int i = 0, ii = params->output_channels; i < ii; i++) {
            ((mp_obj_list_t *) *arg)->items[i] =
                mp_obj_new_float( ((float) (((int8_t *) model_output)[i] - params->output_zero_point)) *
                                  params->output_scale);
        }
    } else {
        for (int i = 0, ii = params->output_channels; i < ii; i++) {
            ((mp_obj_list_t *) *arg)->items[i] =
                mp_obj_new_float( ((float) (((uint8_t *) model_output)[i] - params->output_zero_point)) *
                                  params->output_scale);
        }
    }
}

STATIC mp_obj_t py_tf_classify(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_roi, ARG_min_scale, ARG_scale_mul, ARG_x_overlap, ARG_y_overlap };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_min_scale, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_scale_mul, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_x_overlap, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_y_overlap, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *image = py_helper_arg_to_image(pos_args[1], ARG_IMAGE_ANY);
    rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, image);
    float min_scale = py_helper_arg_to_float(args[ARG_min_scale].u_obj, 1.0f);
    float scale_mul = py_helper_arg_to_float(args[ARG_scale_mul].u_obj, 0.5f);
    float x_overlap = py_helper_arg_to_float(args[ARG_x_overlap].u_obj, 0.0f);
    float y_overlap = py_helper_arg_to_float(args[ARG_y_overlap].u_obj, 0.0f);

    // Sanity checks
    if ((min_scale <= 0.0f) || (min_scale > 1.0f)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 < min_scale <= 1"));
    }

    if ((scale_mul < 0.0f) || (scale_mul >= 1.0f)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= scale_mul < 1"));
    }

    if ((x_overlap != -1.f) && ((x_overlap < 0.0f) || (x_overlap >= 1.0f))) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= x_overlap < 1"));
    }

    if ((y_overlap != -1.0f) && ((y_overlap < 0.0f) || (y_overlap >= 1.0f))) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= y_overlap < 1"));
    }

    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    py_tf_model_obj_t *model = py_tf_load_alloc(pos_args[0]);
    uint8_t *tensor_arena = fb_alloc(model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    mp_obj_t objects_list = mp_obj_new_list(0, NULL);

    for (float scale = 1.0f; scale >= min_scale; scale *= scale_mul) {
        // Either provide a subtle offset to center multiple detection windows or center the only detection window.
        for (int y = roi.y + ((y_overlap != -1.0f)
                ? (fmodf(roi.h, (roi.h * scale)) / 2.0f)
                : ((roi.h - (roi.h * scale)) / 2.0f));
             // Finish when the detection window is outside of the ROI.
             (y + (roi.h * scale)) <= (roi.y + roi.h);
             // Step by an overlap amount accounting for scale or just terminate after one iteration.
             y += ((y_overlap != -1.0f) ? (roi.h * scale * (1.0f - y_overlap)) : roi.h)) {
            // Either provide a subtle offset to center multiple detection windows or center the only detection window.
            for (int x = roi.x + ((x_overlap != -1.0f)
                    ? (fmodf(roi.w, (roi.w * scale)) / 2.0f)
                    : ((roi.w - (roi.w * scale)) / 2.0f));
                 // Finish when the detection window is outside of the ROI.
                 (x + (roi.w * scale)) <= (roi.x + roi.w);
                 // Step by an overlap amount accounting for scale or just terminate after one iteration.
                 x += ((x_overlap != -1.0f) ? (roi.w * scale * (1.0f - x_overlap)) : roi.w)) {

                rectangle_t new_roi;
                rectangle_init(&new_roi, x, y, roi.w * scale, roi.h * scale);

                if (rectangle_overlap(&roi, &new_roi)) {
                    // Check if new_roi is null...

                    py_tf_input_callback_data_t py_tf_input_callback_data;
                    py_tf_input_callback_data.img = image;
                    py_tf_input_callback_data.roi = &new_roi;
                    py_tf_input_callback_data.fscale = 1.0f / GRAYSCALE_RANGE;

                    mp_obj_t py_tf_classify_output_callback_data;

                    if (libtf_invoke(model->model_data,
                                     tensor_arena,
                                     &model->params,
                                     py_tf_input_callback,
                                     &py_tf_input_callback_data,
                                     py_tf_classify_output_callback,
                                     &py_tf_classify_output_callback_data) != 0) {
                        // Note can't use MP_ERROR_TEXT here.
                        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
                    }

                    py_tf_classification_obj_t *o = m_new_obj(py_tf_classification_obj_t);
                    o->base.type = &py_tf_classification_type;
                    o->x = mp_obj_new_int(new_roi.x);
                    o->y = mp_obj_new_int(new_roi.y);
                    o->w = mp_obj_new_int(new_roi.w);
                    o->h = mp_obj_new_int(new_roi.h);
                    o->output = py_tf_classify_output_callback_data;
                    mp_obj_list_append(objects_list, o);
                }
            }
        }
    }

    fb_alloc_free_till_mark();

    return objects_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_classify_obj, 2, py_tf_classify);

STATIC void py_tf_segment_output_callback(void *callback_data,
                                          void *model_output,
                                          libtf_parameters_t *params) {
    mp_obj_t *arg = (mp_obj_t *) callback_data;

    // Disable checking output scaling and zero-point. Nets can be all over the place on the output
    // scaling and zero-point but still work with the code below.

    // if (params->output_datatype == LIBTF_DATATYPE_UINT8) {
    //     if (fast_roundf(params->output_scale * GRAYSCALE_RANGE) != 1) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output scale to be 1/255!"));
    //     }

    //     if (params->output_zero_point != 0) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output zero point to be 0!"));
    //     }
    // }

    // if (params->output_datatype == LIBTF_DATATYPE_INT8) {
    //     if (fast_roundf(params->output_scale * GRAYSCALE_RANGE) != 1) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output scale to be 1/255!"));
    //     }

    //     if (params->output_zero_point != -GRAYSCALE_MID) {
    //         mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output zero point to be -128!"));
    //     }
    // }

    int shift = (params->output_datatype == LIBTF_DATATYPE_INT8) ? GRAYSCALE_MID : 0;

    *arg = mp_obj_new_list(params->output_channels, NULL);

    for (int i = 0, ii = params->output_channels; i < ii; i++) {

        image_t img = {
            .w = params->output_width,
            .h = params->output_height,
            .pixfmt = PIXFORMAT_GRAYSCALE,
            .pixels = xalloc(params->output_width * params->output_height * sizeof(uint8_t))
        };

        ((mp_obj_list_t *) *arg)->items[i] = py_image_from_struct(&img);

        for (int y = 0, yy = params->output_height, xx = params->output_width; y < yy; y++) {
            int row = y * xx * ii;
            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&img, y);

            for (int x = 0; x < xx; x++) {
                int col = x * ii;

                if (params->output_datatype == LIBTF_DATATYPE_FLOAT) {
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x,
                                                   ((float *) model_output)[row + col + i] * GRAYSCALE_RANGE);
                } else {
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x,
                                                   ((uint8_t *) model_output)[row + col + i] ^ shift);
                }
            }
        }
    }
}

STATIC mp_obj_t py_tf_segment(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_roi };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} }
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *image = py_helper_arg_to_image(pos_args[1], ARG_IMAGE_ANY);
    rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, image);

    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    py_tf_model_obj_t *model = py_tf_load_alloc(pos_args[0]);
    uint8_t *tensor_arena = fb_alloc(model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    py_tf_input_callback_data_t py_tf_input_callback_data;
    py_tf_input_callback_data.img = image;
    py_tf_input_callback_data.roi = &roi;
    py_tf_input_callback_data.fscale = 1.0f / GRAYSCALE_RANGE;

    mp_obj_t py_tf_segment_output_callback_data;

    if (libtf_invoke(model->model_data,
                     tensor_arena,
                     &model->params,
                     py_tf_input_callback,
                     &py_tf_input_callback_data,
                     py_tf_segment_output_callback,
                     &py_tf_segment_output_callback_data) != 0) {
        // Note can't use MP_ERROR_TEXT here.
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
    }

    fb_alloc_free_till_mark();

    return py_tf_segment_output_callback_data;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_segment_obj, 2, py_tf_segment);

typedef enum {
    PY_TF_MODEL_FOMO,
    PY_TF_MODEL_YOLOV3,
    PY_TF_MODEL_YOLOV4 = PY_TF_MODEL_YOLOV3,
    PY_TF_MODEL_YOLOV5,
    PY_TF_MODEL_YOLOV7,
    PY_TF_MODEL_COUNT
} py_tf_detect_model_t;

#ifdef IMLIB_ENABLE_TF_YOLO
typedef struct py_tf_yolo_output_callback_data {
    rectangle_t *roi;
    py_tf_detect_model_t model;
    float score_threshold;
    float label_threshold;
    float nms_threshold;
    float nms_sigma;
    mp_obj_list_t **output;
} py_tf_yolo_output_callback_data_t;

typedef struct py_tf_yolo_output_callback_lnk_data {
    rectangle_t rect;
    float score;
    int label_index;
} py_tf_yolo_output_callback_lnk_data_t;

STATIC float py_tf_yolo_get_output(void *model_output, libtf_parameters_t *params, size_t i, size_t j) {
    if (params->output_datatype == LIBTF_DATATYPE_FLOAT) {
        float *output = ((float *) model_output) + (i * params->output_channels);
        return output[j];
    } else if (params->output_datatype == LIBTF_DATATYPE_INT8) {
        int8_t *output = ((int8_t *) model_output) + (i * params->output_channels);
        return (output[j] - params->output_zero_point) * params->output_scale;
    } else {
        uint8_t *output = ((uint8_t *) model_output) + (i * params->output_channels);
        return (output[j] - params->output_zero_point) * params->output_scale;
    }
}

STATIC float py_tf_yolo_iou(py_tf_yolo_output_callback_lnk_data_t *a, py_tf_yolo_output_callback_lnk_data_t *b) {
    int x1 = IM_MAX(a->rect.x, b->rect.x);
    int y1 = IM_MAX(a->rect.y, b->rect.y);
    int x2 = IM_MIN(a->rect.x + a->rect.w, b->rect.x + b->rect.w);
    int y2 = IM_MIN(a->rect.y + a->rect.h, b->rect.y + b->rect.h);
    int w = IM_MAX(0, x2 - x1);
    int h = IM_MAX(0, y2 - y1);
    int rect_intersection = w * h;
    int rect_union = (a->rect.w * a->rect.h) + (b->rect.w * b->rect.h) - rect_intersection;
    return ((float) rect_intersection) / ((float) rect_union);
}

STATIC void py_tf_yolo_output_callback(void *callback_data, void *model_output, libtf_parameters_t *params) {
    py_tf_yolo_output_callback_data_t *arg = (py_tf_yolo_output_callback_data_t *) callback_data;

    if (params->output_height != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output height to be 1!"));
    }

    if (arg->model == PY_TF_MODEL_YOLOV3) {
        if (params->output_channels < 5) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output channels to be >= 5"));
        }
    } else if (arg->model == PY_TF_MODEL_YOLOV5) {
        if (params->output_channels < 6) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output channels to be >= 6"));
        }
    } else {
        if (params->output_channels != 7) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output channels to be 7!"));
        }
    }

    // We have to iterate over each for for each class in YOLOv3.
    size_t classes_per_row = (arg->model == PY_TF_MODEL_YOLOV3) ? (params->output_channels - 4) : 1;

    list_t detections;
    list_init(&detections, sizeof(py_tf_yolo_output_callback_lnk_data_t));

    for (size_t i = 0; i < params->output_width; i++) {
        for (size_t j = 0; j < classes_per_row; j++) {
            py_tf_yolo_output_callback_lnk_data_t lnk_data;
            float xmin, ymin, xmax, ymax;

            if (arg->model == PY_TF_MODEL_YOLOV3) {
                // xmin, ymin, xmax, ymax, score[classes]
                xmin = py_tf_yolo_get_output(model_output, params, i, 0) * params->input_width;
                ymin = py_tf_yolo_get_output(model_output, params, i, 1) * params->input_height;
                xmax = py_tf_yolo_get_output(model_output, params, i, 2) * params->input_width;
                ymax = py_tf_yolo_get_output(model_output, params, i, 3) * params->input_height;
                lnk_data.label_index = j;
                lnk_data.score = py_tf_yolo_get_output(model_output, params, i, 4 + j);
            } else if (arg->model == PY_TF_MODEL_YOLOV5) {
                // cx, cy, cw, ch, score, classes[n]
                float cx = py_tf_yolo_get_output(model_output, params, i, 0);
                float cy = py_tf_yolo_get_output(model_output, params, i, 1);
                float cw = py_tf_yolo_get_output(model_output, params, i, 2) * 0.5f;
                float ch = py_tf_yolo_get_output(model_output, params, i, 3) * 0.5f;
                xmin = (cx - cw) * params->input_width;
                ymin = (cy - ch) * params->input_height;
                xmax = (cx + cw) * params->input_width;
                ymax = (cy + ch) * params->input_height;
                lnk_data.label_index = 0;
                lnk_data.score = py_tf_yolo_get_output(model_output, params, i, 4);

                // Find the class with the highest score above a threshold.
                float max_label = 0.0f;
                size_t max_label_index = 0;
                for (size_t k = 5; k < params->output_channels; k++) {
                    float label = py_tf_yolo_get_output(model_output, params, i, k);
                    if ((label >= arg->label_threshold) && (label > max_label)) {
                        max_label = label;
                        max_label_index = k - 5;
                        break;
                    }
                }

                if (max_label > 0.0f) {
                    lnk_data.label_index = max_label_index;
                } else {
                    continue;
                }
            } else {
                // batch, xmin, ymin, xmax, ymax, class, score
                xmin = py_tf_yolo_get_output(model_output, params, i, 1);
                ymin = py_tf_yolo_get_output(model_output, params, i, 2);
                xmax = py_tf_yolo_get_output(model_output, params, i, 3);
                ymax = py_tf_yolo_get_output(model_output, params, i, 4);
                lnk_data.label_index = fast_floorf(py_tf_yolo_get_output(model_output, params, i, 5));
                lnk_data.score = py_tf_yolo_get_output(model_output, params, i, 6);
            }

            xmin = IM_MIN(IM_MAX(xmin, 0.0f), ((float) (params->input_width)));
            ymin = IM_MIN(IM_MAX(ymin, 0.0f), ((float) (params->input_height)));
            xmax = IM_MIN(IM_MAX(xmax, 0.0f), ((float) (params->input_width)));
            ymax = IM_MIN(IM_MAX(ymax, 0.0f), ((float) (params->input_height)));

            lnk_data.rect.x = fast_floorf(xmin);
            lnk_data.rect.y = fast_floorf(ymin);
            lnk_data.rect.w = fast_floorf(xmax - xmin);
            lnk_data.rect.h = fast_floorf(ymax - ymin);

            if ((lnk_data.rect.w > 0) && (lnk_data.rect.h > 0) &&
                (lnk_data.score >= arg->score_threshold) && (lnk_data.score <= 1.0f)) {
                // Insertion sort detections by score.
                list_lnk_t *it = detections.head;
                for (; it; it = it->next) {
                    if (lnk_data.score > ((py_tf_yolo_output_callback_lnk_data_t *) it->data)->score) {
                        list_insert(&detections, it, &lnk_data);
                        break;
                    }
                }

                if (!it) {
                    list_push_back(&detections, &lnk_data);
                }
            }
        }
    }

    // We're using soft non-max supression below with a gaussian as this provides the best results
    // for YOLO. The gaussian is used to give a soft score penalty to overlapping boxes. On loop
    // entry "detections" is sorted but after each iteration we have to pick the next highest next
    // score again given the score penalty changes the order.

    float sigma_scale = (arg->nms_sigma > 0.0f) ? (-1.0f / arg->nms_sigma) : 0.0f;

    list_t nms_detections;
    list_init(&nms_detections, sizeof(py_tf_yolo_output_callback_lnk_data_t));

    int max_label = 0;

    // The first detection has the higest score since the list is sorted.
    list_lnk_t *max_it = detections.head;
    while (list_size(&detections)) {
        py_tf_yolo_output_callback_lnk_data_t lnk_data;
        list_remove(&detections, max_it, &lnk_data);
        list_push_back(&nms_detections, &lnk_data);

        float max_score = 0.0f;
        for (list_lnk_t *it = detections.head; it; ) {
            py_tf_yolo_output_callback_lnk_data_t *lnk_data2 = list_get_data(it);

            // Advance to next now as "it" will be invalid if we remove the current item.
            list_lnk_t *old_it = it;
            it = it->next;

            float iou = py_tf_yolo_iou(&lnk_data, lnk_data2);
            // Do not use fast_expf() as it does not output 1 when it's input is 0.
            // This will cause the scores of non-overlapping bounding boxes to decay.
            lnk_data2->score *= expf(sigma_scale * iou * iou);

            if (lnk_data2->score < arg->nms_threshold) {
                list_remove(&detections, old_it, NULL);
            } else if (lnk_data2->score > max_score) {
                max_score = lnk_data2->score;
                max_it = old_it;
            }
        }

        // Find the maximum label index for the output list.
        max_label = IM_MAX(lnk_data.label_index, max_label);
    }

    // Create a list per class label.
    *(arg->output) = mp_obj_new_list(max_label + 1, NULL);
    for (size_t i = 0; i <= max_label; i++) {
        ((mp_obj_list_t *) *(arg->output))->items[i] = mp_obj_new_list(0, NULL);
    }

    float x_scale = arg->roi->w / ((float) params->input_width);
    float y_scale = arg->roi->h / ((float) params->input_height);
    // MAX == KeepAspectRatioByExpanding - MIN == KeepAspectRatio
    float scale = IM_MIN(x_scale, y_scale);
    int x_offset = fast_floorf((arg->roi->w - (params->input_width * scale)) / 2.0f) + arg->roi->x;
    int y_offset = fast_floorf((arg->roi->h - (params->input_height * scale)) / 2.0f) + arg->roi->y;

    size_t len = list_size(&nms_detections);
    for (size_t i = 0; i < len; i++) {
        py_tf_yolo_output_callback_lnk_data_t lnk_data;
        list_pop_front(&nms_detections, &lnk_data);
        py_tf_classification_obj_t *o = m_new_obj(py_tf_classification_obj_t);
        o->base.type = &py_tf_classification_type;
        o->x = mp_obj_new_int(fast_floorf(lnk_data.rect.x * scale) + x_offset);
        o->y = mp_obj_new_int(fast_floorf(lnk_data.rect.y * scale) + y_offset);
        o->w = mp_obj_new_int(fast_floorf(lnk_data.rect.w * scale));
        o->h = mp_obj_new_int(fast_floorf(lnk_data.rect.h * scale));
        o->output = mp_obj_new_float(lnk_data.score);
        mp_obj_list_append(((mp_obj_list_t *) *(arg->output))->items[lnk_data.label_index], o);
    }
}
#endif // IMLIB_ENABLE_TF_YOLO

STATIC mp_obj_t py_tf_detect(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {
        ARG_roi, ARG_thresholds, ARG_invert, ARG_model, ARG_score_threshold, ARG_label_threshold,
        ARG_nms_threshold, ARG_nms_sigma
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_thresholds, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_invert,  MP_ARG_INT | MP_ARG_KW_ONLY, {.u_bool = false } },
        { MP_QSTR_model,  MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = PY_TF_MODEL_FOMO } },
        { MP_QSTR_score_threshold,  MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE } },
        { MP_QSTR_label_threshold,  MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE } },
        { MP_QSTR_nms_threshold,  MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE } },
        { MP_QSTR_nms_sigma,  MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    image_t *image = py_helper_arg_to_image(pos_args[1], ARG_IMAGE_ANY);
    rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, image);
    bool invert = args[ARG_invert].u_int;

    if ((args[ARG_model].u_int < 0) || (args[ARG_model].u_int >= PY_TF_MODEL_COUNT)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid model type!"));
    }

    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    py_tf_model_obj_t *model = py_tf_load_alloc(pos_args[0]);
    uint8_t *tensor_arena = fb_alloc(model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    py_tf_input_callback_data_t py_tf_input_callback_data;
    py_tf_input_callback_data.img = image;
    py_tf_input_callback_data.roi = &roi;

    mp_obj_list_t *out_list;

    if (args[ARG_model].u_int == PY_TF_MODEL_FOMO) {
        mp_obj_t py_tf_segment_output_callback_data;
        py_tf_input_callback_data.fscale = 1.0f / GRAYSCALE_RANGE;

        if (libtf_invoke(model->model_data,
                         tensor_arena,
                         &model->params,
                         py_tf_input_callback,
                         &py_tf_input_callback_data,
                         py_tf_segment_output_callback,
                         &py_tf_segment_output_callback_data) != 0) {
            // Note can't use MP_ERROR_TEXT here.
            mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
        }

        list_t thresholds;
        list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
        py_helper_arg_to_thresholds(args[ARG_thresholds].u_obj, &thresholds);

        if (!list_size(&thresholds)) {
            color_thresholds_list_lnk_data_t lnk_data;
            lnk_data.LMin = GRAYSCALE_MID;
            lnk_data.LMax = GRAYSCALE_RANGE;
            lnk_data.AMin = COLOR_A_MIN;
            lnk_data.AMax = COLOR_A_MAX;
            lnk_data.BMin = COLOR_B_MIN;
            lnk_data.BMax = COLOR_B_MAX;
            list_push_back(&thresholds, &lnk_data);
        }

        mp_obj_list_t *img_list = (mp_obj_list_t *) py_tf_segment_output_callback_data;
        out_list = mp_obj_new_list(img_list->len, NULL);

        float fscale = 1.f / GRAYSCALE_RANGE;
        for (int i = 0, ii = img_list->len; i < ii; i++) {
            image_t *img = py_image_cobj(img_list->items[i]);
            float x_scale = roi.w / ((float) img->w);
            float y_scale = roi.h / ((float) img->h);
            // MAX == KeepAspectRatioByExpanding - MIN == KeepAspectRatio
            float scale = IM_MIN(x_scale, y_scale);
            int x_offset = fast_floorf((roi.w - (img->w * scale)) / 2.0f) + roi.x;
            int y_offset = fast_floorf((roi.h - (img->h * scale)) / 2.0f) + roi.y;

            list_t out;
            imlib_find_blobs(&out, img, &((rectangle_t) {0, 0, img->w, img->h}), 1, 1,
                             &thresholds, invert, 1, 1, false, 0,
                             NULL, NULL, NULL, NULL, 0, 0);

            size_t len = list_size(&out);
            mp_obj_list_t *objects_list = mp_obj_new_list(len, NULL);
            for (size_t j = 0; j < len; j++) {
                find_blobs_list_lnk_data_t lnk_data;
                list_pop_front(&out, &lnk_data);

                histogram_t hist;
                hist.LBinCount = GRAYSCALE_RANGE + 1;
                hist.ABinCount = 0;
                hist.BBinCount = 0;
                hist.LBins = fb_alloc(hist.LBinCount * sizeof(float), FB_ALLOC_NO_HINT);
                hist.ABins = NULL;
                hist.BBins = NULL;
                imlib_get_histogram(&hist, img, &lnk_data.rect, &thresholds, invert, NULL);

                statistics_t stats;
                imlib_get_statistics(&stats, img->pixfmt, &hist);
                fb_free(); // fb_alloc(hist.LBinCount * sizeof(float), FB_ALLOC_NO_HINT);

                py_tf_classification_obj_t *o = m_new_obj(py_tf_classification_obj_t);
                o->base.type = &py_tf_classification_type;
                o->x = mp_obj_new_int(fast_floorf(lnk_data.rect.x * scale) + x_offset);
                o->y = mp_obj_new_int(fast_floorf(lnk_data.rect.y * scale) + y_offset);
                o->w = mp_obj_new_int(fast_floorf(lnk_data.rect.w * scale));
                o->h = mp_obj_new_int(fast_floorf(lnk_data.rect.h * scale));
                o->output = mp_obj_new_float(stats.LMean * fscale);
                objects_list->items[j] = o;
            }

            out_list->items[i] = objects_list;
        }
    } else {
        #ifdef IMLIB_ENABLE_TF_YOLO
        py_tf_yolo_output_callback_data_t py_tf_yolo_output_callback_data;
        py_tf_yolo_output_callback_data.roi = &roi;
        py_tf_yolo_output_callback_data.model = (py_tf_detect_model_t) args[ARG_model].u_int;
        py_tf_yolo_output_callback_data.score_threshold = py_helper_arg_to_float(args[ARG_score_threshold].u_obj, 0.5f);
        py_tf_yolo_output_callback_data.label_threshold = py_helper_arg_to_float(args[ARG_label_threshold].u_obj, 0.5f);
        py_tf_yolo_output_callback_data.nms_threshold = py_helper_arg_to_float(args[ARG_nms_threshold].u_obj, 0.1f);
        py_tf_yolo_output_callback_data.nms_sigma = py_helper_arg_to_float(args[ARG_nms_sigma].u_obj, 0.1f);
        py_tf_yolo_output_callback_data.output = &out_list;
        py_tf_input_callback_data.fscale = 1.0f;

        if (libtf_invoke(model->model_data,
                         tensor_arena,
                         &model->params,
                         py_tf_input_callback,
                         &py_tf_input_callback_data,
                         py_tf_yolo_output_callback,
                         &py_tf_yolo_output_callback_data) != 0) {
            // Note can't use MP_ERROR_TEXT here.
            mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
        }
        #else
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("YOLO not enabled!"));
        #endif
    }

    fb_alloc_free_till_mark();

    return out_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_detect_obj, 2, py_tf_detect);

STATIC void py_tf_regression_input_callback(void *callback_data,
                                            void *model_input,
                                            libtf_parameters_t *params) {
    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(*((mp_obj_t *) callback_data), &len, &items);

    if (len == (params->input_height * params->input_width * params->input_channels)) {
        if (params->input_datatype == LIBTF_DATATYPE_FLOAT) {
            float *model_input_float = (float *) model_input;
            for (size_t i = 0; i < len; i++) {
                model_input_float[i] = mp_obj_get_float(items[i]);
            }
        } else {
            uint8_t *model_input_8 = (uint8_t *) model_input;
            for (size_t i = 0; i < len; i++) {
                model_input_8[i] = fast_roundf((mp_obj_get_float(items[i]) /
                                                params->input_scale) + params->input_zero_point);
            }
        }
    } else if (len == params->input_height) {
        for (size_t i = 0; i < len; i++) {
            size_t row_len;
            mp_obj_t *row_items;
            mp_obj_get_array(items[i], &row_len, &row_items);

            if (row_len == (params->input_width * params->input_channels)) {
                if (params->input_datatype == LIBTF_DATATYPE_FLOAT) {
                    float *model_input_float = (float *) model_input;
                    for (size_t j = 0; j < row_len; j++) {
                        size_t index = (i * row_len) + j;
                        model_input_float[index] = mp_obj_get_float(row_items[index]);
                    }
                } else {
                    uint8_t *model_input_8 = (uint8_t *) model_input;
                    for (size_t j = 0; j < row_len; j++) {
                        size_t index = (i * row_len) + j;
                        model_input_8[index] = fast_roundf((mp_obj_get_float(row_items[index]) /
                                                            params->input_scale) + params->input_zero_point);
                    }
                }
            } else if (row_len == params->input_height) {
                for (size_t j = 0; j < row_len; j++) {
                    size_t c_len;
                    mp_obj_t *c_items;
                    mp_obj_get_array(row_items[i], &c_len, &c_items);

                    if (c_len == params->input_channels) {
                        if (params->input_datatype == LIBTF_DATATYPE_FLOAT) {
                            float *model_input_float = (float *) model_input;
                            for (size_t k = 0; k < c_len; k++) {
                                size_t index = (i * row_len) + (j * c_len) + k;
                                model_input_float[index] = mp_obj_get_float(c_items[index]);
                            }
                        } else {
                            uint8_t *model_input_8 = (uint8_t *) model_input;
                            for (size_t k = 0; k < c_len; k++) {
                                size_t index = (i * row_len) + (j * c_len) + k;
                                model_input_8[index] = fast_roundf((mp_obj_get_float(c_items[index]) /
                                                                    params->input_scale) + params->input_zero_point);
                            }
                        }
                    } else {
                        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Channel count mismatch!"));
                    }
                }
            } else {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Column count mismatch!"));
            }
        }
    } else {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Row count mismatch!"));
    }
}

STATIC mp_obj_t py_tf_regression(mp_obj_t model_obj, mp_obj_t array_obj) {
    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    py_tf_model_obj_t *model = py_tf_load_alloc(model_obj);
    uint8_t *tensor_arena = fb_alloc(model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    mp_obj_t py_tf_classify_output_callback_data;

    if (libtf_invoke(model->model_data,
                     tensor_arena,
                     &model->params,
                     py_tf_regression_input_callback,
                     &array_obj,
                     py_tf_classify_output_callback,
                     &py_tf_classify_output_callback_data) != 0) {
        // Note can't use MP_ERROR_TEXT here.
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
    }

    fb_alloc_free_till_mark();

    return py_tf_classify_output_callback_data;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_tf_regression_obj, py_tf_regression);

mp_obj_t py_tf_len(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->model_data_len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_len_obj, py_tf_len);

mp_obj_t py_tf_ram(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.tensor_arena_size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_ram_obj, py_tf_ram);

mp_obj_t py_tf_input_height(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.input_height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_input_height_obj, py_tf_input_height);

mp_obj_t py_tf_input_width(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.input_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_input_width_obj, py_tf_input_width);

mp_obj_t py_tf_input_channels(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.input_channels);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_input_channels_obj, py_tf_input_channels);

mp_obj_t py_tf_input_datatype(mp_obj_t self_in) {
    const char *str = py_tf_map_datatype(((py_tf_model_obj_t *) self_in)->params.input_datatype);
    return mp_obj_new_str(str, strlen(str));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_input_datatype_obj, py_tf_input_datatype);

mp_obj_t py_tf_input_scale(mp_obj_t self_in) {
    return mp_obj_new_float(((py_tf_model_obj_t *) self_in)->params.input_scale);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_input_scale_obj, py_tf_input_scale);

mp_obj_t py_tf_input_zero_point(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.input_zero_point);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_input_zero_point_obj, py_tf_input_zero_point);

mp_obj_t py_tf_output_height(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.output_height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_output_height_obj, py_tf_output_height);

mp_obj_t py_tf_output_width(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.output_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_output_width_obj, py_tf_output_width);

mp_obj_t py_tf_output_channels(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.output_channels);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_output_channels_obj, py_tf_output_channels);

mp_obj_t py_tf_output_datatype(mp_obj_t self_in) {
    const char *str = py_tf_map_datatype(((py_tf_model_obj_t *) self_in)->params.output_datatype);
    return mp_obj_new_str(str, strlen(str));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_output_datatype_obj, py_tf_output_datatype);

mp_obj_t py_tf_output_scale(mp_obj_t self_in) {
    return mp_obj_new_float(((py_tf_model_obj_t *) self_in)->params.output_scale);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_output_scale_obj, py_tf_output_scale);

mp_obj_t py_tf_output_zero_point(mp_obj_t self_in) {
    return mp_obj_new_int(((py_tf_model_obj_t *) self_in)->params.output_zero_point);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_output_zero_point_obj, py_tf_output_zero_point);

STATIC const mp_rom_map_elem_t locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_len),                 MP_ROM_PTR(&py_tf_len_obj) },
    { MP_ROM_QSTR(MP_QSTR_ram),                 MP_ROM_PTR(&py_tf_ram_obj) },
    { MP_ROM_QSTR(MP_QSTR_input_height),        MP_ROM_PTR(&py_tf_input_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_input_width),         MP_ROM_PTR(&py_tf_input_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_input_channels),      MP_ROM_PTR(&py_tf_input_channels_obj) },
    { MP_ROM_QSTR(MP_QSTR_input_datatype),      MP_ROM_PTR(&py_tf_input_datatype_obj) },
    { MP_ROM_QSTR(MP_QSTR_input_scale),         MP_ROM_PTR(&py_tf_input_scale_obj) },
    { MP_ROM_QSTR(MP_QSTR_input_zero_point),    MP_ROM_PTR(&py_tf_input_zero_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_output_height),       MP_ROM_PTR(&py_tf_output_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_output_width),        MP_ROM_PTR(&py_tf_output_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_output_channels),     MP_ROM_PTR(&py_tf_output_channels_obj) },
    { MP_ROM_QSTR(MP_QSTR_output_datatype),     MP_ROM_PTR(&py_tf_output_datatype_obj) },
    { MP_ROM_QSTR(MP_QSTR_output_scale),        MP_ROM_PTR(&py_tf_output_scale_obj) },
    { MP_ROM_QSTR(MP_QSTR_output_zero_point),   MP_ROM_PTR(&py_tf_output_zero_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_classify),            MP_ROM_PTR(&py_tf_classify_obj) },
    { MP_ROM_QSTR(MP_QSTR_segment),             MP_ROM_PTR(&py_tf_segment_obj) },
    { MP_ROM_QSTR(MP_QSTR_detect),              MP_ROM_PTR(&py_tf_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR_regression),          MP_ROM_PTR(&py_tf_regression_obj) }
};

STATIC MP_DEFINE_CONST_DICT(py_tf_locals_dict, locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    py_tf_model_type,
    MP_QSTR_tf_model,
    MP_TYPE_FLAG_NONE,
    print, py_tf_model_print,
    locals_dict, &py_tf_locals_dict
    );

#endif // IMLIB_ENABLE_TF

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_tf) },
    #ifdef IMLIB_ENABLE_TF
    { MP_ROM_QSTR(MP_QSTR_FOMO),                MP_ROM_INT(PY_TF_MODEL_FOMO) },
    #ifdef IMLIB_ENABLE_TF_YOLO
    { MP_ROM_QSTR(MP_QSTR_YOLOV3),              MP_ROM_INT(PY_TF_MODEL_YOLOV3) },
    { MP_ROM_QSTR(MP_QSTR_YOLOV4),              MP_ROM_INT(PY_TF_MODEL_YOLOV4) },
    { MP_ROM_QSTR(MP_QSTR_YOLOV5),              MP_ROM_INT(PY_TF_MODEL_YOLOV5) },
    { MP_ROM_QSTR(MP_QSTR_YOLOV7),              MP_ROM_INT(PY_TF_MODEL_YOLOV7) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_load),                MP_ROM_PTR(&py_tf_load_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_builtin_model),  MP_ROM_PTR(&py_tf_load_builtin_model_obj) },
    { MP_ROM_QSTR(MP_QSTR_free_from_fb),        MP_ROM_PTR(&py_tf_free_from_fb_obj) },
    { MP_ROM_QSTR(MP_QSTR_classify),            MP_ROM_PTR(&py_tf_classify_obj) },
    { MP_ROM_QSTR(MP_QSTR_segment),             MP_ROM_PTR(&py_tf_segment_obj) },
    { MP_ROM_QSTR(MP_QSTR_detect),              MP_ROM_PTR(&py_tf_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR_regression),          MP_ROM_PTR(&py_tf_regression_obj) }
    #else
    { MP_ROM_QSTR(MP_QSTR_load),                MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_builtin_model),  MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_free_from_fb),        MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_classify),            MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_segment),             MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_detect),              MP_ROM_PTR(&py_func_unavailable_obj) },
    { MP_ROM_QSTR(MP_QSTR_regression),          MP_ROM_PTR(&py_func_unavailable_obj) }
    #endif // IMLIB_ENABLE_TF
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t tf_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict
};

MP_REGISTER_MODULE(MP_QSTR_tf, tf_module);
