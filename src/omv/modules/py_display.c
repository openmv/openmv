/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Display Python module.
 */
#include "omv_boardconfig.h"

#if MICROPY_PY_DISPLAY

#include "py/obj.h"
#include "py/objarray.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "py_helper.h"
#include "py_image.h"
#include "py_display.h"

static mp_obj_t py_display_width(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->width);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_width_obj, py_display_width);

static mp_obj_t py_display_height(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->height);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_height_obj, py_display_height);

static mp_obj_t py_display_triple_buffer(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->triple_buffer);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_triple_buffer_obj, py_display_triple_buffer);

static mp_obj_t py_display_bgr(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->bgr);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_bgr_obj, py_display_bgr);

static mp_obj_t py_display_byte_swap(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->byte_swap);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_byte_swap_obj, py_display_byte_swap);

static mp_obj_t py_display_framesize(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->framesize);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_framesize_obj, py_display_framesize);

static mp_obj_t py_display_refresh(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->refresh);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_refresh_obj, py_display_refresh);

static mp_obj_t py_display_deinit(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    if (display_p->deinit != NULL) {
        display_p->deinit(self);
    }
    if (self->bl_controller != mp_const_none) {
        mp_obj_t dest[2];
        mp_load_method_maybe(self->bl_controller, MP_QSTR_deinit, dest);
        if (dest[0] != MP_OBJ_NULL) {
            mp_call_method_n_kw(0, 0, dest);
        }
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_display_deinit_obj, py_display_deinit);

static mp_obj_t py_display_clear(uint n_args, const mp_obj_t *args) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    bool display_off = (n_args > 1 && mp_obj_get_int(args[1]));
    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    if (display_p->clear != NULL) {
        display_p->clear(self, display_off);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_display_clear_obj, 1, 2, py_display_clear);

static mp_obj_t py_display_backlight(uint n_args, const mp_obj_t *args) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args > 1) {
        uint32_t intensity = mp_obj_get_int(args[1]);
        if (intensity > 100) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Intensity ranges between 0 (off) and 100 (full on)"));
        }
        if (self->bl_controller != mp_const_none) {
            // If the display has a backlight controller set, call it first.
            mp_obj_t dest[3];
            mp_load_method(self->bl_controller, MP_QSTR_backlight, dest);
            dest[2] = args[1];
            mp_call_method_n_kw(1, 0, dest);
        } else {
            // Otherwise, if the display protocol's set_backlight is set, call it.
            py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
            if (display_p->set_backlight != NULL) {
                display_p->set_backlight(self, intensity);
            } else {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Display does not support backlight control."));
            }
        }
        self->intensity = intensity;
    } else {
        return mp_obj_new_int(self->intensity);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_display_backlight_obj, 1, 2, py_display_backlight);

static mp_obj_t py_display_write(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {
        ARG_image, ARG_x, ARG_y, ARG_x_scale, ARG_y_scale, ARG_roi,
        ARG_channel, ARG_alpha, ARG_color_palette, ARG_alpha_palette, ARG_hint
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_x, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0 } },
        { MP_QSTR_y, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0 } },
        { MP_QSTR_x_scale, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_y_scale, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_rgb_channel, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = -1 } },
        { MP_QSTR_alpha, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 255 } },
        { MP_QSTR_color_palette, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_alpha_palette, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_hint, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 0 } },
    };

    // Parse args.
    py_display_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    fb_alloc_mark();
    image_t *image = py_helper_arg_to_image(args[ARG_image].u_obj, ARG_IMAGE_ANY | ARG_IMAGE_ALLOC);
    rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, image);

    if (args[ARG_channel].u_int < -1 || args[ARG_channel].u_int > 2) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("RGB channel can be 0, 1, or 2"));
    }

    if (args[ARG_alpha].u_int < 0 || args[ARG_alpha].u_int > 255) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Alpha ranges between 0 and 255"));
    }

    float x_scale = 1.0f;
    float y_scale = 1.0f;
    py_helper_arg_to_scale(args[ARG_x_scale].u_obj, args[ARG_y_scale].u_obj, &x_scale, &y_scale);

    if (y_scale < 0 && !self->triple_buffer) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Vertical flip requires triple buffering!"));
    }

    const uint16_t *color_palette = py_helper_arg_to_palette(args[ARG_color_palette].u_obj, PIXFORMAT_RGB565);
    const uint8_t *alpha_palette = py_helper_arg_to_palette(args[ARG_alpha_palette].u_obj, PIXFORMAT_GRAYSCALE);

    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    display_p->write(self, image, args[ARG_x].u_int, args[ARG_y].u_int, x_scale, y_scale, &roi,
                     args[ARG_channel].u_int, args[ARG_alpha].u_int, color_palette, alpha_palette, args[ARG_hint].u_int);
    fb_alloc_free_till_mark();

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_display_write_obj, 2, py_display_write);

