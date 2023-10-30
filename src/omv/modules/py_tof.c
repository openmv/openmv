/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python module for time of flight sensors.
 */
#include "py/runtime.h"
#include "py/objlist.h"
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_i2c.h"

#if (MICROPY_PY_TOF == 1)
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"
#include "framebuffer.h"

#if (OMV_ENABLE_TOF_VL53L5CX == 1)
#include "vl53l5cx_api.h"
#endif

#define VL53L5CX_ADDR               0x52
#define VL53L5CX_WIDTH              8
#define VL53L5CX_HEIGHT             8
#define VL53L5CX_FRAME_DATA_SIZE    64

static omv_i2c_t tof_bus = {};

static enum {
    TOF_NONE,
    #if (OMV_ENABLE_TOF_VL53L5CX == 1)
    TOF_VL53L5CX,
    #endif
}
tof_sensor = TOF_NONE;

static int tof_width = 0;
static int tof_height = 0;
static bool tof_transposed = false;

#if (OMV_ENABLE_TOF_VL53L5CX == 1)
static VL53L5CX_Configuration vl53l5cx_dev = {
    .platform = {
        .bus = &tof_bus,
        .address = VL53L5CX_ADDR,
    }
};
#endif

// img->w == data_w && img->h == data_h && img->pixfmt == PIXFORMAT_GRAYSCALE
static void tof_fill_image_float_obj(image_t *img, mp_obj_t *data, float min, float max) {
    float tmp = min;
    min = (min < max) ? min : max;
    max = (max > tmp) ? max : tmp;

    float diff = 255.f / (max - min);

    for (int y = 0; y < img->h; y++) {
        int row_offset = y * img->w;
        mp_obj_t *raw_row = data + row_offset;
        uint8_t *row_pointer = ((uint8_t *) img->data) + row_offset;

        for (int x = 0; x < img->w; x++) {
            float raw = mp_obj_get_float(raw_row[x]);

            if (raw < min) {
                raw = min;
            }

            if (raw > max) {
                raw = max;
            }

            int pixel = fast_roundf((raw - min) * diff);
            row_pointer[x] = __USAT(pixel, 8);
        }
    }
}

#if (OMV_ENABLE_TOF_VL53L5CX == 1)
static void tof_vl53l5cx_get_depth(VL53L5CX_Configuration *vl53l5cx_dev, float *frame, uint32_t timeout) {
    uint8_t frame_ready = 0;
    // Note depending on the config in platform.h, this struct can be too big to alloc on the stack.
    VL53L5CX_ResultsData ranging_data;

    for (mp_uint_t start = mp_hal_ticks_ms(); !frame_ready; mp_hal_delay_ms(1)) {
        if (vl53l5cx_check_data_ready(vl53l5cx_dev, &frame_ready) != 0) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("VL53L5CX ranging failed"));
        }

        if ((mp_hal_ticks_ms() - start) >= timeout) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("VL53L5CX ranging timeout"));
        }
    }

    if (vl53l5cx_get_ranging_data(vl53l5cx_dev, &ranging_data) != 0) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("VL53L5CX ranging failed"));
    }

    for (int i = 0, ii = VL53L5CX_WIDTH * VL53L5CX_HEIGHT; i < ii; i++) {
        frame[i] = (float) ranging_data.distance_mm[i];
    }
}

