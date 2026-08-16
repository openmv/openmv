/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
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
 * Image descriptor Python module.
 */
#ifndef __PY_IMAGE_DESCRIPTOR_H__
#define __PY_IMAGE_DESCRIPTOR_H__
#ifdef IMLIB_ENABLE_FEATURES
void *py_cascade_cobj(mp_obj_t cascade);
MP_DECLARE_CONST_FUN_OBJ_KW(py_image_load_cascade_obj);
#endif // IMLIB_ENABLE_FEATURES

#ifdef IMLIB_ENABLE_FIND_KEYPOINTS
typedef struct _py_kp_obj_t {
    mp_obj_base_t base;
    array_t *kpts;
    int threshold;
    bool normalized;
} py_kp_obj_t;

extern const mp_obj_type_t py_kp_type;
py_kp_obj_t *py_kpts_obj(mp_obj_t kpts_obj);
#endif // IMLIB_ENABLE_FIND_KEYPOINTS

#ifdef IMLIB_ENABLE_FIND_LBP
typedef struct _py_lbp_obj_t {
    mp_obj_base_t base;
    uint8_t *hist;
} py_lbp_obj_t;

extern const mp_obj_type_t py_lbp_type;
#endif // IMLIB_ENABLE_FIND_LBP

#ifdef IMLIB_ENABLE_DESCRIPTOR
#ifdef IMLIB_ENABLE_IMAGE_FILE_IO
MP_DECLARE_CONST_FUN_OBJ_KW(py_image_load_descriptor_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(py_image_save_descriptor_obj);
#endif // IMLIB_ENABLE_IMAGE_FILE_IO
MP_DECLARE_CONST_FUN_OBJ_KW(py_image_match_descriptor_obj);
#endif // IMLIB_ENABLE_DESCRIPTOR
#endif // __PY_IMAGE_DESCRIPTOR_H__
