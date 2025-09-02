/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * GIF Python module.
 */
#include "imlib_config.h"
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)

#include "py/mphal.h"
#include "py/runtime.h"

#include "file_utils.h"
#include "framebuffer.h"
#include "imlib.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"

static const mp_obj_type_t py_gif_type;

// Gif object
typedef struct py_gif_obj {
    mp_obj_base_t base;
    uint32_t width;
    uint32_t height;
    bool color;
    bool loop;
    FIL fp;
} py_gif_obj_t;

static void py_gif_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_gif_obj_t *self = self_in;
    mp_printf(print, "<gif width:%d height:%d color:%d loop:%d>", self->width, self->height, self->color, self->loop);
}

static mp_obj_t py_gif_width(mp_obj_t self_in) {
    py_gif_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->width);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_gif_width_obj, py_gif_width);

static mp_obj_t py_gif_height(mp_obj_t self_in) {
    py_gif_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->height);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_gif_height_obj, py_gif_height);

static mp_obj_t py_gif_format(mp_obj_t self_in) {
    py_gif_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->color ? PIXFORMAT_RGB565 : PIXFORMAT_GRAYSCALE);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_gif_format_obj, py_gif_format);

static mp_obj_t py_gif_size(mp_obj_t self_in) {
    py_gif_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(file_size(&self->fp));
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_gif_size_obj, py_gif_size);

static mp_obj_t py_gif_loop(mp_obj_t self_in) {
    py_gif_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->loop);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_gif_loop_obj, py_gif_loop);

static mp_obj_t py_gif_add_frame(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_delay };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_delay, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 10 } },
    };

    // Parse args.
    py_gif_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    image_t *image = py_helper_arg_to_image(pos_args[1], ARG_IMAGE_MUTABLE);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Sanity checks
    if (self->width != image->w || self->height != image->h) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Unexpected image geometry"));
    }
    if (image->pixfmt == PIXFORMAT_JPEG || image->pixfmt == PIXFORMAT_BINARY) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image format is not supported"));
    }

    gif_add_frame(&self->fp, image, args[ARG_delay].u_int);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_gif_add_frame_obj, 2, py_gif_add_frame);

static mp_obj_t py_gif_close(mp_obj_t self_in) {
    py_gif_obj_t *self = MP_OBJ_TO_PTR(self_in);
    gif_close(&self->fp);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_gif_close_obj, py_gif_close);

static mp_obj_t py_gif_open(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_width, ARG_height, ARG_color, ARG_loop };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = -1 } },
        { MP_QSTR_height, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = -1 } },
        { MP_QSTR_color, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = -1 } },
        { MP_QSTR_loop, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_bool = true } },
    };

    // Parse args.
    const char *path = mp_obj_str_get_str(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    py_gif_obj_t *gif = mp_obj_malloc_with_finaliser(py_gif_obj_t, &py_gif_type);

    gif->width = (args[ARG_width].u_int == -1) ? fb->w : args[ARG_width].u_int;
    gif->height = (args[ARG_height].u_int == -1) ? fb->h : args[ARG_height].u_int;
    gif->color = (args[ARG_color].u_int == -1) ? (fb->bpp >= 2) : args[ARG_color].u_bool;
    gif->loop = args[ARG_loop].u_bool;

    file_open(&gif->fp, path, false, FA_WRITE | FA_CREATE_ALWAYS);
    gif_open(&gif->fp, gif->width, gif->height, gif->color, gif->loop);
    return gif;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_gif_open_obj, 1, py_gif_open);

static const mp_rom_map_elem_t py_gif_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_gif)          },
    { MP_ROM_QSTR(MP_QSTR___del__),         MP_ROM_PTR(&py_gif_close_obj)     },

    { MP_OBJ_NEW_QSTR(MP_QSTR_width),       MP_ROM_PTR(&py_gif_width_obj)     },
    { MP_OBJ_NEW_QSTR(MP_QSTR_height),      MP_ROM_PTR(&py_gif_height_obj)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_format),      MP_ROM_PTR(&py_gif_format_obj)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_size),        MP_ROM_PTR(&py_gif_size_obj)      },
    { MP_OBJ_NEW_QSTR(MP_QSTR_loop),        MP_ROM_PTR(&py_gif_loop_obj)      },
    { MP_OBJ_NEW_QSTR(MP_QSTR_add_frame),   MP_ROM_PTR(&py_gif_add_frame_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_close),       MP_ROM_PTR(&py_gif_close_obj)     },
    { NULL, NULL },
};
static MP_DEFINE_CONST_DICT(py_gif_locals_dict, py_gif_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
    py_gif_type,
    MP_QSTR_Gif,
    MP_TYPE_FLAG_NONE,
    print, py_gif_print,
    locals_dict, &py_gif_locals_dict
    );

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_gif) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Gif),         MP_ROM_PTR(&py_gif_open_obj) },
    { NULL, NULL },
};
static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t gif_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_gif, gif_module);
#endif // IMLIB_ENABLE_IMAGE_FILE_IO
