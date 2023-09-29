/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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

STATIC mp_obj_t py_display_width(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_width_obj, py_display_width);

STATIC mp_obj_t py_display_height(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_height_obj, py_display_height);

STATIC mp_obj_t py_display_triple_buffer(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->triple_buffer);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_triple_buffer_obj, py_display_triple_buffer);

STATIC mp_obj_t py_display_bgr(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->bgr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_bgr_obj, py_display_bgr);

STATIC mp_obj_t py_display_byte_swap(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->byte_swap);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_byte_swap_obj, py_display_byte_swap);

STATIC mp_obj_t py_display_framesize(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->framesize);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_framesize_obj, py_display_framesize);

STATIC mp_obj_t py_display_refresh(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->refresh);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_refresh_obj, py_display_refresh);

STATIC mp_obj_t py_display_deinit(mp_obj_t self_in) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    if (display_p->deinit != NULL) {
        display_p->deinit(self);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_display_deinit_obj, py_display_deinit);

STATIC mp_obj_t py_display_clear(uint n_args, const mp_obj_t *args) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    bool display_off = (n_args > 1 && mp_obj_get_int(args[1]));
    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    if (display_p->clear != NULL) {
        display_p->clear(self, display_off);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_display_clear_obj, 1, 2, py_display_clear);

STATIC mp_obj_t py_display_backlight(uint n_args, const mp_obj_t *args) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args > 1) {
        uint32_t intensity = mp_obj_get_int(args[1]);
        if ((intensity < 0) || (255 < intensity)) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= intensity <= 255!"));
        }

        py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
        if (display_p->set_backlight != NULL) {
            display_p->set_backlight(self, intensity);
        }
    } else {
        return mp_obj_new_int(self->intensity);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_display_backlight_obj, 1, 2, py_display_backlight);

STATIC mp_obj_t py_display_write(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    py_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    image_t *arg_img = py_image_cobj(args[1]);

    int arg_x_off = 0;
    py_helper_keyword_int_maybe(n_args, args, 2, kw_args,
                                MP_OBJ_NEW_QSTR(MP_QSTR_x), &arg_x_off);

    int arg_y_off = 0;
    py_helper_keyword_int_maybe(n_args, args, 3, kw_args,
                                MP_OBJ_NEW_QSTR(MP_QSTR_y), &arg_y_off);

    float arg_x_scale = 1.f;
    bool got_x_scale = py_helper_keyword_float_maybe(n_args, args, 4, kw_args,
                                                     MP_OBJ_NEW_QSTR(MP_QSTR_x_scale), &arg_x_scale);

    float arg_y_scale = 1.f;
    bool got_y_scale = py_helper_keyword_float_maybe(n_args, args, 5, kw_args,
                                                     MP_OBJ_NEW_QSTR(MP_QSTR_y_scale), &arg_y_scale);

    rectangle_t arg_roi;
    py_helper_keyword_rectangle_roi(arg_img, n_args, args, 6, kw_args, &arg_roi);

    int arg_rgb_channel = py_helper_keyword_int(n_args, args, 7, kw_args,
                                                MP_OBJ_NEW_QSTR(MP_QSTR_rgb_channel), -1);
    if ((arg_rgb_channel < -1) || (2 < arg_rgb_channel)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("-1 <= rgb_channel <= 2!"));
    }

    int arg_alpha = py_helper_keyword_int(n_args, args, 8, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 256);
    if ((arg_alpha < 0) || (256 < arg_alpha)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= alpha <= 256!"));
    }

    const uint16_t *color_palette = py_helper_keyword_color_palette(n_args, args, 7, kw_args, NULL);
    const uint8_t *alpha_palette = py_helper_keyword_alpha_palette(n_args, args, 8, kw_args, NULL);
    image_hint_t hint = py_helper_keyword_int(n_args, args, 9, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hint), 0);

    float arg_x_size;
    bool got_x_size = py_helper_keyword_float_maybe(n_args, args, 10, kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_x_size), &arg_x_size);

    float arg_y_size;
    bool got_y_size = py_helper_keyword_float_maybe(n_args, args, 11, kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_y_size), &arg_y_size);

    if (got_x_scale && got_x_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Choose either x_scale or x_size not both!"));
    }
    if (got_y_scale && got_y_size) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Choose either y_scale or y_size not both!"));
    }

    if (got_x_size) {
        arg_x_scale = arg_x_size / arg_roi.w;
    }
    if (got_y_size) {
        arg_y_scale = arg_y_size / arg_roi.h;
    }

    if ((!got_x_scale) && (!got_x_size) && got_y_size) {
        arg_x_scale = arg_y_scale;
    }
    if ((!got_y_scale) && (!got_y_size) && got_x_size) {
        arg_y_scale = arg_x_scale;
    }

    if ((!self->triple_buffer) && (arg_y_scale < 0)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Vertical flip requires triple buffering!"));
    }

    fb_alloc_mark();
    py_display_p_t *display_p = (py_display_p_t *) MP_OBJ_TYPE_GET_SLOT(self->base.type, protocol);
    display_p->write(self, arg_img, arg_x_off, arg_y_off, arg_x_scale, arg_y_scale, &arg_roi,
                     arg_rgb_channel, arg_alpha, color_palette, alpha_palette, hint);
    fb_alloc_free_till_mark();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_display_write_obj, 2, py_display_write);

