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
 * OMV Python module.
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "py/obj.h"
#include "usbdbg.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "tinyusb_debug.h"

static mp_obj_t py_omv_version_string() {
    char str[12];
    snprintf(str, 12, "%d.%d.%d",
             FIRMWARE_VERSION_MAJOR,
             FIRMWARE_VERSION_MINOR,
             FIRMWARE_VERSION_PATCH);
    return mp_obj_new_str(str, strlen(str));
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_version_string_obj, py_omv_version_string);

static mp_obj_t py_omv_arch() {
    char *str = OMV_BOARD_ARCH;
    return mp_obj_new_str(str, strlen(str));
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_arch_obj, py_omv_arch);

static mp_obj_t py_omv_board_type() {
    char *str = OMV_BOARD_TYPE;
    return mp_obj_new_str(str, strlen(str));
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_board_type_obj, py_omv_board_type);

static mp_obj_t py_omv_board_id() {
    char str[25];
    snprintf(str, 25, "%08X%08X%08X",
             *((unsigned int *) (OMV_BOARD_UID_ADDR + 8)),
             *((unsigned int *) (OMV_BOARD_UID_ADDR + 4)),
             *((unsigned int *) (OMV_BOARD_UID_ADDR + 0)));
    return mp_obj_new_str(str, strlen(str));
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_board_id_obj, py_omv_board_id);

static mp_obj_t py_omv_debug_mode() {
    #if OMV_TUSBDBG_ENABLE
    return mp_obj_new_bool(tinyusb_debug_enabled());
    #elif MICROPY_HW_ENABLE_USB
    extern int usb_cdc_debug_mode_enabled();
    return mp_obj_new_bool(usb_cdc_debug_mode_enabled());
    #else
    return mp_const_none;
    #endif
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_debug_mode_obj, py_omv_debug_mode);

static mp_obj_t py_omv_disable_fb(size_t n_args, const mp_obj_t *args) {
    framebuffer_t *fb = framebuffer_get(0);

    if (!n_args) {
        return mp_obj_new_bool(!framebuffer_get_streaming(fb));
    }
    framebuffer_set_streaming(fb, !mp_obj_get_int(args[0]));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_omv_disable_fb_obj, 0, 1, py_omv_disable_fb);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_omv) },
    { MP_ROM_QSTR(MP_QSTR_version_major),   MP_ROM_INT(FIRMWARE_VERSION_MAJOR) },
    { MP_ROM_QSTR(MP_QSTR_version_minor),   MP_ROM_INT(FIRMWARE_VERSION_MINOR) },
    { MP_ROM_QSTR(MP_QSTR_version_patch),   MP_ROM_INT(FIRMWARE_VERSION_PATCH) },
    { MP_ROM_QSTR(MP_QSTR_version_string),  MP_ROM_PTR(&py_omv_version_string_obj) },
    { MP_ROM_QSTR(MP_QSTR_arch),            MP_ROM_PTR(&py_omv_arch_obj) },
    { MP_ROM_QSTR(MP_QSTR_board_type),      MP_ROM_PTR(&py_omv_board_type_obj) },
    { MP_ROM_QSTR(MP_QSTR_board_id),        MP_ROM_PTR(&py_omv_board_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_debug_mode),      MP_ROM_PTR(&py_omv_debug_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_fb),      MP_ROM_PTR(&py_omv_disable_fb_obj) }
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t omv_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_omv, omv_module);
