/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LED Python module.
 *
 */
#ifndef __PY_LED_H__
#define __PY_LED_H__
enum led_id {
    LED_RED=0,
    LED_GREEN,
    LED_BLUE,
#ifdef OPENMV2
    LED_IR,
#endif
    LED_MAX,
};

void led_init(enum led_id color);
void led_toggle(enum led_id color);
void led_state(enum led_id color, int state);
const mp_obj_module_t *py_led_init();
#endif // __PY_LED_H__