#ifdef OMV_DSI_DISPLAY_CONTROLLER
STATIC mp_obj_t py_display_dsi_write(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
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
    if (display_p->dsi_write == NULL) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected a DSI display controller"));
    } else if (args[ARG_args].u_obj == mp_const_none) {
        display_p->dsi_write(self, args[ARG_cmd].u_int, NULL, 0, args[ARG_dcs].u_bool);
    } else if (mp_obj_is_int(args[ARG_args].u_obj)) {
        uint8_t arg = mp_obj_get_int(args[ARG_args].u_obj);
        display_p->dsi_write(self, args[ARG_cmd].u_int, &arg, 1, args[ARG_dcs].u_bool);
    } else {
        mp_buffer_info_t rbuf;
        mp_get_buffer_raise(args[ARG_args].u_obj, &rbuf, MP_BUFFER_READ);
        display_p->dsi_write(self, args[ARG_cmd].u_int, rbuf.buf, rbuf.len, args[ARG_dcs].u_bool);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_display_dsi_write_obj, 1, py_display_dsi_write);

STATIC mp_obj_t py_display_dsi_read(uint n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
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
    if (display_p->dsi_read == NULL) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected a DSI display controller"));
    } else if (args[ARG_args].u_obj == mp_const_none) {
        display_p->dsi_read(self, args[ARG_cmd].u_int, NULL, 0, wbuf->items, wbuf->len, args[ARG_dcs].u_bool);
    } else if (mp_obj_is_int(args[ARG_args].u_obj)) {
        uint8_t arg = mp_obj_get_int(args[ARG_args].u_obj);
        display_p->dsi_read(self, args[ARG_cmd].u_int, &arg, 1, wbuf->items, wbuf->len, args[ARG_dcs].u_bool);
    } else {
        mp_buffer_info_t rbuf;
        mp_get_buffer_raise(args[ARG_args].u_obj, &rbuf, MP_BUFFER_READ);
        display_p->dsi_read(self, args[ARG_cmd].u_int, rbuf.buf, rbuf.len,
                            wbuf->items, wbuf->len, args[ARG_dcs].u_bool);
    }
    return MP_OBJ_FROM_PTR(wbuf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_display_dsi_read_obj, 1, py_display_dsi_read);
#endif

STATIC const mp_rom_map_elem_t py_display_locals_dict_table[] = {
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
    #ifdef OMV_DSI_DISPLAY_CONTROLLER
    { MP_ROM_QSTR(MP_QSTR_dsi_write),           MP_ROM_PTR(&py_display_dsi_write_obj)     },
    { MP_ROM_QSTR(MP_QSTR_dsi_read),            MP_ROM_PTR(&py_display_dsi_read_obj)      },
    #endif
};
MP_DEFINE_CONST_DICT(py_display_locals_dict, py_display_locals_dict_table);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
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
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t display_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

#ifdef MP_REGISTER_EXTENSIBLE_MODULE
MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_display, display_module);
#else
MP_REGISTER_MODULE(MP_QSTR_udisplay, display_module);
#endif
#endif // MICROPY_PY_DISPLAY
