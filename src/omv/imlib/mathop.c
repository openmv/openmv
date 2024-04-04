/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image math operations.
 */
#include "imlib.h"

#ifdef IMLIB_ENABLE_MATH_OPS
void imlib_add_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = (image_t *) data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_or_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row0 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint8_t *row1 = (uint8_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    *((uint32_t *) (row0 + x)) = __UQADD8(p0, p1);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                    uint32_t p = __USAT(p0 + p1, 8);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                        uint32_t p = __USAT(p0 + p1, 8);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint16_t *row1 = (uint16_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t r = __USAT16(((p0 >> 11) & 0x1f001f) + ((p1 >> 11) & 0x1f001f), 5);
                    uint32_t g = __USAT16(((p0 >> 5) & 0x3f003f) + ((p1 >> 5) & 0x3f003f), 6);
                    uint32_t b = __USAT16((p0 & 0x1f001f) + (p1 & 0x1f001f), 5);
                    *((uint32_t *) (row0 + x)) = COLOR_R5_G6_B5_TO_RGB565(r, g, b);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                    uint32_t r = __USAT(COLOR_RGB565_TO_R5(p0) + COLOR_RGB565_TO_R5(p1), 5);
                    uint32_t g = __USAT(COLOR_RGB565_TO_G6(p0) + COLOR_RGB565_TO_G6(p1), 6);
                    uint32_t b = __USAT(COLOR_RGB565_TO_B5(p0) + COLOR_RGB565_TO_B5(p1), 5);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                        uint32_t r = __USAT(COLOR_RGB565_TO_R5(p0) + COLOR_RGB565_TO_R5(p1), 5);
                        uint32_t g = __USAT(COLOR_RGB565_TO_G6(p0) + COLOR_RGB565_TO_G6(p1), 6);
                        uint32_t b = __USAT(COLOR_RGB565_TO_B5(p0) + COLOR_RGB565_TO_B5(p1), 5);
                        IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_sub_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = (image_t *) data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row0 = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint32_t *row1 = (uint32_t *) data->dst_row_override;

            if (!mask) {
                #if (__ARM_ARCH > 6)
                // Align to 32-bit boundary.
                for (; (x % 32) && (x < x_end); x++) {
                    uint32_t p0 = IMAGE_GET_BINARY_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_BINARY_PIXEL_FAST(row1, x);
                    uint32_t p = p0 > p1;
                    IMAGE_PUT_BINARY_PIXEL_FAST(row0, x, p);
                }

                for (; (x_end - x) >= 32; x += 32) {
                    uint32_t p0 = row0[x / 32];
                    uint32_t p1 = row1[x / 32];
                    row0[x / 32] = p0 & (~p1); // the same as p0 > p1 for all bits
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_BINARY_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_BINARY_PIXEL_FAST(row1, x);
                    uint32_t p = p0 > p1;
                    IMAGE_PUT_BINARY_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_BINARY_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_BINARY_PIXEL_FAST(row1, x);
                        uint32_t p = p0 > p1;
                        IMAGE_PUT_BINARY_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row0 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint8_t *row1 = (uint8_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    *((uint32_t *) (row0 + x)) = __UQSUB8(p0, p1);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                    uint32_t p = __USAT((int32_t) (p0 - p1), 8);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                        uint32_t p = __USAT((int32_t) (p0 - p1), 8);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint16_t *row1 = (uint16_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t r = __USAT16(__SSUB16((p0 >> 11) & 0x1f001f, (p1 >> 11) & 0x1f001f), 5);
                    uint32_t g = __USAT16(__SSUB16((p0 >> 5) & 0x3f003f, (p1 >> 5) & 0x3f003f), 6);
                    uint32_t b = __USAT16(__SSUB16(p0 & 0x1f001f, p1 & 0x1f001f), 5);
                    *((uint32_t *) (row0 + x)) = COLOR_R5_G6_B5_TO_RGB565(r, g, b);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                    uint32_t r = __USAT((int32_t) (COLOR_RGB565_TO_R5(p0) - COLOR_RGB565_TO_R5(p1)), 5);
                    uint32_t g = __USAT((int32_t) (COLOR_RGB565_TO_G6(p0) - COLOR_RGB565_TO_G6(p1)), 6);
                    uint32_t b = __USAT((int32_t) (COLOR_RGB565_TO_B5(p0) - COLOR_RGB565_TO_B5(p1)), 5);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                        uint32_t r = __USAT((int32_t) (COLOR_RGB565_TO_R5(p0) - COLOR_RGB565_TO_R5(p1)), 5);
                        uint32_t g = __USAT((int32_t) (COLOR_RGB565_TO_G6(p0) - COLOR_RGB565_TO_G6(p1)), 6);
                        uint32_t b = __USAT((int32_t) (COLOR_RGB565_TO_B5(p0) - COLOR_RGB565_TO_B5(p1)), 5);
                        IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_rsub_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = (image_t *) data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row0 = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint32_t *row1 = (uint32_t *) data->dst_row_override;

            if (!mask) {
                #if (__ARM_ARCH > 6)
                // Align to 32-bit boundary.
                for (; (x % 32) && (x < x_end); x++) {
                    uint32_t p0 = IMAGE_GET_BINARY_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_BINARY_PIXEL_FAST(row1, x);
                    uint32_t p = p1 > p0;
                    IMAGE_PUT_BINARY_PIXEL_FAST(row0, x, p);
                }

                for (; (x_end - x) >= 32; x += 32) {
                    uint32_t p0 = row0[x / 32];
                    uint32_t p1 = row1[x / 32];
                    row0[x / 32] = p1 & (~p0); // the same as p1 > p0 for all bits
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_BINARY_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_BINARY_PIXEL_FAST(row1, x);
                    uint32_t p = p1 > p0;
                    IMAGE_PUT_BINARY_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_BINARY_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_BINARY_PIXEL_FAST(row1, x);
                        uint32_t p = p1 > p0;
                        IMAGE_PUT_BINARY_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row0 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint8_t *row1 = (uint8_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    *((uint32_t *) (row0 + x)) = __UQSUB8(p1, p0);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                    uint32_t p = __USAT((int32_t) (p1 - p0), 8);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                        uint32_t p = __USAT((int32_t) (p1 - p0), 8);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint16_t *row1 = (uint16_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t r = __USAT16(__SSUB16((p1 >> 11) & 0x1f001f, (p0 >> 11) & 0x1f001f), 5);
                    uint32_t g = __USAT16(__SSUB16((p1 >> 5) & 0x3f003f, (p0 >> 5) & 0x3f003f), 6);
                    uint32_t b = __USAT16(__SSUB16(p1 & 0x1f001f, p0 & 0x1f001f), 5);
                    *((uint32_t *) (row0 + x)) = COLOR_R5_G6_B5_TO_RGB565(r, g, b);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                    uint32_t r = __USAT((int32_t) (COLOR_RGB565_TO_R5(p1) - COLOR_RGB565_TO_R5(p0)), 5);
                    uint32_t g = __USAT((int32_t) (COLOR_RGB565_TO_G6(p1) - COLOR_RGB565_TO_G6(p0)), 6);
                    uint32_t b = __USAT((int32_t) (COLOR_RGB565_TO_B5(p1) - COLOR_RGB565_TO_B5(p0)), 5);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                        uint32_t r = __USAT((int32_t) (COLOR_RGB565_TO_R5(p1) - COLOR_RGB565_TO_R5(p0)), 5);
                        uint32_t g = __USAT((int32_t) (COLOR_RGB565_TO_G6(p1) - COLOR_RGB565_TO_G6(p0)), 6);
                        uint32_t b = __USAT((int32_t) (COLOR_RGB565_TO_B5(p1) - COLOR_RGB565_TO_B5(p0)), 5);
                        IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_min_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = (image_t *) data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_and_line_op(x, x_end, y_row, data);
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row0 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint8_t *row1 = (uint8_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    __USUB8(p0, p1);
                    *((uint32_t *) (row0 + x)) = __SEL(p1, p0);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                    uint32_t p = IM_MIN(p0, p1);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                        uint32_t p = IM_MIN(p0, p1);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint16_t *row1 = (uint16_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t p0_rb = p0 & 0xf81ff81f, p1_rb = p1 & 0xf81ff81f;
                    __USUB8(p0_rb, p1_rb);
                    uint32_t rb = __SEL(p1_rb, p0_rb);
                    uint32_t p0_g = p0 & 0x07e007e0, p1_g = p1 & 0x07e007e0;
                    __USUB16(p0_g, p1_g);
                    uint32_t g = __SEL(p1_g, p0_g);
                    *((uint32_t *) (row0 + x)) = rb | g;
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                    uint32_t r = IM_MIN(COLOR_RGB565_TO_R5(p0), COLOR_RGB565_TO_R5(p1));
                    uint32_t g = IM_MIN(COLOR_RGB565_TO_G6(p0), COLOR_RGB565_TO_G6(p1));
                    uint32_t b = IM_MIN(COLOR_RGB565_TO_B5(p0), COLOR_RGB565_TO_B5(p1));
                    IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                        uint32_t r = IM_MIN(COLOR_RGB565_TO_R5(p0), COLOR_RGB565_TO_R5(p1));
                        uint32_t g = IM_MIN(COLOR_RGB565_TO_G6(p0), COLOR_RGB565_TO_G6(p1));
                        uint32_t b = IM_MIN(COLOR_RGB565_TO_B5(p0), COLOR_RGB565_TO_B5(p1));
                        IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_max_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = (image_t *) data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_or_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row0 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint8_t *row1 = (uint8_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    __USUB8(p0, p1);
                    *((uint32_t *) (row0 + x)) = __SEL(p0, p1);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                    uint32_t p = IM_MAX(p0, p1);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                        uint32_t p = IM_MAX(p0, p1);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint16_t *row1 = (uint16_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t p0_rb = p0 & 0xf81ff81f, p1_rb = p1 & 0xf81ff81f;
                    __USUB8(p0_rb, p1_rb);
                    uint32_t rb = __SEL(p0_rb, p1_rb);
                    uint32_t p0_g = p0 & 0x07e007e0, p1_g = p1 & 0x07e007e0;
                    __USUB16(p0_g, p1_g);
                    uint32_t g = __SEL(p0_g, p1_g);
                    *((uint32_t *) (row0 + x)) = rb | g;
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                    uint32_t r = IM_MAX(COLOR_RGB565_TO_R5(p0), COLOR_RGB565_TO_R5(p1));
                    uint32_t g = IM_MAX(COLOR_RGB565_TO_G6(p0), COLOR_RGB565_TO_G6(p1));
                    uint32_t b = IM_MAX(COLOR_RGB565_TO_B5(p0), COLOR_RGB565_TO_B5(p1));
                    IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                        uint32_t r = IM_MAX(COLOR_RGB565_TO_R5(p0), COLOR_RGB565_TO_R5(p1));
                        uint32_t g = IM_MAX(COLOR_RGB565_TO_G6(p0), COLOR_RGB565_TO_G6(p1));
                        uint32_t b = IM_MAX(COLOR_RGB565_TO_B5(p0), COLOR_RGB565_TO_B5(p1));
                        IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_difference_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = (image_t *) data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_xor_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row0 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint8_t *row1 = (uint8_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t sub0 = __USUB8(p0, p1);
                    uint32_t sub1 = __USUB8(p1, p0);
                    *((uint32_t *) (row0 + x)) = __SEL(sub1, sub0);
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                    uint32_t p = __USAD8(p0, p1);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row1, x);
                        uint32_t p = __USAD8(p0, p1);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row0, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            uint16_t *row1 = (uint16_t *) data->dst_row_override;

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    uint32_t p0 = *((uint32_t *) (row0 + x));
                    uint32_t p1 = *((uint32_t *) (row1 + x));
                    uint32_t p0_rb = p0 & 0xf81ff81f, p1_rb = p1 & 0xf81ff81f;
                    uint32_t sub0 = __USUB8(p0_rb, p1_rb);
                    uint32_t sub1 = __USUB8(p1_rb, p0_rb);
                    uint32_t rb = __SEL(sub1, sub0);
                    uint32_t p0_g = p0 & 0x07e007e0, p1_g = p1 & 0x07e007e0;
                    sub0 = __USUB16(p0_g, p1_g);
                    sub1 = __USUB16(p1_g, p0_g);
                    uint32_t g = __SEL(sub1, sub0);
                    *((uint32_t *) (row0 + x)) = rb | g;
                }
                #endif

                for (; x < x_end; x++) {
                    uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                    uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                    uint32_t r = __USAD8(COLOR_RGB565_TO_R5(p0), COLOR_RGB565_TO_R5(p1));
                    uint32_t g = __USAD8(COLOR_RGB565_TO_G6(p0), COLOR_RGB565_TO_G6(p1));
                    uint32_t b = __USAD8(COLOR_RGB565_TO_B5(p0), COLOR_RGB565_TO_B5(p1));
                    IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        uint32_t p0 = IMAGE_GET_RGB565_PIXEL_FAST(row0, x);
                        uint32_t p1 = IMAGE_GET_RGB565_PIXEL_FAST(row1, x);
                        uint32_t r = __USAD8(COLOR_RGB565_TO_R5(p0), COLOR_RGB565_TO_R5(p1));
                        uint32_t g = __USAD8(COLOR_RGB565_TO_G6(p0), COLOR_RGB565_TO_G6(p1));
                        uint32_t b = __USAD8(COLOR_RGB565_TO_B5(p0), COLOR_RGB565_TO_B5(p1));
                        IMAGE_PUT_RGB565_PIXEL_FAST(row0, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}
#endif // IMLIB_ENABLE_MATH_OPS
