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
#include "py/obj.h"
#include "py/objtuple.h"
#include "py/runtime.h"

#include "imlib.h"
#include "array.h"
#include "file_utils.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"
#include "py_image_descriptor.h"

// Haar Cascade ///////////////////////////////////////////////////////////////

#ifdef IMLIB_ENABLE_FEATURES
static const mp_obj_type_t py_cascade_type;

typedef struct _py_cascade_obj_t {
    mp_obj_base_t base;
    struct cascade _cobj;
} py_cascade_obj_t;

void *py_cascade_cobj(mp_obj_t cascade) {
    PY_ASSERT_TYPE(cascade, &py_cascade_type);
    return &((py_cascade_obj_t *) cascade)->_cobj;
}

static void py_cascade_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_cascade_obj_t *self = self_in;
    mp_printf(print, "{\"width\":%d, \"height\":%d, \"n_stages\":%d, \"n_features\":%d, \"n_rectangles\":%d}",
              self->_cobj.window.w, self->_cobj.window.h, self->_cobj.n_stages,
              self->_cobj.n_features, self->_cobj.n_rectangles);
}

static MP_DEFINE_CONST_OBJ_TYPE(
    py_cascade_type,
    MP_QSTR_Cascade,
    MP_TYPE_FLAG_NONE,
    print, py_cascade_print
    );
#endif // IMLIB_ENABLE_FEATURES

// Keypoints object ///////////////////////////////////////////////////////////

#ifdef IMLIB_ENABLE_FIND_KEYPOINTS

static void py_kp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_kp_obj_t *self = self_in;
    mp_printf(print,
              "{\"size\":%d, \"threshold\":%d, \"normalized\":%d}",
              array_length(self->kpts),
              self->threshold,
              self->normalized);
}

mp_obj_t py_kp_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
    py_kp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_LEN:
            return MP_OBJ_NEW_SMALL_INT(array_length(self->kpts));

        default:
            return MP_OBJ_NULL; // op not supported
    }
}

static mp_obj_t py_kp_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    if (value == MP_OBJ_SENTINEL) {
        // load
        py_kp_obj_t *self = self_in;
        int size = array_length(self->kpts);
        int i = mp_get_index(self->base.type, size, index, false);
        kp_t *kp = array_at(self->kpts, i);
        return mp_obj_new_tuple(5, (mp_obj_t []) {mp_obj_new_int(kp->x),
                                                  mp_obj_new_int(kp->y),
                                                  mp_obj_new_int(kp->score),
                                                  mp_obj_new_int(kp->octave),
                                                  mp_obj_new_int(kp->angle)});
    }

    return MP_OBJ_NULL; // op not supported
}

MP_DEFINE_CONST_OBJ_TYPE(
    py_kp_type,
    MP_QSTR_kp_desc,
    MP_TYPE_FLAG_NONE,
    print, py_kp_print,
    subscr, py_kp_subscr,
    unary_op, py_kp_unary_op
    );

py_kp_obj_t *py_kpts_obj(mp_obj_t kpts_obj) {
    PY_ASSERT_TYPE(kpts_obj, &py_kp_type);
    return kpts_obj;
}

#endif // IMLIB_ENABLE_FIND_KEYPOINTS

// LBP descriptor /////////////////////////////////////////////////////////////

#ifdef IMLIB_ENABLE_FIND_LBP

static void py_lbp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "{}");
}

MP_DEFINE_CONST_OBJ_TYPE(
    py_lbp_type,
    MP_QSTR_lbp_desc,
    MP_TYPE_FLAG_NONE,
    print, py_lbp_print
    );
#endif // IMLIB_ENABLE_FIND_LBP

// Keypoints Match Object /////////////////////////////////////////////////////

#if defined(IMLIB_ENABLE_DESCRIPTOR) && defined(IMLIB_ENABLE_FIND_KEYPOINTS)



#endif //IMLIB_ENABLE_DESCRIPTOR && IMLIB_ENABLE_FIND_KEYPOINTS

#ifdef IMLIB_ENABLE_FEATURES
mp_obj_t py_image_load_cascade(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_stages };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_stages, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1} },
    };
    cascade_t cascade;
    const char *path = mp_obj_str_get_str(pos_args[0]);

    // Load cascade from file or flash
    if (imlib_load_cascade(&cascade, path) != 0) {
        #if MICROPY_VFS
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to load Haar cascade"));
        #else
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Image I/O is not supported"));
        #endif
    }

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    // Read the number of stages
    int stages = (args[ARG_stages].u_int >= 0) ? args[ARG_stages].u_int : cascade.n_stages;
    // Check the number of stages
    if (stages > 0 && stages < cascade.n_stages) {
        cascade.n_stages = stages;
    }

    // Return micropython cascade object
    py_cascade_obj_t *o = m_new_obj(py_cascade_obj_t);
    o->base.type = &py_cascade_type;
    o->_cobj = cascade;
    return o;
}
MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_cascade_obj, 1, py_image_load_cascade);
#endif // IMLIB_ENABLE_FEATURES

