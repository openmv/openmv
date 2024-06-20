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

#define PY_TF_LOG_BUFFER_SIZE           (512)
#define PY_TF_GRAYSCALE_RANGE           ((COLOR_GRAYSCALE_MAX) -(COLOR_GRAYSCALE_MIN))
#define PY_TF_GRAYSCALE_MID             (((PY_TF_GRAYSCALE_RANGE) +1) / 2)

typedef enum {
    PY_TF_SCALE_NONE,
    PY_TF_SCALE_0_1,
    PY_TF_SCALE_S1_1,
    PY_TF_SCALE_S128_127
} py_tf_scale_t;

char *py_tf_log_buffer = NULL;
static size_t py_tf_log_index = 0;

void py_tf_alloc_log_buffer() {
    py_tf_log_index = 0;
    py_tf_log_buffer = (char *) fb_alloc0(PY_TF_LOG_BUFFER_SIZE + 1, FB_ALLOC_NO_HINT);
}

void libtf_log_handler(const char *s) {
    for (size_t i = 0, j = strlen(s); i < j; i++) {
        if (py_tf_log_index < PY_TF_LOG_BUFFER_SIZE) {
            py_tf_log_buffer[py_tf_log_index++] = s[i];
        }
    }
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

// TF Model Output Object.
typedef struct py_tf_model_output_obj {
    mp_obj_base_t base;
    void *model_output;
    libtf_parameters_t *params;
    size_t output_size;
} py_tf_model_output_obj_t;

STATIC mp_obj_t py_tf_model_output_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    if (value == MP_OBJ_SENTINEL) {
        // load
        py_tf_model_output_obj_t *self = MP_OBJ_TO_PTR(self_in);
        void *model_output = self->model_output;
        libtf_parameters_t *params = self->params;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(self->output_size, index, &slice)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            for (size_t i = 0; i < result->len; i++) {
                size_t j = i + slice.start;
                switch (params->output_datatype) {
                    case LIBTF_DATATYPE_FLOAT: {
                        result->items[i] = mp_obj_new_float(((float *) model_output)[j]);
                        break;
                    }
                    case LIBTF_DATATYPE_INT8: {
                        int8_t mo = ((int8_t *) model_output)[i];
                        result->items[i] = mp_obj_new_float((mo - params->output_zero_point) * params->output_scale);
                        break;
                    }
                    case LIBTF_DATATYPE_UINT8: {
                        uint8_t mo = ((uint8_t *) model_output)[i];
                        result->items[i] = mp_obj_new_float((mo - params->output_zero_point) * params->output_scale);
                        break;
                    }
                }
            }
            return result;
        }
        size_t i = mp_get_index(self->base.type, self->output_size, index, false);
        switch (params->output_datatype) {
            case LIBTF_DATATYPE_FLOAT: {
                return mp_obj_new_float(((float *) model_output)[i]);
            }
            case LIBTF_DATATYPE_INT8: {
                int8_t mo = ((int8_t *) model_output)[i];
                return mp_obj_new_float((mo - params->output_zero_point) * params->output_scale);
            }
            case LIBTF_DATATYPE_UINT8: {
                uint8_t mo = ((uint8_t *) model_output)[i];
                return mp_obj_new_float((mo - params->output_zero_point) * params->output_scale);
            }
        }
    }
    return MP_OBJ_NULL; // op not supported
}

STATIC mp_obj_t py_tf_model_output_get_image(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_channel, ARG_roi, ARG_scale };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_channel, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_scale, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = PY_TF_SCALE_0_1} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_tf_model_output_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    image_t temp = {.w = self->params->output_width, .h = self->params->output_height};
    rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, &temp);

    image_t img = {
        .w = roi.w,
        .h = roi.h,
        .pixfmt = PIXFORMAT_GRAYSCALE,
        .pixels = xalloc(roi.w * roi.h)
    };

    int channel = args[ARG_channel].u_int;

    int shift = (self->params->output_datatype == LIBTF_DATATYPE_INT8) ? PY_TF_GRAYSCALE_MID : 0;
    float fscale = 1.0f, fadd = 0.0f;

    switch (args[ARG_scale].u_int) {
        case PY_TF_SCALE_0_1: // convert 0->1 to 0->255
            fscale = 255.0f;
            break;
        case PY_TF_SCALE_S1_1: // convert -1->1 to 0->255
            fscale = 127.5f;
            fadd = 127.5f;
            break;
        case PY_TF_SCALE_S128_127: // convert -128->127 to 0->255
            fadd = 128.0f;
            break;
        case PY_TF_SCALE_NONE: // convert 0->255 to 0->255
        default:
            break;
    }

    for (int y = 0; y < roi.h; y++) {
        int row_index = (y + roi.y) * self->params->output_width * self->params->output_channels;
        uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&img, y);

        for (int x = 0; x < roi.w; x++) {
            int index = row_index + ((x + roi.x) * self->params->output_channels) + channel;

            if (self->params->output_datatype == LIBTF_DATATYPE_FLOAT) {
                float mo = (((float *) self->model_output)[index] * fscale) + fadd;
                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, fast_floorf(mo));
            } else {
                uint8_t mo = ((uint8_t *) self->model_output)[index] ^ shift;
                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, mo);
            }
        }
    }

    return py_image_from_struct(&img);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_model_output_get_image_obj, 1, py_tf_model_output_get_image);

