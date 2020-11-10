/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Micro Speech Python module.
 */
#include <mp.h>
#include "systick.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py/binary.h"
#include "fb_alloc.h"
#include "omv_boardconfig.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "libtf.h"

#if MICROPY_PY_MICRO_SPEECH
#define kMaxAudioSampleSize     (512)
#define kAudioSampleFrequency   (16000)
// The following values are derived from values used during model training.
// If you change the way you preprocess the input, update all these constants.
#define kFeatureSliceSize       (40)
#define kFeatureSliceCount      (49)
#define kFeatureElementCount    (kFeatureSliceSize * kFeatureSliceCount)
#define kFeatureSliceStrideMs   (20)
#define kFeatureSliceDurationMs (30)

#define RAISE_OS_EXCEPTION(msg)     nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, msg))
typedef struct _py_micro_speech_obj {
    mp_obj_base_t base;
    uint32_t n_slices;
    int8_t spectrogram[kFeatureElementCount];
} py_micro_speech_obj_t;

static const mp_obj_type_t py_micro_speech_type;

static void py_micro_speech_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    //py_micro_speech_obj_t *microspeech = MP_OBJ_TO_PTR(self_in);
    printf("micro speech object\n");
}

mp_obj_t py_micro_speech_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    // check arguments
    //mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    py_micro_speech_obj_t *o = m_new_obj(py_micro_speech_obj_t);
    o->base.type = &py_micro_speech_type;
    o->n_slices = 0;
    memset(o->spectrogram, 0, kFeatureElementCount);
    if (libtf_initialize_micro_features() != 0) {
        RAISE_OS_EXCEPTION("Failed to initialize micro features!");
    }
    return MP_OBJ_FROM_PTR(o);
}

mp_obj_t py_micro_speech_audio_callback(mp_obj_t self_in, mp_obj_t buf_in)
{
    py_micro_speech_obj_t *microspeech = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t pcmbuf;
    mp_get_buffer_raise(buf_in, &pcmbuf, MP_BUFFER_READ);
    //size_t typesize = mp_binary_get_size('@', pcmbuf.typecode, NULL);

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
        slice_index = (kFeatureSliceCount -  1);
        memmove(microspeech->spectrogram,
                microspeech->spectrogram + kFeatureSliceSize,
                kFeatureElementCount - kFeatureSliceSize);
    }
    printf("slice index %ld\n", slice_index);
    size_t num_samples_read;
    int8_t *new_slice = microspeech->spectrogram + (slice_index * kFeatureSliceSize);
    if (libtf_generate_micro_features((int16_t*) pcmbuf.buf,
                kMaxAudioSampleSize, kFeatureSliceSize, new_slice, &num_samples_read)) {
        RAISE_OS_EXCEPTION("Feature generation failed!");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_micro_speech_audio_callback_obj, py_micro_speech_audio_callback);

STATIC const mp_rom_map_elem_t py_micro_speech_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_audio_callback),      MP_ROM_PTR(&py_micro_speech_audio_callback_obj) },
    // class constants
};
STATIC MP_DEFINE_CONST_DICT(py_micro_speech_locals_dict, py_micro_speech_locals_dict_table);

static const mp_obj_type_t py_micro_speech_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroSpeech,
    .print = py_micro_speech_print,
    .make_new = py_micro_speech_make_new,
    .locals_dict = (mp_obj_dict_t*)&py_micro_speech_locals_dict,
};

STATIC const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_micro_speech) },
    { MP_ROM_QSTR(MP_QSTR_MicroSpeech),     MP_ROM_PTR(&py_micro_speech_type) },
};

STATIC MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

const mp_obj_module_t micro_speech_module = {
    .base    = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&module_globals,
};
#endif //MICROPY_PY_MICRO_SPEECH
