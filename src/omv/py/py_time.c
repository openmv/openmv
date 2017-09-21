/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Time Python module.
 *
 */
#include <mp.h>
#include "systick.h"
#include "py_time.h"

/* Clock Type */
typedef struct _py_clock_obj_t {
    mp_obj_base_t base;
    uint32_t t_start;
    uint32_t t_ticks;
    uint32_t t_frame;
} py_clock_obj_t;

mp_obj_t py_clock_tick(mp_obj_t clock_obj)
{
    py_clock_obj_t *clock = (py_clock_obj_t*) clock_obj;
    clock->t_start = systick_current_millis();
    return mp_const_none;
}

mp_obj_t py_clock_fps(mp_obj_t clock_obj)
{
    py_clock_obj_t *clock = (py_clock_obj_t*) clock_obj;
    clock->t_frame++;
    clock->t_ticks += (systick_current_millis()-clock->t_start);
    float fps = 1000.0f / (clock->t_ticks/(float)clock->t_frame);
    if (clock->t_ticks >= 2000) {
        // Reset the FPS clock every 2s
        clock->t_frame = 0;
        clock->t_ticks = 0;
    }
    return mp_obj_new_float(fps);
}

mp_obj_t py_clock_avg(mp_obj_t clock_obj)
{
    py_clock_obj_t *clock = (py_clock_obj_t*) clock_obj;
    clock->t_frame++;
    clock->t_ticks += (systick_current_millis()-clock->t_start);
    return mp_obj_new_float(clock->t_ticks/(float)clock->t_frame);
}

mp_obj_t py_clock_reset(mp_obj_t clock_obj)
{
    py_clock_obj_t *clock = (py_clock_obj_t*) clock_obj;
    clock->t_start = 0;
    clock->t_ticks = 0;
    clock->t_frame = 0;
    return mp_const_none;
}

static void py_clock_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_clock_obj_t *self = self_in;

    /* print some info */
    mp_printf(print, "t_start:%d t_ticks:%d t_frame:%d\n",
            self->t_start, self->t_ticks, self->t_frame);
}

static MP_DEFINE_CONST_FUN_OBJ_1(py_clock_tick_obj,  py_clock_tick);
static MP_DEFINE_CONST_FUN_OBJ_1(py_clock_fps_obj,   py_clock_fps);
static MP_DEFINE_CONST_FUN_OBJ_1(py_clock_avg_obj,   py_clock_avg);
static MP_DEFINE_CONST_FUN_OBJ_1(py_clock_reset_obj, py_clock_reset);

static const mp_map_elem_t locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick),   (mp_obj_t)&py_clock_tick_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_fps),    (mp_obj_t)&py_clock_fps_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_avg),    (mp_obj_t)&py_clock_avg_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset),  (mp_obj_t)&py_clock_reset_obj},
    { NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_clock_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Clock,
    .print = py_clock_print,
    .locals_dict = (mp_obj_t)&locals_dict,
};

static mp_obj_t py_time_ticks()
{
    return mp_obj_new_int(systick_current_millis());
}

static mp_obj_t py_time_sleep(mp_obj_t ms)
{
    systick_sleep(mp_obj_get_int(ms));
    return mp_const_none;
}

static mp_obj_t py_time_clock()
{
    py_clock_obj_t *clock =NULL;
    clock = m_new_obj(py_clock_obj_t);
    clock->base.type = &py_clock_type;
    clock->t_start = 0;
    clock->t_ticks = 0;
    clock->t_frame = 0;

    return clock;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_time_ticks_obj, py_time_ticks);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_time_sleep_obj, py_time_sleep);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_time_clock_obj, py_time_clock);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_time) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ticks),   (mp_obj_t)&py_time_ticks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep),   (mp_obj_t)&py_time_sleep_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clock),   (mp_obj_t)&py_time_clock_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t time_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};