#if defined(IMLIB_ENABLE_DESCRIPTOR)
#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
mp_obj_t py_image_load_descriptor(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    file_t fp;

    uint32_t desc_type;
    mp_obj_t desc = mp_const_none;
    const char *path = mp_obj_str_get_str(args[0]);

    file_open(&fp, path, FA_READ | FA_OPEN_EXISTING);

    // Read descriptor type
    file_read(&fp, &desc_type, sizeof(desc_type));

    // Load descriptor
    switch (desc_type) {
        #if defined(IMLIB_ENABLE_FIND_LBP)
        case DESC_LBP: {
            py_lbp_obj_t *lbp = m_new_obj(py_lbp_obj_t);
            lbp->base.type = &py_lbp_type;

            imlib_lbp_desc_load(&fp, &lbp->hist);
            desc = lbp;
            break;
        }
        #endif  //IMLIB_ENABLE_FIND_LBP
        #if defined(IMLIB_ENABLE_FIND_KEYPOINTS)
        case DESC_ORB: {
            array_t *kpts = NULL;
            array_alloc(&kpts, m_free);

            orb_load_descriptor(&fp, kpts);

            // Return keypoints MP object
            py_kp_obj_t *kp_obj = m_new_obj(py_kp_obj_t);
            kp_obj->base.type = &py_kp_type;
            kp_obj->kpts = kpts;
            kp_obj->threshold = 10;
            kp_obj->normalized = false;
            desc = kp_obj;
            break;
        }
        #endif //IMLIB_ENABLE_FIND_KEYPOINTS
        default:
            // Unsupported descriptor type
            desc = mp_const_none;
            break;
    }

    file_close(&fp);

    // If descriptor is still none, then it's not supported.
    if (desc == mp_const_none) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Descriptor type is not supported"));
    }
    return desc;
}
MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_descriptor_obj, 1, py_image_load_descriptor);

mp_obj_t py_image_save_descriptor(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    file_t fp;

    uint32_t desc_type;
    const char *path = mp_obj_str_get_str(args[1]);

    file_open(&fp, path, FA_WRITE | FA_CREATE_ALWAYS);

    // Find descriptor type
    const mp_obj_type_t *desc_obj_type = mp_obj_get_type(args[0]);
    if (0) {
    #if defined(IMLIB_ENABLE_FIND_LBP)
    } else if (desc_obj_type == &py_lbp_type) {
        desc_type = DESC_LBP;
    #endif //IMLIB_ENABLE_FIND_LBP
    #if defined(IMLIB_ENABLE_FIND_KEYPOINTS)
    } else if (desc_obj_type == &py_kp_type) {
        desc_type = DESC_ORB;
    #endif //IMLIB_ENABLE_FIND_KEYPOINTS
    } else {
        (void) desc_obj_type;
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Descriptor type is not supported"));
    }

    // Write descriptor type
    file_write(&fp, &desc_type, sizeof(desc_type));

    // Write descriptor
    switch (desc_type) {
        #if defined(IMLIB_ENABLE_FIND_LBP)
        case DESC_LBP: {
            py_lbp_obj_t *lbp = ((py_lbp_obj_t *) args[0]);
            imlib_lbp_desc_save(&fp, lbp->hist);
            break;
        }
        #endif //IMLIB_ENABLE_FIND_LBP
        #if defined(IMLIB_ENABLE_FIND_KEYPOINTS)
        case DESC_ORB: {
            py_kp_obj_t *kpts = ((py_kp_obj_t *) args[0]);
            orb_save_descriptor(&fp, kpts->kpts);
            break;
        }
        #endif //IMLIB_ENABLE_FIND_KEYPOINTS
    }

    file_close(&fp);
    return mp_const_true;
}
MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_descriptor_obj, 2, py_image_save_descriptor);
#endif //IMLIB_ENABLE_IMAGE_FILE_IO

static const qstr kptmatch_fields[] = {
    MP_QSTR_x, MP_QSTR_y, MP_QSTR_w, MP_QSTR_h, MP_QSTR_cx, MP_QSTR_cy,
    MP_QSTR_count, MP_QSTR_theta, MP_QSTR_match, MP_QSTR_rect,
};

