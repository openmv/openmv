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
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "omv_protocol.h"

static mp_obj_t py_omv_version_string() {
    char str[12];
    snprintf(str, 12, "%d.%d.%d",
             OMV_FIRMWARE_VERSION_MAJOR,
             OMV_FIRMWARE_VERSION_MINOR,
             OMV_FIRMWARE_VERSION_PATCH);
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
    char str[25] = {0};
    #ifdef OMV_BOARD_UID_ADDR
    snprintf(str, 25, "%08X%08X%08X",
             #if (OMV_BOARD_UID_SIZE == 2)
             0U,
             #else
             *((unsigned int *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 2)),
             #endif
             *((unsigned int *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 1)),
             *((unsigned int *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 0)));
    #endif
    return mp_obj_new_str(str, strlen(str));
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_board_id_obj, py_omv_board_id);

static mp_obj_t py_omv_debug_mode() {
    #if MICROPY_PY_PROTOCOL
    return mp_obj_new_bool(omv_protocol_is_active());
    #else
    return mp_const_false;
    #endif
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_omv_debug_mode_obj, py_omv_debug_mode);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_omv) },
    { MP_ROM_QSTR(MP_QSTR_version_major),   MP_ROM_INT(OMV_FIRMWARE_VERSION_MAJOR) },
    { MP_ROM_QSTR(MP_QSTR_version_minor),   MP_ROM_INT(OMV_FIRMWARE_VERSION_MINOR) },
    { MP_ROM_QSTR(MP_QSTR_version_patch),   MP_ROM_INT(OMV_FIRMWARE_VERSION_PATCH) },
    { MP_ROM_QSTR(MP_QSTR_version_string),  MP_ROM_PTR(&py_omv_version_string_obj) },
    { MP_ROM_QSTR(MP_QSTR_arch),            MP_ROM_PTR(&py_omv_arch_obj) },
    { MP_ROM_QSTR(MP_QSTR_board_type),      MP_ROM_PTR(&py_omv_board_type_obj) },
    { MP_ROM_QSTR(MP_QSTR_board_id),        MP_ROM_PTR(&py_omv_board_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_debug_mode),      MP_ROM_PTR(&py_omv_debug_mode_obj) }
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t omv_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_omv, omv_module);
