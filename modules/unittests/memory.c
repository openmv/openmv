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
 * Memory attributes debug helpers.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include "py/runtime.h"
#include "py/obj.h"

#include "omv_memory.h"

// Returns (cacheable, write_back, dmable) for [addr, addr + size).
// master is the bus master base address, or the port's default DMA if omitted.
static mp_obj_t memory_attrs(size_t n_args, const mp_obj_t *args) {
    const void *addr = (const void *) mp_obj_get_int_truncated(args[0]);
    size_t size = mp_obj_get_int(args[1]);
    const void *master = (n_args > 2) ? (const void *) mp_obj_get_int_truncated(args[2]) : NULL;
    mp_obj_t tuple[3] = {
        mp_obj_new_bool(omv_memory_is_cacheable(addr, size)),
        mp_obj_new_bool(omv_memory_is_write_back(addr, size)),
        mp_obj_new_bool(omv_memory_is_dmable(master, addr, size)),
    };
    return mp_obj_new_tuple(3, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(memory_attrs_obj, 2, 3, memory_attrs);

// Module definition
static const mp_rom_map_elem_t unittest_memory_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_memory) },
    { MP_ROM_QSTR(MP_QSTR_memory_attrs), MP_ROM_PTR(&memory_attrs_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_memory_module_globals, unittest_memory_module_globals_table);

const mp_obj_module_t unittest_memory_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_memory_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_memory, unittest_memory_module);

#endif // MICROPY_PY_UNITTEST
