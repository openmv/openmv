/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MJPEG Python module.
 */
#include "imlib_config.h"
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)

#include "py/obj.h"
#include "py/nlr.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"

#include "ff_wrapper.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

static const mp_obj_type_t py_mjpeg_type;

typedef struct py_mjpeg_obj {
    mp_obj_base_t base;
    bool closed;
    int width;
    int height;
    uint32_t frames;
    uint32_t bytes;
    FIL fp;
} py_mjpeg_obj_t;

STATIC py_mjpeg_obj_t *py_mjpeg_obj(mp_obj_t self) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);

    if (arg_mjpeg->closed) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("File closed!"));
    }

    return arg_mjpeg;
}

STATIC void py_mjpeg_print(const mp_print_t *print, mp_obj_t self, mp_print_kind_t kind) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);
    mp_printf(print, "{\"closed\":%s, \"width\":%u, \"height\":%u, \"count\":%u, \"size\":%u}",
              arg_mjpeg->closed ? "\"true\"" : "\"false\"",
              arg_mjpeg->width,
              arg_mjpeg->height,
              arg_mjpeg->frames,
              f_size(&arg_mjpeg->fp));
}

STATIC mp_obj_t py_mjpeg_is_closed(mp_obj_t self) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int(arg_mjpeg->closed);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_mjpeg_is_closed_obj, py_mjpeg_is_closed);

STATIC mp_obj_t py_mjpeg_width(mp_obj_t self) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int(arg_mjpeg->width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_mjpeg_width_obj, py_mjpeg_width);

STATIC mp_obj_t py_mjpeg_height(mp_obj_t self) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int(arg_mjpeg->height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_mjpeg_height_obj, py_mjpeg_height);

STATIC mp_obj_t py_mjpeg_count(mp_obj_t self) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int(arg_mjpeg->frames);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_mjpeg_count_obj, py_mjpeg_count);

STATIC mp_obj_t py_mjpeg_size(mp_obj_t self) {
    py_mjpeg_obj_t *arg_mjpeg = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int(f_size(&arg_mjpeg->fp));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_mjpeg_size_obj, py_mjpeg_size);

STATIC mp_obj_t py_mjpeg_write(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    py_mjpeg_obj_t *arg_mjpeg = py_mjpeg_obj(args[0]);
    image_t *arg_img = py_image_cobj(args[1]);

    int arg_q = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_quality), 90);
    if ((arg_q < 1) || (100 < arg_q)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("1 <= quality <= 100!"));
    }

    rectangle_t arg_roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 3, kw_args, &arg_roi);

    int arg_rgb_channel = py_helper_keyword_int(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_rgb_channel), -1);
    if ((arg_rgb_channel < -1) || (2 < arg_rgb_channel)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("-1 <= rgb_channel <= 2!"));
    }

    int arg_alpha = py_helper_keyword_int(n_args, args, 5, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 256);
    if ((arg_alpha < 0) || (256 < arg_alpha)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= alpha <= 256!"));
    }

    const uint16_t *color_palette = py_helper_keyword_color_palette(n_args, args, 6, kw_args, NULL);
    const uint8_t *alpha_palette = py_helper_keyword_alpha_palette(n_args, args, 7, kw_args, NULL);

    image_hint_t hint = py_helper_keyword_int(n_args, args, 8, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hint), 0);

    mjpeg_write(&arg_mjpeg->fp, arg_mjpeg->width, arg_mjpeg->height, &arg_mjpeg->frames, &arg_mjpeg->bytes,
                arg_img, arg_q, &arg_roi, arg_rgb_channel, arg_alpha,
                color_palette, alpha_palette, hint);

    return args[0];
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_mjpeg_write_obj, 2, py_mjpeg_write);

