/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * CSI Python module.
 */
#include <stdarg.h>
#include <stdio.h>
#include "py/mphal.h"
#include "py/runtime.h"

#if MICROPY_PY_CSI_NG

#include "omv_csi.h"
#include "omv_gpio.h"

#include "imlib.h"
#include "py_assert.h"
#include "py_image.h"
#if MICROPY_PY_IMU
#include "py_imu.h"
#endif
#include "omv_boardconfig.h"
#include "omv_i2c.h"
#include "py_helper.h"
#include "framebuffer.h"

#define omv_csi_raise_error(err) \
    mp_raise_msg(&mp_type_RuntimeError, (mp_rom_error_text_t) omv_csi_strerror(err))

#define omv_csi_print_error(op)  \
    printf("\x1B[31mWARNING: %s control is not supported by this image sensor.\x1B[0m\n", op);

typedef struct _py_csi_obj_t {
    mp_obj_base_t base;
    omv_csi_t *csi;
    mp_obj_t vsync_cb;
    mp_obj_t frame_cb;
    framebuffer_t *fb; // Track dynamically allocated FBs.
} py_csi_obj_t;

const mp_obj_type_t py_csi_type;

#if MICROPY_PY_IMU
static void omv_csi_set_rotation(omv_csi_t *csi, int pitch_deadzone, int roll_activezone) {
    if (omv_csi_get_auto_rotation(csi)) {
        float pitch = py_imu_pitch_rotation();
        if (((pitch <= (90 - pitch_deadzone)) || ((90 + pitch_deadzone) < pitch))
            && ((pitch <= (270 - pitch_deadzone)) || ((270 + pitch_deadzone) < pitch))) {
            // disable when 90 or 270
            float roll = py_imu_roll_rotation();
            if (((360 - roll_activezone) <= roll) || (roll < (0 + roll_activezone)) ) {
                // center is 0/360, upright
                omv_csi_set_hmirror(csi, false);
                omv_csi_set_vflip(csi, false);
                omv_csi_set_transpose(csi, false);
            } else if (((270 - roll_activezone) <= roll) && (roll < (270 + roll_activezone))) {
                // center is 270, rotated right
                omv_csi_set_hmirror(csi, true);
                omv_csi_set_vflip(csi, false);
                omv_csi_set_transpose(csi, true);
            } else if (((180 - roll_activezone) <= roll) && (roll < (180 + roll_activezone))) {
                // center is 180, upside down
                omv_csi_set_hmirror(csi, true);
                omv_csi_set_vflip(csi, true);
                omv_csi_set_transpose(csi, false);
            } else if (((90 - roll_activezone) <= roll) && (roll < (90 + roll_activezone))) {
                // center is 90, rotated left
                omv_csi_set_hmirror(csi, false);
                omv_csi_set_vflip(csi, true);
                omv_csi_set_transpose(csi, true);
            }
        }
    }
}
#endif // MICROPY_PY_IMU

static mp_obj_t py_csi_devices() {
    mp_obj_t dev_list = mp_obj_new_list(0, NULL);

    for (size_t i=0; i<OMV_CSI_MAX_DEVICES; i++) {
        omv_csi_t *csi = &csi_all[i];
        if (csi->detected) {
            mp_obj_list_append(dev_list, mp_obj_new_int(csi->chip_id));
        }
    }

    return dev_list;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_csi_devices_obj, py_csi_devices);

static mp_obj_t py_csi_deinit(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Abort any ongoing capture.
    omv_csi_abort(self->csi, true, false);

    // Reset FB pointer (realloc'd in make_new).
    if (self->csi->fb->dynamic) {
        self->fb = NULL;
        self->csi->fb = NULL;
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_deinit_obj, py_csi_deinit);

static mp_obj_t py_csi_reset(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_hard };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_hard, MP_ARG_BOOL | MP_ARG_KW_ONLY,  {.u_bool = true} },
    };

    // Parse args.
    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int error = omv_csi_reset(self->csi, args[ARG_hard].u_bool);
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_reset_obj, 1, py_csi_reset);

static mp_obj_t py_csi_shutdown(mp_obj_t self_in, mp_obj_t enable) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    int error = omv_csi_shutdown(self->csi, mp_obj_is_true(enable));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_shutdown_obj, py_csi_shutdown);

static mp_obj_t py_csi_sleep(mp_obj_t self_in, mp_obj_t enable) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    int error = omv_csi_sleep(self->csi, mp_obj_is_true(enable));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_sleep_obj, py_csi_sleep);

static mp_obj_t py_csi_flush(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    image_t tmp;
    framebuffer_init_image(self->csi->fb, &tmp);
    framebuffer_update_jpeg_buffer(&tmp);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_flush_obj, py_csi_flush);

static mp_obj_t py_csi_snapshot(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_time, ARG_frames, ARG_update, ARG_blocking, ARG_image };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_time, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1} },
        { MP_QSTR_frames, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1} },
        { MP_QSTR_update, MP_ARG_BOOL | MP_ARG_KW_ONLY,  {.u_bool = true} },
        { MP_QSTR_blocking, MP_ARG_BOOL | MP_ARG_KW_ONLY,  {.u_bool = true} },
        { MP_QSTR_image, MP_ARG_OBJ | MP_ARG_KW_ONLY,  {.u_rom_obj = MP_ROM_NONE} },
    };

    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t flags = 0;
    image_t image = {0};
    mp_int_t time = args[ARG_time].u_int;
    mp_int_t frames = args[ARG_frames].u_int;

    if (args[ARG_update].u_bool) {
        flags |= OMV_CSI_CAPTURE_FLAGS_UPDATE;
    }

    if (!args[ARG_blocking].u_bool) {
        flags |= OMV_CSI_CAPTURE_FLAGS_NBLOCK;
    }

    if (time == -1 && frames == -1) {   
        int error = omv_csi_snapshot(self->csi, &image, flags);
        if (error != 0) {
            if (error == OMV_CSI_ERROR_WOULD_BLOCK &&
                (flags & OMV_CSI_CAPTURE_FLAGS_NBLOCK)) {
                return mp_const_none;
            }
            omv_csi_raise_error(error);
        }

        // If an image is provided update it and return.
        if (args[ARG_image].u_obj != mp_const_none) {
            image_t *other = py_helper_arg_to_image(args[ARG_image].u_obj, ARG_IMAGE_MUTABLE);
            fb_alloc_mark();
            imlib_draw_image(other, &image, 0, 0, 1.f, 1.f, NULL, -1, 255, NULL, NULL,
                             IMAGE_HINT_SCALE_ASPECT_IGNORE, NULL, NULL, NULL);
            fb_alloc_free_till_mark();
            return mp_const_none;
        }

        #if MICROPY_PY_IMU
        // +-10 degree dead-zone around pitch 90/270.
        // +-35 degree active-zone around roll 0/90/180/270/360.
        omv_csi_set_rotation(self->csi, 10, 35);
        #endif // MICROPY_PY_IMU
 
        return py_image_from_struct(&image);
    } else {
        uint32_t millis = mp_hal_ticks_ms();

        while (time != -1 || frames != -1) {
            if (frames != -1 && frames-- < 0) {
                break;
            }

            if (time != -1 && mp_hal_ticks_ms() - millis > time) {
                break;
            }

            int error = omv_csi_snapshot(self->csi, NULL, flags);
            if (error != 0) {
                omv_csi_raise_error(error);
            }
        }
    
        return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_snapshot_obj, 1, py_csi_snapshot);

