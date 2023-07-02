/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
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

#include "ulab/code/ulab.h"
#include "ulab/code/ndarray.h"

#ifdef IMLIB_ENABLE_TF
#include "py_image.h"
#include "ff_wrapper.h"
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
        file_read_open(&fp, path);
        tf_model->model_data_len = f_size(&fp);
        tf_model->model_data = alloc_mode
            ? fb_alloc(tf_model->model_data_len, FB_ALLOC_PREFER_SIZE)
            : xalloc(tf_model->model_data_len);
        read_data(&fp, tf_model->model_data, tf_model->model_data_len);
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

STATIC mp_obj_t py_tf_load(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    bool alloc_mode = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_load_to_fb), false);
    return int_py_tf_load(args[0], alloc_mode, false);
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

STATIC mp_obj_t py_tf_regression(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    // read model
    py_tf_model_obj_t *arg_model = py_tf_load_alloc(args[0]);

    // read input(2D or 1D) and output size(1D)
    size_t input_size_width = (&arg_model->params)->input_width;
    size_t input_size_height = (&arg_model->params)->input_height;
    size_t output_size = (&arg_model->params)->output_channels;

    // read input
    ndarray_obj_t *arg_input_array = args[1];

    // check for the input size
    if ((input_size_width * input_size_height) != arg_input_array->len) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Input array size is not same as model input size!"));
    }
    float *input_array = (float *) (arg_input_array->array);

    uint8_t *tensor_arena = fb_alloc(arg_model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);


    float output_data[output_size];

    // predict the output using tflite model
    if (libtf_regression(arg_model->model_data,
                         tensor_arena, &arg_model->params, input_array, output_data) != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Coundnt execute the model to predict the output"));
    }

    // read output
    mp_obj_list_t *out = (mp_obj_list_t *) mp_obj_new_list(output_size, NULL);
    for (size_t j = 0; j < (output_size); j++) {
        out->items[j] = mp_obj_new_float(output_data[j]);
    }

    fb_alloc_free_till_mark();
    return out;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_regression_obj, 2, py_tf_regression);

typedef struct py_tf_input_data_callback_data {
    image_t *img;
    rectangle_t *roi;
} py_tf_input_data_callback_data_t;