static mp_obj_t py_display_bus_write(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_cmd, ARG_args, ARG_dcs };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cmd,  MP_ARG_INT | MP_ARG_REQUIRED },
        { MP_QSTR_args, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_dcs,  MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_bool = false } },
    };

    // Parse args.
    py_display_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    if (display_p->bus_write == NULL) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected a display controller"));
    } else if (args[ARG_args].u_obj == mp_const_none) {
        display_p->bus_write(self, args[ARG_cmd].u_int, NULL, 0, args[ARG_dcs].u_bool);
    } else if (mp_obj_is_int(args[ARG_args].u_obj)) {
        uint8_t arg = mp_obj_get_int(args[ARG_args].u_obj);
        display_p->bus_write(self, args[ARG_cmd].u_int, &arg, 1, args[ARG_dcs].u_bool);
    } else {
        mp_buffer_info_t rbuf;
        mp_get_buffer_raise(args[ARG_args].u_obj, &rbuf, MP_BUFFER_READ);
        display_p->bus_write(self, args[ARG_cmd].u_int, rbuf.buf, rbuf.len, args[ARG_dcs].u_bool);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_display_bus_write_obj, 1, py_display_bus_write);

static mp_obj_t py_display_bus_read(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_cmd, ARG_len, ARG_args, ARG_dcs };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cmd,  MP_ARG_INT | MP_ARG_REQUIRED },
        { MP_QSTR_len,  MP_ARG_INT | MP_ARG_REQUIRED },
        { MP_QSTR_args, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_dcs,  MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_bool = false } },
    };

    // Parse args.
    py_display_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    mp_obj_array_t *wbuf = MP_OBJ_TO_PTR(mp_obj_new_bytearray_by_ref(args[ARG_len].u_int,
                                                                     m_new(byte, args[ARG_len].u_int)));
    if (display_p->bus_read == NULL) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected a display controller"));
    } else if (args[ARG_args].u_obj == mp_const_none) {
        display_p->bus_read(self, args[ARG_cmd].u_int, NULL, 0, wbuf->items, wbuf->len, args[ARG_dcs].u_bool);
    } else if (mp_obj_is_int(args[ARG_args].u_obj)) {
        uint8_t arg = mp_obj_get_int(args[ARG_args].u_obj);
        display_p->bus_read(self, args[ARG_cmd].u_int, &arg, 1, wbuf->items, wbuf->len, args[ARG_dcs].u_bool);
    } else {
        mp_buffer_info_t rbuf;
        mp_get_buffer_raise(args[ARG_args].u_obj, &rbuf, MP_BUFFER_READ);
        display_p->bus_read(self, args[ARG_cmd].u_int, rbuf.buf, rbuf.len,
                            wbuf->items, wbuf->len, args[ARG_dcs].u_bool);
    }
    return MP_OBJ_FROM_PTR(wbuf);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_display_bus_read_obj, 1, py_display_bus_read);

