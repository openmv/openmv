/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * TFP410 DVI serializer module.
 */
#include "omv_boardconfig.h"

#if OMV_TFP410_ENABLE
#include "py/obj.h"
#include "py/objarray.h"
#include "py/runtime.h"
#include "mphal.h"
#include "extmod/machine_i2c.h"

#include "omv_gpio.h"
#include "fb_alloc.h"
#include "cec.h"

#define TFP410_I2C_ADDR    0x3F

typedef struct _py_tfp410_obj_t {
    mp_obj_base_t base;
    mp_obj_t i2c_bus;
    uint8_t i2c_addr;
    mp_obj_t hotplug_callback;
} py_tfp410_obj_t;

const mp_obj_type_t py_tfp410_type;

static int dvi_is_connected(py_tfp410_obj_t *self, bool *connected) {
    if (mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1,
                                     &((mp_machine_i2c_buf_t) {.len = 1, .buf = (uint8_t []) {0x09}}), 0) == 1) {
        uint8_t reg;
        if ((mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1,
                                          &((mp_machine_i2c_buf_t) { .len = 1, .buf = &reg }),
                                          MP_MACHINE_I2C_FLAG_READ | MP_MACHINE_I2C_FLAG_STOP) == 0)
            && (mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1, &((mp_machine_i2c_buf_t) {
            .len = 2, .buf = (uint8_t []) {0x09, 0x19} // clear interrupt flag
        }), MP_MACHINE_I2C_FLAG_STOP) == 2)) {
            *connected = (reg & 2);
            return 0;
        }
    }
    // Generate stop on error
    mp_machine_soft_i2c_transfer(self->i2c_bus, self->i2c_addr, 1,
                                 &((mp_machine_i2c_buf_t) {.len = 0, .buf = NULL }), MP_MACHINE_I2C_FLAG_STOP);
    return -1;
}

static void dvi_extint_callback(mp_obj_t self_in) {
    py_tfp410_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->hotplug_callback != mp_const_none) {
        bool connected;
        if (dvi_is_connected(self, &connected) != -1) {
            mp_call_function_1(self->hotplug_callback, mp_obj_new_bool(connected));
        }
    }
}

STATIC mp_obj_t py_dvi_is_connected(mp_obj_t self_in) {
    py_tfp410_obj_t *self = MP_OBJ_TO_PTR(self_in);

    bool connected;
    if (dvi_is_connected(self, &connected) != -1) {
        return mp_obj_new_bool(connected);
    }
    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Display init failed!"));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_dvi_is_connected_obj, py_dvi_is_connected);

STATIC mp_obj_t py_dvi_hotplug_callback(mp_obj_t self_in, mp_obj_t cb) {
    py_tfp410_obj_t *self = MP_OBJ_TO_PTR(self_in);

    self->hotplug_callback = cb;
    if (cb == mp_const_none) {
        omv_gpio_irq_enable(OMV_TFP410_INT_PIN, false);
    } else {
        omv_gpio_config(OMV_TFP410_INT_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
        omv_gpio_irq_register(OMV_TFP410_INT_PIN, dvi_extint_callback, self_in);
        omv_gpio_irq_enable(OMV_TFP410_INT_PIN, true);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_dvi_hotplug_callback_obj, py_dvi_hotplug_callback);

mp_obj_t py_tfp410_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_i2c_addr };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_i2c_addr, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = TFP410_I2C_ADDR } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_tfp410_obj_t *self = m_new_obj_with_finaliser(py_tfp410_obj_t);
    self->base.type = &py_tfp410_type;
    self->hotplug_callback = mp_const_none;
    self->i2c_addr = args[ARG_i2c_addr].u_int;
    self->i2c_bus = MP_OBJ_TYPE_GET_SLOT(
        &mp_machine_soft_i2c_type, make_new) (&mp_machine_soft_i2c_type, 2, 0, (const mp_obj_t []) {
        (mp_obj_t) OMV_TFP410_SCL_PIN, (mp_obj_t) OMV_TFP410_SDA_PIN
    });

    omv_gpio_config(OMV_TFP410_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_TFP410_RESET_PIN, 0);
    mp_hal_delay_ms(1);
    omv_gpio_write(OMV_TFP410_RESET_PIN, 1);
    mp_hal_delay_ms(1);

    if (mp_machine_soft_i2c_transfer(self->i2c_bus, TFP410_I2C_ADDR, 1, &((mp_machine_i2c_buf_t) {
        .len = 4, .buf = (uint8_t []) {0x08, 0xB7, 0x19, 0x80} // addr, CTL_1, CTL_2, CTL_3
    }), MP_MACHINE_I2C_FLAG_STOP) != 4) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to init DVI bus"));
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t py_tfp410_deinit(mp_obj_t self_in) {
    omv_gpio_irq_enable(OMV_TFP410_INT_PIN, false);

    omv_gpio_write(OMV_TFP410_RESET_PIN, 0);
    mp_hal_delay_ms(1);
    omv_gpio_write(OMV_TFP410_RESET_PIN, 1);
    mp_hal_delay_ms(1);

    omv_gpio_deinit(OMV_TFP410_INT_PIN);
    omv_gpio_deinit(OMV_TFP410_RESET_PIN);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tfp410_deinit_obj, py_tfp410_deinit);

STATIC const mp_rom_map_elem_t py_tfp410_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),                MP_ROM_QSTR(MP_QSTR_tfp410)             },
    { MP_ROM_QSTR(MP_QSTR___del__),                 MP_ROM_PTR(&py_tfp410_deinit_obj)       },
    { MP_ROM_QSTR(MP_QSTR_isconnected),             MP_ROM_PTR(&py_dvi_is_connected_obj)    },
    { MP_ROM_QSTR(MP_QSTR_hotplug_callback),        MP_ROM_PTR(&py_dvi_hotplug_callback_obj)},
};
MP_DEFINE_CONST_DICT(py_tfp410_locals_dict, py_tfp410_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    py_tfp410_type,
    MP_QSTR_TFP410,
    MP_TYPE_FLAG_NONE,
    make_new, py_tfp410_make_new,
    locals_dict, &py_tfp410_locals_dict
    );

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_tfp410) },
    { MP_ROM_QSTR(MP_QSTR_TFP410),      MP_ROM_PTR(&py_tfp410_type) },
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t tfp410_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_tfp410, tfp410_module);
#endif // OMV_TFP410_ENABLE