static mp_obj_t tof_get_depth_obj(int w, int h, float *frame, bool mirror, bool flip, bool dst_transpose, bool src_transpose) {
    mp_obj_list_t *list = (mp_obj_list_t *) mp_obj_new_list(w * h, NULL);
    float min = FLT_MAX;
    float max = -FLT_MAX;
    int w_1 = w - 1;
    int h_1 = h - 1;

    if (!src_transpose) {
        for (int y = 0; y < h; y++) {
            int y_dst = flip ? (h_1 - y) : y;
            float *raw_row = frame + (y * w);
            mp_obj_t *list_row = list->items + (y_dst * w);
            mp_obj_t *t_list_row = list->items + y_dst;

            for (int x = 0; x < w; x++) {
                int x_dst = mirror ? (w_1 - x) : x;
                float raw = raw_row[x];

                if (raw < min) {
                    min = raw;
                }

                if (raw > max) {
                    max = raw;
                }

                mp_obj_t f = mp_obj_new_float(raw);

                if (!dst_transpose) {
                    list_row[x_dst] = f;
                } else {
                    t_list_row[x_dst * h] = f;
                }
            }
        }
    } else {
        for (int x = 0; x < w; x++) {
            int x_dst = mirror ? (w_1 - x) : x;
            float *raw_row = frame + (x * h);
            mp_obj_t *t_list_row = list->items + (x_dst * h);
            mp_obj_t *list_row = list->items + x_dst;

            for (int y = 0; y < h; y++) {
                int y_dst = flip ? (h_1 - y) : y;
                float raw = raw_row[y];

                if (raw < min) {
                    min = raw;
                }

                if (raw > max) {
                    max = raw;
                }

                mp_obj_t f = mp_obj_new_float(raw);

                if (!dst_transpose) {
                    list_row[y_dst * w] = f;
                } else {
                    t_list_row[y_dst] = f;
                }
            }
        }
    }

    mp_obj_t tuple[3] = {
        MP_OBJ_FROM_PTR(list),
        mp_obj_new_float(min),
        mp_obj_new_float(max)
    };
    return mp_obj_new_tuple(3, tuple);
}
#endif

static mp_obj_t py_tof_deinit() {
    tof_width = 0;
    tof_height = 0;
    tof_transposed = false;

    if (tof_sensor != TOF_NONE) {
        #if (OMV_ENABLE_TOF_VL53L5CX == 1)
        if (tof_sensor == TOF_VL53L5CX) {
            vl53l5cx_stop_ranging(&vl53l5cx_dev);
        }
        #endif
        omv_i2c_deinit(&tof_bus);
        tof_sensor = TOF_NONE;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_deinit_obj, py_tof_deinit);

mp_obj_t py_tof_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    py_tof_deinit();
    bool first_init = true;
    int type = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_type), -1);

    if (type == -1) {
        TOF_SCAN_RETRY:
        omv_i2c_init(&tof_bus, TOF_I2C_ID, OMV_I2C_SPEED_STANDARD);
        // Scan and detect any supported sensor.
        uint8_t dev_list[10];
        int dev_size = omv_i2c_scan(&tof_bus, dev_list, sizeof(dev_list));
        for (int i = 0; i < dev_size && type == -1; i++) {
            switch (dev_list[i]) {
                #if (OMV_ENABLE_TOF_VL53L5CX == 1)
                case (VL53L5CX_ADDR): {
                    type = TOF_VL53L5CX;
                    break;
                }
                #endif
                default:
                    continue;
            }
        }

        if (type == -1 && first_init) {
            first_init = false;
            // Recover bus and scan one more time.
            omv_i2c_pulse_scl(&tof_bus);
            goto TOF_SCAN_RETRY;
        }

        omv_i2c_deinit(&tof_bus);
    }

    // Initialize the detected sensor.
    first_init = true;
    switch (type) {
        case TOF_NONE: {
            return mp_const_none;
        }
        #if (OMV_ENABLE_TOF_VL53L5CX == 1)
        case TOF_VL53L5CX: {
            int error = 0;
            uint8_t isAlive = 0;
            TOF_VL53L5CX_RETRY:
            //vl53l5cx_dev.platform.bus     = tof_bus;
            //vl53l5cx_dev.platform.address = VL53L5CX_ADDRESS;
            omv_i2c_init(&tof_bus, TOF_I2C_ID, OMV_I2C_SPEED_FAST);

            // Check sensor and initialize.
            error |= vl53l5cx_is_alive(&vl53l5cx_dev, &isAlive);
            error |= vl53l5cx_init(&vl53l5cx_dev);

            // Set resolution (number of zones).
            // NOTE: This function must be called before updating the ranging frequency.
            error |= vl53l5cx_set_resolution(&vl53l5cx_dev, VL53L5CX_RESOLUTION_8X8);

            // Set ranging frequency (FPS).
            // For 4x4 the allowed ranging frequency range is 1 -> 60.
            // For 8x8 the allowed ranging frequency range is 1 -> 15.
            error |= vl53l5cx_set_ranging_frequency_hz(&vl53l5cx_dev, 15);

            // Set ranging mode to continuous:
            // The device continuously grabs frames with the set ranging frequency.
            // Maximum ranging depth and ambient immunity are better.
            // This mode is advised for fast ranging measurements or high performances.
            error |= vl53l5cx_set_ranging_mode(&vl53l5cx_dev, VL53L5CX_RANGING_MODE_CONTINUOUS);

            error |= vl53l5cx_set_sharpener_percent(&vl53l5cx_dev, 50);

            // Start ranging.
            error |= vl53l5cx_start_ranging(&vl53l5cx_dev);

            if (error != 0 && first_init) {
                first_init = false;
                omv_i2c_pulse_scl(&tof_bus);
                goto TOF_VL53L5CX_RETRY;
            } else if (error != 0) {
                py_tof_deinit();
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Failed to init the VL53L5CX"));
            }
            tof_sensor = TOF_VL53L5CX;
            tof_width = VL53L5CX_WIDTH;
            tof_height = VL53L5CX_HEIGHT;
            break;
        }
        #endif
        default: {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Failed to detect a supported TOF sensor."));
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tof_init_obj, 0, py_tof_init);

static mp_obj_t py_tof_type() {
    if (tof_sensor == TOF_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(tof_sensor);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_type_obj, py_tof_type);

static mp_obj_t py_tof_width() {
    if (tof_sensor == TOF_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(tof_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_width_obj, py_tof_width);

static mp_obj_t py_tof_height() {
    if (tof_sensor == TOF_NONE) {
        return mp_const_none;
    }
    return mp_obj_new_int(tof_height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_height_obj, py_tof_height);

static mp_obj_t py_tof_refresh() {
    switch (tof_sensor) {
        #if (OMV_ENABLE_TOF_VL53L5CX == 1)
        case TOF_VL53L5CX:
            return mp_obj_new_int(15);
        #endif
        default:
            return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_tof_refresh_obj, py_tof_refresh);

mp_obj_t py_tof_read_depth(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    bool arg_hmirror = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hmirror), false);
    bool arg_vflip = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_vflip), false);
    tof_transposed = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_transpose), false);
    int arg_timeout = py_helper_keyword_int(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_timeout), -1);

    switch (tof_sensor) {
        #if (OMV_ENABLE_TOF_VL53L5CX == 1)
        case TOF_VL53L5CX: {
            fb_alloc_mark();
            float *frame = fb_alloc(VL53L5CX_WIDTH * VL53L5CX_HEIGHT * sizeof(float), FB_ALLOC_PREFER_SPEED);
            tof_vl53l5cx_get_depth(&vl53l5cx_dev, frame, arg_timeout);
            mp_obj_t result = tof_get_depth_obj(VL53L5CX_WIDTH, VL53L5CX_HEIGHT, frame,
                                                arg_hmirror ^ true, arg_vflip, tof_transposed, true);
            fb_alloc_free_till_mark();
            return result;
        }
        #endif
        default:
            return mp_const_none;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tof_read_depth_obj, 0, py_tof_read_depth);