STATIC const mp_rom_map_elem_t py_tf_model_output_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_image), MP_ROM_PTR(&py_tf_model_output_get_image_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_tf_model_output_locals_dict, py_tf_model_output_locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    py_tf_model_output_type,
    MP_QSTR_tf_model_output,
    MP_TYPE_FLAG_NONE,
    subscr, py_tf_model_output_subscr,
    locals_dict, &py_tf_model_output_locals_dict
    );

// TF Input/Output callback functions.
typedef struct py_tf_input_callback_data {
    image_t *img;
    rectangle_t *roi;
    py_tf_scale_t scale;
    float mean[3];
    float stdev[3];
} py_tf_input_callback_data_t;

STATIC void py_tf_input_callback(void *callback_data,
                                 void *model_input,
                                 libtf_parameters_t *params) {
    py_tf_input_callback_data_t *arg = (py_tf_input_callback_data_t *) callback_data;

    int shift = (params->input_datatype == LIBTF_DATATYPE_INT8) ? PY_TF_GRAYSCALE_MID : 0;
    float fscale = 1.0f, fadd = 0.0f;

    switch (arg->scale) {
        case PY_TF_SCALE_0_1: // convert 0->255 to 0->1
            fscale = 1.0f / 255.0f;
            break;
        case PY_TF_SCALE_S1_1: // convert 0->255 to -1->1
            fscale = 2.0f / 255.0f;
            fadd = -1.0f;
            break;
        case PY_TF_SCALE_S128_127: // convert 0->255 to -128->127
            fadd = -128.0f;
            break;
        case PY_TF_SCALE_NONE: // convert 0->255 to 0->255
        default:
            break;
    }

    float fscale_r = fscale, fadd_r = fadd;
    float fscale_g = fscale, fadd_g = fadd;
    float fscale_b = fscale, fadd_b = fadd;

    // To normalize the input image we need to subtract the mean and divide by the standard deviation.
    // We can do this by applying the normalization to fscale and fadd outside the loop.

    // Red
    fadd_r = (fadd_r - arg->mean[0]) / arg->stdev[0];
    fscale_r /= arg->stdev[0];

    // Green
    fadd_g = (fadd_g - arg->mean[1]) / arg->stdev[1];
    fscale_g /= arg->stdev[1];

    // Blue
    fadd_b = (fadd_b - arg->mean[2]) / arg->stdev[2];
    fscale_b /= arg->stdev[2];

    // Grayscale -> Y = 0.299R + 0.587G + 0.114B
    float mean = (arg->mean[0] * 0.299f) + (arg->mean[1] * 0.587f) + (arg->mean[2] * 0.114f);
    float std = (arg->stdev[0] * 0.299f) + (arg->stdev[1] * 0.587f) + (arg->stdev[2] * 0.114f);
    fadd = (fadd - mean) / std;
    fscale /= std;

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
                model_input_f32[size] = (model_input_u8[size] * fscale) + fadd;
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
                    model_input_8[size] ^= PY_TF_GRAYSCALE_MID;
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
                model_input_f32[rgb_size] = (COLOR_RGB565_TO_R8(pixel) * fscale_r) + fadd_r;
                model_input_f32[rgb_size + 1] = (COLOR_RGB565_TO_G8(pixel) * fscale_g) + fadd_g;
                model_input_f32[rgb_size + 2] = (COLOR_RGB565_TO_B8(pixel) * fscale_b) + fadd_b;
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

STATIC void py_tf_output_callback(void *callback_data,
                                  void *model_output,
                                  libtf_parameters_t *params) {
    mp_obj_t *arg = (mp_obj_t *) callback_data;
    size_t len = params->output_height * params->output_width * params->output_channels;
    *arg = mp_obj_new_list(len, NULL);

    if (params->output_datatype == LIBTF_DATATYPE_FLOAT) {
        for (size_t i = 0; i < len; i++) {
            ((mp_obj_list_t *) *arg)->items[i] =
                mp_obj_new_float(((float *) model_output)[i]);
        }
    } else if (params->output_datatype == LIBTF_DATATYPE_INT8) {
        for (size_t i = 0; i < len; i++) {
            ((mp_obj_list_t *) *arg)->items[i] =
                mp_obj_new_float( ((float) (((int8_t *) model_output)[i] - params->output_zero_point)) *
                                  params->output_scale);
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            ((mp_obj_list_t *) *arg)->items[i] =
                mp_obj_new_float( ((float) (((uint8_t *) model_output)[i] - params->output_zero_point)) *
                                  params->output_scale);
        }
    }
}

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

typedef struct py_tf_predict_callback_data {
    mp_obj_t model;
    rectangle_t *roi;
    mp_obj_t callback;
    mp_obj_t *out;
} py_tf_predict_callback_data_t;

STATIC void py_tf_predict_output_callback(void *callback_data,
                                          void *model_output,
                                          libtf_parameters_t *params) {
    py_tf_predict_callback_data_t *arg = (py_tf_predict_callback_data_t *) callback_data;
    py_tf_model_obj_t *model = MP_OBJ_TO_PTR(arg->model);
    mp_obj_t rect = mp_obj_new_tuple(4, (mp_obj_t []) {mp_obj_new_int(arg->roi->x),
                                                       mp_obj_new_int(arg->roi->y),
                                                       mp_obj_new_int(arg->roi->w),
                                                       mp_obj_new_int(arg->roi->h)});

    // This will support multiple output tensors once the API is updated.
    mp_obj_list_t *list = MP_OBJ_TO_PTR(mp_obj_new_list(0, NULL));

    py_tf_model_output_obj_t *o = m_new_obj(py_tf_model_output_obj_t);
    o->base.type = &py_tf_model_output_type;
    o->model_output = model_output;
    o->params = params;
    o->output_size = params->output_height * params->output_width * params->output_channels;
    mp_obj_list_append(list, o);

    model->model_output_list = list;
    *(arg->out) = mp_call_function_2(arg->callback, model, rect);
    model->model_output_list = mp_const_none;
}

// TF Model Object.
static const mp_obj_type_t py_tf_model_type;

STATIC void py_tf_model_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_tf_model_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print,
              "{\"len\":%d, \"ram\":%d, "
              "\"input_height\":%d, \"input_width\":%d, \"input_channels\":%d, \"input_datatype\":\"%s\", "
              "\"input_scale\":%f, \"input_zero_point\":%d, "
              "\"output_height\":%d, \"output_width\":%d, \"output_channels\":%d, \"output_datatype\":\"%s\", "
              "\"output_scale\":%f, \"output_zero_point\":%d}",
              self->size, self->params.tensor_arena_size,
              self->params.input_height, self->params.input_width, self->params.input_channels,
              py_tf_map_datatype(self->params.input_datatype),
              (double) self->params.input_scale, self->params.input_zero_point,
              self->params.output_height, self->params.output_width, self->params.output_channels,
              py_tf_map_datatype(self->params.output_datatype),
              (double) self->params.output_scale, self->params.output_zero_point);
}

