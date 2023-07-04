/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Micro Speech Python module.
 */
#include <stdio.h>
#include "py/obj.h"
#include "py/objarray.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "systick.h"
#include "py/binary.h"

#include "py_assert.h"
#include "py_helper.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "libtf.h"
#include "py_tf.h"
#include "omv_common.h"

#if MICROPY_PY_MICRO_SPEECH
#define kMaxAudioSampleSize        (512)
#define kAudioSampleFrequency      (16000)
// The following values are derived from values used during model training.
// If you change the way you preprocess the input, update all these constants.
#define kFeatureSliceSize          (40)
#define kFeatureSliceCount         (49)
#define kFeatureElementCount       (kFeatureSliceSize * kFeatureSliceCount)
#define kFeatureSliceStrideMs      (20)
#define kFeatureSliceDurationMs    (30)
#define kCategoryCount             (4)
#define kAverageWindowSamples      (1020 / kFeatureSliceDurationMs)
#define RAISE_OS_EXCEPTION(msg)    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(msg))

typedef struct _py_micro_speech_obj {
    mp_obj_base_t base;
    uint32_t n_slices;
    bool new_slices;
    int8_t spectrogram[kFeatureElementCount];
} py_micro_speech_obj_t;

static const mp_obj_type_t py_micro_speech_type;

static void py_micro_speech_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_micro_speech_obj_t *microspeech = MP_OBJ_TO_PTR(self_in);
    printf("MicroSpeech obj n_slices: %lu new_slices :%d!\n",
           microspeech->n_slices, microspeech->new_slices);
}

mp_obj_t py_micro_speech_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    //mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    py_micro_speech_obj_t *o = m_new_obj(py_micro_speech_obj_t);
    o->base.type = &py_micro_speech_type;
    o->n_slices = 0;
    o->new_slices = false;
    memset(o->spectrogram, 0, kFeatureElementCount);
    if (libtf_initialize_micro_features() != 0) {
        RAISE_OS_EXCEPTION("Failed to initialize micro features!");
    }
    return MP_OBJ_FROM_PTR(o);
}

mp_obj_t py_micro_speech_audio_callback(mp_obj_t self_in, mp_obj_t buf_in) {
    py_micro_speech_obj_t *microspeech = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t pcmbuf;
    mp_get_buffer_raise(buf_in, &pcmbuf, MP_BUFFER_READ);

    if ((pcmbuf.len / 2) != kMaxAudioSampleSize) {
        RAISE_OS_EXCEPTION("Audio data size too small!");
    }

    uint32_t slice_index = 0;
    if (microspeech->n_slices < kFeatureSliceCount) {
        slice_index = microspeech->n_slices++;
    } else {
        // Spectrogram is full, move old data up in the spectrogram
        // and put the new slice at the end of the spectrogram.
        // +-----------+             +-----------+
        // | data@20ms |         --> | data@40ms |
        // +-----------+       --    +-----------+
        // | data@40ms |     --  --> | data@60ms |
        // +-----------+   --  --    +-----------+
        // | data@60ms | --  --      | data@80ms |
        // +-----------+   --        +-----------+
        // | data@80ms | --          |<new slice>|
        // +-----------+             +-----------+
        slice_index = (kFeatureSliceCount - 1);
        memmove(microspeech->spectrogram,
                microspeech->spectrogram + kFeatureSliceSize,
                kFeatureElementCount - kFeatureSliceSize);
        microspeech->new_slices = true;
    }

    size_t num_samples_read;
    int8_t *new_slice = microspeech->spectrogram + (slice_index * kFeatureSliceSize);
    if (libtf_generate_micro_features((int16_t *) pcmbuf.buf,
                                      kMaxAudioSampleSize, kFeatureSliceSize, new_slice, &num_samples_read)) {
        RAISE_OS_EXCEPTION("Feature generation failed!");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_micro_speech_audio_callback_obj, py_micro_speech_audio_callback);

STATIC void py_tf_input_callback(void *callback_data, void *model_input, libtf_parameters_t *params) {
    // Copy feature buffer to input tensor
    for (int i = 0; i < kFeatureElementCount; i++) {
        ((int8_t *) model_input)[i] = ((int8_t *) callback_data)[i];
    }
}

STATIC void py_tf_output_callback(void *callback_data, void *model_output, libtf_parameters_t *params) {
    uint8_t *scores = (uint8_t *) callback_data;

    if (params->output_height != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output height to be 1!"));
    }

    if (params->output_width != 1) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output width to be 1!"));
    }

    if (params->output_channels != 4) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected model output channels to be 4!"));
    }

    for (int i = 0, ii = params->output_channels; i < ii; i++) {
        scores[i] = ((uint8_t *) model_output)[i] - params->output_zero_point;
        debug_printf("%.2f ", (double) ((((uint8_t *) model_output)[i] - params->output_zero_point) * params->output_scale));
    }
}

