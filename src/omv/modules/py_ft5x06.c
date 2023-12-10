/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FT5X06 touch panel Python module.
 */
#include "omv_boardconfig.h"

#if OMV_FT5X06_ENABLE

#include "py/obj.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "extmod/machine_i2c.h"

#include "py_helper.h"
#include "omv_gpio.h"
#include "py_display.h"

#define FT5X06_I2C_ADDR     0x38
#define NUM_TOUCH_POINTS    5

typedef enum ft5x06_gesture {
    FT5X06_GESTURE_MOVE_UP    = 0x1C, // Rotated by 90 degrees - 0x10,
    FT5X06_GESTURE_MOVE_LEFT  = 0x10, // Rotated by 90 degrees - 0x14,
    FT5X06_GESTURE_MOVE_DOWN  = 0x14, // Rotated by 90 degrees - 0x18,
    FT5X06_GESTURE_MOVE_RIGHT = 0x18, // Rotated by 90 degrees - 0x1C,
    FT5X06_GESTURE_ZOOM_IN    = 0x48,
    FT5X06_GESTURE_ZOOM_OUT   = 0x49,
    FT5X06_GESTURE_NONE       = 0x00
} ft5x06_gesture_t;

typedef enum ft5x06_event {
    FT5X06_EVENT_PUT_DOWN     = 0x0,
    FT5X06_EVENT_PUT_UP       = 0x1,
    FT5X06_EVENT_CONTACT      = 0x2
} ft5x06_event_t;

typedef struct _py_ft5x06_obj_t {
    mp_obj_base_t base;
    mp_obj_t i2c_bus;
    uint8_t i2c_addr;
    mp_obj_t touch_callback;
    volatile uint8_t touch_gesture;
    volatile uint8_t touch_points;
    volatile uint8_t touch_points_old;
    volatile uint8_t touch_flag[NUM_TOUCH_POINTS];
    volatile uint8_t touch_id[NUM_TOUCH_POINTS];
    volatile uint16_t x[NUM_TOUCH_POINTS];
    volatile uint16_t y[NUM_TOUCH_POINTS];
} py_ft5x06_obj_t;

const mp_obj_type_t py_ft5x06_type;

STATIC mp_obj_t py_ft5x06_update_points(mp_obj_t self_in);

static void ft5x06_extint_callback(mp_obj_t self_in) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->touch_callback != mp_const_none) {
        mp_obj_t tp = py_ft5x06_update_points(self_in);
        mp_call_function_1(self->touch_callback, tp);
    }
}

STATIC mp_obj_t py_ft5x06_get_gesture(mp_obj_t self_in) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->touch_gesture);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_ft5x06_get_gesture_obj, py_ft5x06_get_gesture);

STATIC mp_obj_t py_ft5x06_get_points(mp_obj_t self_in) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->touch_points);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_ft5x06_get_points_obj, py_ft5x06_get_points);

STATIC mp_obj_t py_ft5x06_get_point_flag(mp_obj_t self_in, mp_obj_t index) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int i = mp_obj_get_int(index);

    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(self->touch_flag[i]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_ft5x06_get_point_flag_obj, py_ft5x06_get_point_flag);

STATIC mp_obj_t py_ft5x06_get_point_id(mp_obj_t self_in, mp_obj_t index) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int i = mp_obj_get_int(index);

    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(self->touch_id[i]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_ft5x06_get_point_id_obj, py_ft5x06_get_point_id);

STATIC mp_obj_t py_ft5x06_get_point_x(mp_obj_t self_in, mp_obj_t index) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int i = mp_obj_get_int(index);

    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(self->x[i]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_ft5x06_get_point_x_obj, py_ft5x06_get_point_x);

STATIC mp_obj_t py_ft5x06_get_point_y(mp_obj_t self_in, mp_obj_t index) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int i = mp_obj_get_int(index);

    if ((i < 0) || (NUM_TOUCH_POINTS <= i)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Index out of bounds!"));
    }
    return mp_obj_new_int(self->y[i]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_ft5x06_get_point_y_obj, py_ft5x06_get_point_y);