mp_obj_t py_tof_draw_depth(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    image_t *dst_img = py_helper_arg_to_image(args[0], ARG_IMAGE_MUTABLE);

    image_t src_img;
    src_img.pixfmt = PIXFORMAT_GRAYSCALE;

    size_t len;
    mp_obj_t *items, *arg_to;
    mp_obj_get_array(args[1], &len, &items);

    if (len == 3) {
        src_img.w = mp_obj_get_int(items[0]);
        src_img.h = mp_obj_get_int(items[1]);
        mp_obj_get_array_fixed_n(items[2], src_img.w * src_img.h, &arg_to);
    } else if (tof_sensor != TOF_NONE) {
        src_img.w = tof_transposed ? tof_height : tof_width;
        src_img.h = tof_transposed ? tof_width : tof_height;
        // Handle if the user passed an array of the array.
        if (len == 1) {
            mp_obj_get_array_fixed_n(*items, src_img.w * src_img.h, &arg_to);
        } else {
            mp_obj_get_array_fixed_n(args[1], src_img.w * src_img.h, &arg_to);
        }
    } else {
        mp_raise_msg(&mp_type_TypeError, MP_ERROR_TEXT("Invalid depth array!"));
    }

    int arg_x_off = 0;
    int arg_y_off = 0;
    uint offset = 2;
    if (n_args > 2) {
        if (MP_OBJ_IS_TYPE(args[2], &mp_type_tuple) || MP_OBJ_IS_TYPE(args[2], &mp_type_list)) {
            mp_obj_t *arg_vec;
            mp_obj_get_array_fixed_n(args[2], 2, &arg_vec);
            arg_x_off = mp_obj_get_int(arg_vec[0]);
            arg_y_off = mp_obj_get_int(arg_vec[1]);
            offset = 3;
        } else if (n_args > 3) {
            arg_x_off = mp_obj_get_int(args[2]);
            arg_y_off = mp_obj_get_int(args[3]);
            offset = 4;
        } else if (n_args > 2) {
            mp_raise_msg(&mp_type_TypeError, MP_ERROR_TEXT("Expected x and y offset!"));
        }
    }

    float arg_x_scale = 1.f;
    bool got_x_scale = py_helper_keyword_float_maybe(n_args, args,
                                                     offset + 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_scale), &arg_x_scale);

    float arg_y_scale = 1.f;
    bool got_y_scale = py_helper_keyword_float_maybe(n_args, args,
                                                     offset + 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_scale), &arg_y_scale);

    rectangle_t arg_roi;
    py_helper_keyword_rectangle_roi(&src_img, n_args, args, offset + 2, kw_args, &arg_roi);

    float tmp_x_scale = dst_img->w / ((float) arg_roi.w);
    float tmp_y_scale = dst_img->h / ((float) arg_roi.h);
    float tmp_scale = IM_MIN(tmp_x_scale, tmp_y_scale);

    if (n_args == 2) {
        arg_x_off = fast_floorf((dst_img->w - (arg_roi.w * tmp_scale)) / 2.f);
        arg_y_off = fast_floorf((dst_img->h - (arg_roi.h * tmp_scale)) / 2.f);
    }

    if (!got_x_scale) {
        arg_x_scale = tmp_scale;
    }

    if (!got_y_scale) {
        arg_y_scale = tmp_scale;
    }

    int arg_rgb_channel = py_helper_keyword_int(n_args, args, offset + 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_rgb_channel), -1);
    if ((arg_rgb_channel < -1) || (2 < arg_rgb_channel)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("-1 <= rgb_channel <= 2!"));
    }

    int arg_alpha = py_helper_keyword_int(n_args, args, offset + 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 128);
    if ((arg_alpha < 0) || (256 < arg_alpha)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= alpha <= 256!"));
    }

    const uint16_t *color_palette = py_helper_keyword_color_palette(n_args, args, offset + 5, kw_args, rainbow_table);
    const uint8_t *alpha_palette = py_helper_keyword_alpha_palette(n_args, args, offset + 6, kw_args, NULL);

    image_hint_t hint = py_helper_keyword_int(n_args, args, offset + 7, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hint), 0);

    float arg_x_size;
    bool got_x_size = py_helper_keyword_float_maybe(n_args,
                                                    args,
                                                    offset + 8,
                                                    kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_x_size),
                                                    &arg_x_size);

    float arg_y_size;
    bool got_y_size = py_helper_keyword_float_maybe(n_args,
                                                    args,
                                                    offset + 9,
                                                    kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_y_size),
                                                    &arg_y_size);

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

    mp_obj_t scale_obj = py_helper_keyword_object(n_args, args, offset + 10, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale), NULL);
    float min = FLT_MAX, max = -FLT_MAX;

    if (scale_obj) {
        mp_obj_t *arg_scale;
        mp_obj_get_array_fixed_n(scale_obj, 2, &arg_scale);
        min = mp_obj_get_float(arg_scale[0]);
        max = mp_obj_get_float(arg_scale[1]);
    } else {
        for (int i = 0, ii = src_img.w * src_img.h; i < ii; i++) {
            float temp = mp_obj_get_float(arg_to[i]);
            if (temp < min) {
                min = temp;
            }
            if (temp > max) {
                max = temp;
            }
        }
    }

    fb_alloc_mark();

    src_img.data = fb_alloc(src_img.w * src_img.h * sizeof(uint8_t), FB_ALLOC_NO_HINT);
    tof_fill_image_float_obj(&src_img, arg_to, min, max);

    imlib_draw_image(dst_img, &src_img, arg_x_off, arg_y_off, arg_x_scale, arg_y_scale, &arg_roi,
                     arg_rgb_channel, arg_alpha, color_palette, alpha_palette, hint, NULL, NULL, NULL);

    fb_alloc_free_till_mark();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tof_draw_depth_obj, 2, py_tof_draw_depth);

