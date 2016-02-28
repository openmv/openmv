/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GIF Python module.
 *
 */
#include "mp.h"
#include "imlib.h"
#include "ff.h"
#include "py_image.h"
#include "py_assert.h"
#include "py_helper.h"
#include "omv_boardconfig.h"

static const mp_obj_type_t py_gif_type;
extern const char *ffs_strerror(FRESULT res);

// Gif class
typedef struct _py_gif_obj_t {
    mp_obj_base_t base;
    int width;
    int height;
    bool loop;
    FIL fp;
} py_gif_obj_t;

static mp_obj_t py_gif_open(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_gif_obj_t *gif;
    gif = m_new_obj(py_gif_obj_t);
    gif->width  = mp_obj_get_int(args[0]);
    gif->height = mp_obj_get_int(args[1]);
    gif->loop   = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_loop), 1);
    gif->base.type = &py_gif_type;

    FRESULT res;
    const char *path = mp_obj_str_get_str(args[2]);
    if ((res = f_open(&gif->fp, path, FA_WRITE|FA_CREATE_ALWAYS)) != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    gif_open(&gif->fp, gif->width, gif->height, gif->loop);
    return gif; 
}

static mp_obj_t py_gif_add_frame(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_gif_obj_t *gif = args[0];
    image_t *image = py_image_cobj(args[1]);
    int delay = py_helper_lookup_int(kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_delay), 0);

    gif_add_frame(&gif->fp, image, delay);
    return mp_const_none;
}

static mp_obj_t py_gif_close(mp_obj_t self)
{
    py_gif_obj_t *gif = self;
    gif_close(&gif->fp);
    return mp_const_none;
}

static void py_gif_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_gif_obj_t *self = self_in;
    mp_printf(print, "<gif width:%d height:%d loop:%d>", self->width, self->height, self->loop);
}


STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_gif_close_obj, py_gif_close);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_gif_open_obj, 3, py_gif_open);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_gif_add_frame_obj, 2, py_gif_add_frame);

static const mp_map_elem_t locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_close),       (mp_obj_t)&py_gif_close_obj     },
    { MP_OBJ_NEW_QSTR(MP_QSTR_add_frame),   (mp_obj_t)&py_gif_add_frame_obj },
    { NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_gif_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Gif,
    .print = py_gif_print,
    .locals_dict = (mp_obj_t)&locals_dict,
};

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_gif)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_Gif),         (mp_obj_t)&py_gif_open_obj  },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t gif_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_image,
    .globals = (mp_obj_t)&globals_dict,
};
