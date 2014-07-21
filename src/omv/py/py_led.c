#include "mp.h"
#include "led.h"
#include "py_led.h"
static mp_obj_t py_led_on(mp_obj_t led_id) {
    led_state(mp_obj_get_int(led_id), 1);
    return mp_const_none;
}

static mp_obj_t py_led_off(mp_obj_t led_id) {
    led_state(mp_obj_get_int(led_id), 0);
    return mp_const_none;
}

static mp_obj_t py_led_toggle(mp_obj_t led_id) {
    led_toggle(mp_obj_get_int(led_id));
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_led_on_obj,     py_led_on);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_led_off_obj,    py_led_off);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_led_toggle_obj, py_led_toggle);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_led) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RED),     MP_OBJ_NEW_SMALL_INT(LED_RED)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_GREEN),   MP_OBJ_NEW_SMALL_INT(LED_GREEN)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_BLUE),    MP_OBJ_NEW_SMALL_INT(LED_BLUE)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_IR),      MP_OBJ_NEW_SMALL_INT(LED_IR)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_on),      (mp_obj_t)&py_led_on_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_off),     (mp_obj_t)&py_led_off_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_toggle),  (mp_obj_t)&py_led_toggle_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

static const mp_obj_module_t py_led_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_led,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_led_init()
{
    /* Init LED */
    led_init(LED_BLUE);
    return &py_led_module;
}
