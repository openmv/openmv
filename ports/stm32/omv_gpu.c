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
 * GPU driver for STM32 port.
 */
#include "omv_boardconfig.h"
#if (OMV_GPU_ENABLE == 1)
#include STM32_HAL_H

#include "py/mphal.h"
#include "py/runtime.h"

#include "imlib.h"
#include "dma.h"

#if OMV_GPU_NEMA
#include "nema_core.h"
#include "nema_error.h"
#endif

#if OMV_GPU_NEMA_MM_STATIC
uint8_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(NEMA_BUFFER[OMV_GPU_NEMA_BUFFER_SIZE], 32), ".dma_buffer");
#endif

int omv_gpu_init() {
    int error = 0;
    #if OMV_GPU_NEMA
    nema_init();
    error = nema_get_error();
    #endif
    return error;
}

void omv_gpu_deinit() {
}

uint32_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(CLUT_BUFFER[256], 32), ".dma_buffer");

#if OMV_GPU_NEMA
// Not documented anywhere but this is how NEMA expects the CLUT index to be calculated.
#define NEMA_CLUT_INDEX(i) (((i << 4) | (i >> 4)) & 0xFF)

static void omv_gpu_load_clut(const uint16_t *color_palette, const uint8_t *alpha_palette) {
    for (int i = 0; i < 256; i++) {
        int r, g, b;
        if (color_palette) {
            int pixel = color_palette[i];
            r = COLOR_RGB565_TO_R8(pixel);
            g = COLOR_RGB565_TO_G8(pixel);
            b = COLOR_RGB565_TO_B8(pixel);
        } else {
            r = g = b = i;
        }
        int a = (alpha_palette) ? alpha_palette[i] : 255;
        CLUT_BUFFER[NEMA_CLUT_INDEX(i)] = nema_rgba(r, g, b, a);
    }
}

