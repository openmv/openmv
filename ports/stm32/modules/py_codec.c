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
 * Codec Python module - codec.H264Encoder on the VC8000.
 *
 * encode() takes an image and returns one H.264 access unit in Annex-B
 * format as a memoryview into an internal buffer that stays valid until
 * the next encode() call.
 *
 * The VC8000 is shared with the JPEG compressor (see stm_jpeg.c): both
 * paths program the full register set per operation and run synchronously,
 * so JPEG compression may be freely interleaved between encoded frames.
 * Only one live H264Encoder is allowed at a time.
 */
#include "board_config.h"
#if defined(OMV_VENC_CODEC_ENABLE) && (OMV_VENC_CODEC_ENABLE == 1)

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"

#if MICROPY_PY_ULAB
#include "ulab/code/ndarray.h"
#endif

#include "imlib.h"
#include "umalloc.h"
#include STM32_HAL_H
#include "h264encapi.h"

static const mp_obj_type_t py_h264_encoder_type;

// The encoder instance state lives across VM execution, so a second live
// instance would corrupt it (single EWL instance and single hardware core).
static bool py_h264_encoder_active;

typedef struct py_h264_encoder_obj {
    mp_obj_base_t base;
    H264EncInst inst;
    uint32_t width;
    uint32_t height;
    uint32_t fps;
    uint32_t keyframe_interval;  // frames between keyframes
    uint32_t refresh_interval;   // rolling refresh over N frames (GDR), 0 = off
    uint32_t mb_w;        // macroblock grid size
    uint32_t mb_h;
    uint32_t frames;      // frames encoded since stream start
    uint32_t since_keyframe;
    uint32_t mse;         // last frame mean squared error * 256
    int8_t *mv;           // last frame hw motion vector output
    bool keyframe;        // last access unit was an IDR
    bool resync;          // a frame was lost to overflow; IDR on next encode
    int32_t quality;      // fixed-QP quality [0..100], -1 = rate-controlled
    uint32_t src_w;       // last input geometry programmed into the
    uint32_t src_h;       // pre-processor (input size and roi offset)
    uint32_t src_x;
    uint32_t src_y;
    uint32_t denom;       // 90 kHz ticks per nominal frame
    mp_int_t last_timestamp_us;
    H264EncPictureType input_type;
    mp_obj_t sps_pps;     // bytes, captured at stream start
    uint8_t *out_buf;
    uint32_t out_size;
} py_h264_encoder_obj_t;

// The hardware writes one 48-byte macroblock info record per macroblock
// (encOutputMbInfo_s; pre-Foxtail H1 cores used 32 bytes - the stale
// 4-byte format comment in h264encapi.h predates both): the first motion
// vector's y is an int8 at offset 4, x an int16 at offset 8, and the
// inter SAD a uint16 at offset 26, all little-endian.
#define H264_MB_INFO_SIZE    48
#define H264_MB_INFO_MV_Y    4
#define H264_MB_INFO_MV_X    8
#define H264_MB_INFO_SAD     26

static void py_h264_encoder_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_h264_encoder_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->inst == NULL) {
        mp_printf(print, "{\"closed\":\"true\"}");
        return;
    }

    H264EncRateCtrl rate = {};
    H264EncGetRateCtrl(self->inst, &rate);
    mp_printf(print, "{\"closed\":\"false\", \"width\":%u, \"height\":%u, \"fps\":%u, \"bitrate\":%u, "
              "\"quality\":%d, \"keyframe_interval\":%u, \"refresh_interval\":%u, \"count\":%u}",
              self->width, self->height, self->fps, (unsigned int) rate.bitPerSecond, (int) self->quality,
              self->keyframe_interval, self->refresh_interval, self->frames);
}

// Raise unless a codec call succeeded. On a failure inside the constructor
// the finaliser performs the hardware cleanup.
static void py_h264_encoder_check(H264EncRet ret) {
    if ((ret != H264ENC_OK) && (ret != H264ENC_FRAME_READY)) {
        if (ret == H264ENC_OUTPUT_BUFFER_OVERFLOW) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("H264 output buffer overflow"));
        }

        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("H264 encoder error %d"), (int) ret);
    }
}

static py_h264_encoder_obj_t *py_h264_encoder_get(mp_obj_t self_in) {
    // inst doubles as the lifecycle sentinel: NULL until H264EncInit
    // succeeds and again after deinit().
    py_h264_encoder_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->inst == NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Encoder is closed"));
    }

    return self;
}