mp_obj_t py_tof_snapshot(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    if (tof_sensor == TOF_NONE) {
        return mp_const_none;
    }

    bool arg_hmirror = py_helper_keyword_int(n_args, args, 0, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hmirror), false);
    bool arg_vflip = py_helper_keyword_int(n_args, args, 1, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_vflip), false);
    bool arg_transpose = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_transpose), false);

    image_t src_img;
    src_img.w = arg_transpose ? tof_height : tof_width;
    src_img.h = arg_transpose ? tof_width : tof_height;
    src_img.pixfmt = PIXFORMAT_GRAYSCALE;

    float arg_x_scale = 1.f;
    bool got_x_scale = py_helper_keyword_float_maybe(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_x_scale), &arg_x_scale);

    float arg_y_scale = 1.f;
    bool got_y_scale = py_helper_keyword_float_maybe(n_args, args, 4, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_y_scale), &arg_y_scale);

    rectangle_t arg_roi;
    py_helper_keyword_rectangle_roi(&src_img, n_args, args, 5, kw_args, &arg_roi);

    int arg_rgb_channel = py_helper_keyword_int(n_args, args, 6, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_rgb_channel), -1);
    if ((arg_rgb_channel < -1) || (2 < arg_rgb_channel)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("-1 <= rgb_channel <= 2!"));
    }

    int arg_alpha = py_helper_keyword_int(n_args, args, 7, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_alpha), 128);
    if ((arg_alpha < 0) || (256 < arg_alpha)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("0 <= alpha <= 256!"));
    }

    const uint16_t *color_palette = py_helper_keyword_color_palette(n_args, args, 8, kw_args, rainbow_table);
    const uint8_t *alpha_palette = py_helper_keyword_alpha_palette(n_args, args, 9, kw_args, NULL);

    image_hint_t hint = py_helper_keyword_int(n_args, args, 10, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_hint), 0);

    float arg_x_size;
    bool got_x_size = py_helper_keyword_float_maybe(n_args, args, 11, kw_args,
                                                    MP_OBJ_NEW_QSTR(MP_QSTR_x_size), &arg_x_size);

    float arg_y_size;
    bool got_y_size = py_helper_keyword_float_maybe(n_args, args, 12, kw_args,
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

    mp_obj_t scale_obj = py_helper_keyword_object(n_args, args, 13, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_scale), NULL);
    float min, max;

    if (scale_obj) {
        mp_obj_t *arg_scale;
        mp_obj_get_array_fixed_n(scale_obj, 2, &arg_scale);
        min = mp_obj_get_float(arg_scale[0]);
        max = mp_obj_get_float(arg_scale[1]);
    }

    int arg_pixformat = py_helper_keyword_int(n_args, args, 14, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_pixformat), PIXFORMAT_RGB565);
    if ((arg_pixformat != PIXFORMAT_GRAYSCALE) && (arg_pixformat != PIXFORMAT_RGB565)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid pixformat!"));
    }

    mp_obj_t copy_to_fb_obj = py_helper_keyword_object(n_args, args, 15, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_copy_to_fb), NULL);
    bool copy_to_fb = false;
    image_t *arg_other = NULL;

    if (copy_to_fb_obj) {
        if (mp_obj_is_integer(copy_to_fb_obj)) {
            copy_to_fb = mp_obj_get_int(copy_to_fb_obj);
        } else {
            arg_other = py_helper_arg_to_image(copy_to_fb_obj, ARG_IMAGE_UNCOMPRESSED);
        }
    }

    if (copy_to_fb) {
        framebuffer_update_jpeg_buffer();
    }

    image_t dst_img;
    dst_img.w = fast_floorf(arg_roi.w * arg_x_scale);
    dst_img.h = fast_floorf(arg_roi.h * arg_y_scale);
    dst_img.pixfmt = (arg_pixformat == PIXFORMAT_RGB565) ? PIXFORMAT_RGB565 : PIXFORMAT_GRAYSCALE;

    size_t size = image_size(&dst_img);

    if (copy_to_fb) {
        py_helper_set_to_framebuffer(&dst_img);
    } else if (arg_other) {
        bool fb = py_helper_is_equal_to_framebuffer(arg_other);
        size_t buf_size = fb ? framebuffer_get_buffer_size() : image_size(arg_other);
        PY_ASSERT_TRUE_MSG((size <= buf_size),
                           "The new image won't fit in the target frame buffer!");
        dst_img.data = arg_other->data;
        memcpy(arg_other, &dst_img, sizeof(image_t));
        py_helper_update_framebuffer(&dst_img);
    } else {
        dst_img.data = xalloc(size);
    }

    int arg_timeout = py_helper_keyword_int(n_args, args, 16, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_timeout), 1000);

    fb_alloc_mark();
    src_img.data = fb_alloc(src_img.w * src_img.h * sizeof(uint8_t), FB_ALLOC_NO_HINT);

    switch (tof_sensor) {
        #if (OMV_ENABLE_TOF_VL53L5CX == 1)
        case TOF_VL53L5CX: {
            float *frame = fb_alloc(VL53L5CX_WIDTH * VL53L5CX_HEIGHT * sizeof(float), FB_ALLOC_PREFER_SPEED);
            tof_vl53l5cx_get_depth(&vl53l5cx_dev, frame, arg_timeout);

            if (!scale_obj) {
                fast_get_min_max(frame, VL53L5CX_WIDTH * VL53L5CX_HEIGHT, &min, &max);
            }

            imlib_fill_image_from_float(&src_img, VL53L5CX_WIDTH, VL53L5CX_HEIGHT, frame, min, max,
                                        arg_hmirror ^ true, arg_vflip, arg_transpose, true);
            fb_free();
            break;
        }
        #endif
        default:
            break;
    }

    imlib_draw_image(&dst_img, &src_img, 0, 0, arg_x_scale, arg_y_scale,
                     &arg_roi, arg_rgb_channel, arg_alpha, color_palette, alpha_palette,
                     (hint & (~IMAGE_HINT_CENTER)) | IMAGE_HINT_BLACK_BACKGROUND, NULL, NULL, NULL);

    fb_alloc_free_till_mark();

    return py_image_from_struct(&dst_img);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_tof_snapshot_obj, 0, py_tof_snapshot);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_tof)                },
    { MP_ROM_QSTR(MP_QSTR_TOF_NONE),            MP_ROM_INT(TOF_NONE)                    },
    #if (OMV_ENABLE_TOF_VL53L5CX == 1)
    { MP_ROM_QSTR(MP_QSTR_TOF_VL53L5CX),        MP_ROM_INT(TOF_VL53L5CX)                },
    #endif
    { MP_ROM_QSTR(MP_QSTR_PALETTE_RAINBOW),     MP_ROM_INT(COLOR_PALETTE_RAINBOW)       },
    { MP_ROM_QSTR(MP_QSTR_PALETTE_IRONBOW),     MP_ROM_INT(COLOR_PALETTE_IRONBOW)       },
    { MP_ROM_QSTR(MP_QSTR_GRAYSCALE),           MP_ROM_INT(PIXFORMAT_GRAYSCALE)         },
    { MP_ROM_QSTR(MP_QSTR_RGB565),              MP_ROM_INT(PIXFORMAT_RGB565)            },
    { MP_ROM_QSTR(MP_QSTR_init),                MP_ROM_PTR(&py_tof_init_obj)            },
    { MP_ROM_QSTR(MP_QSTR_deinit),              MP_ROM_PTR(&py_tof_deinit_obj)          },
    { MP_ROM_QSTR(MP_QSTR_type),                MP_ROM_PTR(&py_tof_type_obj)            },
    { MP_ROM_QSTR(MP_QSTR_width),               MP_ROM_PTR(&py_tof_width_obj)           },
    { MP_ROM_QSTR(MP_QSTR_height),              MP_ROM_PTR(&py_tof_height_obj)          },
    { MP_ROM_QSTR(MP_QSTR_refresh),             MP_ROM_PTR(&py_tof_refresh_obj)         },
    { MP_ROM_QSTR(MP_QSTR_read_depth),          MP_ROM_PTR(&py_tof_read_depth_obj)      },
    { MP_ROM_QSTR(MP_QSTR_draw_depth),          MP_ROM_PTR(&py_tof_draw_depth_obj)      },
    { MP_ROM_QSTR(MP_QSTR_snapshot),            MP_ROM_PTR(&py_tof_snapshot_obj)        }
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t tof_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_tof, tof_module);
#endif
