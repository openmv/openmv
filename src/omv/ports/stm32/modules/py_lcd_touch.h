/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LCD Python module.
 */
#ifndef __PY_LCD_TOUCH_H__
#define __PY_LCD_TOUCH_H__
typedef enum py_lcd_touch_gesture {
    PY_LCD_TOUCH_GESTURE_MOVE_UP    = 0x1C, // Rotated by 90 degrees - 0x10,
    PY_LCD_TOUCH_GESTURE_MOVE_LEFT  = 0x10, // Rotated by 90 degrees - 0x14,
    PY_LCD_TOUCH_GESTURE_MOVE_DOWN  = 0x14, // Rotated by 90 degrees - 0x18,
    PY_LCD_TOUCH_GESTURE_MOVE_RIGHT = 0x18, // Rotated by 90 degrees - 0x1C,
    PY_LCD_TOUCH_GESTURE_ZOOM_IN    = 0x48,
    PY_LCD_TOUCH_GESTURE_ZOOM_OUT   = 0x49,
    PY_LCD_TOUCH_GESTURE_NONE       = 0x00
} py_lcd_touch_gesture_t;

typedef enum py_lcd_touch_event {
    PY_LCD_TOUCH_EVENT_PUT_DOWN = 0x0,
    PY_LCD_TOUCH_EVENT_PUT_UP   = 0x1,
    PY_LCD_TOUCH_EVENT_CONTACT  = 0x2
} py_lcd_touch_event_t;

void lcd_touch_init();
void lcd_touch_deinit();
mp_obj_t lcd_touch_update_touch_points();
void lcd_touch_register_touch_cb(mp_obj_t cb);
mp_obj_t lcd_touch_get_gesture();
mp_obj_t lcd_touch_get_points();
mp_obj_t lcd_touch_get_point_flag(mp_obj_t index);
mp_obj_t lcd_touch_get_point_id(mp_obj_t index);
mp_obj_t lcd_touch_get_point_x_position(mp_obj_t index);
mp_obj_t lcd_touch_get_point_y_position(mp_obj_t index);
#endif // __PY_LCD_TOUCH_H__