static nema_tex_format_t omv_gpu_pixfmt(uint32_t omv_pixfmt) {
    switch (omv_pixfmt) {
        case PIXFORMAT_GRAYSCALE:
            return NEMA_L8;
        case PIXFORMAT_RGB565:
            return NEMA_RGB565;
    }
    __builtin_unreachable();
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
    // GPU2D can only draw on RGB565/GRAYSCALE buffers.
    if ((dst_img->pixfmt != PIXFORMAT_RGB565) && (dst_img->pixfmt != PIXFORMAT_GRAYSCALE)) {
        return -1;
    }

    // GPU2D can only read from RGB565 or GRAYSCALE buffers.
    // If the source image is RGB565, it must not have a color or alpha palette.
    if ((src_img->pixfmt != PIXFORMAT_GRAYSCALE) &&
        ((src_img->pixfmt != PIXFORMAT_RGB565) || color_palette || alpha_palette)) {
        return -1;
    }

    // If the source format is grayscale with a color/alpha palette bilinear scaling is not supported.
    if ((src_img->pixfmt == PIXFORMAT_GRAYSCALE) && (hint & IMAGE_HINT_BILINEAR) &&
        (color_palette || alpha_palette)) {
        return -1;
    }

    // GPU2D cannot handle generic hmirror or vflip.
    if (hint & (IMAGE_HINT_HMIRROR | IMAGE_HINT_VFLIP)) {
        return -1;
    }

    float dx0 = dst_rect->x;
    float dy0 = dst_rect->y;
    float dx1 = dst_rect->x + dst_rect->w;
    float dy1 = dst_rect->y;
    float dx2 = dst_rect->x + dst_rect->w;
    float dy2 = dst_rect->y + dst_rect->h;
    float dx3 = dst_rect->x;
    float dy3 = dst_rect->y + dst_rect->h;

    if (transform) {
        nema_mat3x3_mul_vec(*((nema_matrix3x3_t *) transform), &dx0, &dy0);
        nema_mat3x3_mul_vec(*((nema_matrix3x3_t *) transform), &dx1, &dy1);
        nema_mat3x3_mul_vec(*((nema_matrix3x3_t *) transform), &dx2, &dy2);
        nema_mat3x3_mul_vec(*((nema_matrix3x3_t *) transform), &dx3, &dy3);
    }

    // Create command list.
    #if OMV_GPU_NEMA_MM_STATIC
    nema_buffer_t bo = {
        .size = sizeof(NEMA_BUFFER),
        .base_virt = NEMA_BUFFER,
        .base_phys = (uint32_t) NEMA_BUFFER,
    };
    nema_cmdlist_t cl = nema_cl_create_prealloc(&bo);
    #else
    nema_cmdlist_t cl = nema_cl_create_sized(OMV_GPU_NEMA_BUFFER_SIZE);
    #endif

    // Bind command list.
    nema_cl_bind_circular(&cl);

    // Set up destination texture.
    nema_tex_format_t dst_pixfmt = omv_gpu_pixfmt(dst_img->pixfmt);
    nema_bind_dst_tex((uintptr_t) dst_img->data, dst_img->w, dst_img->h, dst_pixfmt, -1);

    // Set up source texture.
    nema_tex_mode_t blit_mode = (hint & IMAGE_HINT_BILINEAR) ? NEMA_FILTER_BL : NEMA_FILTER_PS;
    uint32_t blops = NEMA_BLOP_MODULATE_A;

    if (alpha_palette || color_palette) {
        omv_gpu_load_clut(color_palette, alpha_palette);
        blops |= NEMA_BLOP_LUT;
        nema_bind_lut_tex((uintptr_t) src_img->data, src_img->w, src_img->h, NEMA_L8, -1,
                          NEMA_FILTER_PS, (uintptr_t) CLUT_BUFFER, NEMA_RGBA8888);
    } else {
        nema_tex_format_t src_pixfmt = omv_gpu_pixfmt(src_img->pixfmt);
        nema_bind_src_tex((uintptr_t) src_img->data, src_img->w, src_img->h, src_pixfmt, -1, blit_mode);
    }

    // Configure operations.
    uint32_t dst_bf = (hint & IMAGE_HINT_BLACK_BACKGROUND) ? NEMA_BF_ZERO : NEMA_BF_INVSRCALPHA;
    nema_set_blend_blit(nema_blending_mode(NEMA_BF_SRCALPHA, dst_bf, blops));
    nema_set_const_color(nema_rgba(0, 0, 0, alpha));
    nema_set_clip(0, 0, dst_img->w, dst_img->h);
    nema_enable_aa(true, true, true, true);
    nema_blit_subrect_quad_fit(dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3,
                               src_rect->x, src_rect->y, src_rect->w, src_rect->h);

    SCB_CleanInvalidateDCache_by_Addr(dst_img->data, image_size(dst_img));
    SCB_CleanDCache_by_Addr(src_img->data, image_size(src_img));

    // Ensure the GPU cache is clean before starting the GPU operation.
    // Will start invalidating the GPU cache if not already invalidated.
    HAL_ICACHE_WaitForInvalidateComplete();

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    #if !OMV_GPU_NEMA_MM_STATIC
    nema_cl_destroy(&cl);
    #endif

    // Start invalidation of the GPU cache for the next operation.
    // This is done asynchronously so that the CPU can continue working.
    HAL_ICACHE_Invalidate_IT();

    SCB_InvalidateDCache_by_Addr(dst_img->data, image_size(dst_img));
    return 0;
}
#else
int omv_gpu_draw_image(image_t *src_img,
                       rectangle_t *src_rect,
                       image_t *dst_img,
                       rectangle_t *dst_rect,
                       int alpha,
                       const uint16_t *color_palette,
                       const uint8_t *alpha_palette,
                       image_hint_t hint,
                       float *transform) {
    // DMA2D can only draw on RGB565 buffers and the destination/source buffers must be accessible by DMA.
    if ((dst_img->pixfmt != PIXFORMAT_RGB565) || (!DMA_BUFFER(dst_img->data)) || (!DMA_BUFFER(src_img->data))) {
        return -1;
    }

    // DMA2D can only read from RGB565 or GRAYSCALE buffers.
    // If the source image is RGB565, it must not have a color or alpha palette.
    if ((src_img->pixfmt != PIXFORMAT_GRAYSCALE) &&
        ((src_img->pixfmt != PIXFORMAT_RGB565) || color_palette || alpha_palette)) {
        return -1;
    }

    // DMA2D cannot scale so ensure that the source and destination rectangles are the same size.
    // The bilinear/nearest-neighbor flag doesn't matter since we can't scale.
    if ((dst_rect->w != src_rect->w) || (dst_rect->h != src_rect->h)) {
        return -1;
    }

    // DMA2D cannot hmirror or vflip.
    if (hint & (IMAGE_HINT_HMIRROR | IMAGE_HINT_VFLIP)) {
        return -1;
    }

    // DMA2D must always fetch the background on the F4 and F7 series so do this in software.
    #if defined(STM32F4) || defined(STM32F7)
    if (hint & IMAGE_HINT_BLACK_BACKGROUND) {
        return -1;
    }
    #endif

    // DMA2D cannot do matrix transformations.
    if (transform) {
        return -1;
    }

    DMA2D_HandleTypeDef dma2d = {
        .Instance = DMA2D,
        .Init.ColorMode = DMA2D_OUTPUT_RGB565,
        .Init.OutputOffset = dst_img->w - dst_rect->w,
    };

    if (dst_img->pixfmt != src_img->pixfmt) {
        dma2d.Init.Mode = DMA2D_M2M_PFC;
    }

    if ((alpha != 255) || alpha_palette) {
        dma2d.Init.Mode = DMA2D_M2M_BLEND;
    }

    #if defined(STM32H7)
    if (hint & IMAGE_HINT_BLACK_BACKGROUND) {
        dma2d.Init.Mode = DMA2D_M2M_BLEND_BG;
    }
    #endif

    HAL_DMA2D_Init(&dma2d);

    dma2d.LayerCfg[0].InputOffset = dst_img->w - dst_rect->w;
    dma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
    dma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
    dma2d.LayerCfg[0].InputAlpha = 0xff;

    HAL_DMA2D_ConfigLayer(&dma2d, 0);

    if (src_img->pixfmt == PIXFORMAT_GRAYSCALE) {
        dma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8;
        dma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;

        if (!alpha_palette) {
            if (!color_palette) {
                for (int i = 0; i < 256; i++) {
                    CLUT_BUFFER[i] = (0xff << 24) | COLOR_Y_TO_RGB888(i);
                }
            } else {
                for (int i = 0; i < 256; i++) {
                    int pixel = color_palette[i];
                    CLUT_BUFFER[i] = (0xff << 24) |
                                     (COLOR_RGB565_TO_R8(pixel) << 16) |
                                     (COLOR_RGB565_TO_G8(pixel) << 8) |
                                     COLOR_RGB565_TO_B8(pixel);
                }
            }
        } else {
            if (!color_palette) {
                for (int i = 0; i < 256; i++) {
                    CLUT_BUFFER[i] = (alpha_palette[i] << 24) | COLOR_Y_TO_RGB888(i);
                }
            } else {
                for (int i = 0; i < 256; i++) {
                    int pixel = color_palette[i];
                    CLUT_BUFFER[i] = (alpha_palette[i] << 24) |
                                     (COLOR_RGB565_TO_R8(pixel) << 16) |
                                     (COLOR_RGB565_TO_G8(pixel) << 8) |
                                     COLOR_RGB565_TO_B8(pixel);
                }
            }
        }

        DMA2D_CLUTCfgTypeDef cfg;
        cfg.pCLUT = CLUT_BUFFER;
        cfg.CLUTColorMode = DMA2D_CCM_ARGB8888;
        cfg.Size = 255;
        HAL_DMA2D_CLUTLoad(&dma2d, cfg, 1);
        HAL_DMA2D_PollForTransfer(&dma2d, 1000);
    } else {
        dma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
        dma2d.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    }

    dma2d.LayerCfg[1].InputOffset = src_img->w - src_rect->w;
    dma2d.LayerCfg[1].InputAlpha = alpha;

    HAL_DMA2D_ConfigLayer(&dma2d, 1);

    uint16_t *dst16 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(dst_img, dst_rect->y) + dst_rect->x;

    #if __DCACHE_PRESENT
    // Ensures any cached writes to dst16 are flushed.
    if (dst_img->w == dst_rect->w) {
        SCB_CleanInvalidateDCache_by_Addr(dst16, dst_rect->w * dst_rect->h * sizeof(uint16_t));
    } else {
        uint16_t *dst16_tmp = dst16;
        for (int i = 0; i < dst_rect->h; i++) {
            SCB_CleanInvalidateDCache_by_Addr(dst16_tmp, dst_rect->w * sizeof(uint16_t));
            dst16_tmp += dst_img->w;
        }
    }
    #endif

    uint32_t src;

    if (src_img->pixfmt == PIXFORMAT_GRAYSCALE) {
        uint8_t *src8 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(src_img, src_rect->y) + src_rect->x;
        src = (uint32_t) src8;

        #if __DCACHE_PRESENT
        if (src_img->w == src_rect->w) {
            SCB_CleanDCache_by_Addr(src8, src_rect->w * src_rect->h);
        } else {
            uint8_t *src8_tmp = src8;
            for (int i = 0; i < src_rect->h; i++) {
                SCB_CleanDCache_by_Addr(src8_tmp, src_rect->w);
                src8_tmp += src_img->w;
            }
        }
        #endif
    } else {
        uint16_t *src16 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src_img, src_rect->y) + src_rect->x;
        src = (uint32_t) src16;

        #if __DCACHE_PRESENT
        if (src_img->w == src_rect->w) {
            SCB_CleanDCache_by_Addr(src16, src_rect->w * src_rect->h * sizeof(uint16_t));
        } else {
            uint16_t *src16_tmp = src16;
            for (int i = 0; i < src_rect->h; i++) {
                SCB_CleanDCache_by_Addr(src16_tmp, src_rect->w * sizeof(uint16_t));
                src16_tmp += src_img->w;
            }
        }
        #endif
    }

    uint32_t dst = (uint32_t) dst16;

    #if defined(STM32H7)
    if (hint & IMAGE_HINT_BLACK_BACKGROUND) {
        dst = 0;
    }
    #endif

    HAL_DMA2D_BlendingStart(&dma2d, src, dst, (uint32_t) dst16, dst_rect->w, dst_rect->h);
    HAL_DMA2D_PollForTransfer(&dma2d, 1000);

    #if __DCACHE_PRESENT
    // Ensures any cached reads to dst16 are dropped.
    if (dst_img->w == dst_rect->w) {
        SCB_InvalidateDCache_by_Addr(dst16, dst_rect->w * dst_rect->h * sizeof(uint16_t));
    } else {
        uint16_t *dst16_tmp = dst16;
        for (int i = 0; i < dst_rect->h; i++) {
            SCB_InvalidateDCache_by_Addr(dst16_tmp, dst_rect->w * sizeof(uint16_t));
            dst16_tmp += dst_img->w;
        }
    }
    #endif

    HAL_DMA2D_DeInit(&dma2d);
    return 0;
}
#endif // OMV_GPU_NEMA
#endif // OMV_GPU_ENABLE