STATIC mp_obj_t py_tf_model_predict(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_roi, ARG_callback, ARG_scale, ARG_mean, ARG_stdev };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_callback, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_scale, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = PY_TF_SCALE_0_1} },
        { MP_QSTR_mean, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_stdev, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    fb_alloc_mark();
    py_tf_alloc_log_buffer();

    py_tf_model_obj_t *model = MP_OBJ_TO_PTR(pos_args[0]);
    uint8_t *tensor_arena = fb_alloc(model->params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);

    mp_obj_t output_callback_data;
    int invoke_result;

    if (MP_OBJ_IS_TYPE(pos_args[1], &mp_type_tuple) || MP_OBJ_IS_TYPE(pos_args[1], &mp_type_list)) {
        invoke_result = libtf_invoke(model->data,
                                     tensor_arena,
                                     &model->params,
                                     py_tf_regression_input_callback,
                                     (void *) &pos_args[1],
                                     py_tf_output_callback,
                                     &output_callback_data);
    } else {
        image_t *image = py_helper_arg_to_image(pos_args[1], ARG_IMAGE_ANY);
        rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, image);
        py_tf_input_callback_data_t py_tf_input_callback_data = {
            .img = image,
            .roi = &roi,
            .scale = args[ARG_scale].u_int,
            .mean = {0.0f, 0.0f, 0.0f},
            .stdev = {1.0f, 1.0f, 1.0f}
        };
        py_helper_arg_to_float_array(args[ARG_mean].u_obj, py_tf_input_callback_data.mean, 3);
        py_helper_arg_to_float_array(args[ARG_stdev].u_obj, py_tf_input_callback_data.stdev, 3);

        if (args[ARG_callback].u_obj != mp_const_none) {
            py_tf_predict_callback_data_t py_tf_predict_output_callback_data;
            py_tf_predict_output_callback_data.model = model;
            py_tf_predict_output_callback_data.roi = &roi;
            py_tf_predict_output_callback_data.callback = args[ARG_callback].u_obj;
            py_tf_predict_output_callback_data.out = &output_callback_data;
            invoke_result = libtf_invoke(model->data,
                                         tensor_arena,
                                         &model->params,
                                         py_tf_input_callback,
                                         &py_tf_input_callback_data,
                                         py_tf_predict_output_callback,
                                         &py_tf_predict_output_callback_data);
        } else {
            invoke_result = libtf_invoke(model->data,
                                         tensor_arena,
                                         &model->params,
                                         py_tf_input_callback,
                                         &py_tf_input_callback_data,
                                         py_tf_output_callback,
                                         &output_callback_data);
        }
    }

    if (invoke_result != 0) {
        // Note can't use MP_ERROR_TEXT here.
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_log_buffer);
    }

    fb_alloc_free_till_mark();

    return output_callback_data;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tf_model_predict_obj, 2, py_tf_model_predict);

