/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python Machine Learning Module.
 */
#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/binary.h"

#include "py_helper.h"
#include "imlib_config.h"

#ifdef IMLIB_ENABLE_TFLM
#include "py_image.h"
#include "file_utils.h"
#include "py_ml.h"
#include "tflm_builtin_models.h"
#include "ulab/code/ndarray.h"

static const char *py_ml_map_dtype(py_ml_dtype_t dtype) {
    if (dtype == PY_ML_DTYPE_UINT8) {
        return "uint8";
    } else if (dtype == PY_ML_DTYPE_INT8) {
        return "int8";
    } else if (dtype == PY_ML_DTYPE_INT16) {
        return "int16";
    } else {
        return "float";
    }
}

static size_t py_ml_tuple_sum(mp_obj_tuple_t *o) {
    if (o->len < 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unexpected tensor shape"));
    }

    size_t size = mp_obj_get_int(o->items[0]);
    for (size_t i = 1; i < o->len; i++) {
        size *= mp_obj_get_int(o->items[i]);
    }
    return size;
}

static void py_ml_ndarray_input_callback(void *self, py_ml_model_obj_t *model, size_t index) {
    ndarray_obj_t *input_array = MP_OBJ_TO_PTR(self);
    void *model_input = ml_backend_get_input(model, index);
    mp_obj_tuple_t *input_shape = MP_OBJ_TO_PTR(model->input_shape->items[index]);

    if (input_array->ndim != input_shape->len) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Input shape does not match the model input shape"));
    }

    for (size_t i = 0; i < input_array->ndim; i++) {
        if (input_array->shape[i] != mp_obj_get_int(input_shape->items[i])) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Input shape does not match the model input shape"));
        }
    }

    if (model->input_dtype == PY_ML_DTYPE_FLOAT) {
        float *model_input_float = (float *) model_input;
        for (size_t i = 0; i < input_array->len; i++) {
            float value = ndarray_get_float_index(input_array->array, input_array->dtype, i);
            model_input_float[i] = value;
        }
    } else if ((model->input_dtype == PY_ML_DTYPE_INT8) || (model->input_dtype == PY_ML_DTYPE_UINT8)) {
        int8_t *model_input_8 = (int8_t *) model_input;
        for (size_t i = 0; i < input_array->len; i++) {
            float value = ndarray_get_float_index(input_array->array, input_array->dtype, i);
            model_input_8[i] = (int8_t) ((value / model->input_scale) + model->input_zero_point);
        }
    } else {
        int16_t *model_input_16 = (int16_t *) model_input;
        for (size_t i = 0; i < input_array->len; i++) {
            float value = ndarray_get_float_index(input_array->array, input_array->dtype, i);
            model_input_16[i] = (int16_t) ((value / model->input_scale) + model->input_zero_point);
        }
    }
}

static mp_obj_t py_ml_output_callback(py_ml_model_obj_t *model) {
    mp_obj_list_t *output_list = MP_OBJ_TO_PTR(mp_obj_new_list(model->outputs_size, NULL));
    for (size_t i = 0; i < model->outputs_size; i++) {
        void *model_output = ml_backend_get_output(model, i);
        size_t size = py_ml_tuple_sum(MP_OBJ_TO_PTR(model->output_shape->items[i]));
        mp_obj_tuple_t *output = MP_OBJ_TO_PTR(mp_obj_new_tuple(size, NULL));

        if (model->output_dtype == PY_ML_DTYPE_FLOAT) {
            for (size_t j = 0; j < size; j++) {
                output->items[j] = mp_obj_new_float(((float *) model_output)[j]);
            }
        } else if (model->output_dtype == PY_ML_DTYPE_INT8) {
            for (size_t j = 0; j < size; j++) {
                float v = (((int8_t *) model_output)[j] - model->output_zero_point);
                output->items[j] = mp_obj_new_float(v * model->output_scale);
            }
        } else if (model->output_dtype == PY_ML_DTYPE_UINT8) {
            for (size_t j = 0; j < size; j++) {
                float v = (((uint8_t *) model_output)[j] - model->output_zero_point);
                output->items[j] = mp_obj_new_float(v * model->output_scale);
            }
        } else {
            for (size_t j = 0; j < size; j++) {
                float v = (((int8_t *) model_output)[j] - model->output_zero_point);
                output->items[j] = mp_obj_new_float(v * model->output_scale);
            }
        }
        output_list->items[i] = MP_OBJ_FROM_PTR(output);
    }
    return MP_OBJ_FROM_PTR(output_list);
}

// TF Model Object.
static const mp_obj_type_t py_ml_model_type;

static void py_ml_model_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_ml_model_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print,
              "{size: %d, ram: %d, inputs_size: %d, input_dtype: %s, input_scale: %f, input_zero_point: %d, "
              "outputs_size: %d output_dtype: %s, output_scale: %f, output_zero_point: %d}",
              self->size, self->memory_size, self->inputs_size, py_ml_map_dtype(self->input_dtype),
              (double) self->input_scale, self->input_zero_point, self->outputs_size, py_ml_map_dtype(self->output_dtype),
              (double) self->output_scale, self->output_zero_point);
}

extern const mp_obj_type_t py_ml_image_arg_type;

