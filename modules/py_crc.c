/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * CRC Python module for testing CRC implementations.
 */

#if MICROPY_PY_CRC
#include "py/runtime.h"
#include "py/obj.h"
#include "omv_crc.h"

// CRC16 function
static mp_obj_t py_crc16(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kwargs) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_data, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kwargs, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[0].u_obj, &bufinfo, MP_BUFFER_READ);

    uint16_t crc;
    if (args[1].u_obj == MP_OBJ_NULL) {
        crc = omv_crc16_start(bufinfo.buf, bufinfo.len);
    } else {
        uint16_t prev_crc = mp_obj_get_int_truncated(args[1].u_obj) & 0xFFFF;
        crc = omv_crc16_update(prev_crc, bufinfo.buf, bufinfo.len);
    }
    return mp_obj_new_int_from_uint(crc);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_crc16_obj, 1, py_crc16);

// CRC32 function
static mp_obj_t py_crc32(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kwargs) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_data, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kwargs, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[0].u_obj, &bufinfo, MP_BUFFER_READ);

    uint32_t crc;
    if (args[1].u_obj == MP_OBJ_NULL) {
        crc = omv_crc32_start(bufinfo.buf, bufinfo.len);
    } else {
        uint32_t prev_crc = mp_obj_get_int_truncated(args[1].u_obj);
        crc = omv_crc32_update(prev_crc, bufinfo.buf, bufinfo.len);
    }
    return mp_obj_new_int_from_uint(crc);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_crc32_obj, 1, py_crc32);

// Module globals table
static const mp_rom_map_elem_t crc_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_crc) },
    { MP_ROM_QSTR(MP_QSTR_crc16), MP_ROM_PTR(&py_crc16_obj) },
    { MP_ROM_QSTR(MP_QSTR_crc32), MP_ROM_PTR(&py_crc32_obj) },
};

static MP_DEFINE_CONST_DICT(crc_module_globals, crc_module_globals_table);

// Define module object
const mp_obj_module_t crc_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &crc_module_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_crc, crc_user_cmodule);
#endif // MICROPY_PY_CRC
