#include <libmp.h>
#include "systick.h"
#include "py_time.h"
#include "py_clock.h"

static mp_obj_t py_time_ticks()
{
    return mp_obj_new_int(systick_current_millis());
}

static mp_obj_t py_time_clock()
{
    return py_clock();
}

static mp_obj_t py_time_sleep(mp_obj_t ms)
{
    systick_sleep(mp_obj_get_int(ms));
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_time_ticks_obj, py_time_ticks);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_time_clock_obj, py_time_clock);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_time_sleep_obj, py_time_sleep);

STATIC const mp_map_elem_t module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_time) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ticks),   (mp_obj_t)&py_time_ticks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clock),   (mp_obj_t)&py_time_clock_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep),   (mp_obj_t)&py_time_sleep_obj },
};

STATIC const mp_map_t module_globals = {
    .all_keys_are_qstrs = 1,
    .table_is_fixed_array = 1,
    .used  = sizeof(module_globals_table) / sizeof(mp_map_elem_t),
    .alloc = sizeof(module_globals_table) / sizeof(mp_map_elem_t),
    .table = (mp_map_elem_t*)module_globals_table,
};

static const mp_obj_module_t py_time_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_time,
    .globals = (mp_map_t*)&module_globals,
};

const mp_obj_module_t *py_time_init()
{
    return &py_time_module;
}
