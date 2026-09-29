/*
 * Copyright (C) 2023-2026 OpenMV, LLC.
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
 * GPU driver for the i.MX RT port, backed by the PXP (Pixel Pipeline).
 */
#include "imlib.h"
#if (OMV_GPU_ENABLE == 1)
#include "py/mphal.h"
#include "py/runtime.h"

#include "fsl_pxp.h"
#include "omv_gpu.h"

// A well-formed PXP operation completes in well under a millisecond; this bound
// only exists to escape a hardware fault that terminates processing without
// ever raising the complete flag (otherwise the wait loop would spin forever).
#define OMV_PXP_TIMEOUT_MS      (1000)

int omv_gpu_init() {
    return 0;
}

void omv_gpu_deinit() {
    PXP_Deinit(PXP);
}

int omv_gpu_draw_image(image_t *src_img,
                       rectangle_t *src_rect,
                       image_t *dst_img,
                       rectangle_t *dst_rect,
                       int alpha,
                       const uint16_t *color_palette,
                       const uint8_t *alpha_palette,
                       image_hint_t hint,
                       float *transform) {
    // PXP input must be GRAYSCALE, RGB565, YUV422 or YVU422.
    if (src_img->pixfmt != PIXFORMAT_GRAYSCALE && src_img->pixfmt != PIXFORMAT_RGB565 &&
        src_img->pixfmt != PIXFORMAT_YUV422 && src_img->pixfmt != PIXFORMAT_YVU422) {
        return -1;
    }

    // PXP output must be GRAYSCALE or RGB565.
    if (dst_img->pixfmt != PIXFORMAT_GRAYSCALE && dst_img->pixfmt != PIXFORMAT_RGB565) {
        return -1;
    }

    // A GRAYSCALE output can only come from a GRAYSCALE or YUV/YVU422 input.
    if (dst_img->pixfmt == PIXFORMAT_GRAYSCALE && src_img->pixfmt != PIXFORMAT_GRAYSCALE &&
        src_img->pixfmt != PIXFORMAT_YUV422 && src_img->pixfmt != PIXFORMAT_YVU422) {
        return -1;
    }

    // The PXP works in 8x8 or 16x16 output blocks but places and clips a rect at any pixel, so
    // any size is fine. The one exception is a vertical flip of a scaled draw: the flip engine
    // truncates the last block row of the scaled output, which shifts the image unless the
    // destination height is a multiple of the block size.
    int block = (src_rect->w % 16 || src_rect->h % 16 || dst_rect->w % 16 || dst_rect->h % 16) ? 8 : 16;
    if (hint & IMAGE_HINT_VFLIP && (src_rect->w != dst_rect->w || src_rect->h != dst_rect->h) && dst_rect->h % block) {
        return -1;
    }

    // PXP has no LUT for alpha or color palettes.
    if (color_palette || alpha_palette) {
        return -1;
    }

    // Blending goes through the alpha surface, which takes RGB formats only and is never scaled
    // or flipped: RGB565 over RGB565 at 1:1. Its alpha has 7 bits of precision, so a blend can
    // differ from the CPU's by one 5/6-bit step.
    bool blend = alpha != 255;
    if (blend && (src_img->pixfmt != PIXFORMAT_RGB565 || dst_img->pixfmt != PIXFORMAT_RGB565 ||
                  src_rect->w != dst_rect->w || src_rect->h != dst_rect->h ||
                  hint & (IMAGE_HINT_HMIRROR | IMAGE_HINT_VFLIP))) {
        return -1;
    }

    // Affine transforms are not supported by the PXP process-surface path.
    if (transform) {
        return -1;
    }

    // PXP cannot reduce the image size by more than 16x.
    if (dst_rect->w < src_rect->w / 16 || dst_rect->h < src_rect->h / 16) {
        return -1;
    }

    // PXP cannot enlarge the image size by more than 4096x.
    if (dst_rect->w > src_rect->w * 4096 || dst_rect->h > src_rect->h * 4096) {
        return -1;
    }


    // The PXP scaler is always bilinear, so a scaled draw is only taken when that is what was
    // asked for; a nearest-neighbour request stays on the CPU and gets nearest-neighbour output.
    if (!(hint & IMAGE_HINT_BILINEAR) && (src_rect->w != dst_rect->w || src_rect->h != dst_rect->h)) {
        return -1;
    }

    PXP_Init(PXP);

    // 16x16 blocks halve the number of memory requests; 8x8 when a dimension is not a multiple.
    PXP_SetProcessBlockSize(PXP, block == 16 ? kPXP_BlockSize16 : kPXP_BlockSize8);

    pxp_ps_buffer_config_t input_buffer_config = {};

    if (src_img->pixfmt == PIXFORMAT_GRAYSCALE) {
        input_buffer_config.pixelFormat = kPXP_PsPixelFormatY8;
    } else if (src_img->pixfmt == PIXFORMAT_RGB565) {
        input_buffer_config.pixelFormat = kPXP_PsPixelFormatRGB565;
    } else if (src_img->pixfmt == PIXFORMAT_YUV422) {
        input_buffer_config.pixelFormat = kPXP_PsPixelFormatUYVY1P422;
        input_buffer_config.swapByte = true;
    } else if (src_img->pixfmt == PIXFORMAT_YVU422) {
        input_buffer_config.pixelFormat = kPXP_PsPixelFormatVYUY1P422;
        input_buffer_config.swapByte = true;
    }

    if (src_img->pixfmt == PIXFORMAT_GRAYSCALE) {
        uint8_t *src8 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(src_img, src_rect->y) + src_rect->x;
        input_buffer_config.bufferAddr = (uint32_t) src8;
        input_buffer_config.pitchBytes = src_img->w;
    } else {
        uint16_t *src16 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src_img, src_rect->y) + src_rect->x;
        input_buffer_config.bufferAddr = (uint32_t) src16;
        input_buffer_config.pitchBytes = src_img->w * sizeof(uint16_t);
    }

    uintptr_t dst_addr = dst_img->pixfmt == PIXFORMAT_GRAYSCALE ?
                         (uintptr_t) (IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(dst_img, dst_rect->y) + dst_rect->x) :
                         (uintptr_t) (IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(dst_img, dst_rect->y) + dst_rect->x);

    // Drawing an image onto itself: blocks are read and written in raster order, so a source that
    // starts at or after the destination in memory is always read before it is overwritten (a
    // copy or a scale-down toward the origin). Flips read blocks the earlier ones already wrote.
    if (dst_img->data == src_img->data &&
        (hint & (IMAGE_HINT_HMIRROR | IMAGE_HINT_VFLIP) || dst_addr > input_buffer_config.bufferAddr)) {
        PXP_Deinit(PXP);
        return -1;
    }

    PXP_SetProcessSurfaceBufferConfig(PXP, &input_buffer_config);
    PXP_SetProcessSurfaceScaler(PXP, src_rect->w, src_rect->h, dst_rect->w, dst_rect->h);
    PXP_SetProcessSurfacePosition(PXP, 0, 0, dst_rect->w - 1, dst_rect->h - 1);

    if (blend) {
        // Alpha surface = the source; process surface = the destination pixels it is composited
        // onto, in place.
        pxp_as_buffer_config_t as_config = {
            .pixelFormat = kPXP_AsPixelFormatRGB565,
            .bufferAddr = input_buffer_config.bufferAddr,
            .pitchBytes = input_buffer_config.pitchBytes,
        };
        PXP_SetAlphaSurfaceBufferConfig(PXP, &as_config);
        PXP_SetAlphaSurfacePosition(PXP, 0, 0, dst_rect->w - 1, dst_rect->h - 1);
        pxp_as_blend_config_t blend_config = {
            .alpha = alpha,
            .invertAlpha = false,
            .alphaMode = kPXP_AlphaOverride,
            .ropMode = kPXP_RopMaskAs,
        };
        PXP_SetAlphaSurfaceBlendConfig(PXP, &blend_config);
        input_buffer_config.bufferAddr = dst_addr;
        input_buffer_config.pitchBytes = dst_img->w * sizeof(uint16_t);
        PXP_SetProcessSurfaceBufferConfig(PXP, &input_buffer_config);
    }

    pxp_output_buffer_config_t output_buffer_config = {};

    if (dst_img->pixfmt == PIXFORMAT_GRAYSCALE) {
        uint8_t *dst8 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(dst_img, dst_rect->y) + dst_rect->x;
        output_buffer_config.pixelFormat = kPXP_OutputPixelFormatY8;
        output_buffer_config.buffer0Addr = (uint32_t) dst8;
        output_buffer_config.pitchBytes = dst_img->w;
    } else {
        uint16_t *dst16 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(dst_img, dst_rect->y) + dst_rect->x;
        output_buffer_config.pixelFormat = kPXP_OutputPixelFormatRGB565;
        output_buffer_config.buffer0Addr = (uint32_t) dst16;
        output_buffer_config.pitchBytes = dst_img->w * sizeof(uint16_t);
        // Convert the input color space to RGB when it is not already RGB565.
        PXP_EnableCsc1(PXP, src_img->pixfmt != PIXFORMAT_RGB565);
    }

    output_buffer_config.width = dst_rect->w;
    output_buffer_config.height = dst_rect->h;
    PXP_SetOutputBufferConfig(PXP, &output_buffer_config);

    pxp_flip_mode_t flip_mode = kPXP_FlipDisable;
    flip_mode |= hint & IMAGE_HINT_HMIRROR ? kPXP_FlipHorizontal : 0;
    flip_mode |= hint & IMAGE_HINT_VFLIP ? kPXP_FlipVertical : 0;
    // Flip at the output stage: flipping the process surface before the scaler breaks scaled draws.
    PXP_SetRotateConfig(PXP, kPXP_RotateOutputBuffer, kPXP_Rotate0, flip_mode);

    #if __DCACHE_PRESENT
    // Push the whole source image out of the cache so the PXP reads current
    // data, and clean+invalidate the whole destination so no dirty line is
    // written back over the PXP's output. Operating on the contiguous image
    // buffer (not per-row sub-regions) keeps this on cache-line boundaries.
    SCB_CleanDCache_by_Addr(src_img->data, image_size(src_img));
    SCB_CleanInvalidateDCache_by_Addr(dst_img->data, image_size(dst_img));
    #endif

    PXP_ClearStatusFlags(PXP, kPXP_CompleteFlag | kPXP_Axi0ReadErrorFlag | kPXP_Axi0WriteErrorFlag);
    PXP_Start(PXP);

    // Wait for completion, but bail out on an AXI bus error (the pipeline
    // terminates without ever setting the complete flag) or on a timeout.
    uint32_t flags;
    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        flags = PXP_GetStatusFlags(PXP);
        if (flags & (kPXP_CompleteFlag | kPXP_Axi0ReadErrorFlag | kPXP_Axi0WriteErrorFlag)) {
            break;
        }
        if (mp_hal_ticks_ms() - start >= OMV_PXP_TIMEOUT_MS) {
            break;
        }
    }

    bool faulted = flags & (kPXP_Axi0ReadErrorFlag | kPXP_Axi0WriteErrorFlag);

    PXP_Reset(PXP);
    PXP_Deinit(PXP);

    if (faulted || !(flags & kPXP_CompleteFlag)) {
        // Fall back to the CPU. Nothing reliable was written to the output.
        return -1;
    }

    #if __DCACHE_PRESENT
    // Drop any stale cached reads of the destination now that the PXP wrote it.
    SCB_InvalidateDCache_by_Addr(dst_img->data, image_size(dst_img));
    #endif

    return 0;
}
#endif // (OMV_GPU_ENABLE == 1)