static mp_obj_t py_h264_encoder_encode(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_roi, ARG_timestamp_us, ARG_keyframe };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_roi, MP_ARG_OBJ | MP_ARG_KW_ONLY, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_timestamp_us, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1 } },
        { MP_QSTR_keyframe, MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false } },
    };

    // Parse args.
    py_h264_encoder_obj_t *self = py_h264_encoder_get(pos_args[0]);
    image_t *img = py_helper_arg_to_image(pos_args[1], ARG_IMAGE_UNCOMPRESSED);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // The pre-processor crops the encoded window out of the input image,
    // so the roi selects what to encode with no copies. The roi size must
    // match the encoder size.
    rectangle_t roi = py_helper_arg_to_roi(args[ARG_roi].u_obj, img);

    if ((roi.w != self->width) || (roi.h != self->height)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("ROI geometry does not match the encoder"));
    }

    H264EncPictureType input_type;
    switch (img->pixfmt) {
        case PIXFORMAT_RGB565:
            input_type = H264ENC_RGB565;
            break;
        case PIXFORMAT_YVU422: // matches YUV422 format for vc8000
            // An odd roi shift lands mid-macropixel and swaps the U/V phase,
            // which the pre-processor cannot correct for interleaved input.
            if (roi.x % 2) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("ROI x offset must be even for YUV images"));
            }

            input_type = H264ENC_YUV422_INTERLEAVED_YUYV;
            break;
        default:
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Image format is not supported"));
    }

    // The pre-processor fetches source rows at a 16-pixel rounded stride,
    // but image rows are packed at the image width.
    if (img->w % 16) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Image width must be a multiple of 16"));
    }

    // In rolling-refresh mode the driver silently converts requested
    // keyframes into a refresh sweep start, so no IDR would be produced.
    if (args[ARG_keyframe].u_bool && (self->refresh_interval != 0)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("keyframe requests require refresh_interval=0"));
    }

    if ((input_type != self->input_type) || (img->w != self->src_w) || (img->h != self->src_h) ||
        (roi.x != self->src_x) || (roi.y != self->src_y)) {
        H264EncPreProcessingCfg preproc;
        py_h264_encoder_check(H264EncGetPreProcessing(self->inst, &preproc));
        preproc.inputType = input_type;
        preproc.origWidth = img->w;
        preproc.origHeight = img->h;
        preproc.xOffset = roi.x;
        preproc.yOffset = roi.y;
        py_h264_encoder_check(H264EncSetPreProcessing(self->inst, &preproc));
        self->input_type = input_type;
        self->src_w = img->w;
        self->src_h = img->h;
        self->src_x = roi.x;
        self->src_y = roi.y;
    }

    // Frame duration in 90 kHz ticks from microsecond timestamps, so rate
    // control tracks the real (wobbling) frame rate. Defaults to the clock;
    // pass timestamp_us= to encode against a custom timeline (e.g. faster
    // than realtime). Non-monotonic timestamps fall back to the nominal
    // frame duration, which also self-heals tick wraparound.
    mp_int_t timestamp_us = args[ARG_timestamp_us].u_int;

    if (timestamp_us < 0) {
        timestamp_us = (mp_int_t) mp_hal_ticks_us();
    }

    uint32_t increment = self->denom;
    mp_int_t delta_us = timestamp_us - self->last_timestamp_us;

    if ((self->frames > 0) && (delta_us > 0)) {
        // Clamp before scaling so long idle gaps cannot overflow.
        delta_us = IM_MIN(delta_us, 10 * 1000000);
        increment = (delta_us * 9) / 100;
    }

    self->last_timestamp_us = timestamp_us;

    // In rolling-refresh mode the stream refreshes continuously instead
    // of with periodic keyframes. A pending resync forces an IDR in
    // either mode: the stream is not decodable again until one.
    bool intra = args[ARG_keyframe].u_bool || (self->frames == 0) || self->resync ||
                 ((self->refresh_interval == 0) &&
                  (self->since_keyframe >= self->keyframe_interval));

    H264EncIn encIn = {
        .busLuma = (ptr_t) img->data,
        .busChromaU = 0,
        .busChromaV = 0,
        .timeIncrement = self->frames ? increment : 0,
        .pOutBuf = (u32 *) self->out_buf,
        .busOutBuf = (size_t) self->out_buf,
        .outBufSize = self->out_size,
        .codingType = intra ? H264ENC_INTRA_FRAME : H264ENC_PREDICTED_FRAME,
        .ipf = H264ENC_REFERENCE_AND_REFRESH,
        .ltrf = H264ENC_NO_REFERENCE_NO_REFRESH,
        .sendAUD = 0,
    };

    // Clean src for vc8000.
    SCB_CleanDCache_by_Addr((uint32_t *) img->data, image_size(img));

    // Drop any writes to dest.
    SCB_InvalidateDCache_by_Addr((uint32_t *) self->out_buf, self->out_size);

    H264EncOut encOut = {};
    H264EncRet ret = H264EncStrmEncode(self->inst, &encIn, &encOut, NULL, NULL, NULL);

    // An overflowed frame is lost, so the stream cannot be decoded again
    // until an IDR. Arm the resync even if the caller catches and
    // ignores the error; it clears once a frame encodes successfully.
    if (ret == H264ENC_OUTPUT_BUFFER_OVERFLOW) {
        self->resync = true;
    }

    py_h264_encoder_check(ret);
    self->resync = false;

    // Drop any speculative reads of dest and the hw motion vector output.
    SCB_InvalidateDCache_by_Addr((uint32_t *) self->out_buf, encOut.streamSize);
    SCB_InvalidateDCache_by_Addr((uint32_t *) encOut.motionVectors,
                                 self->mb_w * self->mb_h * H264_MB_INFO_SIZE);

    self->mse = encOut.mse_mul256;
    self->mv = encOut.motionVectors;
    self->keyframe = (encOut.codingType == H264ENC_INTRA_FRAME);
    self->since_keyframe = self->keyframe ? 1 : (self->since_keyframe + 1);
    self->frames += 1;

    // Valid until the next encode() call, like snapshot().
    return mp_obj_new_memoryview('B', encOut.streamSize, self->out_buf);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(py_h264_encoder_encode_obj, 2, py_h264_encoder_encode);

