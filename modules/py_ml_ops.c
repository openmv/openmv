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
 * Machine Learning pre- and post-processing operations.
 */
#include <math.h>
#include "py/runtime.h"
#include "py/obj.h"

#if MICROPY_PY_ML_OPS
#include "ulab/code/ndarray.h"
#include "imlib.h"
#include "py_helper.h"
#include "simd.h"

#define ML_LOG_MEL_EPS          (1e-6f)

// Returns a dense array of a type, and its last 3 dimensions in h, w and c. Leading
// dimensions must be 1 (e.g. a batch of 1). A 2D array has c = 1.
static void *py_ml_ops_get_typed_array(mp_obj_t arg, uint8_t dtype, size_t *h, size_t *w, size_t *c) {
    if (!MP_OBJ_IS_TYPE(arg, &ulab_ndarray_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("Expected a ndarray"));
    }

    ndarray_obj_t *array = MP_OBJ_TO_PTR(arg);

    if (array->dtype != dtype || !ndarray_is_dense(array) || array->ndim < 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("Expected a dense ndarray with at least 2 dimensions"));
    }

    size_t ndim = array->ndim;
    size_t c_dims = (ndim >= 3) ? 1 : 0;

    for (size_t i = 0; i < ndim - 2 - c_dims; i++) {
        if (array->shape[ULAB_MAX_DIMS - ndim + i] != 1) {
            mp_raise_ValueError(MP_ERROR_TEXT("Expected leading dimensions of 1"));
        }
    }

    *c = c_dims ? array->shape[ULAB_MAX_DIMS - 1] : 1;
    *w = array->shape[ULAB_MAX_DIMS - 1 - c_dims];
    *h = array->shape[ULAB_MAX_DIMS - 2 - c_dims];
    return array->array;
}

static float *py_ml_ops_get_array(mp_obj_t arg, size_t *h, size_t *w, size_t *c) {
    return py_ml_ops_get_typed_array(arg, NDARRAY_FLOAT, h, w, c);
}

static ndarray_obj_t *py_ml_ops_new_array(size_t h, size_t w) {
    size_t shape[ULAB_MAX_DIMS] = {0};
    shape[ULAB_MAX_DIMS - 2] = h;
    shape[ULAB_MAX_DIMS - 1] = w;
    return ndarray_new_dense_ndarray(2, shape, NDARRAY_FLOAT);
}

// e^x as 2^n * 2^f, with 2^f on [0, 1) from its Taylor series (relative error < 1e-5).
static inline float py_ml_ops_expf(float x) {
    float t = fminf(fmaxf(x, -87.0f), 88.0f) * 1.44269504f;
    float n = floorf(t);
    float f = t - n;
    float p = 0.000154035304f;
    p = p * f + 0.00133335581f;
    p = p * f + 0.00961812911f;
    p = p * f + 0.0555041087f;
    p = p * f + 0.240226507f;
    p = p * f + 0.693147182f;
    p = p * f + 1.0f;
    union {
        float f;
        int32_t i;
    } u = { .f = p };
    u.i += ((int32_t) n) << 23;
    return u.f;
}

// py_ml_ops_expf() of each lane.
static inline v128_t py_ml_ops_vexpf(v128_t x) {
    v128_t t = vmul_n_f32(vmin_f32(vmax_f32(x, vdup_f32(-87.0f)), vdup_f32(88.0f)), 1.44269504f);
    v128_t n = vfloor_f32(t);
    v128_t f = vsub_f32(t, n);
    v128_t p = vdup_f32(0.000154035304f);
    p = vmla_f32(p, f, vdup_f32(0.00133335581f));
    p = vmla_f32(p, f, vdup_f32(0.00961812911f));
    p = vmla_f32(p, f, vdup_f32(0.0555041087f));
    p = vmla_f32(p, f, vdup_f32(0.240226507f));
    p = vmla_f32(p, f, vdup_f32(0.693147182f));
    p = vmla_f32(p, f, vdup_f32(1.0f));
    return vadd_s32(p, vlsl_s32(vcvt_f32_s32(n), 23));
}