static const mp_rom_map_elem_t py_display_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_display)              },
    { MP_ROM_QSTR(MP_QSTR___del__),             MP_ROM_PTR(&py_display_deinit_obj)        },
    { MP_ROM_QSTR(MP_QSTR_width),               MP_ROM_PTR(&py_display_width_obj)         },
    { MP_ROM_QSTR(MP_QSTR_height),              MP_ROM_PTR(&py_display_height_obj)        },
    { MP_ROM_QSTR(MP_QSTR_triple_buffer),       MP_ROM_PTR(&py_display_triple_buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgr),                 MP_ROM_PTR(&py_display_bgr_obj)           },
    { MP_ROM_QSTR(MP_QSTR_byte_swap),           MP_ROM_PTR(&py_display_byte_swap_obj)     },
    { MP_ROM_QSTR(MP_QSTR_framesize),           MP_ROM_PTR(&py_display_framesize_obj)     },
    { MP_ROM_QSTR(MP_QSTR_refresh),             MP_ROM_PTR(&py_display_refresh_obj)       },
    { MP_ROM_QSTR(MP_QSTR_clear),               MP_ROM_PTR(&py_display_clear_obj)         },
    { MP_ROM_QSTR(MP_QSTR_backlight),           MP_ROM_PTR(&py_display_backlight_obj)     },
    { MP_ROM_QSTR(MP_QSTR_write),               MP_ROM_PTR(&py_display_write_obj)         },
    { MP_ROM_QSTR(MP_QSTR_bus_write),           MP_ROM_PTR(&py_display_bus_write_obj)     },
    { MP_ROM_QSTR(MP_QSTR_bus_read),            MP_ROM_PTR(&py_display_bus_read_obj)      },
};
MP_DEFINE_CONST_DICT(py_display_locals_dict, py_display_locals_dict_table);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_display)            },
    { MP_ROM_QSTR(MP_QSTR_QVGA),                MP_ROM_INT(DISPLAY_RESOLUTION_QVGA)     },
    { MP_ROM_QSTR(MP_QSTR_TQVGA),               MP_ROM_INT(DISPLAY_RESOLUTION_TQVGA)    },
    { MP_ROM_QSTR(MP_QSTR_FHVGA),               MP_ROM_INT(DISPLAY_RESOLUTION_FHVGA)    },
    { MP_ROM_QSTR(MP_QSTR_FHVGA2),              MP_ROM_INT(DISPLAY_RESOLUTION_FHVGA2)   },
    { MP_ROM_QSTR(MP_QSTR_VGA),                 MP_ROM_INT(DISPLAY_RESOLUTION_VGA)      },
    { MP_ROM_QSTR(MP_QSTR_THVGA),               MP_ROM_INT(DISPLAY_RESOLUTION_THVGA)    },
    { MP_ROM_QSTR(MP_QSTR_FWVGA),               MP_ROM_INT(DISPLAY_RESOLUTION_FWVGA)    },
    { MP_ROM_QSTR(MP_QSTR_FWVGA2),              MP_ROM_INT(DISPLAY_RESOLUTION_FWVGA2)   },
    { MP_ROM_QSTR(MP_QSTR_TFWVGA),              MP_ROM_INT(DISPLAY_RESOLUTION_TFWVGA)   },
    { MP_ROM_QSTR(MP_QSTR_TFWVGA2),             MP_ROM_INT(DISPLAY_RESOLUTION_TFWVGA2)  },
    { MP_ROM_QSTR(MP_QSTR_SVGA),                MP_ROM_INT(DISPLAY_RESOLUTION_SVGA)     },
    { MP_ROM_QSTR(MP_QSTR_WSVGA),               MP_ROM_INT(DISPLAY_RESOLUTION_WSVGA)    },
    { MP_ROM_QSTR(MP_QSTR_XGA),                 MP_ROM_INT(DISPLAY_RESOLUTION_XGA)      },
    { MP_ROM_QSTR(MP_QSTR_SXGA),                MP_ROM_INT(DISPLAY_RESOLUTION_SXGA)     },
    { MP_ROM_QSTR(MP_QSTR_SXGA2),               MP_ROM_INT(DISPLAY_RESOLUTION_SXGA2)    },
    { MP_ROM_QSTR(MP_QSTR_UXGA),                MP_ROM_INT(DISPLAY_RESOLUTION_UXGA)     },
    { MP_ROM_QSTR(MP_QSTR_HD),                  MP_ROM_INT(DISPLAY_RESOLUTION_HD)       },
    { MP_ROM_QSTR(MP_QSTR_FHD),                 MP_ROM_INT(DISPLAY_RESOLUTION_FHD)      },

    #ifdef OMV_SPI_DISPLAY_CONTROLLER
    { MP_ROM_QSTR(MP_QSTR_SPIDisplay),          MP_ROM_PTR(&py_spi_display_type)        },
    #endif
    #ifdef OMV_RGB_DISPLAY_CONTROLLER
    { MP_ROM_QSTR(MP_QSTR_RGBDisplay),          MP_ROM_PTR(&py_rgb_display_type)        },
    #endif
    #ifdef OMV_DSI_DISPLAY_CONTROLLER
    { MP_ROM_QSTR(MP_QSTR_DSIDisplay),          MP_ROM_PTR(&py_dsi_display_type)        },
    #endif
    #if OMV_DISPLAY_CEC_ENABLE || OMV_DISPLAY_DDC_ENABLE
    { MP_ROM_QSTR(MP_QSTR_DisplayData),         MP_ROM_PTR(&py_display_data_type)       },
    #endif
};
static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t display_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_display, display_module);
#endif // MICROPY_PY_DISPLAY
