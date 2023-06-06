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
#include "omv_boardconfig.h"

#if MICROPY_PY_LCD
#include STM32_HAL_H

#include "py/obj.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "extmod/machine_i2c.h"

#include "py_helper.h"
#include "py_lcd_touch.h"
#include "omv_gpio.h"

#ifdef OMV_TOUCH_PRESENT
#define FT5X06_I2C_ADDR 0x38
static mp_obj_base_t *lcd_touch_bus = NULL;
static mp_obj_t lcd_touch_user_cb = NULL;

#define NUM_TOUCH_POINTS 5
static volatile uint8_t lcd_touch_gesture = 0;
static volatile uint8_t lcd_touch_points = 0, lcd_touch_points_old = 0;
static volatile uint8_t lcd_touch_flag[NUM_TOUCH_POINTS] = {};
static volatile uint8_t lcd_touch_id[NUM_TOUCH_POINTS] = {};
static volatile uint16_t lcd_touch_x_position[NUM_TOUCH_POINTS] = {};
static volatile uint16_t lcd_touch_y_position[NUM_TOUCH_POINTS] = {};

mp_obj_t lcd_touch_get_gesture()
{
    return mp_obj_new_int(lcd_touch_gesture);
}

mp_obj_t lcd_touch_get_points()
{
    return mp_obj_new_int(lcd_touch_points);
}

mp_obj_t lcd_touch_get_point_flag(mp_obj_t index)
{
    int i = mp_obj_get_int(index);
    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(lcd_touch_flag[i]);
}

mp_obj_t lcd_touch_get_point_id(mp_obj_t index)
{
    int i = mp_obj_get_int(index);
    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(lcd_touch_id[i]);
}

mp_obj_t lcd_touch_get_point_x_position(mp_obj_t index)
{
    int i = mp_obj_get_int(index);
    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(lcd_touch_x_position[i]);
}

mp_obj_t lcd_touch_get_point_y_position(mp_obj_t index)
{
    int i = mp_obj_get_int(index);
    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(lcd_touch_y_position[i]);
}

mp_obj_t lcd_touch_update_touch_points()
{
    mp_obj_base_t *bus = lcd_touch_bus ? lcd_touch_bus : ((mp_obj_base_t *) MP_OBJ_TYPE_GET_SLOT(
                &mp_machine_soft_i2c_type, make_new)(&mp_machine_soft_i2c_type, 2, 0, (const mp_obj_t []) {
        (mp_obj_t) OMV_TOUCH_SCL_PIN, (mp_obj_t) OMV_TOUCH_SDA_PIN
    }));

    if (mp_machine_soft_i2c_transfer(bus, FT5X06_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
        .len = 1, .buf = (uint8_t []) {0x01} // addr
    }), MP_MACHINE_I2C_FLAG_STOP) == 1) {
        uint8_t regs[30];

        if (mp_machine_soft_i2c_transfer(bus, FT5X06_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
            .len = 30, .buf = regs
        }), MP_MACHINE_I2C_FLAG_READ | MP_MACHINE_I2C_FLAG_STOP) == 0) {
            int points = regs[1] & 0xF;
            if (points > NUM_TOUCH_POINTS) points = NUM_TOUCH_POINTS;

            // Update valid touch points...
            for (int i = 0; i < points; i++) {
                lcd_touch_flag[i] = regs[2 + (i * 6)] >> 6;
                lcd_touch_id[i] = regs[4 + (i * 6)] >> 4;
                lcd_touch_x_position[i] = ((regs[2 + (i * 6)] & 0xF) << 8) | regs[3 + (i * 6)];
                lcd_touch_y_position[i] = ((regs[4 + (i * 6)] & 0xF) << 8) | regs[5 + (i * 6)];
            }

            // Reset invalid touch points...
            for (int i = points; i < NUM_TOUCH_POINTS; i++) {
                lcd_touch_flag[i] = PY_LCD_TOUCH_EVENT_PUT_UP;
            }

            // Latch gesture as long as touch is valid.
            if (points && regs[0]) lcd_touch_gesture = regs[0];
            else if (!points) lcd_touch_gesture = PY_LCD_TOUCH_GESTURE_NONE;

            // When the number of points increase the result is immediately valid.
            if (points >= lcd_touch_points) {
                lcd_touch_points = points;
            // When the number of points decrease track the last valid number of events.
            } else if (points <= lcd_touch_points_old) {
                lcd_touch_points = lcd_touch_points_old;
            }

            lcd_touch_points_old = points;
            return mp_obj_new_int(lcd_touch_points);
        }
    }

    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to update the number of touch points!"));
}