static mp_obj_t py_h264_encoder_sps_pps(mp_obj_t self_in) {
    py_h264_encoder_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return self->sps_pps;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_h264_encoder_sps_pps_obj, py_h264_encoder_sps_pps);

static mp_obj_t py_h264_encoder_keyframe(mp_obj_t self_in) {
    py_h264_encoder_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->keyframe);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_h264_encoder_keyframe_obj, py_h264_encoder_keyframe);

static mp_obj_t py_h264_encoder_count(mp_obj_t self_in) {
    py_h264_encoder_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->frames);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_h264_encoder_count_obj, py_h264_encoder_count);

static mp_obj_t py_h264_encoder_mse(mp_obj_t self_in) {
    // Mean squared reconstruction error of the last encoded frame,
    // measured by the hardware. Lower is better, 0.0 is a perfect frame.
    py_h264_encoder_obj_t *self = py_h264_encoder_get(self_in);
    return mp_obj_new_float(self->mse / 256.0f);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_h264_encoder_mse_obj, py_h264_encoder_mse);

#if MICROPY_PY_ULAB
static const qstr py_h264_encoder_motion_fields[] = {
    MP_QSTR_mv,
    MP_QSTR_sad,
};

static mp_obj_t py_h264_encoder_motion(mp_obj_t self_in) {
    // Hardware motion estimation results for the last encoded frame: mv is
    // an int16 ndarray of shape (mb_h, mb_w, 2) holding each 16x16
    // macroblock's motion vector in quarter-pel units (divide by 4 for
    // pixels). Standard H.264 semantics: the vector points from the block
    // to its match in the previous frame, i.e. the negative of apparent
    // content motion. sad is a uint16 ndarray of shape (mb_h, mb_w)
    // holding the match error (higher = new content). Keyframes do no
    // motion estimation - check keyframe() first.
    py_h264_encoder_obj_t *self = py_h264_encoder_get(self_in);

    if (self->mv == NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("No frame encoded"));
    }

    size_t mv_shape[ULAB_MAX_DIMS] = {[ULAB_MAX_DIMS - 3] = self->mb_h, self->mb_w, 2};
    size_t sad_shape[ULAB_MAX_DIMS] = {[ULAB_MAX_DIMS - 2] = self->mb_h, self->mb_w};
    ndarray_obj_t *mv = ndarray_new_dense_ndarray(3, mv_shape, NDARRAY_INT16);
    ndarray_obj_t *sad = ndarray_new_dense_ndarray(2, sad_shape, NDARRAY_UINT16);

    int16_t *mv_array = (int16_t *) mv->array;
    uint16_t *sad_array = (uint16_t *) sad->array;

    for (size_t i = 0, n = self->mb_w * self->mb_h; i < n; i++) {
        int8_t *rec = self->mv + (i * H264_MB_INFO_SIZE);
        mv_array[i * 2] = (rec[H264_MB_INFO_MV_X + 1] << 8) | ((uint8_t) rec[H264_MB_INFO_MV_X]);
        mv_array[i * 2 + 1] = rec[H264_MB_INFO_MV_Y];
        sad_array[i] = (((uint8_t) rec[H264_MB_INFO_SAD + 1]) << 8) | ((uint8_t) rec[H264_MB_INFO_SAD]);
    }

    mp_obj_t items[] = { MP_OBJ_FROM_PTR(mv), MP_OBJ_FROM_PTR(sad) };
    return mp_obj_new_attrtuple(py_h264_encoder_motion_fields, MP_ARRAY_SIZE(items), items);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_h264_encoder_motion_obj, py_h264_encoder_motion);