STATIC mp_obj_t py_mjpeg_sync(mp_obj_t self, mp_obj_t fps_obj) {
    py_mjpeg_obj_t *arg_mjpeg = py_mjpeg_obj(self);
    mjpeg_sync(&arg_mjpeg->fp, &arg_mjpeg->frames, &arg_mjpeg->bytes, mp_obj_get_float(fps_obj));
    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_mjpeg_sync_obj, py_mjpeg_sync);

STATIC mp_obj_t py_mjpeg_close(mp_obj_t self, mp_obj_t fps_obj) {
    py_mjpeg_obj_t *arg_mjpeg = py_mjpeg_obj(self);
    mjpeg_close(&arg_mjpeg->fp, &arg_mjpeg->frames, &arg_mjpeg->bytes, mp_obj_get_float(fps_obj));
    arg_mjpeg->closed = true;
    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_mjpeg_close_obj, py_mjpeg_close);

STATIC mp_obj_t py_mjpeg_open(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    py_mjpeg_obj_t *mjpeg = m_new_obj_with_finaliser(py_mjpeg_obj_t);
    mjpeg->base.type = &py_mjpeg_type;
    mjpeg->closed = false;
    mjpeg->width = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_width), MAIN_FB()->w);
    mjpeg->height = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_height), MAIN_FB()->h);
    mjpeg->frames = 0;
    mjpeg->bytes = 0;
    file_write_open(&mjpeg->fp, mp_obj_str_get_str(args[0]));
    mjpeg_open(&mjpeg->fp, mjpeg->width, mjpeg->height);
    return mjpeg;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_mjpeg_open_obj, 1, py_mjpeg_open);

STATIC const mp_rom_map_elem_t py_mjpeg_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_Mjpeg)          },
    { MP_ROM_QSTR(MP_QSTR___del__),     MP_ROM_PTR(&py_mjpeg_close_obj)     },
    { MP_ROM_QSTR(MP_QSTR_is_closed),   MP_ROM_PTR(&py_mjpeg_is_closed_obj) },
    { MP_ROM_QSTR(MP_QSTR_width),       MP_ROM_PTR(&py_mjpeg_width_obj)     },
    { MP_ROM_QSTR(MP_QSTR_height),      MP_ROM_PTR(&py_mjpeg_height_obj)    },
    { MP_ROM_QSTR(MP_QSTR_count),       MP_ROM_PTR(&py_mjpeg_count_obj)     },
    { MP_ROM_QSTR(MP_QSTR_size),        MP_ROM_PTR(&py_mjpeg_size_obj)      },
    { MP_ROM_QSTR(MP_QSTR_add_frame),   MP_ROM_PTR(&py_mjpeg_write_obj)     },
    { MP_ROM_QSTR(MP_QSTR_write),       MP_ROM_PTR(&py_mjpeg_write_obj)     },
    { MP_ROM_QSTR(MP_QSTR_sync),        MP_ROM_PTR(&py_mjpeg_sync_obj)      },
    { MP_ROM_QSTR(MP_QSTR_close),       MP_ROM_PTR(&py_mjpeg_close_obj)     },
};

STATIC MP_DEFINE_CONST_DICT(py_mjpeg_locals_dict, py_mjpeg_locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    py_mjpeg_type,
    MP_QSTR_Mjpeg,
    MP_TYPE_FLAG_NONE,
    print, py_mjpeg_print,
    locals_dict, &py_mjpeg_locals_dict
    );

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_mjpeg)      },
    { MP_ROM_QSTR(MP_QSTR_mjpeg),       MP_ROM_PTR(&py_mjpeg_open_obj)  },
    { MP_ROM_QSTR(MP_QSTR_Mjpeg),       MP_ROM_PTR(&py_mjpeg_open_obj)  },
    { MP_ROM_QSTR(MP_QSTR_MJPEG),       MP_ROM_PTR(&py_mjpeg_open_obj)  },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t mjpeg_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_mjpeg, mjpeg_module);

#endif // IMLIB_ENABLE_IMAGE_FILE_IO
