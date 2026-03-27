/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * UMA Python module.
 */
#if MICROPY_PY_UMALLOC
#include "py/obj.h"
#include "py/runtime.h"
#include "framebuffer.h"
#include "umalloc.h"

static mp_obj_t py_umalloc_init(void) {
    uma_init();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_umalloc_init_obj, py_umalloc_init);

static mp_obj_t py_umalloc_collect(void) {
    uma_collect();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_umalloc_collect_obj, py_umalloc_collect);

static mp_obj_t py_umalloc_stats(void) {
    uma_stats_t s;
    uma_get_stats(&s);
    mp_obj_t tuple[6] = {
        mp_obj_new_int(s.used_count),
        mp_obj_new_int(s.free_count),
        mp_obj_new_int(s.persist_count),
        mp_obj_new_int(s.used_bytes),
        mp_obj_new_int(s.free_bytes),
        mp_obj_new_int(s.persist_bytes),
    };
    return mp_obj_new_tuple(6, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_umalloc_stats_obj, py_umalloc_stats);

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),   MP_OBJ_NEW_QSTR(MP_QSTR_umalloc) },
    { MP_ROM_QSTR(MP_QSTR_init),       MP_ROM_PTR(&py_umalloc_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_collect),    MP_ROM_PTR(&py_umalloc_collect_obj) },
    { MP_ROM_QSTR(MP_QSTR_stats),      MP_ROM_PTR(&py_umalloc_stats_obj) },
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t umalloc_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_umalloc, umalloc_module);
#endif // MICROPY_PY_UMALLOC