static void lcd_touch_extint_callback(omv_gpio_t pin, void *data)
{
    if (lcd_touch_user_cb) {
        mp_call_function_1(lcd_touch_user_cb, lcd_touch_update_touch_points());
    }
}

void lcd_touch_deinit()
{
    omv_gpio_irq_enable(OMV_TOUCH_INT_PIN, false);

    lcd_touch_user_cb = NULL;
    lcd_touch_bus = NULL;
    lcd_touch_gesture = 0;
    lcd_touch_points = lcd_touch_points_old = 0;
    memset((void *) lcd_touch_flag, 0, sizeof(lcd_touch_flag));
    memset((void *) lcd_touch_id, 0, sizeof(lcd_touch_id));
    memset((void *) lcd_touch_x_position, 0, sizeof(lcd_touch_x_position));
    memset((void *) lcd_touch_y_position, 0, sizeof(lcd_touch_y_position));

    omv_gpio_write(OMV_TOUCH_RESET_PIN, 0);
    mp_hal_delay_ms(1);

    omv_gpio_write(OMV_TOUCH_RESET_PIN, 1);
    mp_hal_delay_ms(39);

    omv_gpio_deinit(OMV_TOUCH_INT_PIN);
    omv_gpio_deinit(OMV_TOUCH_RESET_PIN);
    HAL_GPIO_DeInit(OMV_TOUCH_SDA_PIN->gpio, OMV_TOUCH_SDA_PIN->pin_mask);
    HAL_GPIO_DeInit(OMV_TOUCH_SCL_PIN->gpio, OMV_TOUCH_SCL_PIN->pin_mask);
}

void lcd_touch_init()
{
    omv_gpio_config(OMV_TOUCH_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_TOUCH_RESET_PIN, 0);
    mp_hal_delay_ms(1);
    omv_gpio_write(OMV_TOUCH_RESET_PIN, 1);
    mp_hal_delay_ms(39);

    lcd_touch_bus = (mp_obj_base_t *) MP_OBJ_TYPE_GET_SLOT(
            &mp_machine_soft_i2c_type, make_new)(&mp_machine_soft_i2c_type, 2, 0, (const mp_obj_t []) {
        (mp_obj_t) OMV_TOUCH_SCL_PIN, (mp_obj_t) OMV_TOUCH_SDA_PIN
    });

    if (mp_machine_soft_i2c_transfer(lcd_touch_bus, FT5X06_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
        .len = 2, .buf = (uint8_t []) {0x00, 0x00} // addr, DEVICE_MODE
    }), MP_MACHINE_I2C_FLAG_STOP) == 2) {
        omv_gpio_config(OMV_TOUCH_INT_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_irq_register(OMV_TOUCH_INT_PIN, lcd_touch_extint_callback, NULL);
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Touch init failed!"));
    }
}

void lcd_touch_register_touch_cb(mp_obj_t cb)
{
    omv_gpio_irq_enable(OMV_TOUCH_INT_PIN, false);
    lcd_touch_user_cb = cb;
    if (cb != mp_const_none) {
        omv_gpio_irq_enable(OMV_TOUCH_INT_PIN, true);
    }
}
#endif // OMV_TOUCH_PRESENT

#endif // MICROPY_PY_LCD