// Convex upsampling (RAFT): each output pixel is a weighted average of the 3x3
// neighbors of its low resolution pixel, with the softmax of its 9 weights. The
// weights are (w - offset) * scale, of float or int8 values (e.g. a raw output,
// which is faster). The weights are (h, w, factor, factor, 9), with the 9 neighbors
// last (RAFT's (9, factor, factor) order needs a permute), and edges repeat the
// border pixel (RAFT pads with zeros). Returns a float array, or with out=, writes
// into a GRAYSCALE image (values rounded and clamped to 0-255) and returns it.
static mp_obj_t py_ml_convex_upsample(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_src, ARG_weights, ARG_factor, ARG_scale, ARG_offset, ARG_out };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_weights, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_factor,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_scale,   MP_ARG_KW_ONLY | MP_ARG_OBJ,  {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_offset,  MP_ARG_KW_ONLY | MP_ARG_OBJ,  {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_out,     MP_ARG_KW_ONLY | MP_ARG_OBJ,  {.u_rom_obj = MP_ROM_NONE} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    size_t h, w, c, wh, ww, wc;
    float *src = py_ml_ops_get_array(args[ARG_src].u_obj, &h, &w, &c);
    bool is_int8 = MP_OBJ_IS_TYPE(args[ARG_weights].u_obj, &ulab_ndarray_type) &&
                   ((ndarray_obj_t *) MP_OBJ_TO_PTR(args[ARG_weights].u_obj))->dtype == NDARRAY_INT8;
    void *weights = py_ml_ops_get_typed_array(args[ARG_weights].u_obj,
                                              is_int8 ? NDARRAY_INT8 : NDARRAY_FLOAT, &wh, &ww, &wc);
    int factor = args[ARG_factor].u_int;
    float scale = (args[ARG_scale].u_obj == mp_const_none) ? 1.0f : mp_obj_get_float(args[ARG_scale].u_obj);
    float offset = (args[ARG_offset].u_obj == mp_const_none) ? 0.0f : mp_obj_get_float(args[ARG_offset].u_obj);

    if (factor < 1 || c != 1 || wh != h || ww != w || wc != (size_t) (factor * factor * 9)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Expected src (h, w) and weights (h, w, factor * factor * 9)"));
    }

    size_t ow = w * factor;
    mp_obj_t out = args[ARG_out].u_obj;
    float *dst = NULL;
    uint8_t *dst8 = NULL;

    if (out == mp_const_none) {
        ndarray_obj_t *array = py_ml_ops_new_array(h * factor, ow);
        out = MP_OBJ_FROM_PTR(array);
        dst = (float *) array->array;
    } else {
        image_t *img = py_helper_arg_to_image(out, ARG_IMAGE_MUTABLE | ARG_IMAGE_GRAYSCALE);
        if ((size_t) img->w != ow || (size_t) img->h != h * factor) {
            mp_raise_ValueError(MP_ERROR_TEXT("Expected out of (w * factor, h * factor)"));
        }
        dst8 = img->data;
    }

    // The exponential of each int8 value, relative to the largest possible one. Values
    // more than 87 / |scale| below it clamp to e^-87, which only matters if all 9 weights
    // of a sub-pixel do (the softmax becomes an average).
    float lut[256];
    if (is_int8) {
        float max = ((scale > 0.0f ? 127.0f : -128.0f) - offset) * scale;
        for (int i = 0; i < 256; i++) {
            lut[i] = py_ml_ops_expf((i - 128 - offset) * scale - max);
        }
    }

    // The sub-pixels of a row are computed FLOAT32_VECTOR_SIZE at a time; the
    // weights of consecutive sub-pixels are 9 apart.
    const v128_t offsets = vmul_n_u32(vidup_u32(0, 1), 9);

    for (size_t y = 0; y < h; y++) {
        // The rows above and below, with clamped edges.
        const float *r0 = src + (y ? y - 1 : 0) * w;
        const float *r1 = src + y * w;
        const float *r2 = src + MIN(y + 1, h - 1) * w;

        for (size_t x = 0; x < w; x++) {
            // The 3x3 neighbors, with clamped edges.
            size_t xl = x ? x - 1 : 0;
            size_t xr = MIN(x + 1, w - 1);
            float n[9] = { r0[xl], r0[x], r0[xr], r1[xl], r1[x], r1[xr], r2[xl], r2[x], r2[xr] };

            size_t pos = y * factor * ow + x * factor;
            size_t base = (y * w + x) * wc;

            for (int sy = 0; sy < factor; sy++, pos += ow) {
                for (int sx = 0; sx < factor; sx += FLOAT32_VECTOR_SIZE) {
                    v128_predicate_t pred = vpredicate_32(factor - sx);
                    size_t i = base + (sy * factor + sx) * 9;
                    v128_t sum = vdup_f32(0.0f);
                    v128_t acc = vdup_f32(0.0f);

                    if (is_int8) {
                        const int8_t *pw = (const int8_t *) weights + i;
                        for (int k = 0; k < 9; k++) {
                            v128_t q = vldr_s8_widen_s32_gather_pred(pw + k, offsets, pred);
                            v128_t e = vldr_f32_gather_pred(lut, vadd_n_s32(q, 128), pred);
                            sum = vadd_f32(sum, e);
                            acc = vmla_n_f32(e, n[k], acc);
                        }
                    } else {
                        const float *pw = (const float *) weights + i;
                        v128_t l[9], max = vdup_f32(-INFINITY);
                        for (int k = 0; k < 9; k++) {
                            l[k] = vldr_f32_gather_pred(pw + k, offsets, pred);
                            l[k] = vmul_n_f32(vsub_f32(l[k], vdup_f32(offset)), scale);
                            max = vmax_f32(max, l[k]);
                        }
                        for (int k = 0; k < 9; k++) {
                            v128_t e = py_ml_ops_vexpf(vsub_f32(l[k], max));
                            sum = vadd_f32(sum, e);
                            acc = vmla_n_f32(e, n[k], acc);
                        }
                    }

                    v128_t v = vdiv_f32(acc, sum);
                    if (dst8) {
                        v = vmin_f32(vmax_f32(vadd_f32(v, vdup_f32(0.5f)), vdup_f32(0.0f)), vdup_f32(255.0f));
                        vstr_u32_narrow_u8_pred(dst8 + pos + sx, vcvt_f32_s32(v), pred);
                    } else {
                        vstr_f32_pred(dst + pos + sx, v, pred);
                    }
                }
            }
        }
    }

    return out;
}
MP_DEFINE_CONST_FUN_OBJ_KW(py_ml_convex_upsample_obj, 3, py_ml_convex_upsample);

// Depth to space: (h, w, factor * factor) -> (h * factor, w * factor).
static mp_obj_t py_ml_depth_to_space(mp_obj_t x_obj, mp_obj_t factor_obj) {
    size_t h, w, c;
    float *src = py_ml_ops_get_array(x_obj, &h, &w, &c);
    int factor = mp_obj_get_int(factor_obj);

    if (factor < 1 || c != (size_t) (factor * factor)) {
        mp_raise_ValueError(MP_ERROR_TEXT("Expected x (h, w, factor * factor)"));
    }

    size_t ow = w * factor;
    ndarray_obj_t *out = py_ml_ops_new_array(h * factor, ow);
    float *dst = (float *) out->array;

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++, src += c) {
            float *row = dst + y * factor * ow + x * factor;
            for (int sy = 0; sy < factor; sy++, row += ow) {
                for (int sx = 0; sx < factor; sx += FLOAT32_VECTOR_SIZE) {
                    v128_predicate_t pred = vpredicate_32(factor - sx);
                    vstr_f32_pred(row + sx, vldr_f32_pred(src + sy * factor + sx, pred), pred);
                }
            }
        }
    }

    return MP_OBJ_FROM_PTR(out);
}
MP_DEFINE_CONST_FUN_OBJ_2(py_ml_depth_to_space_obj, py_ml_depth_to_space);