STATIC mp_obj_t py_ft5x06_callback(mp_obj_t self_in, mp_obj_t cb) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);

    self->touch_callback = cb;
    if (cb == mp_const_none) {
        omv_gpio_irq_enable(OMV_FT5X06_INT_PIN, false);
    } else {
        omv_gpio_config(OMV_FT5X06_INT_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_irq_register(OMV_FT5X06_INT_PIN, ft5x06_extint_callback, self_in);
        omv_gpio_irq_enable(OMV_FT5X06_INT_PIN, true);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_ft5x06_callback_obj, py_ft5x06_callback);

STATIC mp_obj_t py_ft5x06_update_points(mp_obj_t self_in) {
    py_ft5x06_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1, &((mp_machine_i2c_buf_t) {
        .len = 1, .buf = (uint8_t []) {0x01} // addr
    }), MP_MACHINE_I2C_FLAG_STOP) == 1) {
        uint8_t regs[30];

        if (mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1, &((mp_machine_i2c_buf_t) {
            .len = 30, .buf = regs
        }), MP_MACHINE_I2C_FLAG_READ | MP_MACHINE_I2C_FLAG_STOP) == 0) {
            int points = regs[1] & 0xF;
            if (points > NUM_TOUCH_POINTS) {
                points = NUM_TOUCH_POINTS;
            }

            // Update valid touch points...
            for (int i = 0; i < points; i++) {
                self->touch_flag[i] = regs[2 + (i * 6)] >> 6;
                self->touch_id[i] = regs[4 + (i * 6)] >> 4;
                self->x[i] = ((regs[2 + (i * 6)] & 0xF) << 8) | regs[3 + (i * 6)];
                self->y[i] = ((regs[4 + (i * 6)] & 0xF) << 8) | regs[5 + (i * 6)];
            }

            // Reset invalid touch points...
            for (int i = points; i < NUM_TOUCH_POINTS; i++) {
                self->touch_flag[i] = FT5X06_EVENT_PUT_UP;
            }

            // Latch gesture as long as touch is valid.
            if (points && regs[0]) {
                self->touch_gesture = regs[0];
            } else if (!points) {
                self->touch_gesture = FT5X06_GESTURE_NONE;
            }

            // When the number of points increase the result is immediately valid.
            if (points >= self->touch_points) {
                self->touch_points = points;
                // When the number of points decrease track the last valid number of events.
            } else if (points <= self->touch_points_old) {
                self->touch_points = self->touch_points_old;
            }

            self->touch_points_old = points;
            return mp_obj_new_int(self->touch_points);
        }
    }

    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to update the number of touch points!"));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_ft5x06_update_points_obj, py_ft5x06_update_points);

STATIC mp_obj_t py_ft5x06_deinit(mp_obj_t self_in) {
    omv_gpio_irq_enable(OMV_FT5X06_INT_PIN, false);

    omv_gpio_write(OMV_FT5X06_RESET_PIN, 0);
    mp_hal_delay_ms(1);

    omv_gpio_write(OMV_FT5X06_RESET_PIN, 1);
    mp_hal_delay_ms(39);

    omv_gpio_deinit(OMV_FT5X06_INT_PIN);
    omv_gpio_deinit(OMV_FT5X06_RESET_PIN);

    HAL_GPIO_DeInit(OMV_FT5X06_SDA_PIN->gpio, OMV_FT5X06_SDA_PIN->pin_mask);
    HAL_GPIO_DeInit(OMV_FT5X06_SCL_PIN->gpio, OMV_FT5X06_SCL_PIN->pin_mask);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_ft5x06_deinit_obj, py_ft5x06_deinit);

