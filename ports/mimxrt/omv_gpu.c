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

// Set to 1 to print the PXP fault registers (status, faulting block, AXI error
// id) whenever the hardware terminates an operation with a bus error. Off by
// default; a fault falls the operation back to the CPU either way.
#define OMV_PXP_DEBUG           (0)

#if OMV_PXP_DEBUG
static void omv_pxp_report_fault(uint32_t stat,
                                 image_t *src_img, rectangle_t *src_rect,
                                 image_t *dst_img, rectangle_t *dst_rect) {
    // BLOCKY/BLOCKX name the output block the pipeline was working on when it
    // faulted, so the bottom/right edge shows up as the last block index.
    unsigned blocky = (stat & PXP_STAT_BLOCKY_MASK) >> PXP_STAT_BLOCKY_SHIFT;
    unsigned blockx = (stat & PXP_STAT_BLOCKX_MASK) >> PXP_STAT_BLOCKX_SHIFT;
    mp_printf(&mp_plat_print,
              "PXP fault: stat=0x%08x %s%sblock=(%u,%u) axi_id=%u "
              "src=%dx%d rect(%d,%d,%d,%d) dst=%dx%d rect(%d,%d,%d,%d)\n",
              (unsigned) stat,
              (stat & kPXP_Axi0ReadErrorFlag) ? "READ " : "",
              (stat & kPXP_Axi0WriteErrorFlag) ? "WRITE " : "",
              blockx, blocky, PXP_GetAxiErrorId(PXP, 0),
              src_img->w, src_img->h, src_rect->x, src_rect->y, src_rect->w, src_rect->h,
              dst_img->w, dst_img->h, dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h);
}
#endif

// Scaler settings for one axis: the decimation shift and the 2.12 fixed-point step.
//
// Scaling down (or 1:1) uses the SDK's ratio, step = in / out: the last output pixel then
// samples input (out - 1) * in / out = in - in / out <= in - 1, so the bilinear filter's +1
// neighbour stays inside the rect (the PXP pre-decimates by 2^dec when the ratio is above 2).
//
// Scaling up needs a different mapping. With step = in / out the last output pixel samples
// input in - in / out > in - 1, so the filter's +1 neighbour is one pixel past the rect:
// horizontally that is the next row's first pixel (wrong data along the last column), and
// vertically it is the row after the buffer (wrong data along the last row, and an AXI read
// error when the buffer ends at the end of a memory region -- the historical scale-up fault).
// Mapping output 0..out-1 onto input 0..in-1 instead, step = (in - 1) / (out - 1) rounded
// down, keeps the last sample at or below in - 1 so nothing past the rect is ever fetched.
static uint32_t omv_pxp_scaler(uint32_t in, uint32_t out, uint8_t *dec) {
    if (out > in) {
        *dec = 0;
        return (out > 1) ? (((in - 1) << 12) / (out - 1)) : (1 << 12);
    }

    uint32_t fact = (in << 12) / out;
    if (fact >= (16 << 12)) {
        *dec = 3;
        return 0x2000;
    }
    *dec = (fact > (8 << 12)) ? 3 : (fact > (4 << 12)) ? 2 : (fact > (2 << 12)) ? 1 : 0;
    fact >>= *dec;
    return fact ? fact : 1;
}

// The PXP fetches one source line beyond the rect it was given, whatever the scaler settings
// (measured on the RT1062 with a source buffer ending at the top of SDRAM: a 1:1 copy faults
// with less than one line of slack after the buffer and passes with one). Reading past the
// end of a RAM region is an AXI bus error that aborts the operation, so the extra line must
// still lie inside the region holding the buffer. Two lines are required for margin.
#define OMV_PXP_OVERFETCH_LINES (2)

typedef struct {
    uintptr_t start;
    uintptr_t end;
} omv_pxp_region_t;

// RAM regions that can hold an image buffer, in address order (from the linker script).
#define OMV_PXP_REGION(name) { (uintptr_t) &__##name##_start, (uintptr_t) &__##name##_end }
#if defined(OMV_DTCM_ORIGIN)
extern char __dtcm_start, __dtcm_end;
#endif
#if defined(OMV_OCRM1_ORIGIN)
extern char __ocrm1_start, __ocrm1_end;
#endif
#if defined(OMV_OCRM2_ORIGIN)
extern char __ocrm2_start, __ocrm2_end;
#endif
#if defined(OMV_DRAM_ORIGIN)
extern char __sdram_start, __sdram_end;
#endif

