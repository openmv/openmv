/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * ML Image Arg class.
 */
#include "imlib_config.h"

#ifdef IMLIB_ENABLE_TFLM
#include "py/runtime.h"
#include "py_helper.h"
#include "py_image.h"
#include "py_ml.h"

#define PY_ML_GRAYSCALE_RANGE   ((COLOR_GRAYSCALE_MAX) -(COLOR_GRAYSCALE_MIN))
#define PY_ML_GRAYSCALE_MID     (((PY_ML_GRAYSCALE_RANGE) +1) / 2)

static void py_ml_image_arg_input_callback(void *self, py_ml_model_obj_t *model, size_t index) {
    py_image_arg_obj_t *image_arg = MP_OBJ_TO_PTR(self);
    void *model_input = ml_backend_get_input(model, index);
    mp_obj_tuple_t *input_shape = MP_OBJ_TO_PTR(model->input_shape->items[index]);
    int input_dtype = mp_obj_get_int(model->input_dtype->items[index]);

    if ((input_shape->len != 3) && (input_shape->len != 4)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected a tensor of (1, h, w, c) or (h, w, c)!"));
    }

    int offset = (input_shape->len == 4) ? 1 : 0;
    int input_height = mp_obj_get_int(input_shape->items[offset]);
    int input_width = mp_obj_get_int(input_shape->items[offset + 1]);
    int input_channels = mp_obj_get_int(input_shape->items[offset + 2]);

    if ((input_height <= 0) || (input_width <= 0)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid tensor height or width!"));
    }

    int shift = (input_dtype == PY_ML_DTYPE_INT8) ? PY_ML_GRAYSCALE_MID : 0;
    float fscale = 1.0f, fadd = 0.0f;

    switch (image_arg->scale) {
        case PY_ML_SCALE_0_1: // convert 0->255 to 0->1
            fscale = 1.0f / 255.0f;
            break;
        case PY_ML_SCALE_S1_1: // convert 0->255 to -1->1
            fscale = 2.0f / 255.0f;
            fadd = -1.0f;
            break;
        case PY_ML_SCALE_S128_127: // convert 0->255 to -128->127
            fadd = -128.0f;
            break;
        case PY_ML_SCALE_NONE: // convert 0->255 to 0->255
        default:
            break;
    }

    float fscale_r = fscale, fadd_r = fadd;
    float fscale_g = fscale, fadd_g = fadd;
    float fscale_b = fscale, fadd_b = fadd;

    // To normalize the input image we need to subtract the mean and divide by the standard deviation.
    // We can do this by applying the normalization to fscale and fadd outside the loop.

    // Red
    fadd_r = (fadd_r - image_arg->mean[0]) / image_arg->stdev[0];
    fscale_r /= image_arg->stdev[0];

    // Green
    fadd_g = (fadd_g - image_arg->mean[1]) / image_arg->stdev[1];
    fscale_g /= image_arg->stdev[1];

    // Blue
    fadd_b = (fadd_b - image_arg->mean[2]) / image_arg->stdev[2];
    fscale_b /= image_arg->stdev[2];

    // Grayscale -> Y = 0.299R + 0.587G + 0.114B
    float mean = (image_arg->mean[0] * 0.299f) + (image_arg->mean[1] * 0.587f) + (image_arg->mean[2] * 0.114f);
    float std = (image_arg->stdev[0] * 0.299f) + (image_arg->stdev[1] * 0.587f) + (image_arg->stdev[2] * 0.114f);
    fadd = (fadd - mean) / std;
    fscale /= std;

    image_t dst_img;
    dst_img.w = input_width;
    dst_img.h = input_height;
    dst_img.data = (uint8_t *) model_input;

    if (input_channels == 1) {
        dst_img.pixfmt = PIXFORMAT_GRAYSCALE;
    } else if (input_channels == 3) {
        dst_img.pixfmt = PIXFORMAT_RGB565;
    } else {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model input channels to be 1 or 3!"));
    }

    imlib_draw_image(&dst_img, image_arg->image, 0, 0, 1.0f, 1.0f, &image_arg->roi,
                     -1, 256, NULL, NULL, IMAGE_HINT_BILINEAR | IMAGE_HINT_CENTER |
                     IMAGE_HINT_SCALE_ASPECT_EXPAND | IMAGE_HINT_BLACK_BACKGROUND, NULL, NULL, NULL);

    int size = (input_width * input_height) - 1; // must be int per countdown loop

    if (input_channels == 1) {
        // GRAYSCALE
        if (input_dtype == PY_ML_DTYPE_FLOAT) {
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
                    model_input_8[size] ^= PY_ML_GRAYSCALE_MID;
                }
            }
        }
    } else if (input_channels == 3) {
        // RGB888
        int rgb_size = size * 3; // must be int per countdown loop
        if (input_dtype == PY_ML_DTYPE_FLOAT) {
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

static void py_ml_image_arg_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    py_image_arg_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (dest[0] == MP_OBJ_NULL) {
        // Load attribute.
        switch (attr) {
            case MP_QSTR_image:
                dest[0] = py_image_from_struct(self->image);
                break;
            case MP_QSTR_roi:
                dest[0] = mp_obj_new_tuple(4, (mp_obj_t []) {mp_obj_new_int(self->roi.x),
                                                             mp_obj_new_int(self->roi.y),
                                                             mp_obj_new_int(self->roi.w),
                                                             mp_obj_new_int(self->roi.h)});
                break;
            case MP_QSTR_scale:
                dest[0] = mp_obj_new_int(self->scale);
                break;
            case MP_QSTR_mean:
                dest[0] = mp_obj_new_tuple(3, (mp_obj_t []) {mp_obj_new_float(self->mean[0]),
                                                             mp_obj_new_float(self->mean[1]),
                                                             mp_obj_new_float(self->mean[2])});
                break;
            case MP_QSTR_stdev:
                dest[0] = mp_obj_new_tuple(3, (mp_obj_t []) {mp_obj_new_float(self->stdev[0]),
                                                             mp_obj_new_float(self->stdev[1]),
                                                             mp_obj_new_float(self->stdev[2])});
                break;
            default:
                // Continue lookup in locals_dict.
                dest[1] = MP_OBJ_SENTINEL;
                break;
        }
    }
}

const mp_obj_type_t py_ml_image_arg_type;

mp_obj_t py_ml_image_arg_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_image, ARG_roi, ARG_scale, ARG_mean, ARG_stdev };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_scale, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = PY_ML_SCALE_0_1} },
        { MP_QSTR_mean, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_stdev, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_image_arg_obj_t *image_arg = m_new_obj(py_image_arg_obj_t);
    image_arg->base.type = &py_ml_image_arg_type;
    image_arg->input_callback = py_ml_image_arg_input_callback;
    image_arg->image = py_helper_arg_to_image(args[ARG_image].u_obj, ARG_IMAGE_ANY);
    image_arg->roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, image_arg->image);
    image_arg->scale = args[ARG_scale].u_int;
    memcpy(image_arg->mean, (float[]) {0.0f, 0.0f, 0.0f}, 3 * sizeof(float));
    memcpy(image_arg->stdev, (float[]) {1.0f, 1.0f, 1.0f}, 3 * sizeof(float));
    py_helper_arg_to_float_array(args[ARG_mean].u_obj, image_arg->mean, 3);
    py_helper_arg_to_float_array(args[ARG_stdev].u_obj, image_arg->stdev, 3);
    return MP_OBJ_FROM_PTR(image_arg);
}

MP_DEFINE_CONST_OBJ_TYPE(
    py_ml_image_arg_type,
    MP_QSTR_ml_image_arg,
    MP_TYPE_FLAG_NONE,
    attr, py_ml_image_arg_attr,
    make_new, py_ml_image_arg_make_new
    );
#endif // IMLIB_ENABLE_TFLM