STATIC void py_tf_input_data_callback(void *callback_data,
                                      void *model_input,
                                      libtf_parameters_t *params) {
    py_tf_input_data_callback_data_t *arg = (py_tf_input_data_callback_data_t *) callback_data;

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
    float fscale = 1.0f / GRAYSCALE_RANGE;

    float xscale = params->input_width / ((float) arg->roi->w);
    float yscale = params->input_height / ((float) arg->roi->h);
    // MAX == KeepAspectRationByExpanding - MIN == KeepAspectRatio
    float scale = IM_MAX(xscale, yscale);

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

    imlib_draw_image(&dst_img, arg->img, 0, 0, scale, scale, arg->roi,
                     -1, 256, NULL, NULL, IMAGE_HINT_BILINEAR | IMAGE_HINT_BLACK_BACKGROUND,
                     NULL, NULL);

    int size = (params->input_width * params->input_height) - 1; // must be int per countdown loop

    if (params->input_channels == 1) {
        // GRAYSCALE
        if (params->input_datatype == LIBTF_DATATYPE_FLOAT) {
            // convert u8 -> f32
            uint8_t *model_input_u8 = (uint8_t *) model_input;
            float *model_input_f32 = (float *) model_input;

            for (; size >= 0; size -= 1) {
                model_input_f32[size] = model_input_u8[size] * fscale;
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
                model_input_f32[rgb_size] = COLOR_RGB565_TO_R8(pixel) * fscale;
                model_input_f32[rgb_size + 1] = COLOR_RGB565_TO_G8(pixel) * fscale;
                model_input_f32[rgb_size + 2] = COLOR_RGB565_TO_B8(pixel) * fscale;
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

typedef struct py_tf_classify_output_data_callback_data {
    mp_obj_t out;
} py_tf_classify_output_data_callback_data_t;

STATIC void py_tf_classify_output_data_callback(void *callback_data,
                                                void *model_output,
                                                libtf_parameters_t *params) {
    py_tf_classify_output_data_callback_data_t *arg = (py_tf_classify_output_data_callback_data_t *) callback_data;

    if (params->output_height != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output height to be 1!"));
    }

    if (params->output_width != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output width to be 1!"));
    }

    arg->out = mp_obj_new_list(params->output_channels, NULL);

    if (params->output_datatype == LIBTF_DATATYPE_FLOAT) {
        for (int i = 0, ii = params->output_channels; i < ii; i++) {
            ((mp_obj_list_t *) arg->out)->items[i] =
                mp_obj_new_float(((float *) model_output)[i]);
        }
    } else if (params->output_datatype == LIBTF_DATATYPE_INT8) {
        for (int i = 0, ii = params->output_channels; i < ii; i++) {
            ((mp_obj_list_t *) arg->out)->items[i] =
                mp_obj_new_float( ((float) (((int8_t *) model_output)[i] - params->output_zero_point)) * params->output_scale);
        }
    } else {
        for (int i = 0, ii = params->output_channels; i < ii; i++) {
            ((mp_obj_list_t *) arg->out)->items[i] =
                mp_obj_new_float( ((float) (((uint8_t *) model_output)[i] - params->output_zero_point)) * params->output_scale);
        }
    }
}

STATIC mp_obj_t py_tf_classify(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    py_tf_model_obj_t *arg_model = py_tf_load_alloc(args[0]);
    image_t *arg_img = py_image_cobj(args[1]);

    rectangle_t roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 2, kw_args, &roi);

    float arg_min_scale = py_helper_keyword_float(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_min_scale), 1.0f);

    if ((arg_min_scale <= 0.0f) || (1.0f < arg_min_scale)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 < min_scale <= 1"));
    }

    float arg_scale_mul = py_helper_keyword_float(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale_mul), 0.5f);

    if ((arg_scale_mul < 0.0f) || (1.0f <= arg_scale_mul)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= scale_mul < 1"));
    }

    float arg_x_overlap = py_helper_keyword_float(n_args, args, 5, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_overlap), 0.0f);

    if ((arg_x_overlap != -1.f) && ((arg_x_overlap < 0.0f) || (1.0f <= arg_x_overlap))) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= x_overlap < 1"));
    }

    float arg_y_overlap = py_helper_keyword_float(n_args, args, 6, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_overlap), 0.0f);

    if ((arg_y_overlap != -1.0f) && ((arg_y_overlap < 0.0f) || (1.0f <= arg_y_overlap))) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= y_overlap < 1"));
    }

    uint8_t *tensor_arena = fb_alloc(arg_model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    mp_obj_t objects_list = mp_obj_new_list(0, NULL);

    for (float scale = 1.0f; scale >= arg_min_scale; scale *= arg_scale_mul) {
        // Either provide a subtle offset to center multiple detection windows or center the only detection window.
        for (int y = roi.y + ((arg_y_overlap != -1.0f)
                ? (fmodf(roi.h, (roi.h * scale)) / 2.0f)
                : ((roi.h - (roi.h * scale)) / 2.0f));
             // Finish when the detection window is outside of the ROI.
             (y + (roi.h * scale)) <= (roi.y + roi.h);
             // Step by an overlap amount accounting for scale or just terminate after one iteration.
             y += ((arg_y_overlap != -1.0f) ? (roi.h * scale * (1.0f - arg_y_overlap)) : roi.h)) {
            // Either provide a subtle offset to center multiple detection windows or center the only detection window.
            for (int x = roi.x + ((arg_x_overlap != -1.0f)
                    ? (fmodf(roi.w, (roi.w * scale)) / 2.0f)
                    : ((roi.w - (roi.w * scale)) / 2.0f));
                 // Finish when the detection window is outside of the ROI.
                 (x + (roi.w * scale)) <= (roi.x + roi.w);
                 // Step by an overlap amount accounting for scale or just terminate after one iteration.
                 x += ((arg_x_overlap != -1.0f) ? (roi.w * scale * (1.0f - arg_x_overlap)) : roi.w)) {

                rectangle_t new_roi;
                rectangle_init(&new_roi, x, y, roi.w * scale, roi.h * scale);

                if (rectangle_overlap(&roi, &new_roi)) {
                    // Check if new_roi is null...

                    py_tf_input_data_callback_data_t py_tf_input_data_callback_data;
                    py_tf_input_data_callback_data.img = arg_img;
                    py_tf_input_data_callback_data.roi = &new_roi;

                    py_tf_classify_output_data_callback_data_t py_tf_classify_output_data_callback_data;

                    if (libtf_invoke(arg_model->model_data,
                                     tensor_arena,
                                     &arg_model->params,
                                     py_tf_input_data_callback,
                                     &py_tf_input_data_callback_data,
                                     py_tf_classify_output_data_callback,
                                     &py_tf_classify_output_data_callback_data) != 0) {
                        // Note can't use MP_ERROR_TEXT here.
                        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
                    }

                    py_tf_classification_obj_t *o = m_new_obj(py_tf_classification_obj_t);
                    o->base.type = &py_tf_classification_type;
                    o->x = mp_obj_new_int(new_roi.x);
                    o->y = mp_obj_new_int(new_roi.y);
                    o->w = mp_obj_new_int(new_roi.w);
                    o->h = mp_obj_new_int(new_roi.h);
                    o->output = py_tf_classify_output_data_callback_data.out;
                    mp_obj_list_append(objects_list, o);
                }
            }
        }
    }

    fb_alloc_free_till_mark();

    return objects_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_classify_obj, 2, py_tf_classify);