#endif // MICROPY_PY_ULAB

static mp_obj_t py_h264_encoder_bitrate(size_t n_args, const mp_obj_t *args) {
    py_h264_encoder_obj_t *self = py_h264_encoder_get(args[0]);

    H264EncRateCtrl rate;
    py_h264_encoder_check(H264EncGetRateCtrl(self->inst, &rate));

    if (n_args == 1) {
        return mp_obj_new_int(rate.bitPerSecond);
    }

    // Rate control can be adjusted while streaming, e.g. to adapt to the
    // network. Takes effect at the next encoded frame and switches out of
    // constant-quality mode if it was enabled.
    rate.pictureRc = 1;
    rate.bitPerSecond = mp_obj_get_int(args[1]);
    rate.qpHdr = -1;  // a leftover fixed QP below qpMin would be rejected
    rate.qpMin = 10;  // restore the rate control operating window
    rate.qpMax = 51;
    py_h264_encoder_check(H264EncSetRateCtrl(self->inst, &rate));
    self->quality = -1;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_h264_encoder_bitrate_obj, 1, 2, py_h264_encoder_bitrate);

static mp_obj_t py_h264_encoder_quality(size_t n_args, const mp_obj_t *args) {
    py_h264_encoder_obj_t *self = py_h264_encoder_get(args[0]);

    if (n_args == 1) {
        return mp_obj_new_int(self->quality);  // -1 when rate-controlled
    }

    mp_int_t quality = mp_obj_get_int(args[1]);

    if ((quality < 0) || (quality > 100)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid quality"));
    }

    // Constant-quality mode: quality [0..100] maps linearly to a fixed
    // QP [51..0]. Takes effect at the next encoded frame. The QP window is
    // opened fully since the rate control default (qpMin=10) would reject
    // high quality settings.
    H264EncRateCtrl rate;
    py_h264_encoder_check(H264EncGetRateCtrl(self->inst, &rate));
    rate.pictureRc = 0;
    rate.qpHdr = 51 - ((quality * 51) / 100);
    rate.qpMin = 0;
    rate.qpMax = 51;
    py_h264_encoder_check(H264EncSetRateCtrl(self->inst, &rate));
    self->quality = quality;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_h264_encoder_quality_obj, 1, 2, py_h264_encoder_quality);

