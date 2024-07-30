/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GPU driver.
 */
#ifndef __OMV_GPU_H__
#define __OMV_GPU_H__
#include "imlib.h"

int omv_gpu_init();
void omv_gpu_deinit();

// Draws src_rect from src_img to dst_rect in dst_img.
// If the sizes of src_rect and dst_rect are different then the image must be scaled.
// Alpha is the alpha value (0-256) to use when blending the source image with the destination image.
// If color_palette is not NULL, it is used to map the source image's 8-bit Y to an 16-bit RGB565 color.
// If alpha_palette is not NULL, it is used to map the source image's 8-bit Y to an 8-bit alpha value.
//
// Valid Hint Flags:
//
// * IMAGE_HINT_BILINEAR: Use 2D bilinear interpolation when scaling the image versus nearest-neighbor.
// * IMAGE_HINT_HMIRROR: The destination image result should be horizontally mirrored from the source image.
// * IMAGE_HINT_VFLIP: The destination image result should be vertically flipped from the source image.
// * IMAGE_HINT_BLACK_BACKGROUND: The destination image should be considered to be black (0) for alpha blending.

int omv_gpu_draw_image(image_t *src_img,
                       rectangle_t *src_rect,
                       image_t *dst_img,
                       rectangle_t *dst_rect,
                       int alpha,
                       const uint16_t *color_palette,
                       const uint8_t *alpha_palette,
                       image_hint_t hint);
#endif // __OMV_GPU_H__