mp_obj_t py_ft5x06_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_i2c_addr };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_i2c_addr, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = FT5X06_I2C_ADDR } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_ft5x06_obj_t *self = m_new_obj_with_finaliser(py_ft5x06_obj_t);
    self->base.type = &py_ft5x06_type;
    self->i2c_addr = args[ARG_i2c_addr].u_int;
    self->i2c_bus = MP_OBJ_TYPE_GET_SLOT(
        &mp_machine_soft_i2c_type, make_new) (&mp_machine_soft_i2c_type, 2, 0, (const mp_obj_t []) {
        (mp_obj_t) OMV_FT5X06_SCL_PIN, (mp_obj_t) OMV_FT5X06_SDA_PIN
    });

    omv_gpio_config(OMV_FT5X06_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_FT5X06_RESET_PIN, 0);
    mp_hal_delay_ms(1);
    omv_gpio_write(OMV_FT5X06_RESET_PIN, 1);
    mp_hal_delay_ms(39);

    if (mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1, &((mp_machine_i2c_buf_t) {
        .len = 2, .buf = (uint8_t []) {0x00, 0x00} // addr, DEVICE_MODE
    }), MP_MACHINE_I2C_FLAG_STOP) != 2) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("TouchPanel init failed."));
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t py_ft5x06_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),                MP_ROM_QSTR(MP_QSTR_ft5x06)                },
    { MP_ROM_QSTR(MP_QSTR___del__),                 MP_ROM_PTR(&py_ft5x06_deinit_obj)          },

    { MP_ROM_QSTR(MP_QSTR_FLAG_PRESSED),            MP_ROM_INT(FT5X06_EVENT_PUT_DOWN)          },
    { MP_ROM_QSTR(MP_QSTR_FLAG_RELEASED),           MP_ROM_INT(FT5X06_EVENT_PUT_UP)            },
    { MP_ROM_QSTR(MP_QSTR_FLAG_MOVED),              MP_ROM_INT(FT5X06_EVENT_CONTACT)           },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_MOVE_UP),         MP_ROM_INT(FT5X06_GESTURE_MOVE_UP)         },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_MOVE_LEFT),       MP_ROM_INT(FT5X06_GESTURE_MOVE_LEFT)       },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_MOVE_DOWN),       MP_ROM_INT(FT5X06_GESTURE_MOVE_DOWN)       },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_MOVE_RIGHT),      MP_ROM_INT(FT5X06_GESTURE_MOVE_RIGHT)      },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_ZOOM_IN),         MP_ROM_INT(FT5X06_GESTURE_ZOOM_IN)         },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_ZOOM_OUT),        MP_ROM_INT(FT5X06_GESTURE_ZOOM_OUT)        },
    { MP_ROM_QSTR(MP_QSTR_GESTURE_NONE),            MP_ROM_INT(FT5X06_GESTURE_NONE)            },

    { MP_ROM_QSTR(MP_QSTR_get_gesture),             MP_ROM_PTR(&py_ft5x06_get_gesture_obj)     },
    { MP_ROM_QSTR(MP_QSTR_get_points),              MP_ROM_PTR(&py_ft5x06_get_points_obj)      },
    { MP_ROM_QSTR(MP_QSTR_get_point_flag),          MP_ROM_PTR(&py_ft5x06_get_point_flag_obj)  },
    { MP_ROM_QSTR(MP_QSTR_get_point_id),            MP_ROM_PTR(&py_ft5x06_get_point_id_obj)    },
    { MP_ROM_QSTR(MP_QSTR_get_point_x),             MP_ROM_PTR(&py_ft5x06_get_point_x_obj)     },
    { MP_ROM_QSTR(MP_QSTR_get_point_y),             MP_ROM_PTR(&py_ft5x06_get_point_y_obj)     },
    { MP_ROM_QSTR(MP_QSTR_touch_callback),          MP_ROM_PTR(&py_ft5x06_callback_obj)        },
    { MP_ROM_QSTR(MP_QSTR_update_points),           MP_ROM_PTR(&py_ft5x06_update_points_obj)   },
};
STATIC MP_DEFINE_CONST_DICT(py_ft5x06_locals_dict, py_ft5x06_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    py_ft5x06_type,
    MP_QSTR_FT5X06,
    MP_TYPE_FLAG_NONE,
    make_new, py_ft5x06_make_new,
    locals_dict, &py_ft5x06_locals_dict
    );

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_ft5x06) },
    { MP_ROM_QSTR(MP_QSTR_FT5X06),      MP_ROM_PTR(&py_ft5x06_type) },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t ft5x06_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_ft5x06, ft5x06_module);
#endif // OMV_FT5X06_ENABLE