static mp_obj_t py_image_match_descriptor(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_threshold, ARG_filter_outliers };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_threshold,      MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 85} },
        { MP_QSTR_filter_outliers, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false} },
    };
    mp_arg_val_t kw_vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, kw_vals);

    mp_obj_t match_obj = mp_const_none;
    const mp_obj_type_t *desc1_type = mp_obj_get_type(pos_args[0]);
    const mp_obj_type_t *desc2_type = mp_obj_get_type(pos_args[1]);
    PY_ASSERT_TRUE_MSG((desc1_type == desc2_type), "Descriptors have different types!");

    if (0) {
    #if defined(IMLIB_ENABLE_FIND_LBP)
    } else if (desc1_type == &py_lbp_type) {
        py_lbp_obj_t *lbp1 = ((py_lbp_obj_t *) pos_args[0]);
        py_lbp_obj_t *lbp2 = ((py_lbp_obj_t *) pos_args[1]);

        // Sanity checks
        PY_ASSERT_TYPE(lbp1, &py_lbp_type);
        PY_ASSERT_TYPE(lbp2, &py_lbp_type);

        // Match descriptors
        match_obj = mp_obj_new_int(imlib_lbp_desc_distance(lbp1->hist, lbp2->hist));
    #endif //IMLIB_ENABLE_FIND_LBP
    #if defined(IMLIB_ENABLE_FIND_KEYPOINTS)
    } else if (desc1_type == &py_kp_type) {
        py_kp_obj_t *kpts1 = ((py_kp_obj_t *) pos_args[0]);
        py_kp_obj_t *kpts2 = ((py_kp_obj_t *) pos_args[1]);
        int threshold = kw_vals[ARG_threshold].u_int;
        int filter_outliers = kw_vals[ARG_filter_outliers].u_bool;

        // Sanity checks
        PY_ASSERT_TYPE(kpts1, &py_kp_type);
        PY_ASSERT_TYPE(kpts2, &py_kp_type);
        PY_ASSERT_TRUE_MSG((threshold >= 0 && threshold <= 100), "Expected threshold between 0 and 100");

        int theta = 0;          // Estimated angle of rotation
        int count = 0;          // Number of matches
        point_t c = {0};        // Centroid
        rectangle_t r = {0};    // Bounding rectangle
        // List of matching keypoints indices
        mp_obj_t match_list = mp_obj_new_list(0, NULL);

        if (array_length(kpts1->kpts) && array_length(kpts1->kpts)) {
            int *match = uma_malloc(array_length(kpts1->kpts) * sizeof(int) * 2, UMA_DTCM);

            // Match the two keypoint sets
            count = orb_match_keypoints(kpts1->kpts, kpts2->kpts, match, threshold, &r, &c, &theta);

            // Add matching keypoints to Python list.
            for (int i = 0; i < count * 2; i += 2) {
                mp_obj_t index_obj[2] = {
                    mp_obj_new_int(match[i + 0]),
                    mp_obj_new_int(match[i + 1]),
                };
                mp_obj_list_append(match_list, mp_obj_new_tuple(2, index_obj));
            }

            uma_free(match);

            if (filter_outliers == true) {
                count = orb_filter_keypoints(kpts2->kpts, &r, &c);
            }
        }

        mp_obj_t rx = mp_obj_new_int(r.x);
        mp_obj_t ry = mp_obj_new_int(r.y);
        mp_obj_t rw = mp_obj_new_int(r.w);
        mp_obj_t rh = mp_obj_new_int(r.h);
        mp_obj_t items[] = {
            rx, ry, rw, rh,
            mp_obj_new_int(c.x), mp_obj_new_int(c.y),
            mp_obj_new_int(count), mp_obj_new_int(theta), match_list,
            mp_obj_new_tuple(4, (mp_obj_t []) {rx, ry, rw, rh}),
        };
        match_obj = mp_obj_new_attrtuple(kptmatch_fields, MP_ARRAY_SIZE(kptmatch_fields), items);
    #endif //IMLIB_ENABLE_FIND_KEYPOINTS
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Descriptor type is not supported"));
    }

    return match_obj;
}
MP_DEFINE_CONST_FUN_OBJ_KW(py_image_match_descriptor_obj, 2, py_image_match_descriptor);
#endif //IMLIB_ENABLE_DESCRIPTOR

#if defined(IMLIB_ENABLE_FIND_KEYPOINTS) && defined(IMLIB_ENABLE_IMAGE_FILE_IO)
int py_image_descriptor_from_roi(image_t *img, const char *path, rectangle_t *roi) {
    file_t fp;
    array_t *kpts = orb_find_keypoints(img, false, 20, 1.5f, 100, CORNER_AGAST, roi);
    if (array_length(kpts)) {
        file_open(&fp, path, FA_WRITE | FA_CREATE_ALWAYS);
        orb_save_descriptor(&fp, kpts);
        file_close(&fp);
    }
    return 0;
}
#endif // IMLIB_ENABLE_KEYPOINTS && IMLIB_ENABLE_IMAGE_FILE_IO