static mp_obj_t py_h264_encoder_deinit(mp_obj_t self_in) {
    py_h264_encoder_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (self->inst != NULL) {
        py_h264_encoder_active = false;

        if (self->out_buf != NULL) {
            H264EncIn encIn = {
                .pOutBuf = (u32 *) self->out_buf,
                .busOutBuf = (size_t) self->out_buf,
                .outBufSize = self->out_size,
            };

            H264EncOut encOut = {};
            H264EncStrmEnd(self->inst, &encIn, &encOut);
            // StrmEnd wrote an end-of-sequence NAL through the cache;
            // retire the dirty lines so they cannot evict over the
            // block's next owner (e.g. a new encoder's DMA buffers).
            SCB_InvalidateDCache_by_Addr((uint32_t *) self->out_buf, encOut.streamSize);
            uma_free(self->out_buf);
            self->out_buf = NULL;
        }

        H264EncRelease(self->inst);
        self->inst = NULL;
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_h264_encoder_deinit_obj, py_h264_encoder_deinit);

static mp_obj_t py_h264_encoder_make_new(const mp_obj_type_t *type, size_t n_args,
                                         size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_width, ARG_height, ARG_fps, ARG_bitrate, ARG_quality, ARG_keyframe_interval, ARG_refresh_interval };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_fps, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 30 } },
        { MP_QSTR_bitrate, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 1000000 } },
        { MP_QSTR_quality, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = -1 } },
        { MP_QSTR_keyframe_interval, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 30 } },
        { MP_QSTR_refresh_interval, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 0 } },
    };

    // Parse args.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    uint32_t width = args[ARG_width].u_int;
    uint32_t height = args[ARG_height].u_int;

    if (py_h264_encoder_active) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("H264 encoder is already in use"));
    }

    // Bounds mirror the encoder API: frameRateNum [1..(1<<20)-1], gopLen
    // (keyframe_interval) [1..300], gdrDuration (refresh_interval) [0..300].
    // Encoder minimums per H264Init.c: width 132, height 96; max width is
    // the hardware capability fuse (1920 on the N6).
    if ((width & 3) || (width < 132) || (1920 < width) ||
        (height & 1) || (height < 96) || (1088 < height)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported resolution"));
    }

    if ((args[ARG_fps].u_int < 1) || (args[ARG_fps].u_int > ((1 << 20) - 1))) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid fps"));
    }

    if ((args[ARG_keyframe_interval].u_int < 1) || (args[ARG_keyframe_interval].u_int > 300)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("keyframe_interval must be 1 to 300"));
    }

    if ((args[ARG_refresh_interval].u_int < 0) || (args[ARG_refresh_interval].u_int > 300)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("refresh_interval must be 0 to 300"));
    }

    if (args[ARG_quality].u_int > 100) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid quality"));
    }

    // If a setup step below fails, the finaliser releases whatever was
    // initialized so far.
    py_h264_encoder_obj_t *self = mp_obj_malloc_with_finaliser(py_h264_encoder_obj_t,
                                                               &py_h264_encoder_type);
    self->width = width;
    self->height = height;
    self->fps = args[ARG_fps].u_int;
    self->keyframe_interval = args[ARG_keyframe_interval].u_int;
    self->refresh_interval = args[ARG_refresh_interval].u_int;
    self->mb_w = (width + 15) / 16;
    self->mb_h = (height + 15) / 16;
    self->frames = 0;
    self->since_keyframe = 0;
    self->mse = 0;
    self->mv = NULL;
    self->keyframe = false;
    self->resync = false;
    self->inst = NULL; // set by a successful init; doubles as the lifecycle sentinel
    self->quality = (args[ARG_quality].u_int >= 0) ? args[ARG_quality].u_int : -1;
    self->src_w = width;
    self->src_h = height;
    self->src_x = 0;
    self->src_y = 0;
    self->denom = IM_MAX(90000U / self->fps, 1U);
    self->last_timestamp_us = 0;
    self->input_type = H264ENC_RGB565;
    self->out_buf = NULL;

    H264EncConfig cfg = {
        .streamType = H264ENC_BYTE_STREAM,
        .viewMode = H264ENC_BASE_VIEW_DOUBLE_BUFFER,
        .level = H264ENC_LEVEL_4_1,
        .width = width,
        .height = height,
        // 90 kHz timebase so encode(pts=) can express real microsecond
        // frame durations; denom is the nominal frame duration in ticks.
        .frameRateNum = 90000,
        .frameRateDenom = self->denom,
        .refFrameAmount = 1,
    };

    py_h264_encoder_check(H264EncInit(&cfg, &self->inst));
    py_h264_encoder_active = true;

    H264EncCodingCtrl coding;
    H264EncRateCtrl rate;
    H264EncPreProcessingCfg preproc;
    py_h264_encoder_check(H264EncGetCodingCtrl(self->inst, &coding));
    py_h264_encoder_check(H264EncGetRateCtrl(self->inst, &rate));
    py_h264_encoder_check(H264EncGetPreProcessing(self->inst, &preproc));

    coding.sliceSize = 0;    // one slice per picture
    coding.idrHeader = 1;    // repeat SPS/PPS at every IDR (late-joiner friendly)
    coding.enableCabac = 1;  // Main profile
    coding.seiMessages = 0;
    // Rolling refresh: an intra band sweeps the full frame every
    // refresh_interval frames instead of periodic keyframes, keeping the
    // bitrate smooth for streaming.
    coding.gdrDuration = self->refresh_interval;

    if (self->quality >= 0) {
        // Constant-quality mode: quality [0..100] maps linearly to a fixed
        // QP [51..0], bitrate is ignored. Open the QP window fully (the
        // rate control default qpMin=10 would reject high qualities).
        rate.pictureRc = 0;
        rate.qpHdr = 51 - ((self->quality * 51) / 100);
        rate.qpMin = 0;
        rate.qpMax = 51;
    } else {
        rate.pictureRc = 1;
        rate.qpHdr = -1;     // let rate control pick the initial QP
    }

    // Tracked in both modes so bitrate() reports the requested value
    // instead of the driver's level-maximum default.
    rate.bitPerSecond = args[ARG_bitrate].u_int;

    rate.pictureSkip = 0;
    rate.hrd = 0;
    rate.gopLen = self->keyframe_interval;

    preproc.origWidth = width;
    preproc.origHeight = height;
    preproc.xOffset = 0;
    preproc.yOffset = 0;
    preproc.inputType = self->input_type;
    preproc.rotation = H264ENC_ROTATE_0;
    preproc.colorConversion.type = H264ENC_RGBTOYUV_BT601;

    py_h264_encoder_check(H264EncSetCodingCtrl(self->inst, &coding));
    py_h264_encoder_check(H264EncSetRateCtrl(self->inst, &rate));
    py_h264_encoder_check(H264EncSetPreProcessing(self->inst, &preproc));

    // UMA_PERSIST: the output buffer lives across VM execution and must
    // survive the uma_collect() called on caught exceptions.
    // One raw YUV420 frame: a coded frame larger than its input is
    // pathological, and constant-quality mode can run at QP 0 where
    // complex content approaches raw size.
    self->out_size = IMLIB_IMAGE_MAX_SIZE(IM_MIN(uma_avail(0), width * height * 3 / 2 + 4096));
    self->out_buf = uma_malloc(self->out_size, UMA_CACHE | UMA_PERSIST);

    // Generate SPS/PPS (written by software, no cache maintenance needed).
    H264EncIn encIn = {
        .pOutBuf = (u32 *) self->out_buf,
        .busOutBuf = (size_t) self->out_buf,
        .outBufSize = self->out_size,
    };

    H264EncOut encOut = {};
    py_h264_encoder_check(H264EncStrmStart(self->inst, &encIn, &encOut));
    self->sps_pps = mp_obj_new_bytes(self->out_buf, encOut.streamSize);
    return MP_OBJ_FROM_PTR(self);
}

