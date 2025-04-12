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
 * MP assertions.
 */
#ifndef __PY_ASSERT_H__
#define __PY_ASSERT_H__
#define PY_ASSERT_TRUE(cond)                              \
    do {                                                  \
        if ((cond) == 0) {                                \
            mp_raise_msg(&mp_type_OSError,                \
                         MP_ERROR_TEXT(                   \
                             "Operation not supported")); \
        }                                                 \
    } while (0)

#define PY_ASSERT_TRUE_MSG(cond, msg)         \
    do {                                      \
        if ((cond) == 0) {                    \
            mp_raise_msg(&mp_type_OSError,    \
                         MP_ERROR_TEXT(msg)); \
        }                                     \
    } while (0)

#define PY_ASSERT_FALSE_MSG(cond, msg)        \
    do {                                      \
        if ((cond) == 1) {                    \
            mp_raise_msg(&mp_type_OSError,    \
                         MP_ERROR_TEXT(msg)); \
        }                                     \
    } while (0)

#define PY_ASSERT_TYPE(obj, type)                            \
    do {                                                     \
        __typeof__ (obj) _a = (obj);                         \
        __typeof__ (type) _b = (type);                       \
        if (!MP_OBJ_IS_TYPE(_a, _b)) {                       \
            mp_raise_msg_varg(&mp_type_TypeError,            \
                              MP_ERROR_TEXT(                 \
                                  "Can't convert %s to %s"), \
                              mp_obj_get_type_str(_a),       \
                              mp_obj_get_type_str(_b));      \
        }                                                    \
    } while (0)
/* IS_TYPE doesn't work for str objs */
#define PY_ASSERT_STR(obj)                     \
    do {                                       \
        __typeof__ (obj) _a = (obj);           \
        if (!MP_OBJ_IS_STR(_a)) {              \
            mp_raise_msg_varg(                 \
                &mp_type_TypeError,            \
                MP_ERROR_TEXT(                 \
                    "Can't convert %s to %s"), \
                mp_obj_get_type_str(_a),       \
                str_type.name);                \
        }                                      \
    } while (0)

#endif // __PY_ASSERT_H__
