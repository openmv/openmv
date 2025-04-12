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
 * GPU driver for Alif port.
 */
#include <stdio.h>
#include <assert.h>

#include "py/mphal.h"
#include "py/runtime.h"

#include "dave_d0lib.h"
#include "dave_registermap.h"
#include "dave_driver.h"
#include "omv_gpu.h"

static d2_device *dev;

extern char _gpu_memory_start;
extern char _gpu_memory_end;

#define OMV_GPU_CHECK_ERROR(err) \
    omv_gpu_check_error(err, __LINE__)
static void omv_gpu_check_error(uint32_t error, uint32_t line);

#define D2_WIDTH(x) (((uint16_t) (x) << 4) & 0x3FF0) // 10:4 fixed-point number (4 bits fraction)
#define D2_POINT(x) (((uint16_t) (x) << 4) & 0x7FF0) // 1:11:4 FP (1 bit sign, 11 bits integer, 4 bits fraction)
#define D2_BLITP(x) (((uint16_t) (x) << 0) & 0x03FF) // unsigned short. The allowed range is 0 to 1023.

extern NORETURN void __fatal_error(const char *msg);

int omv_gpu_init() {
    if (dev != NULL) {
        return 0;
    }

    #if defined(WITH_MM_DYNAMIC)
    if (d0_initdefaultheapmanager() != 1) {
        __fatal_error("Failed to init GPU heap manager");
    }
    #elif defined(WITH_MM_FIXED_RANGE)
    if (d0_initheapmanager(&_gpu_memory_start,
                           &_gpu_memory_end - &_gpu_memory_start,
                           d0_mm_fixed_range,
                           NULL,    // unified memory.
                           0,
                           0,
                           d0_mm_fixed_range,
                           d0_ma_unified) != 1) {
        __fatal_error("Failed to init GPU heap manager");
    }
    #else
    #error "GPU heap manager is not defined"
    #endif

    dev = d2_opendevice(d2_df_no_irq);
    if (dev == NULL) {
        __fatal_error("Failed to init GPU device");
    }

    if (d2_inithw(dev, 0) != D2_OK) {
        __fatal_error("Failed to init GPU hardware");
    }

#if 0
    d2_setblendmode(dev, d2_bm_alpha, d2_bm_one_minus_alpha);
    d2_setalphamode(dev, d2_am_constant); // default in d2_newcontext
    d2_setlinecap(dev, d2_lc_butt);
    d2_setantialiasing(dev, 1);
    d2_setblur(dev, 0);
    d2_settextureoperation(dev, d2_to_one, d2_to_copy, d2_to_copy, d2_to_copy);
    d2_setfillmode(dev, d2_fm_texture);
    d2_settexturemode(dev, d2_tm_filter);
#endif
    return 0;
}

void omv_gpu_deinit() {
    if (dev) {
        d2_closedevice(dev);
        dev = NULL;
    }
}

static d2_u32 omv_gpu_pixfmt(uint32_t omv_pixfmt) {
    switch (omv_pixfmt) {
        case PIXFORMAT_BINARY:
            return d2_mode_alpha1;
        case PIXFORMAT_BAYER_ANY:
        case PIXFORMAT_GRAYSCALE:
            return d2_mode_alpha8;
        case PIXFORMAT_RGB565:
            return d2_mode_rgb565;
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Pixel format is not supported"));
}

int omv_gpu_draw_image(image_t *src_img,
                       rectangle_t *src_rect,
                       image_t *dst_img,
                       rectangle_t *dst_rect,
                       int alpha,
                       const uint16_t *color_palette,
                       const uint8_t *alpha_palette,
                       image_hint_t hint) {
    // Belnding is not supported yet.
    if (color_palette || alpha_palette) {
        return -1;
    }
    OMV_PROFILE_START();
    d2_s32 err;
    d2_u32 blit_flags = 0;

    d2_renderbuffer *rbuffer = d2_getrenderbuffer(dev, 0);
    err = d2_selectrenderbuffer(dev, rbuffer);
    OMV_GPU_CHECK_ERROR(err);

    d2_u32 dst_pixfmt = omv_gpu_pixfmt(dst_img->pixfmt);
    err = d2_framebuffer(dev, dst_img->data, dst_img->w, dst_img->w, dst_img->h, dst_pixfmt);
    OMV_GPU_CHECK_ERROR(err);

    d2_u32 src_pixfmt = omv_gpu_pixfmt(src_img->pixfmt);
    err = d2_setblitsrc(dev, src_img->data, src_img->w, src_img->w, src_img->h, src_pixfmt);
    OMV_GPU_CHECK_ERROR(err);

    if (hint & IMAGE_HINT_BILINEAR) {
        blit_flags |= d2_bf_filter;
    }

    if (src_pixfmt == d2_mode_alpha8 || dst_pixfmt == d2_mode_alpha8) {
        blit_flags |= d2_bf_usealpha;
    }

    // Flush source image
    SCB_CleanDCache_by_Addr(src_img->data, image_size(src_img));

    // Copy source image to the framebuffer.
    err = d2_blitcopy(dev,
                      src_rect->w,
                      src_rect->h,
                      D2_BLITP(src_rect->x),
                      D2_BLITP(src_rect->y),
                      D2_WIDTH(dst_rect->w),
                      D2_WIDTH(dst_rect->h),
                      D2_POINT(dst_rect->x),
                      D2_POINT(dst_rect->y),
                      blit_flags | d2_bf_no_blitctxbackup);
    OMV_GPU_CHECK_ERROR(err);

    err = d2_executerenderbuffer(dev, rbuffer, d2_ef_default);
    OMV_GPU_CHECK_ERROR(err);

    err = d2_flushframe(dev);
    OMV_GPU_CHECK_ERROR(err);

    // Invalidate the framebuffer image.
    SCB_InvalidateDCache_by_Addr(dst_img->data, image_size(dst_img));
    OMV_PROFILE_PRINT();
    return 0;
}

static void omv_gpu_check_error(uint32_t error, uint32_t line) {
    static const char *errors[] = {
        "success",
        "memory allocation failed %d",
        "invalid device %d",
        "invalid rendering context %d",
        "invalid renderbuffer context %d",
        "hardware device already in use %d",
        "device already assigned %d",
        "cannot operate on default context %d",
        "index is out of bounds %d",
        "rendermode not supported %d",
        "width out of legal range %d",
        "height out of legal range %d",
        "illegal framebuffer address %d",
        "parameter too close to zero %d",
        "parameter is negative %d",
        "parameter value is too large %d",
        "unsupported mode %d",
        "source pointer may not be null %d",
        "operation cannot execute while hardware is busy %d",
        "cannot operate on default buffer %d",
        "d2_df_no_dlist is not supported in low_localmemmode %d",
        "not enough dlistblocks. please adjust in d2_lowlocalmemmode(...) %d",
    };

    if (error == D2_OK) {
        return;
    }

    if (error > (sizeof(errors) / sizeof(errors[0]))) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unknown GPU error."));
    }

    mp_raise_msg_varg(&mp_type_RuntimeError, (mp_rom_error_text_t) errors[error], line);
}