static ndarray_obj_t *py_ml_audio_ndarray(mp_obj_t obj, uint8_t ndim, const char *name) {
    if (!mp_obj_is_type(obj, &ulab_ndarray_type)) {
        mp_raise_msg_varg(&mp_type_TypeError, MP_ERROR_TEXT("%s must be an ndarray"), name);
    }
    ndarray_obj_t *array = MP_OBJ_TO_PTR(obj);
    if (array->dtype != NDARRAY_FLOAT || array->ndim != ndim || !ndarray_is_dense(array)) {
        mp_raise_msg_varg(&mp_type_ValueError,
                          MP_ERROR_TEXT("%s must be a dense %dD float ndarray"), name, ndim);
    }
    return array;
}

static float py_ml_hz_to_mel(float hz) {
    return 2595.0f * log10f(1.0f + hz / 700.0f);
}

static float py_ml_mel_to_hz(float mel) {
    return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
}

// Log-mel feature extractor, as torchaudio's MelSpectrogram (HTK mel scale, no filter
// normalization, power 2, center=False) followed by log(x + 1e-6). The window and the mel
// filter weights are computed once, when the object is created.
typedef struct _py_ml_logmel_obj_t {
    mp_obj_base_t base;
    size_t n_fft;
    size_t n_bins;
    size_t n_mels;
    size_t hop;
    arm_rfft_fast_instance_f32 rfft;
    float *window;      // Hann window, n_fft.
    float *input;       // FFT input, n_fft.
    float *spectrum;    // FFT output, n_fft.
    float *power;       // Power spectrum, n_bins.
    uint16_t *ranges;   // First bin and number of bins of each filter, n_mels * 2.
    float *weights;     // Each filter's weights for its bins, in order.
} py_ml_logmel_obj_t;