STATIC mp_obj_t py_micro_speech_listen(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    py_micro_speech_obj_t *microspeech = args[0];
    py_tf_model_obj_t *arg_model = args[1];
    float threshold = py_helper_keyword_float(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_threshold), 0.9f);
    uint32_t timeout = py_helper_keyword_int(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_timeout), 1000);
    size_t labels_filter_len = 0;
    mp_obj_t *labels_filter = py_helper_keyword_iterable(n_args, args,
                                                         4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_filter), &labels_filter_len);

    fb_alloc_mark();
    py_tf_alloc_putchar_buffer();

    uint32_t tensor_arena_size;
    uint8_t *tensor_arena = fb_alloc_all(&tensor_arena_size, FB_ALLOC_PREFER_SIZE);
    libtf_parameters_t params;

    if (libtf_get_parameters(arg_model->model_data, tensor_arena, tensor_arena_size, &params) != 0) {
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
    }

    fb_free(); // free fb_alloc_all()

    tensor_arena = fb_alloc(params.tensor_arena_size, FB_ALLOC_PREFER_SPEED | FB_ALLOC_CACHE_ALIGN);
    int8_t spectrogram[kFeatureElementCount];

    uint32_t return_label = 0;
    uint32_t results_count = 0;
    uint8_t previous_scores[kAverageWindowSamples][kCategoryCount];
    uint32_t average_scores[kCategoryCount];
    memset(previous_scores, 0, kAverageWindowSamples * kCategoryCount);
    memset(average_scores, 0, kCategoryCount * sizeof(*average_scores));

    uint32_t start = HAL_GetTick();
    while (timeout == 0 || (HAL_GetTick() - start) < timeout) {
        __WFI();

        if (microspeech->new_slices == false) {
            continue;
        }

        // Copy spectrogram atomically
        __disable_irq();
        microspeech->new_slices = false;
        memcpy(spectrogram, microspeech->spectrogram, kFeatureElementCount);
        __enable_irq();

        // Run model on updated spectrogram
        if (libtf_invoke(arg_model->model_data,
                         tensor_arena,
                         &params,
                         py_tf_input_callback,
                         spectrogram,
                         py_tf_output_callback,
                         previous_scores[results_count]) != 0) {
            mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) py_tf_putchar_buffer);
        }

        // If we have enough samples calculate average scores.
        if ((HAL_GetTick() - start) > (kAverageWindowSamples * kFeatureSliceDurationMs)) {
            uint32_t highest_index = 0, highest_score = 0;

            // Re/Calculate the average score for all labels in the window.
            for (int i = 0; i < kAverageWindowSamples; i++) {
                for (int c = 0; c < kCategoryCount; c++) {
                    if (i == 0) {
                        average_scores[c] = previous_scores[i][c];
                    } else {
                        average_scores[c] += previous_scores[i][c];
                    }
                }
            }

            // Find the label index with the highest average score.
            for (int i = 0; i < kCategoryCount; i++) {
                if (average_scores[i] > highest_score) {
                    highest_index = i;
                    highest_score = average_scores[i];
                }
            }

            // If the highest average score is higher than the threshold return a command.
            if (average_scores[highest_index] / (kAverageWindowSamples * 255.0f) > threshold) {
                bool command_filtered = (labels_filter != NULL);

                // If a list of labels is provided to filter commands, check if the
                // detected command is in that list, otherwise continue the detection.
                if (labels_filter != NULL) {
                    for (int i = 0; i < labels_filter_len; i++) {
                        if (highest_index == mp_obj_get_int(labels_filter[i])) {
                            command_filtered = false;
                            break;
                        }
                    }
                }

                if (command_filtered == false) {
                    return_label = highest_index;
                    // Clear spectrogram
                    __disable_irq();
                    microspeech->n_slices = 0;
                    microspeech->new_slices = false;
                    __enable_irq();
                    break;
                }
            }
        }

        results_count = (results_count + 1) % kAverageWindowSamples;
    }

    fb_alloc_free_till_mark();
    return mp_obj_new_int(return_label);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_micro_speech_listen_obj, 2, py_micro_speech_listen);


STATIC const mp_rom_map_elem_t py_micro_speech_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_audio_callback),      MP_ROM_PTR(&py_micro_speech_audio_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR_listen),              MP_ROM_PTR(&py_micro_speech_listen_obj) },
    // class constants
};
STATIC MP_DEFINE_CONST_DICT(py_micro_speech_locals_dict, py_micro_speech_locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    py_micro_speech_type,
    MP_QSTR_MicroSpeech,
    MP_TYPE_FLAG_NONE,
    print, py_micro_speech_print,
    make_new, py_micro_speech_make_new,
    locals_dict, &py_micro_speech_locals_dict
    );

STATIC const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_micro_speech) },
    { MP_ROM_QSTR(MP_QSTR_MicroSpeech),     MP_ROM_PTR(&py_micro_speech_type) },
};

STATIC MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

const mp_obj_module_t micro_speech_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_micro_speech, micro_speech_module);

#endif // MICROPY_PY_MICRO_SPEECH
