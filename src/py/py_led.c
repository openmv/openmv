#include <libmp.h>
#include "led.h"
#include "py_led.h"

struct sym_entry {
    const char *sym;
    int val;
} static constants[] = {
    {"RED",     LED_RED},
    {"GREEN",   LED_GREEN},
    {"BLUE",    LED_BLUE},
    {NULL}
};

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

mp_obj_t py_led_init()
{
    /* Init LED */
    led_init(LED_BLUE);

    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("led"));
    rt_store_attr(m, qstr_from_str("on"), rt_make_function_n(1, py_led_on));
    rt_store_attr(m, qstr_from_str("off"), rt_make_function_n(1, py_led_off));
    rt_store_attr(m, qstr_from_str("toggle"), rt_make_function_n(1, py_led_toggle));

    /* Store module attributes */
    for (struct sym_entry *p = constants; p->sym != NULL; p++) {
        rt_store_attr(m, QSTR_FROM_STR_STATIC(p->sym), MP_OBJ_NEW_SMALL_INT((machine_int_t)p->val));
    }

    return m;
}