// LogMel(n_mels=80, n_fft=512, hop=160, sample_rate=16000, f_min=0, f_max=sample_rate/2)
static mp_obj_t py_ml_logmel_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_n_mels, ARG_n_fft, ARG_hop, ARG_sample_rate, ARG_f_min, ARG_f_max };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_n_mels, MP_ARG_INT, {.u_int = 80 } },
        { MP_QSTR_n_fft, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 512 } },
        { MP_QSTR_hop, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 160 } },
        { MP_QSTR_sample_rate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 16000 } },
        { MP_QSTR_f_min, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE } },
        { MP_QSTR_f_max, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t n_mels = args[ARG_n_mels].u_int;
    mp_int_t n_fft = args[ARG_n_fft].u_int;
    mp_int_t hop = args[ARG_hop].u_int;
    float sample_rate = args[ARG_sample_rate].u_int;
    float f_min = (args[ARG_f_min].u_obj == mp_const_none) ? 0.0f :
                  mp_obj_get_float(args[ARG_f_min].u_obj);
    float f_max = (args[ARG_f_max].u_obj == mp_const_none) ? sample_rate / 2.0f :
                  mp_obj_get_float(args[ARG_f_max].u_obj);

    if (n_mels <= 0 || hop <= 0 || f_min < 0.0f || f_max <= f_min || f_max > sample_rate / 2.0f) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("invalid arguments"));
    }

    py_ml_logmel_obj_t *self = mp_obj_malloc(py_ml_logmel_obj_t, type);
    if (n_fft <= 0 || arm_rfft_fast_init_f32(&self->rfft, n_fft) != ARM_MATH_SUCCESS) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("unsupported FFT size"));
    }

    self->n_fft = n_fft;
    self->n_bins = n_fft / 2 + 1;
    self->n_mels = n_mels;
    self->hop = hop;
    self->window = m_new(float, n_fft);
    self->input = m_new(float, n_fft);
    self->spectrum = m_new(float, n_fft);
    self->power = m_new(float, self->n_bins);
    self->ranges = m_new(uint16_t, n_mels * 2);

    for (mp_int_t i = 0; i < n_fft; i++) {
        self->window[i] = 0.5f - 0.5f * cosf(2.0f * PI * i / n_fft);
    }

    // Filter edges, evenly spaced on the mel scale. Filter m rises from edges[m] to edges[m + 1]
    // and falls to edges[m + 2], and covers the bins strictly between edges[m] and edges[m + 2].
    float *edges = m_new(float, n_mels + 2);
    float mel_min = py_ml_hz_to_mel(f_min);
    float mel_step = (py_ml_hz_to_mel(f_max) - mel_min) / (n_mels + 1);
    for (mp_int_t i = 0; i < n_mels + 2; i++) {
        edges[i] = py_ml_mel_to_hz(mel_min + mel_step * i);
    }

    float bin_hz = sample_rate / n_fft;
    size_t n_weights = 0;
    for (mp_int_t m = 0; m < n_mels; m++) {
        size_t start = MIN((size_t) floorf(edges[m] / bin_hz) + 1, self->n_bins);
        size_t end = MIN((size_t) ceilf(edges[m + 2] / bin_hz), self->n_bins);
        self->ranges[m * 2 + 0] = start;
        self->ranges[m * 2 + 1] = (end > start) ? end - start : 0;
        n_weights += self->ranges[m * 2 + 1];
    }

    self->weights = m_new(float, MAX(n_weights, 1));
    for (size_t m = 0, w = 0; m < (size_t) n_mels; m++) {
        float lo = edges[m], mid = edges[m + 1], hi = edges[m + 2];
        for (size_t k = self->ranges[m * 2 + 0], i = 0; i < self->ranges[m * 2 + 1]; k++, i++) {
            float hz = k * bin_hz;
            self->weights[w++] = MAX((hz <= mid) ? (hz - lo) / (mid - lo) : (hi - hz) / (hi - mid), 0.0f);
        }
    }
    m_del(float, edges, n_mels + 2);
    return MP_OBJ_FROM_PTR(self);
}