typedef struct py_tf_segment_output_data_callback_data {
    mp_obj_t out;
} py_tf_segment_output_data_callback_data_t;

STATIC void py_tf_segment_output_data_callback(void *callback_data,
                                               void *model_output,
                                               libtf_parameters_t *params) {
    py_tf_segment_output_data_callback_data_t *arg = (py_tf_segment_output_data_callback_data_t *) callback_data;

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

    arg->out = mp_obj_new_list(params->output_channels, NULL);

    for (int i = 0, ii = params->output_channels; i < ii; i++) {

        image_t img = {
            .w = params->output_width,
            .h = params->output_height,
            .pixfmt = PIXFORMAT_GRAYSCALE,
            .pixels = xalloc(params->output_width * params->output_height * sizeof(uint8_t))
        };

        ((mp_obj_list_t *) arg->out)->items[i] = py_image_from_struct(&img);

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

STATIC mp_obj_t int_py_tf_segment(bool detecting_mode, uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    py_tf_model_obj_t *arg_model = py_tf_load_alloc(args[0]);
    image_t *arg_img = py_image_cobj(args[1]);

    rectangle_t roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 2, kw_args, &roi);

    uint8_t *tensor_arena = fb_alloc(arg_model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    py_tf_input_data_callback_data_t py_tf_input_data_callback_data;
    py_tf_input_data_callback_data.img = arg_img;
    py_tf_input_data_callback_data.roi = &roi;

    py_tf_segment_output_data_callback_data_t py_tf_segment_output_data_callback_data;

    if (libtf_invoke(arg_model->model_data,
                     tensor_arena,
                     &arg_model->params,
                     py_tf_input_data_callback,
                     &py_tf_input_data_callback_data,
                     py_tf_segment_output_data_callback,
                     &py_tf_segment_output_data_callback_data) != 0) {
        // Note can't use MP_ERROR_TEXT here.
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
    }

    fb_alloc_free_till_mark();

    if (!detecting_mode) {
        return py_tf_segment_output_data_callback_data.out;
    }

    list_t thresholds;
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
    py_helper_keyword_thresholds(n_args, args, 3, kw_args, &thresholds);

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

    bool invert = py_helper_keyword_int(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_invert), false);

    mp_obj_list_t *img_list = (mp_obj_list_t *) py_tf_segment_output_data_callback_data.out;
    mp_obj_list_t *out_list = mp_obj_new_list(img_list->len, NULL);

    fb_alloc_mark();

    float fscale = 1.f / GRAYSCALE_RANGE;
    for (int i = 0, ii = img_list->len; i < ii; i++) {
        image_t *img = py_image_cobj(img_list->items[i]);
        float x_scale = roi.w / ((float) img->w);
        float y_scale = roi.h / ((float) img->h);

        list_t out;
        imlib_find_blobs(&out, img, &((rectangle_t) {0, 0, img->w, img->h}), 1, 1,
                         &thresholds, invert, 1, 1, false, 0,
                         NULL, NULL, NULL, NULL, 0, 0);

        mp_obj_list_t *objects_list = mp_obj_new_list(list_size(&out), NULL);
        for (int j = 0, jj = list_size(&out); j < jj; j++) {
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
            o->x = mp_obj_new_int(fast_floorf(lnk_data.rect.x * x_scale) + roi.x);
            o->y = mp_obj_new_int(fast_floorf(lnk_data.rect.y * y_scale) + roi.y);
            o->w = mp_obj_new_int(fast_floorf(lnk_data.rect.w * x_scale));
            o->h = mp_obj_new_int(fast_floorf(lnk_data.rect.h * y_scale));
            o->output = mp_obj_new_float(stats.LMean * fscale);
            objects_list->items[j] = o;
        }

        out_list->items[i] = objects_list;
    }

    fb_alloc_free_till_mark();

    return out_list;
}

STATIC mp_obj_t py_tf_segment(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return int_py_tf_segment(false, n_args, args, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_segment_obj, 2, py_tf_segment);

STATIC mp_obj_t py_tf_detect(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return int_py_tf_segment(true, n_args, args, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_detect_obj, 2, py_tf_detect);

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