static mp_obj_t py_csi_width(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(resolution[self->csi->framesize][0]);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_width_obj, py_csi_width);

static mp_obj_t py_csi_height(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(resolution[self->csi->framesize][1]);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_height_obj, py_csi_height);

static mp_obj_t py_csi_cid(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(self->csi->chip_id);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_cid_obj, py_csi_cid);

static mp_obj_t py_csi_readable(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    vbuffer_t *head = framebuffer_get_head(self->csi->fb, FB_PEEK);
    return mp_obj_new_bool(head != NULL);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_readable_obj, py_csi_readable);

static mp_obj_t py_csi_pixformat(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        if (self->csi->pixformat == PIXFORMAT_INVALID) {
            omv_csi_raise_error(OMV_CSI_ERROR_INVALID_PIXFORMAT);
        }
        return mp_obj_new_int(self->csi->pixformat);
    }

    int error = omv_csi_set_pixformat(self->csi, mp_obj_get_int(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_pixformat_obj, 1, 2, py_csi_pixformat);

static mp_obj_t py_csi_framesize(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        if (self->csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
            omv_csi_raise_error(OMV_CSI_ERROR_INVALID_FRAMESIZE);
        }
        return mp_obj_new_int(self->csi->framesize);
    }

    int error = omv_csi_set_framesize(self->csi, mp_obj_get_int(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_framesize_obj, 1, 2, py_csi_framesize);

static mp_obj_t py_csi_framerate(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        if (self->csi->framerate == 0) {
            omv_csi_raise_error(OMV_CSI_ERROR_INVALID_FRAMERATE);
        }

        return mp_obj_new_int(self->csi->framerate);
    }

    int error = omv_csi_set_framerate(self->csi, mp_obj_get_int(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_framerate_obj, 1, 2, py_csi_framerate);

static mp_obj_t py_csi_window(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (self->csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        omv_csi_raise_error(OMV_CSI_ERROR_INVALID_FRAMESIZE);
    }

    if (n_args == 1) {
        return mp_obj_new_tuple(4, (mp_obj_t []) {mp_obj_new_int(framebuffer_get_x(self->csi->fb)),
                                                  mp_obj_new_int(framebuffer_get_y(self->csi->fb)),
                                                  mp_obj_new_int(framebuffer_get_u(self->csi->fb)),
                                                  mp_obj_new_int(framebuffer_get_v(self->csi->fb))});
    }

    mp_obj_t *array;
    mp_uint_t array_len;
    mp_obj_get_array(args[1], &array_len, &array);

    rectangle_t r;
    rectangle_t t = {
        .x = 0,
        .y = 0,
        .w = resolution[self->csi->framesize][0],
        .h = resolution[self->csi->framesize][1],
    };

    if (array_len == 2) {
        r.w = mp_obj_get_int(array[0]);
        r.h = mp_obj_get_int(array[1]);
        r.x = (t.w / 2) - (r.w / 2);
        r.y = (t.h / 2) - (r.h / 2);
    } else if (array_len == 4) {
        r.x = mp_obj_get_int(array[0]);
        r.y = mp_obj_get_int(array[1]);
        r.w = mp_obj_get_int(array[2]);
        r.h = mp_obj_get_int(array[3]);
    } else {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Expected (w, h) or (x, y, w, h) tuple/list."));
    }

    if ((r.w < 1) || (r.h < 1)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid ROI dimensions!"));
    }

    if (!rectangle_overlap(&r, &t)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("ROI does not overlap on the image!"));
    }

    rectangle_intersected(&r, &t);

    int error = omv_csi_set_windowing(self->csi, r.x, r.y, r.w, r.h);
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_window_obj, 1, 2, py_csi_window);

static mp_obj_t py_csi_gainceiling(mp_obj_t self_in, mp_obj_t gainceiling) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    omv_csi_gainceiling_t gain;

    switch (mp_obj_get_int(gainceiling)) {
        case 2:
            gain = OMV_CSI_GAINCEILING_2X;
            break;
        case 4:
            gain = OMV_CSI_GAINCEILING_4X;
            break;
        case 8:
            gain = OMV_CSI_GAINCEILING_8X;
            break;
        case 16:
            gain = OMV_CSI_GAINCEILING_16X;
            break;
        case 32:
            gain = OMV_CSI_GAINCEILING_32X;
            break;
        case 64:
            gain = OMV_CSI_GAINCEILING_64X;
            break;
        case 128:
            gain = OMV_CSI_GAINCEILING_128X;
            break;
        default:
            omv_csi_raise_error(OMV_CSI_ERROR_INVALID_ARGUMENT);
            break;
    }

    if (omv_csi_set_gainceiling(self->csi, gain) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_gainceiling_obj, py_csi_gainceiling);

static mp_obj_t py_csi_brightness(mp_obj_t self_in, mp_obj_t brightness) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (omv_csi_set_brightness(self->csi, mp_obj_get_int(brightness)) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_brightness_obj, py_csi_brightness);

static mp_obj_t py_csi_contrast(mp_obj_t self_in, mp_obj_t contrast) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (omv_csi_set_contrast(self->csi, mp_obj_get_int(contrast)) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_contrast_obj, py_csi_contrast);

static mp_obj_t py_csi_saturation(mp_obj_t self_in, mp_obj_t saturation) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (omv_csi_set_saturation(self->csi, mp_obj_get_int(saturation)) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_saturation_obj, py_csi_saturation);

static mp_obj_t py_csi_quality(mp_obj_t self_in, mp_obj_t qs) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    int q = mp_obj_get_int(qs);
    PY_ASSERT_TRUE((q >= 0 && q <= 100));

    // Invert and map from 0 to 255
    q = 255 * (100 - q) / 100;

    if (omv_csi_set_quality(self->csi, q) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_quality_obj, py_csi_quality);

static mp_obj_t py_csi_colorbar(mp_obj_t self_in, mp_obj_t enable) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (omv_csi_set_colorbar(self->csi, mp_obj_is_true(enable)) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_colorbar_obj, py_csi_colorbar);

static mp_obj_t py_csi_auto_gain(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_enable, ARG_gain_db, ARG_gain_db_ceiling };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_enable, MP_ARG_BOOL | MP_ARG_REQUIRED },
        { MP_QSTR_gain_db, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_gain_db_ceiling, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };

    // Parse args.
    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    bool enable = args[ARG_enable].u_bool;
    float gain_db = py_helper_arg_to_float(args[ARG_gain_db].u_obj, NAN);
    float gain_db_ceiling = py_helper_arg_to_float(args[ARG_gain_db_ceiling].u_obj, NAN);

    int error = omv_csi_set_auto_gain(self->csi, enable, gain_db, gain_db_ceiling);
    if (error != 0) {
        if (error != OMV_CSI_ERROR_CTL_UNSUPPORTED) {
            omv_csi_raise_error(error);
        }
        omv_csi_print_error("Auto Gain");
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_auto_gain_obj, 1, py_csi_auto_gain);

static mp_obj_t py_csi_gain_db(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    float gain_db;

    int error = omv_csi_get_gain_db(self->csi, &gain_db);
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_obj_new_float(gain_db);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_gain_db_obj, py_csi_gain_db);

static mp_obj_t py_csi_auto_exposure(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_enable, ARG_exposure_us };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_enable, MP_ARG_BOOL | MP_ARG_REQUIRED },
        { MP_QSTR_exposure_us, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1} },
    };

    // Parse args.
    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int error = omv_csi_set_auto_exposure(self->csi, args[ARG_enable].u_bool, args[ARG_exposure_us].u_int);
    if (error != 0) {
        if (error != OMV_CSI_ERROR_CTL_UNSUPPORTED) {
            omv_csi_raise_error(error);
        }
        omv_csi_print_error("Auto Exposure");
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_auto_exposure_obj, 1, py_csi_auto_exposure);