static const mp_rom_map_elem_t py_h264_encoder_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),     MP_ROM_PTR(&py_h264_encoder_deinit_obj)   },
    { MP_ROM_QSTR(MP_QSTR_encode),      MP_ROM_PTR(&py_h264_encoder_encode_obj)   },
    { MP_ROM_QSTR(MP_QSTR_sps_pps),     MP_ROM_PTR(&py_h264_encoder_sps_pps_obj)  },
    { MP_ROM_QSTR(MP_QSTR_keyframe),    MP_ROM_PTR(&py_h264_encoder_keyframe_obj) },
    { MP_ROM_QSTR(MP_QSTR_count),       MP_ROM_PTR(&py_h264_encoder_count_obj)    },
    { MP_ROM_QSTR(MP_QSTR_mse),         MP_ROM_PTR(&py_h264_encoder_mse_obj)      },
    #if MICROPY_PY_ULAB
    { MP_ROM_QSTR(MP_QSTR_motion),      MP_ROM_PTR(&py_h264_encoder_motion_obj)   },
    #endif
    { MP_ROM_QSTR(MP_QSTR_bitrate),     MP_ROM_PTR(&py_h264_encoder_bitrate_obj)  },
    { MP_ROM_QSTR(MP_QSTR_quality),     MP_ROM_PTR(&py_h264_encoder_quality_obj)  },
    { MP_ROM_QSTR(MP_QSTR_deinit),      MP_ROM_PTR(&py_h264_encoder_deinit_obj)   },
};

static MP_DEFINE_CONST_DICT(py_h264_encoder_locals_dict, py_h264_encoder_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
    py_h264_encoder_type,
    MP_QSTR_H264Encoder,
    MP_TYPE_FLAG_NONE,
    make_new, py_h264_encoder_make_new,
    print, py_h264_encoder_print,
    locals_dict, &py_h264_encoder_locals_dict
    );

static const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_codec)        },
    { MP_ROM_QSTR(MP_QSTR_H264Encoder), MP_ROM_PTR(&py_h264_encoder_type) },
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t codec_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_codec, codec_module);
#endif // OMV_VENC_CODEC_ENABLE