// True if [start, end) lies inside mapped RAM (adjacent regions count as one).
static bool omv_pxp_readable(uintptr_t start, uintptr_t end) {
    static const omv_pxp_region_t regions[] = {
        #if defined(OMV_DTCM_ORIGIN)
        OMV_PXP_REGION(dtcm),
        #endif
        #if defined(OMV_OCRM1_ORIGIN)
        OMV_PXP_REGION(ocrm1),
        #endif
        #if defined(OMV_OCRM2_ORIGIN)
        OMV_PXP_REGION(ocrm2),
        #endif
        #if defined(OMV_DRAM_ORIGIN)
        OMV_PXP_REGION(sdram),
        #endif
    };
    size_t n = sizeof(regions) / sizeof(regions[0]);
    for (size_t i = 0; i < n; i++) {
        if ((start >= regions[i].start) && (start < regions[i].end)) {
            // Extend across regions that start exactly where this one ends.
            uintptr_t limit = regions[i].end;
            for (size_t j = i + 1; (j < n) && (regions[j].start == limit); j++) {
                limit = regions[j].end;
            }
            return end <= limit;
        }
    }
    return false;
}

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
    if ((src_img->pixfmt != PIXFORMAT_GRAYSCALE) && (src_img->pixfmt != PIXFORMAT_RGB565) &&
        (src_img->pixfmt != PIXFORMAT_YUV422) && (src_img->pixfmt != PIXFORMAT_YVU422)) {
        return -1;
    }

    // PXP output must be GRAYSCALE or RGB565.
    if ((dst_img->pixfmt != PIXFORMAT_GRAYSCALE) && (dst_img->pixfmt != PIXFORMAT_RGB565)) {
        return -1;
    }

    // A GRAYSCALE output can only come from a GRAYSCALE or YUV/YVU422 input.
    if ((dst_img->pixfmt == PIXFORMAT_GRAYSCALE) && (src_img->pixfmt != PIXFORMAT_GRAYSCALE) &&
        (src_img->pixfmt != PIXFORMAT_YUV422) && (src_img->pixfmt != PIXFORMAT_YVU422)) {
        return -1;
    }

    // PXP processes the output in 8x8 or 16x16 pixel blocks.
    if ((src_rect->w % 8) || (src_rect->h % 8) || (dst_rect->w % 8) || (dst_rect->h % 8)) {
        return -1;
    }

    // PXP has no LUT for alpha or color palettes and this path does not blend
    // (alpha is 0-255 with 255 fully opaque, as elsewhere in imlib).
    if ((alpha != 255) || color_palette || alpha_palette) {
        return -1;
    }

    // Affine transforms are not supported by the PXP process-surface path.
    if (transform) {
        return -1;
    }

    // PXP cannot reduce the image size by more than 16x.
    if ((dst_rect->w < (src_rect->w / 16)) || (dst_rect->h < (src_rect->h / 16))) {
        return -1;
    }

    // PXP cannot enlarge the image size by more than 4096x.
    if ((dst_rect->w > (src_rect->w * 4096)) || (dst_rect->h > (src_rect->h * 4096))) {
        return -1;
    }

    // PXP cannot hmirror or vflip when scaling.
    if ((hint & (IMAGE_HINT_HMIRROR | IMAGE_HINT_VFLIP)) &&
        ((src_rect->w != dst_rect->w) || (src_rect->h != dst_rect->h))) {
        return -1;
    }

    // PXP always applies bilinear scaling, which beats nearest-neighbor, so the
    // IMAGE_HINT_BILINEAR hint is ignored.

    PXP_Init(PXP);

    // Prefer 16x16 blocks; fall back to 8x8 when a dimension is not a multiple.
    if ((src_rect->w % 16) || (src_rect->h % 16) || (dst_rect->w % 16) || (dst_rect->h % 16)) {
        PXP_SetProcessBlockSize(PXP, kPXP_BlockSize8);
    } else {
        PXP_SetProcessBlockSize(PXP, kPXP_BlockSize16);
    }

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

    // Refuse a source whose over-fetched lines would fall off the end of its RAM region.
    uintptr_t src_line_end = input_buffer_config.bufferAddr + (src_rect->h + OMV_PXP_OVERFETCH_LINES) *
                             input_buffer_config.pitchBytes;
    if (!omv_pxp_readable(input_buffer_config.bufferAddr, src_line_end)) {
        PXP_Deinit(PXP);
        return -1;
    }

    PXP_SetProcessSurfaceBufferConfig(PXP, &input_buffer_config);
    uint8_t dec_x, dec_y;
    uint32_t scale_x = omv_pxp_scaler(src_rect->w, dst_rect->w, &dec_x);
    uint32_t scale_y = omv_pxp_scaler(src_rect->h, dst_rect->h, &dec_y);
    PXP->PS_CTRL = (PXP->PS_CTRL & ~(PXP_PS_CTRL_DECX_MASK | PXP_PS_CTRL_DECY_MASK)) |
                   PXP_PS_CTRL_DECX(dec_x) | PXP_PS_CTRL_DECY(dec_y);
    PXP->PS_SCALE = PXP_PS_SCALE_XSCALE(scale_x) | PXP_PS_SCALE_YSCALE(scale_y);
    PXP_SetProcessSurfacePosition(PXP, 0, 0, dst_rect->w - 1, dst_rect->h - 1);

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
    flip_mode |= (hint & IMAGE_HINT_HMIRROR) ? kPXP_FlipHorizontal : 0;
    flip_mode |= (hint & IMAGE_HINT_VFLIP) ? kPXP_FlipVertical : 0;
    PXP_SetRotateConfig(PXP, kPXP_RotateProcessSurface, kPXP_Rotate0, flip_mode);

    #if __DCACHE_PRESENT
    // Push the whole source image out of the cache so the PXP reads current
    // data, and clean+invalidate the whole destination so no dirty line is
    // written back over the PXP's output. Operating on the contiguous image
    // buffer (not per-row sub-regions) keeps this on cache-line boundaries.
    SCB_CleanDCache_by_Addr(src_img->data, image_size(src_img));
    SCB_CleanInvalidateDCache_by_Addr(dst_img->data, image_size(dst_img));
    #endif

    PXP_ClearStatusFlags(PXP, kPXP_CompleteFlag | kPXP_Axi0ReadErrorFlag | kPXP_Axi0WriteErrorFlag);
    #if OMV_PXP_DEBUG
    mp_uint_t t0 = mp_hal_ticks_us();
    #endif
    PXP_Start(PXP);

    // Wait for completion, but bail out on an AXI bus error (the pipeline
    // terminates without ever setting the complete flag) or on a timeout.
    uint32_t flags;
    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        flags = PXP_GetStatusFlags(PXP);
        if (flags & (kPXP_CompleteFlag | kPXP_Axi0ReadErrorFlag | kPXP_Axi0WriteErrorFlag)) {
            break;
        }
        if ((mp_hal_ticks_ms() - start) >= OMV_PXP_TIMEOUT_MS) {
            break;
        }
    }

    bool faulted = flags & (kPXP_Axi0ReadErrorFlag | kPXP_Axi0WriteErrorFlag);
    #if OMV_PXP_DEBUG
    if (faulted) {
        omv_pxp_report_fault(PXP->STAT, src_img, src_rect, dst_img, dst_rect);
    } else {
        // One line per accelerated op: buffer addresses (to spot a source that ends on a memory
        // region boundary), block size, and the hardware time.
        mp_printf(&mp_plat_print, "PXP %s: s=%p d=%p blk=%d us=%u\n",
                  (flags & kPXP_CompleteFlag) ? "ok" : "timeout",
                  (void *) input_buffer_config.bufferAddr, (void *) output_buffer_config.buffer0Addr,
                  (PXP->CTRL & PXP_CTRL_BLOCK_SIZE_MASK) ? 16 : 8,
                  (unsigned) (mp_hal_ticks_us() - t0));
    }
    #endif

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