static mp_obj_t py_csi_exposure_us(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int exposure_us;

    int error = omv_csi_get_exposure_us(self->csi, &exposure_us);
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_obj_new_int(exposure_us);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_exposure_us_obj, py_csi_exposure_us);

static mp_obj_t py_csi_auto_whitebal(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_enable, ARG_rgb_gain_db };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_enable, MP_ARG_BOOL | MP_ARG_REQUIRED },
        { MP_QSTR_rgb_gain_db, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
    };

    // Parse args.
    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    bool enable = args[ARG_enable].u_bool;
    float rgb_gain_db[3] = {NAN, NAN, NAN};
    py_helper_arg_to_float_array(args[ARG_rgb_gain_db].u_obj, rgb_gain_db, 3);

    int error = omv_csi_set_auto_whitebal(self->csi, enable, rgb_gain_db[0], rgb_gain_db[1], rgb_gain_db[2]);
    if (error != 0) {
        if (error != OMV_CSI_ERROR_CTL_UNSUPPORTED) {
            omv_csi_raise_error(error);
        }
        omv_csi_print_error("Auto White Balance");
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_auto_whitebal_obj, 1, py_csi_auto_whitebal);

static mp_obj_t py_csi_rgb_gain_db(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    float r_gain_db = 0.0, g_gain_db = 0.0, b_gain_db = 0.0;

    int error = omv_csi_get_rgb_gain_db(self->csi, &r_gain_db, &g_gain_db, &b_gain_db);
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_obj_new_tuple(3, (mp_obj_t []) {
        mp_obj_new_float(r_gain_db),
        mp_obj_new_float(g_gain_db),
        mp_obj_new_float(b_gain_db)
    });
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_rgb_gain_db_obj, py_csi_rgb_gain_db);

static mp_obj_t py_csi_auto_blc(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_enable, ARG_regs };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_enable, MP_ARG_BOOL | MP_ARG_REQUIRED },
        { MP_QSTR_regs,   MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
    };

    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int regs[self->csi->blc_size];
    int enable = args[ARG_enable].u_bool;
    bool regs_present = args[ARG_regs].u_obj != mp_const_none;

    if (regs_present) {
        mp_obj_t *arg_array;
        mp_obj_get_array_fixed_n(args[ARG_regs].u_obj, self->csi->blc_size, &arg_array);
        for (uint32_t i = 0; i < self->csi->blc_size; i++) {
            regs[i] = mp_obj_get_int(arg_array[i]);
        }
    }

    int error = omv_csi_set_auto_blc(self->csi, enable, regs_present ? regs : NULL);
    if (error != 0) {
        if (error != OMV_CSI_ERROR_CTL_UNSUPPORTED) {
            omv_csi_raise_error(error);
        }
        omv_csi_print_error("Auto BLC");
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_auto_blc_obj, 1, py_csi_auto_blc);

static mp_obj_t py_csi_blc_regs(mp_obj_t self_in) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int regs[self->csi->blc_size];

    int error = omv_csi_get_blc_regs(self->csi, regs);
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    mp_obj_list_t *l = mp_obj_new_list(self->csi->blc_size, NULL);
    for (uint32_t i = 0; i < self->csi->blc_size; i++) {
        l->items[i] = mp_obj_new_int(regs[i]);
    }

    return l;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_csi_blc_regs_obj, py_csi_blc_regs);

static mp_obj_t py_csi_hmirror(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return mp_obj_new_bool(omv_csi_get_hmirror(self->csi));
    }

    int error = omv_csi_set_hmirror(self->csi, mp_obj_is_true(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_hmirror_obj, 1, 2, py_csi_hmirror);

static mp_obj_t py_csi_vflip(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return mp_obj_new_bool(omv_csi_get_vflip(self->csi));
    }

    int error = omv_csi_set_vflip(self->csi, mp_obj_is_true(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_vflip_obj, 1, 2, py_csi_vflip);

static mp_obj_t py_csi_transpose(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return mp_obj_new_bool(omv_csi_get_transpose(self->csi));
    }

    int error = omv_csi_set_transpose(self->csi, mp_obj_is_true(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_transpose_obj, 1, 2, py_csi_transpose);

static mp_obj_t py_csi_auto_rotation(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return mp_obj_new_bool(omv_csi_get_auto_rotation(self->csi));
    }

    int error = omv_csi_set_auto_rotation(self->csi, mp_obj_is_true(args[1]));
    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_auto_rotation_obj, 1, 2, py_csi_auto_rotation);

static mp_obj_t py_csi_framebuffers(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    framebuffer_t *fb = self->csi->fb;

    if (n_args == 1) {
        return mp_obj_new_int(fb->n_buffers);
    }

    mp_int_t num = mp_obj_get_int(args[1]);

    if (num < 1) {
        omv_csi_raise_error(OMV_CSI_ERROR_INVALID_ARGUMENT);
    }

    if (num != fb->n_buffers) {
        int error = omv_csi_set_framebuffers(self->csi, num);
        if (error != 0) {
            omv_csi_raise_error(error);
        }
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_framebuffers_obj, 1, 2, py_csi_framebuffers);

static mp_obj_t py_csi_special_effect(mp_obj_t self_in, mp_obj_t sde) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (omv_csi_set_special_effect(self->csi, mp_obj_get_int(sde)) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_special_effect_obj, py_csi_special_effect);

static mp_obj_t py_csi_lens_correction(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_enable, ARG_radi, ARG_coef };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_enable, MP_ARG_BOOL | MP_ARG_REQUIRED },
        { MP_QSTR_radi,   MP_ARG_BOOL | MP_ARG_REQUIRED },
        { MP_QSTR_coef,   MP_ARG_BOOL | MP_ARG_REQUIRED },
    };

    py_csi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (omv_csi_set_lens_correction(self->csi,
                                    args[ARG_enable].u_bool,
                                    args[ARG_radi].u_int,
                                    args[ARG_coef].u_int) != 0) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_csi_lens_correction_obj, 1, py_csi_lens_correction);

static void omv_csi_vsync_callback(void *data) {
    py_csi_obj_t *self = data;
    
    if (mp_obj_is_callable(self->vsync_cb)) {
        uint32_t vsync_state = 0;
        #ifdef OMV_CSI_VSYNC_PIN
        vsync_state = omv_gpio_read(OMV_CSI_VSYNC_PIN);
        #endif
        mp_call_function_1(self->vsync_cb, mp_obj_new_int(vsync_state));
    }
}

static mp_obj_t py_csi_vsync_callback(size_t n_args, const mp_obj_t *args) {
    omv_csi_cb_t cb;
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return self->vsync_cb;
    }
    
    if (!mp_obj_is_callable(args[1])) {
        self->vsync_cb = mp_const_none;
        cb = (omv_csi_cb_t) { NULL, NULL };
    } else {
        self->vsync_cb = args[1];
        cb = (omv_csi_cb_t) { .fun = omv_csi_vsync_callback, .arg = self };
    }

    omv_csi_set_vsync_callback(self->csi, cb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_vsync_callback_obj, 1, 2, py_csi_vsync_callback);