// LogMel(pcm, out, row=0) -> frames
//
// Computes the log-mel features of float PCM samples in [-1, 1] (like torchaudio; int16-scale
// samples offset every feature by a constant): for each frame of n_fft samples, every
// hop samples, the log of each mel band is written to out[row + i] (n_mels wide). Frames are
// computed while samples remain and out has rows left. Returns the number of frames.
static mp_obj_t py_ml_logmel_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    py_ml_logmel_obj_t *self = MP_OBJ_TO_PTR(self_in);
    enum { ARG_pcm, ARG_out, ARG_row };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pcm, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_out, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_row, MP_ARG_INT, {.u_int = 0 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    ndarray_obj_t *pcm = py_ml_audio_ndarray(args[ARG_pcm].u_obj, 1, "pcm");
    ndarray_obj_t *out = py_ml_audio_ndarray(args[ARG_out].u_obj, 2, "out");
    size_t out_rows = out->shape[ULAB_MAX_DIMS - 2];
    mp_int_t row = args[ARG_row].u_int;

    if (out->shape[ULAB_MAX_DIMS - 1] != self->n_mels) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("out width must be n_mels"));
    }
    if (row < 0 || (size_t) row > out_rows) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("invalid row"));
    }

    size_t frames = 0;
    if (pcm->len >= self->n_fft) {
        frames = MIN((pcm->len - self->n_fft) / self->hop + 1, out_rows - row);
    }

    const float *samples = (const float *) pcm->array;
    float *features = (float *) out->array + row * self->n_mels;

    for (size_t f = 0; f < frames; f++, samples += self->hop, features += self->n_mels) {
        arm_mult_f32(samples, self->window, self->input, self->n_fft);
        // Packed output: [DC, Nyquist], then the real and imaginary parts of bins 1..n_fft/2-1.
        arm_rfft_fast_f32(&self->rfft, self->input, self->spectrum, 0);
        self->power[0] = self->spectrum[0] * self->spectrum[0];
        self->power[self->n_bins - 1] = self->spectrum[1] * self->spectrum[1];
        arm_cmplx_mag_squared_f32(self->spectrum + 2, self->power + 1, self->n_bins - 2);

        const float *w = self->weights;
        for (size_t m = 0; m < self->n_mels; m++) {
            float sum = 0.0f;
            if (self->ranges[m * 2 + 1]) {
                arm_dot_prod_f32(self->power + self->ranges[m * 2 + 0], w, self->ranges[m * 2 + 1], &sum);
                w += self->ranges[m * 2 + 1];
            }
            features[m] = logf(sum + ML_LOG_MEL_EPS);
        }
    }
    return mp_obj_new_int(frames);
}

MP_DEFINE_CONST_OBJ_TYPE(
    py_ml_logmel_type,
    MP_QSTR_LogMel,
    MP_TYPE_FLAG_NONE,
    make_new, py_ml_logmel_make_new,
    call, py_ml_logmel_call
    );
#endif // MICROPY_PY_ML_OPS