STATIC void py_tf_model_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    py_tf_model_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *str;
    if (dest[0] == MP_OBJ_NULL) {
        // Load attribute.
        switch (attr) {
            case MP_QSTR_len:
                dest[0] = mp_obj_new_int(self->size);
                break;
            case MP_QSTR_ram:
                dest[0] = mp_obj_new_int(self->params.tensor_arena_size);
                break;
            case MP_QSTR_input_shape:
                dest[0] = self->input_shape;
                break;
            case MP_QSTR_input_datatype:
                str = py_tf_map_datatype(self->params.input_datatype);
                dest[0] = mp_obj_new_str(str, strlen(str));
                break;
            case MP_QSTR_input_scale:
                dest[0] = mp_obj_new_float(self->params.input_scale);
                break;
            case MP_QSTR_input_zero_point:
                dest[0] = mp_obj_new_int(self->params.input_zero_point);
                break;
            case MP_QSTR_output_shape:
                dest[0] = self->output_shape;
                break;
            case MP_QSTR_output_datatype:
                str = py_tf_map_datatype(self->params.output_datatype);
                dest[0] = mp_obj_new_str(str, strlen(str));
                break;
            case MP_QSTR_output_scale:
                dest[0] = mp_obj_new_float(self->params.output_scale);
                break;
            case MP_QSTR_output_zero_point:
                dest[0] = mp_obj_new_int(self->params.output_zero_point);
                break;
            case MP_QSTR_output:
                dest[0] = self->model_output_list;
                break;
            default:
                // Continue lookup in locals_dict.
                dest[1] = MP_OBJ_SENTINEL;
                break;
        }
    }
}