static mp_obj_t py_ml_model_predict(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_callback };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_ml_model_obj_t *model = MP_OBJ_TO_PTR(pos_args[0]);

    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(pos_args[1], &len, &items);

    if (len != model->inputs_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Number of inputs does not match the model number of inputs!"));
    }

    for (size_t i = 0; i < len; i++) {
        if (MP_OBJ_IS_TYPE(items[i], &ulab_ndarray_type)) {
            py_ml_ndarray_input_callback(items[i], model, i);
        } else if (MP_OBJ_IS_TYPE(items[i], &py_ml_image_arg_type)) {
            py_image_arg_obj_t *image_arg = MP_OBJ_TO_PTR(items[i]);
            image_arg->input_callback(image_arg, model, i);
        } else {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported input type!"));
        }
    }

    ml_backend_run_inference(model);

    mp_obj_t output_data = py_ml_output_callback(model);

    if (args[ARG_callback].u_obj != mp_const_none) {
        mp_obj_t fun_args[3] = { MP_OBJ_FROM_PTR(model), pos_args[1], output_data };
        output_data = mp_call_function_n_kw(args[ARG_callback].u_obj, 3, 0, fun_args);
    }

    return output_data;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_ml_model_predict_obj, 2, py_ml_model_predict);

static void py_ml_model_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    py_ml_model_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *str;
    if (dest[0] == MP_OBJ_NULL) {
        // Load attribute.
        switch (attr) {
            case MP_QSTR_len:
                dest[0] = mp_obj_new_int(self->size);
                break;
            case MP_QSTR_ram:
                dest[0] = mp_obj_new_int(self->memory_size);
                break;
            case MP_QSTR_input_shape:
                dest[0] = MP_OBJ_FROM_PTR(self->input_shape);
                break;
            case MP_QSTR_input_dtype:
                str = py_ml_map_dtype(self->input_dtype);
                dest[0] = mp_obj_new_str(str, strlen(str));
                break;
            case MP_QSTR_input_scale:
                dest[0] = mp_obj_new_float(self->input_scale);
                break;
            case MP_QSTR_input_zero_point:
                dest[0] = mp_obj_new_int(self->input_zero_point);
                break;
            case MP_QSTR_output_shape:
                dest[0] = MP_OBJ_FROM_PTR(self->output_shape);
                break;
            case MP_QSTR_output_dtype:
                str = py_ml_map_dtype(self->output_dtype);
                dest[0] = mp_obj_new_str(str, strlen(str));
                break;
            case MP_QSTR_output_scale:
                dest[0] = mp_obj_new_float(self->output_scale);
                break;
            case MP_QSTR_output_zero_point:
                dest[0] = mp_obj_new_int(self->output_zero_point);
                break;
            default:
                // Continue lookup in locals_dict.
                dest[1] = MP_OBJ_SENTINEL;
                break;
        }
    }
}

mp_obj_t py_ml_model_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
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

    py_ml_model_obj_t *model = mp_obj_malloc_with_finaliser(py_ml_model_obj_t, &py_ml_model_type);
    model->data = NULL;
    model->fb_alloc = args[ARG_load_to_fb].u_int;
    mp_obj_list_t *labels = NULL;

    for (const tflm_builtin_model_t *_model = &tflm_builtin_models[0]; _model->name != NULL; _model++) {
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

    if (model->fb_alloc) {
        // The model's data will Not be free'd on exceptions.
        fb_alloc_mark_permanent();
    } else {
        fb_alloc_free_till_mark();
    }


    ml_backend_init_model(model);

    if (model->input_scale == 0.0f) {
        model->input_scale = 1.0;
    }

    if (model->output_scale == 0.0f) {
        model->output_scale = 1.0;
    }

    if (labels == NULL) {
        return MP_OBJ_FROM_PTR(model);
    } else {
        return mp_obj_new_tuple(2, (mp_obj_t []) {MP_OBJ_FROM_PTR(labels), MP_OBJ_FROM_PTR(model)});
    }
}

static mp_obj_t py_ml_model_deinit(mp_obj_t self_in) {
    py_ml_model_obj_t *model = MP_OBJ_TO_PTR(self_in);
    if (model->fb_alloc) {
        fb_alloc_free_till_mark_past_mark_permanent();
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_ml_model_deinit_obj, py_ml_model_deinit);

static const mp_rom_map_elem_t py_ml_model_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),             MP_ROM_PTR(&py_ml_model_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_predict),             MP_ROM_PTR(&py_ml_model_predict_obj) },
};

static MP_DEFINE_CONST_DICT(py_ml_model_locals_dict, py_ml_model_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
    py_ml_model_type,
    MP_QSTR_ml_model,
    MP_TYPE_FLAG_NONE,
    attr, py_ml_model_attr,
    print, py_ml_model_print,
    make_new, py_ml_model_make_new,
    locals_dict, &py_ml_model_locals_dict
    );

extern const mp_obj_type_t py_ml_nms_type;

static const mp_rom_map_elem_t py_ml_globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_OBJ_NEW_QSTR(MP_QSTR_ml) },
    { MP_ROM_QSTR(MP_QSTR_Image),               MP_ROM_PTR(&py_ml_image_arg_type) },
    { MP_ROM_QSTR(MP_QSTR_Model),               MP_ROM_PTR(&py_ml_model_type) },
    { MP_ROM_QSTR(MP_QSTR_NMS),                 MP_ROM_PTR(&py_ml_nms_type) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_NONE),          MP_ROM_INT(PY_ML_SCALE_NONE) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_0_1),           MP_ROM_INT(PY_ML_SCALE_0_1) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_S1_1),          MP_ROM_INT(PY_ML_SCALE_S1_1) },
    { MP_ROM_QSTR(MP_QSTR_SCALE_S128_127),      MP_ROM_INT(PY_ML_SCALE_S128_127) },
};

static MP_DEFINE_CONST_DICT(py_ml_globals_dict, py_ml_globals_dict_table);

const mp_obj_module_t ml_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &py_ml_globals_dict
};

// Alias for backwards compatibility
MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_tf, ml_module);
MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_ml, ml_module);
#endif // IMLIB_ENABLE_TFLM