static void omv_csi_frame_callback(void *data) {
    py_csi_obj_t *self = data;

    if (mp_obj_is_callable(self->frame_cb)) {
        mp_call_function_0(self->frame_cb);
    }
}

static mp_obj_t py_csi_frame_callback(size_t n_args, const mp_obj_t *args) {
    omv_csi_cb_t cb;
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        return self->frame_cb;
    }

    if (!mp_obj_is_callable(args[1])) {
        self->frame_cb = mp_const_none;
        cb = (omv_csi_cb_t) { NULL, NULL };
    } else {
        self->frame_cb = args[1];
        cb = (omv_csi_cb_t) { .fun = omv_csi_frame_callback, .arg = self };
    }

    omv_csi_set_frame_callback(self->csi, cb);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_frame_callback_obj, 1, 2, py_csi_frame_callback);

static mp_obj_t py_csi_ioctl(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int request = mp_obj_get_int(args[1]);

    mp_obj_t ret_obj = mp_const_none;
    int error = OMV_CSI_ERROR_INVALID_ARGUMENT;

    // Skip self + request so ioctl args start at args[0].
    args += 2;
    n_args -= 2;

    switch (request) {
        case OMV_CSI_IOCTL_SET_READOUT_WINDOW: {
            if (n_args == 1) {
                int x, y, w, h;
                mp_obj_t *array;
                mp_uint_t array_len;
                mp_obj_get_array(args[0], &array_len, &array);

                if (array_len == 4) {
                    x = mp_obj_get_int(array[0]);
                    y = mp_obj_get_int(array[1]);
                    w = mp_obj_get_int(array[2]);
                    h = mp_obj_get_int(array[3]);
                } else if (array_len == 2) {
                    w = mp_obj_get_int(array[0]);
                    h = mp_obj_get_int(array[1]);
                    x = 0;
                    y = 0;
                } else {
                    mp_raise_msg(&mp_type_ValueError,
                                 MP_ERROR_TEXT("Expected (w, h) or (x, y, w, h) tuple/list."));
                }

                error = omv_csi_ioctl(self->csi, request, x, y, w, h);
            }
            break;
        }

        case OMV_CSI_IOCTL_GET_READOUT_WINDOW: {
            int x, y, w, h;
            error = omv_csi_ioctl(self->csi, request, &x, &y, &w, &h);
            if (error == 0) {
                ret_obj = mp_obj_new_tuple(4, (mp_obj_t []) {mp_obj_new_int(x),
                                                             mp_obj_new_int(y),
                                                             mp_obj_new_int(w),
                                                             mp_obj_new_int(h)});
            }
            break;
        }

        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE:
        case OMV_CSI_IOCTL_SET_FOV_WIDE:
        case OMV_CSI_IOCTL_SET_NIGHT_MODE: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            }
            break;
        }

        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE:
        case OMV_CSI_IOCTL_GET_FOV_WIDE:
        case OMV_CSI_IOCTL_GET_NIGHT_MODE: {
            int enabled;
            error = omv_csi_ioctl(self->csi, request, &enabled);
            if (error == 0) {
                ret_obj = mp_obj_new_bool(enabled);
            }
            break;
        }

        #if (OMV_OV5640_AF_ENABLE == 1)
        case OMV_CSI_IOCTL_TRIGGER_AUTO_FOCUS:
        case OMV_CSI_IOCTL_PAUSE_AUTO_FOCUS:
        case OMV_CSI_IOCTL_RESET_AUTO_FOCUS: {
            error = omv_csi_ioctl(self->csi, request);
            break;
        }
        case OMV_CSI_IOCTL_WAIT_ON_AUTO_FOCUS: {
            error = omv_csi_ioctl(self->csi, request, (n_args < 1) ? 5000 : mp_obj_get_int(args[0]));
            break;
        }
        #endif

        case OMV_CSI_IOCTL_LEPTON_GET_WIDTH: {
            int width;
            error = omv_csi_ioctl(self->csi, request, &width);
            if (error == 0) {
                ret_obj = mp_obj_new_int(width);
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_GET_HEIGHT: {
            int height;
            error = omv_csi_ioctl(self->csi, request, &height);
            if (error == 0) {
                ret_obj = mp_obj_new_int(height);
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_GET_RADIOMETRY: {
            int radiometry;
            error = omv_csi_ioctl(self->csi, request, &radiometry);
            if (error == 0) {
                ret_obj = mp_obj_new_int(radiometry);
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_GET_REFRESH: {
            int refresh;
            error = omv_csi_ioctl(self->csi, request, &refresh);
            if (error == 0) {
                ret_obj = mp_obj_new_int(refresh);
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_GET_RESOLUTION: {
            int resolution;
            error = omv_csi_ioctl(self->csi, request, &resolution);
            if (error == 0) {
                ret_obj = mp_obj_new_int(resolution);
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_RUN_COMMAND: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_SET_ATTRIBUTE: {
            if (n_args == 2) {
                size_t data_len;
                int command = mp_obj_get_int(args[0]);
                uint16_t *data = (uint16_t *) mp_obj_str_get_data(args[1], &data_len);
                PY_ASSERT_TRUE_MSG(data_len > 0, "0 bytes transferred!");
                error = omv_csi_ioctl(self->csi, request, command, data, data_len / sizeof(uint16_t));
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_GET_ATTRIBUTE: {
            if (n_args == 2) {
                int command = mp_obj_get_int(args[0]);
                size_t data_len = mp_obj_get_int(args[1]);
                PY_ASSERT_TRUE_MSG(data_len > 0, "0 bytes transferred!");
                uint16_t *data = m_malloc(data_len * sizeof(uint16_t));
                error = omv_csi_ioctl(self->csi, request, command, data, data_len);
                if (error == 0) {
                    ret_obj = mp_obj_new_bytearray_by_ref(data_len * sizeof(uint16_t), data);
                }
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_GET_FPA_TEMP:
        case OMV_CSI_IOCTL_LEPTON_GET_AUX_TEMP: {
            int temp;
            error = omv_csi_ioctl(self->csi, request, &temp);
            if (error == 0) {
                ret_obj = mp_obj_new_float((((float) temp) / 100) - 273.15f);
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_SET_MODE:
            if (n_args == 2) {
                int high_temp = (n_args == 2) ? false : mp_obj_get_int(args[1]);
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]), high_temp);
            }
            break;

        case OMV_CSI_IOCTL_LEPTON_GET_MODE: {
            int enabled, high_temp;
            error = omv_csi_ioctl(self->csi, request, &enabled, &high_temp);
            if (error == 0) {
                ret_obj = mp_obj_new_tuple(2, (mp_obj_t []) {
                        mp_obj_new_bool(enabled), mp_obj_new_bool(high_temp)
                        });
            }
            break;
        }

        case OMV_CSI_IOCTL_LEPTON_SET_RANGE:
            if (n_args == 2) {
                // GCC will not let us pass floats to ... so we have to pass float pointers instead.
                float min = mp_obj_get_float(args[0]);
                float max = mp_obj_get_float(args[1]);
                error = omv_csi_ioctl(self->csi, request, &min, &max);
            }
            break;

        case OMV_CSI_IOCTL_LEPTON_GET_RANGE: {
            float min, max;
            error = omv_csi_ioctl(self->csi, request, &min, &max);
            if (error == 0) {
                ret_obj = mp_obj_new_tuple(2, (mp_obj_t []) {mp_obj_new_float(min), mp_obj_new_float(max)});
            }
            break;
        }

        #if (OMV_HM01B0_ENABLE == 1)
        case OMV_CSI_IOCTL_HIMAX_MD_ENABLE: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            }
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_WINDOW: {
            if (n_args == 1) {
                int x, y, w, h;
                mp_obj_t *array;
                mp_uint_t array_len;
                mp_obj_get_array(args[0], &array_len, &array);

                if (array_len == 4) {
                    x = mp_obj_get_int(array[0]);
                    y = mp_obj_get_int(array[1]);
                    w = mp_obj_get_int(array[2]);
                    h = mp_obj_get_int(array[3]);
                } else if (array_len == 2) {
                    w = mp_obj_get_int(array[0]);
                    h = mp_obj_get_int(array[1]);
                    x = 0;
                    y = 0;
                } else {
                    mp_raise_msg(&mp_type_ValueError,
                                 MP_ERROR_TEXT("The tuple/list must either be (x, y, w, h) or (w, h)"));
                }

                error = omv_csi_ioctl(self->csi, request, x, y, w, h);
            }
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_THRESHOLD: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            }
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_CLEAR: {
            error = omv_csi_ioctl(self->csi, request);
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_OSC_ENABLE: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            }
            break;
        }
        #endif // (OMV_HM01B0_ENABLE == 1)

        case OMV_CSI_IOCTL_GET_RGB_STATS: {
            uint32_t r, gb, gr, b;
            error = omv_csi_ioctl(self->csi, request, &r, &gb, &gr, &b);
            if (error == 0) {
                ret_obj = mp_obj_new_tuple(4, (mp_obj_t []) {mp_obj_new_int(r),
                                                             mp_obj_new_int(gb),
                                                             mp_obj_new_int(gr),
                                                             mp_obj_new_int(b)});
            }
            break;
        }

        #if (OMV_GENX320_ENABLE == 1)
        case OMV_CSI_IOCTL_GENX320_SET_BIASES: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            }
            break;
        }
        case OMV_CSI_IOCTL_GENX320_SET_BIAS: {
            if (n_args == 2) {
                error = omv_csi_ioctl(self->csi,
                                      request,
                                      mp_obj_get_int(args[0]),
                                      mp_obj_get_int(args[1]));
            }
            break;
        }
        case OMV_CSI_IOCTL_GENX320_SET_AFK: {
            if (n_args == 1) {
                error = omv_csi_ioctl(self->csi, request, mp_obj_get_int(args[0]));
            } else if (n_args == 3) {
                error = omv_csi_ioctl(self->csi,
                                      request,
                                      mp_obj_get_int(args[0]),
                                      mp_obj_get_int(args[1]),
                                      mp_obj_get_int(args[2]));
            }
            break;
        }
        #endif // (OMV_GENX320_ENABLE == 1)

        default: {
            omv_csi_raise_error(OMV_CSI_ERROR_CTL_UNSUPPORTED);
            break;
        }
    }

    if (error != 0) {
        omv_csi_raise_error(error);
    }

    return ret_obj;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_ioctl_obj, 2, 5, py_csi_ioctl);

static mp_obj_t py_csi_color_palette(size_t n_args, const mp_obj_t *args) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        const uint16_t *palette = omv_csi_get_color_palette(self->csi);
        if (palette == rainbow_table) {
            return mp_obj_new_int(COLOR_PALETTE_RAINBOW);
        } else if (palette == ironbow_table) {
            return mp_obj_new_int(COLOR_PALETTE_IRONBOW);
        #if (MICROPY_PY_TOF == 1)
        } else if (palette == depth_table) {
            return mp_obj_new_int(COLOR_PALETTE_DEPTH);
        #endif // MICROPY_PY_TOF == 1
        #if (OMV_GENX320_ENABLE == 1)
        } else if (palette == evt_dark_table) {
            return mp_obj_new_int(COLOR_PALETTE_EVT_DARK);
        } else if (palette == evt_light_table) {
            return mp_obj_new_int(COLOR_PALETTE_EVT_LIGHT);
        #endif // OMV_GENX320_ENABLE == 1
        }
    } else {
        int palette = mp_obj_get_int(args[1]);
        switch (palette) {
            case COLOR_PALETTE_RAINBOW:
                omv_csi_set_color_palette(self->csi, rainbow_table);
                break;
            case COLOR_PALETTE_IRONBOW:
                omv_csi_set_color_palette(self->csi, ironbow_table);
                break;
            #if (MICROPY_PY_TOF == 1)
            case COLOR_PALETTE_DEPTH:
                omv_csi_set_color_palette(self->csi, depth_table);
                break;
            #endif // MICROPY_PY_TOF == 1
            #if (OMV_GENX320_ENABLE == 1)
            case COLOR_PALETTE_EVT_DARK:
                omv_csi_set_color_palette(self->csi, evt_dark_table);
                break;
            case COLOR_PALETTE_EVT_LIGHT:
                omv_csi_set_color_palette(self->csi, evt_light_table);
                break;
            #endif // OMV_GENX320_ENABLE == 1
            default:
                omv_csi_raise_error(OMV_CSI_ERROR_INVALID_ARGUMENT);
                break;
        }
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_csi_color_palette_obj, 1, 2, py_csi_color_palette);

static mp_obj_t py_csi_write_reg(mp_obj_t self_in, mp_obj_t addr, mp_obj_t val) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    omv_csi_write_reg(self->csi, mp_obj_get_int(addr), mp_obj_get_int(val));

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(py_csi_write_reg_obj, py_csi_write_reg);

static mp_obj_t py_csi_read_reg(mp_obj_t self_in, mp_obj_t addr) {
    py_csi_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_obj_new_int(omv_csi_read_reg(self->csi, mp_obj_get_int(addr)));
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_csi_read_reg_obj, py_csi_read_reg);

mp_obj_t py_csi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_id, ARG_delays, ARG_fflush, ARG_fb_size };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cid, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1 } },
        { MP_QSTR_delays, MP_ARG_BOOL | MP_ARG_KW_ONLY,  {.u_bool = true} },
        { MP_QSTR_fflush, MP_ARG_BOOL | MP_ARG_KW_ONLY,  {.u_bool = true} },
        { MP_QSTR_fb_size, MP_ARG_INT | MP_ARG_KW_ONLY,  {.u_int = 2 * 1024 * 1024} },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    omv_csi_t *csi = omv_csi_get(args[ARG_id].u_int);

    if (!csi || !csi->detected) {
        omv_csi_raise_error(OMV_CSI_ERROR_ISC_UNDETECTED);
    }

    py_csi_obj_t *self = mp_obj_malloc_with_finaliser(py_csi_obj_t, &py_csi_type);
    self->csi = csi;
    self->frame_cb = mp_const_none;
    self->vsync_cb = mp_const_none;
    csi->disable_delays = !args[ARG_delays].u_bool;
    csi->disable_full_flush = !args[ARG_fflush].u_bool;

    if (csi->fb == NULL) {
        size_t fb_size = args[ARG_fb_size].u_int;
        csi->fb = (framebuffer_t *) m_new(uint8_t, fb_size + sizeof(framebuffer_t));
        framebuffer_init_fb(csi->fb, fb_size, true);
        self->fb = csi->fb; // Track GC heap alloc
    }
    
    #if MICROPY_PY_IMU
    // +-10 degree dead-zone around pitch 90/270.
    // +-45 degree active-zone around roll 0/90/180/270/360.
    omv_csi_set_rotation(self->csi, 10, 45);
    #endif // MICROPY_PY_IMU

    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t py_csi_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_CSI) },
    { MP_ROM_QSTR(MP_QSTR___del__),             MP_ROM_PTR(&py_csi_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset),               MP_ROM_PTR(&py_csi_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_shutdown),            MP_ROM_PTR(&py_csi_shutdown_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep),               MP_ROM_PTR(&py_csi_sleep_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush),               MP_ROM_PTR(&py_csi_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_snapshot),            MP_ROM_PTR(&py_csi_snapshot_obj) },
    { MP_ROM_QSTR(MP_QSTR_width),               MP_ROM_PTR(&py_csi_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height),              MP_ROM_PTR(&py_csi_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_cid),                 MP_ROM_PTR(&py_csi_cid_obj) },
    { MP_ROM_QSTR(MP_QSTR_readable),            MP_ROM_PTR(&py_csi_readable_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixformat),           MP_ROM_PTR(&py_csi_pixformat_obj) },
    { MP_ROM_QSTR(MP_QSTR_framesize),           MP_ROM_PTR(&py_csi_framesize_obj) },
    { MP_ROM_QSTR(MP_QSTR_framerate),           MP_ROM_PTR(&py_csi_framerate_obj) },
    { MP_ROM_QSTR(MP_QSTR_window),              MP_ROM_PTR(&py_csi_window_obj) },
    { MP_ROM_QSTR(MP_QSTR_gainceiling),         MP_ROM_PTR(&py_csi_gainceiling_obj) },
    { MP_ROM_QSTR(MP_QSTR_contrast),            MP_ROM_PTR(&py_csi_contrast_obj) },
    { MP_ROM_QSTR(MP_QSTR_brightness),          MP_ROM_PTR(&py_csi_brightness_obj) },
    { MP_ROM_QSTR(MP_QSTR_saturation),          MP_ROM_PTR(&py_csi_saturation_obj) },
    { MP_ROM_QSTR(MP_QSTR_quality),             MP_ROM_PTR(&py_csi_quality_obj) },
    { MP_ROM_QSTR(MP_QSTR_colorbar),            MP_ROM_PTR(&py_csi_colorbar_obj) },
    { MP_ROM_QSTR(MP_QSTR_auto_gain),           MP_ROM_PTR(&py_csi_auto_gain_obj) },
    { MP_ROM_QSTR(MP_QSTR_gain_db),             MP_ROM_PTR(&py_csi_gain_db_obj) },
    { MP_ROM_QSTR(MP_QSTR_auto_exposure),       MP_ROM_PTR(&py_csi_auto_exposure_obj) },
    { MP_ROM_QSTR(MP_QSTR_exposure_us),         MP_ROM_PTR(&py_csi_exposure_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_auto_whitebal),       MP_ROM_PTR(&py_csi_auto_whitebal_obj) },
    { MP_ROM_QSTR(MP_QSTR_rgb_gain_db),         MP_ROM_PTR(&py_csi_rgb_gain_db_obj) },
    { MP_ROM_QSTR(MP_QSTR_auto_blc),            MP_ROM_PTR(&py_csi_auto_blc_obj) },
    { MP_ROM_QSTR(MP_QSTR_blc_regs),            MP_ROM_PTR(&py_csi_blc_regs_obj) },
    { MP_ROM_QSTR(MP_QSTR_hmirror),             MP_ROM_PTR(&py_csi_hmirror_obj) },
    { MP_ROM_QSTR(MP_QSTR_vflip),               MP_ROM_PTR(&py_csi_vflip_obj) },
    { MP_ROM_QSTR(MP_QSTR_transpose),           MP_ROM_PTR(&py_csi_transpose_obj) },
    { MP_ROM_QSTR(MP_QSTR_auto_rotation),       MP_ROM_PTR(&py_csi_auto_rotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_framebuffers),        MP_ROM_PTR(&py_csi_framebuffers_obj) },
    { MP_ROM_QSTR(MP_QSTR_lens_correction),     MP_ROM_PTR(&py_csi_lens_correction_obj) },
    { MP_ROM_QSTR(MP_QSTR_special_effect),      MP_ROM_PTR(&py_csi_special_effect_obj) },
    { MP_ROM_QSTR(MP_QSTR_vsync_callback),      MP_ROM_PTR(&py_csi_vsync_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR_frame_callback),      MP_ROM_PTR(&py_csi_frame_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl),               MP_ROM_PTR(&py_csi_ioctl_obj) },
    { MP_ROM_QSTR(MP_QSTR_color_palette),       MP_ROM_PTR(&py_csi_color_palette_obj) },
    { MP_ROM_QSTR(MP_QSTR___write_reg),         MP_ROM_PTR(&py_csi_write_reg_obj) },
    { MP_ROM_QSTR(MP_QSTR___read_reg),          MP_ROM_PTR(&py_csi_read_reg_obj) },
};
MP_DEFINE_CONST_DICT(py_csi_locals_dict, py_csi_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    py_csi_type,
    MP_QSTR_CSI,
    MP_TYPE_FLAG_NONE,
    make_new, py_csi_make_new,
    locals_dict, &py_csi_locals_dict
    );

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_csi) },
    { MP_ROM_QSTR(MP_QSTR_CSI),             MP_ROM_PTR(&py_csi_type) },
    { MP_ROM_QSTR(MP_QSTR_devices),         MP_ROM_PTR(&py_csi_devices_obj) },

    // Pixel Formats
    { MP_ROM_QSTR(MP_QSTR_BINARY),          MP_ROM_INT(PIXFORMAT_BINARY) },      /* 1BPP/BINARY*/
    { MP_ROM_QSTR(MP_QSTR_GRAYSCALE),       MP_ROM_INT(PIXFORMAT_GRAYSCALE) },   /* 1BPP/GRAYSCALE*/
    { MP_ROM_QSTR(MP_QSTR_RGB565),          MP_ROM_INT(PIXFORMAT_RGB565) },      /* 2BPP/RGB565*/
    { MP_ROM_QSTR(MP_QSTR_BAYER),           MP_ROM_INT(PIXFORMAT_BAYER) },       /* 1BPP/RAW*/
    { MP_ROM_QSTR(MP_QSTR_YUV422),          MP_ROM_INT(PIXFORMAT_YUV422) },      /* 2BPP/YUV422*/
    { MP_ROM_QSTR(MP_QSTR_JPEG),            MP_ROM_INT(PIXFORMAT_JPEG) },        /* JPEG/COMPRESSED*/

    // Image Sensors
    { MP_ROM_QSTR(MP_QSTR_OV2640),          MP_ROM_INT(OV2640_ID) },
    { MP_ROM_QSTR(MP_QSTR_OV5640),          MP_ROM_INT(OV5640_ID) },
    { MP_ROM_QSTR(MP_QSTR_OV7670),          MP_ROM_INT(OV7670_ID) },
    { MP_ROM_QSTR(MP_QSTR_OV7690),          MP_ROM_INT(OV7690_ID) },
    { MP_ROM_QSTR(MP_QSTR_OV7725),          MP_ROM_INT(OV7725_ID) },
    { MP_ROM_QSTR(MP_QSTR_OV9650),          MP_ROM_INT(OV9650_ID) },
    { MP_ROM_QSTR(MP_QSTR_MT9V022),         MP_ROM_INT(MT9V0X2_ID) },
    { MP_ROM_QSTR(MP_QSTR_MT9V024),         MP_ROM_INT(MT9V0X4_ID) },
    { MP_ROM_QSTR(MP_QSTR_MT9V032),         MP_ROM_INT(MT9V0X2_ID) },
    { MP_ROM_QSTR(MP_QSTR_MT9V034),         MP_ROM_INT(MT9V0X4_ID) },
    { MP_ROM_QSTR(MP_QSTR_MT9M114),         MP_ROM_INT(MT9M114_ID) },
    { MP_ROM_QSTR(MP_QSTR_BOSON320),        MP_ROM_INT(BOSON_320_ID) },
    { MP_ROM_QSTR(MP_QSTR_BOSON640),        MP_ROM_INT(BOSON_640_ID) },
    { MP_ROM_QSTR(MP_QSTR_LEPTON),          MP_ROM_INT(LEPTON_ID) },
    { MP_ROM_QSTR(MP_QSTR_HM01B0),          MP_ROM_INT(HM01B0_ID) },
    { MP_ROM_QSTR(MP_QSTR_HM0360),          MP_ROM_INT(HM0360_ID) },
    { MP_ROM_QSTR(MP_QSTR_GC2145),          MP_ROM_INT(GC2145_ID) },
    { MP_ROM_QSTR(MP_QSTR_GENX320ES),       MP_ROM_INT(GENX320_ID_ES) },
    { MP_ROM_QSTR(MP_QSTR_GENX320),         MP_ROM_INT(GENX320_ID_MP) },
    { MP_ROM_QSTR(MP_QSTR_PAG7920),         MP_ROM_INT(PAG7920_ID) },
    { MP_ROM_QSTR(MP_QSTR_PAG7936),         MP_ROM_INT(PAG7936_ID) },
    { MP_ROM_QSTR(MP_QSTR_PAJ6100),         MP_ROM_INT(PAJ6100_ID) },
    { MP_ROM_QSTR(MP_QSTR_FROGEYE2020),     MP_ROM_INT(FROGEYE2020_ID) },
    { MP_ROM_QSTR(MP_QSTR_SOFTCSI),         MP_ROM_INT(SOFTCSI_ID) },

    // Special effects
    { MP_ROM_QSTR(MP_QSTR_NORMAL),          MP_ROM_INT(OMV_CSI_SDE_NORMAL) },    /* Normal/No SDE */
    { MP_ROM_QSTR(MP_QSTR_NEGATIVE),        MP_ROM_INT(OMV_CSI_SDE_NEGATIVE) },  /* Negative image */

    // C/SIF Resolutions
    { MP_ROM_QSTR(MP_QSTR_QQCIF),           MP_ROM_INT(OMV_CSI_FRAMESIZE_QQCIF) },    /* 88x72     */
    { MP_ROM_QSTR(MP_QSTR_QCIF),            MP_ROM_INT(OMV_CSI_FRAMESIZE_QCIF) },     /* 176x144   */
    { MP_ROM_QSTR(MP_QSTR_CIF),             MP_ROM_INT(OMV_CSI_FRAMESIZE_CIF) },      /* 352x288   */
    { MP_ROM_QSTR(MP_QSTR_QQSIF),           MP_ROM_INT(OMV_CSI_FRAMESIZE_QQSIF) },    /* 88x60     */
    { MP_ROM_QSTR(MP_QSTR_QSIF),            MP_ROM_INT(OMV_CSI_FRAMESIZE_QSIF) },     /* 176x120   */
    { MP_ROM_QSTR(MP_QSTR_SIF),             MP_ROM_INT(OMV_CSI_FRAMESIZE_SIF) },      /* 352x240   */
    // VGA Resolutions
    { MP_ROM_QSTR(MP_QSTR_QQQQVGA),         MP_ROM_INT(OMV_CSI_FRAMESIZE_QQQQVGA) },  /* 40x30     */
    { MP_ROM_QSTR(MP_QSTR_QQQVGA),          MP_ROM_INT(OMV_CSI_FRAMESIZE_QQQVGA) },   /* 80x60     */
    { MP_ROM_QSTR(MP_QSTR_QQVGA),           MP_ROM_INT(OMV_CSI_FRAMESIZE_QQVGA) },    /* 160x120   */
    { MP_ROM_QSTR(MP_QSTR_QVGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_QVGA) },     /* 320x240   */
    { MP_ROM_QSTR(MP_QSTR_VGA),             MP_ROM_INT(OMV_CSI_FRAMESIZE_VGA) },      /* 640x480   */
    { MP_ROM_QSTR(MP_QSTR_HQQQQVGA),        MP_ROM_INT(OMV_CSI_FRAMESIZE_HQQQQVGA) }, /* 40x20     */
    { MP_ROM_QSTR(MP_QSTR_HQQQVGA),         MP_ROM_INT(OMV_CSI_FRAMESIZE_HQQQVGA) },  /* 80x40     */
    { MP_ROM_QSTR(MP_QSTR_HQQVGA),          MP_ROM_INT(OMV_CSI_FRAMESIZE_HQQVGA) },   /* 160x80    */
    { MP_ROM_QSTR(MP_QSTR_HQVGA),           MP_ROM_INT(OMV_CSI_FRAMESIZE_HQVGA) },    /* 240x160   */
    { MP_ROM_QSTR(MP_QSTR_HVGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_HVGA) },     /* 480x320   */
    // FFT Resolutions
    { MP_ROM_QSTR(MP_QSTR_B64X32),          MP_ROM_INT(OMV_CSI_FRAMESIZE_64X32) },    /* 64x32     */
    { MP_ROM_QSTR(MP_QSTR_B64X64),          MP_ROM_INT(OMV_CSI_FRAMESIZE_64X64) },    /* 64x64     */
    { MP_ROM_QSTR(MP_QSTR_B128X64),         MP_ROM_INT(OMV_CSI_FRAMESIZE_128X64) },   /* 128x64    */
    { MP_ROM_QSTR(MP_QSTR_B128X128),        MP_ROM_INT(OMV_CSI_FRAMESIZE_128X128) },  /* 128x128   */
    // Himax Resolutions
    { MP_ROM_QSTR(MP_QSTR_B160X160),        MP_ROM_INT(OMV_CSI_FRAMESIZE_160X160) },  /* 160x160   */
    { MP_ROM_QSTR(MP_QSTR_B320X320),        MP_ROM_INT(OMV_CSI_FRAMESIZE_320X320) },  /* 320x320   */
    // Other Resolutions
    { MP_ROM_QSTR(MP_QSTR_LCD),             MP_ROM_INT(OMV_CSI_FRAMESIZE_LCD) },      /* 128x160   */
    { MP_ROM_QSTR(MP_QSTR_QQVGA2),          MP_ROM_INT(OMV_CSI_FRAMESIZE_QQVGA2) },   /* 128x160   */
    { MP_ROM_QSTR(MP_QSTR_WVGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_WVGA) },     /* 720x480   */
    { MP_ROM_QSTR(MP_QSTR_WVGA2),           MP_ROM_INT(OMV_CSI_FRAMESIZE_WVGA2) },    /* 752x480   */
    { MP_ROM_QSTR(MP_QSTR_SVGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_SVGA) },     /* 800x600   */
    { MP_ROM_QSTR(MP_QSTR_XGA),             MP_ROM_INT(OMV_CSI_FRAMESIZE_XGA) },      /* 1024x768  */
    { MP_ROM_QSTR(MP_QSTR_WXGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_WXGA) },     /* 1280x768  */
    { MP_ROM_QSTR(MP_QSTR_SXGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_SXGA) },     /* 1280x1024 */
    { MP_ROM_QSTR(MP_QSTR_SXGAM),           MP_ROM_INT(OMV_CSI_FRAMESIZE_SXGAM) },    /* 1280x960  */
    { MP_ROM_QSTR(MP_QSTR_UXGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_UXGA) },     /* 1600x1200 */
    { MP_ROM_QSTR(MP_QSTR_HD),              MP_ROM_INT(OMV_CSI_FRAMESIZE_HD) },       /* 1280x720  */
    { MP_ROM_QSTR(MP_QSTR_FHD),             MP_ROM_INT(OMV_CSI_FRAMESIZE_FHD) },      /* 1920x1080 */
    { MP_ROM_QSTR(MP_QSTR_QHD),             MP_ROM_INT(OMV_CSI_FRAMESIZE_QHD) },      /* 2560x1440 */
    { MP_ROM_QSTR(MP_QSTR_QXGA),            MP_ROM_INT(OMV_CSI_FRAMESIZE_QXGA) },     /* 2048x1536 */
    { MP_ROM_QSTR(MP_QSTR_WQXGA),           MP_ROM_INT(OMV_CSI_FRAMESIZE_WQXGA) },    /* 2560x1600 */
    { MP_ROM_QSTR(MP_QSTR_WQXGA2),          MP_ROM_INT(OMV_CSI_FRAMESIZE_WQXGA2) },   /* 2592x1944 */

    // OMV_CSI_IOCTLs
    { MP_ROM_QSTR(MP_QSTR_IOCTL_SET_READOUT_WINDOW),    MP_ROM_INT(OMV_CSI_IOCTL_SET_READOUT_WINDOW) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GET_READOUT_WINDOW),    MP_ROM_INT(OMV_CSI_IOCTL_GET_READOUT_WINDOW) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_SET_TRIGGERED_MODE),    MP_ROM_INT(OMV_CSI_IOCTL_SET_TRIGGERED_MODE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GET_TRIGGERED_MODE),    MP_ROM_INT(OMV_CSI_IOCTL_GET_TRIGGERED_MODE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_SET_FOV_WIDE),          MP_ROM_INT(OMV_CSI_IOCTL_SET_FOV_WIDE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GET_FOV_WIDE),          MP_ROM_INT(OMV_CSI_IOCTL_GET_FOV_WIDE) },
    #if (OMV_OV5640_AF_ENABLE == 1)
    { MP_ROM_QSTR(MP_QSTR_IOCTL_TRIGGER_AUTO_FOCUS),    MP_ROM_INT(OMV_CSI_IOCTL_TRIGGER_AUTO_FOCUS) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_PAUSE_AUTO_FOCUS),      MP_ROM_INT(OMV_CSI_IOCTL_PAUSE_AUTO_FOCUS) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_RESET_AUTO_FOCUS),      MP_ROM_INT(OMV_CSI_IOCTL_RESET_AUTO_FOCUS) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_WAIT_ON_AUTO_FOCUS),    MP_ROM_INT(OMV_CSI_IOCTL_WAIT_ON_AUTO_FOCUS) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_IOCTL_SET_NIGHT_MODE),        MP_ROM_INT(OMV_CSI_IOCTL_SET_NIGHT_MODE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GET_NIGHT_MODE),        MP_ROM_INT(OMV_CSI_IOCTL_GET_NIGHT_MODE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_WIDTH),      MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_WIDTH) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_HEIGHT),     MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_HEIGHT) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_RADIOMETRY), MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_RADIOMETRY) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_REFRESH),    MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_REFRESH) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_RESOLUTION), MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_RESOLUTION) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_RUN_COMMAND),    MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_RUN_COMMAND) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_SET_ATTRIBUTE),  MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_SET_ATTRIBUTE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_ATTRIBUTE),  MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_ATTRIBUTE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_FPA_TEMP),   MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_FPA_TEMP) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_AUX_TEMP),   MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_AUX_TEMP) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_SET_MODE),       MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_SET_MODE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_MODE),       MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_MODE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_SET_RANGE),      MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_SET_RANGE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_LEPTON_GET_RANGE),      MP_ROM_INT(OMV_CSI_IOCTL_LEPTON_GET_RANGE) },
    #if (OMV_HM01B0_ENABLE == 1)
    { MP_ROM_QSTR(MP_QSTR_IOCTL_HIMAX_MD_ENABLE),       MP_ROM_INT(OMV_CSI_IOCTL_HIMAX_MD_ENABLE) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_HIMAX_MD_WINDOW),       MP_ROM_INT(OMV_CSI_IOCTL_HIMAX_MD_WINDOW) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_HIMAX_MD_THRESHOLD),    MP_ROM_INT(OMV_CSI_IOCTL_HIMAX_MD_THRESHOLD) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_HIMAX_MD_CLEAR),        MP_ROM_INT(OMV_CSI_IOCTL_HIMAX_MD_CLEAR) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_HIMAX_OSC_ENABLE),      MP_ROM_INT(OMV_CSI_IOCTL_HIMAX_OSC_ENABLE) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GET_RGB_STATS),         MP_ROM_INT(OMV_CSI_IOCTL_GET_RGB_STATS) },

    #if (OMV_GENX320_ENABLE == 1)
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GENX320_SET_BIASES),     MP_ROM_INT(OMV_CSI_IOCTL_GENX320_SET_BIASES) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIASES_DEFAULT),       MP_ROM_INT(OMV_CSI_GENX320_BIASES_DEFAULT) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIASES_LOW_LIGHT),     MP_ROM_INT(OMV_CSI_GENX320_BIASES_LOW_LIGHT) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIASES_ACTIVE_MARKER), MP_ROM_INT(OMV_CSI_GENX320_BIASES_ACTIVE_MARKER) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIASES_LOW_NOISE),     MP_ROM_INT(OMV_CSI_GENX320_BIASES_LOW_NOISE) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIASES_HIGH_SPEED),    MP_ROM_INT(OMV_CSI_GENX320_BIASES_HIGH_SPEED) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GENX320_SET_BIAS),       MP_ROM_INT(OMV_CSI_IOCTL_GENX320_SET_BIAS) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIAS_DIFF_OFF),        MP_ROM_INT(OMV_CSI_GENX320_BIAS_DIFF_OFF) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIAS_DIFF_ON),         MP_ROM_INT(OMV_CSI_GENX320_BIAS_DIFF_ON) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIAS_FO),              MP_ROM_INT(OMV_CSI_GENX320_BIAS_FO) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIAS_HPF),             MP_ROM_INT(OMV_CSI_GENX320_BIAS_HPF) },
    { MP_ROM_QSTR(MP_QSTR_GENX320_BIAS_REFR),            MP_ROM_INT(OMV_CSI_GENX320_BIAS_REFR) },
    { MP_ROM_QSTR(MP_QSTR_IOCTL_GENX320_SET_AFK),        MP_ROM_INT(OMV_CSI_IOCTL_GENX320_SET_AFK) },
    #endif
};
static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t csi_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_csi, csi_module);
#endif // MICROPY_PY_CSI_NG