mp_obj_t py_tf_model_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_path, ARG_load_to_fb };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_path, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_load_to_fb, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_bool = false } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    fb_alloc_mark();

    const char *path = mp_obj_str_get_str(args[ARG_path].u_obj);

    py_tf_model_obj_t *model = m_new_obj_with_finaliser(py_tf_model_obj_t);
    model->base.type = &py_tf_model_type;
    model->data = NULL;
    model->fb_alloc = args[ARG_load_to_fb].u_int;
    mp_obj_list_t *labels = NULL;

    for (int i = 0; i < MP_ARRAY_SIZE(libtf_builtin_models); i++) {
        const libtf_builtin_model_t *_model = &libtf_builtin_models[i];
        if (!strcmp(path, _model->name)) {
            // Load model data.
            model->size = _model->size;
            model->data = (unsigned char *) _model->data;

            // Load model labels
            labels = MP_OBJ_TO_PTR(mp_obj_new_list(_model->n_labels, NULL));
            for (int l = 0; l < _model->n_labels; l++) {
                const char *label = _model->labels[l];
                labels->items[l] = mp_obj_new_str(label, strlen(label));
            }
            break;
        }
    }

    if (model->data == NULL) {
        #if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
        FIL fp;
        file_open(&fp, path, false, FA_READ | FA_OPEN_EXISTING);
        model->size = f_size(&fp);
        model->data = model->fb_alloc ? fb_alloc(model->size, FB_ALLOC_PREFER_SIZE) : xalloc(model->size);
        file_read(&fp, model->data, model->size);
        file_close(&fp);
        #else
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image I/O is not supported"));
        #endif
    }

    py_tf_alloc_log_buffer();
    uint32_t tensor_arena_size;
    uint8_t *tensor_arena = fb_alloc_all(&tensor_arena_size, FB_ALLOC_PREFER_SIZE);
    if (libtf_get_parameters(model->data, tensor_arena, tensor_arena_size, &model->params) != 0) {
        // Note can't use MP_ERROR_TEXT here...
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_log_buffer);
    }
    fb_free(); // free tensor_arena
    fb_free(); // free log buffer

    model->input_shape = mp_obj_new_tuple(3, (mp_obj_t []) {mp_obj_new_int(model->params.input_height),
                                                            mp_obj_new_int(model->params.input_width),
                                                            mp_obj_new_int(model->params.input_channels)});

    model->output_shape = mp_obj_new_tuple(3, (mp_obj_t []) {mp_obj_new_int(model->params.output_height),
                                                             mp_obj_new_int(model->params.output_width),
                                                             mp_obj_new_int(model->params.output_channels)});

    model->model_output_list = mp_const_none;

    if (model->fb_alloc) {
        // The model data will Not be free'd on exceptions.
        fb_alloc_mark_permanent();
    } else {
        fb_alloc_free_till_mark();
    }

    if (labels == NULL) {
        return MP_OBJ_FROM_PTR(model);
    } else {
        return mp_obj_new_tuple(2, (mp_obj_t []) {MP_OBJ_FROM_PTR(labels), MP_OBJ_FROM_PTR(model)});
    }
}

STATIC mp_obj_t py_tf_model_deinit(mp_obj_t self_in) {
    py_tf_model_obj_t *model = MP_OBJ_TO_PTR(self_in);
    if (model->fb_alloc) {
        fb_alloc_free_till_mark_past_mark_permanent();
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tf_model_deinit_obj, py_tf_model_deinit);

STATIC const mp_rom_map_elem_t py_tf_model_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),             MP_ROM_PTR(&py_tf_model_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_predict),             MP_ROM_PTR(&py_tf_model_predict_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_tf_model_locals_dict, py_tf_model_locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    py_tf_model_type,
    MP_QSTR_tf_model,
    MP_TYPE_FLAG_NONE,
    attr, py_tf_model_attr,
    print, py_tf_model_print,
    make_new, py_tf_model_make_new,
    locals_dict, &py_tf_model_locals_dict
    );

extern const mp_obj_type_t py_tf_nms_type;

STATIC const mp_rom_map_elem_t py_tf_globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_OBJ_NEW_QSTR(MP_QSTR_tf) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_NONE),          MP_ROM_INT(PY_TF_SCALE_NONE) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_0_1),           MP_ROM_INT(PY_TF_SCALE_0_1) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_S1_1),          MP_ROM_INT(PY_TF_SCALE_S1_1) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_S128_127),      MP_ROM_INT(PY_TF_SCALE_S128_127) },
    { MP_ROM_QSTR(MP_QSTR_Model),               MP_ROM_PTR(&py_tf_model_type) },
    { MP_ROM_QSTR(MP_QSTR_NMS),                 MP_ROM_PTR(&py_tf_nms_type) },
};

STATIC MP_DEFINE_CONST_DICT(py_tf_globals_dict, py_tf_globals_dict_table);

const mp_obj_module_t tf_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &py_tf_globals_dict
};

MP_REGISTER_MODULE(MP_QSTR_tf, tf_module);

#endif // IMLIB_ENABLE_TF
