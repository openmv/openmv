/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image math operations.
 */
#include "imlib.h"

void imlib_zero_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    if (IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x)) {
                        IMAGE_CLEAR_BINARY_PIXEL_FAST(row, x);
                    }
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row) &&
                        IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x)) {
                        IMAGE_CLEAR_BINARY_PIXEL_FAST(row, x);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    if (IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x)) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, 0);
                    }
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row) &&
                        IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x)) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, 0);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    if (IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x)) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, 0);
                    }
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row) &&
                        IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x)) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, 0);
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

void imlib_mask_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(data->dst_img, y_row);
            for (; x < x_end; x++) {
                if (image_get_mask_pixel(mask, x, y_row)) {
                    int otherPixel = IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x);
                    IMAGE_PUT_BINARY_PIXEL_FAST(row, x, otherPixel);
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);
            for (; x < x_end; x++) {
                if (image_get_mask_pixel(mask, x, y_row)) {
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, otherPixel);
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);
            for (; x < x_end; x++) {
                if (image_get_mask_pixel(mask, x, y_row)) {
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, otherPixel);
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

#if defined(IMLIB_ENABLE_MATH_OPS) && defined(IMLIB_ENABLE_BINARY_OPS)
void imlib_add_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_or_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint8_t *) data->dst_row_override) + x));
                    *((uint32_t *) (row + x)) = __UQADD8(dataPixels, otherPixels);
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    int p = dataPixel + otherPixel;
                    #ifdef ARM_MATH_DSP
                    p = __USAT(p, 8);
                    #else
                    p = IM_MIN(p, COLOR_GRAYSCALE_MAX);
                    #endif
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        int p = dataPixel + otherPixel;
                        #ifdef ARM_MATH_DSP
                        p = __USAT(p, 8);
                        #else
                        p = IM_MIN(p, COLOR_GRAYSCALE_MAX);
                        #endif
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint16_t *) data->dst_row_override) + x));
                    int r_pixels = __USAT16(((dataPixels >> 11) & 0x1f001f) + ((otherPixels >> 11) & 0x1f001f), 5);
                    int g_pixels = __USAT16(((dataPixels >> 5) & 0x3f003f) + ((otherPixels >> 5) & 0x3f003f), 6);
                    int b_pixels = __USAT16((dataPixels & 0x1f001f) + (otherPixels & 0x1f001f), 5);
                    *((uint32_t *) (row + x)) = (r_pixels << 11) | (g_pixels << 5) | b_pixels;
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int r = COLOR_RGB565_TO_R5(dataPixel) + COLOR_RGB565_TO_R5(otherPixel);
                    int g = COLOR_RGB565_TO_G6(dataPixel) + COLOR_RGB565_TO_G6(otherPixel);
                    int b = COLOR_RGB565_TO_B5(dataPixel) + COLOR_RGB565_TO_B5(otherPixel);
                    #ifdef ARM_MATH_DSP
                    r = __USAT(r, 5);
                    g = __USAT(g, 6);
                    b = __USAT(b, 5);
                    #else
                    r = IM_MIN(r, COLOR_R5_MAX);
                    g = IM_MIN(g, COLOR_G6_MAX);
                    b = IM_MIN(b, COLOR_B5_MAX);
                    #endif
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int r = COLOR_RGB565_TO_R5(dataPixel) + COLOR_RGB565_TO_R5(otherPixel);
                        int g = COLOR_RGB565_TO_G6(dataPixel) + COLOR_RGB565_TO_G6(otherPixel);
                        int b = COLOR_RGB565_TO_B5(dataPixel) + COLOR_RGB565_TO_B5(otherPixel);
                        #ifdef ARM_MATH_DSP
                        r = __USAT(r, 5);
                        g = __USAT(g, 6);
                        b = __USAT(b, 5);
                        #else
                        r = IM_MIN(r, COLOR_R5_MAX);
                        g = IM_MIN(g, COLOR_G6_MAX);
                        b = IM_MIN(b, COLOR_B5_MAX);
                        #endif
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_BINARY_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x);
                    int p = dataPixel > otherPixel;
                    IMAGE_PUT_BINARY_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_BINARY_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x);
                        int p = dataPixel > otherPixel;
                        IMAGE_PUT_BINARY_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint8_t *) data->dst_row_override) + x));
                    *((uint32_t *) (row + x)) = __UQSUB8(dataPixels, otherPixels);
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    int p = dataPixel - otherPixel;
                    #ifdef ARM_MATH_DSP
                    p = __USAT(p, 8);
                    #else
                    p = IM_MAX(p, 0);
                    #endif
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        int p = dataPixel - otherPixel;
                        #ifdef ARM_MATH_DSP
                        p = __USAT(p, 8);
                        #else
                        p = IM_MAX(p, 0);
                        #endif
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint16_t *) data->dst_row_override) + x));
                    int r_pixels = __USAT16(__SSUB16((dataPixels >> 11) & 0x1f001f, (otherPixels >> 11) & 0x1f001f), 5);
                    int g_pixels = __USAT16(__SSUB16((dataPixels >> 5) & 0x3f003f, (otherPixels >> 5) & 0x3f003f), 6);
                    int b_pixels = __USAT16(__SSUB16(dataPixels & 0x1f001f, otherPixels & 0x1f001f), 5);
                    *((uint32_t *) (row + x)) = (r_pixels << 11) | (g_pixels << 5) | b_pixels;
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int dR = COLOR_RGB565_TO_R5(dataPixel);
                    int dG = COLOR_RGB565_TO_G6(dataPixel);
                    int dB = COLOR_RGB565_TO_B5(dataPixel);
                    int oR = COLOR_RGB565_TO_R5(otherPixel);
                    int oG = COLOR_RGB565_TO_G6(otherPixel);
                    int oB = COLOR_RGB565_TO_B5(otherPixel);
                    int r = dR - oR;
                    int g = dG - oG;
                    int b = dB - oB;
                    #ifdef ARM_MATH_DSP
                    r = __USAT(r, 5);
                    g = __USAT(g, 6);
                    b = __USAT(b, 5);
                    #else
                    r = IM_MAX(r, 0);
                    g = IM_MAX(g, 0);
                    b = IM_MAX(b, 0);
                    #endif
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int dR = COLOR_RGB565_TO_R5(dataPixel);
                        int dG = COLOR_RGB565_TO_G6(dataPixel);
                        int dB = COLOR_RGB565_TO_B5(dataPixel);
                        int oR = COLOR_RGB565_TO_R5(otherPixel);
                        int oG = COLOR_RGB565_TO_G6(otherPixel);
                        int oB = COLOR_RGB565_TO_B5(otherPixel);
                        int r = dR - oR;
                        int g = dG - oG;
                        int b = dB - oB;
                        #ifdef ARM_MATH_DSP
                        r = __USAT(r, 5);
                        g = __USAT(g, 6);
                        b = __USAT(b, 5);
                        #else
                        r = IM_MAX(r, 0);
                        g = IM_MAX(g, 0);
                        b = IM_MAX(b, 0);
                        #endif
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_BINARY_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x);
                    int p = otherPixel > dataPixel;
                    IMAGE_PUT_BINARY_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_BINARY_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_BINARY_PIXEL_FAST(((uint32_t *) data->dst_row_override), x);
                        int p = otherPixel > dataPixel;
                        IMAGE_PUT_BINARY_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint8_t *) data->dst_row_override) + x));
                    *((uint32_t *) (row + x)) = __UQSUB8(otherPixels, dataPixels);
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    int p = otherPixel - dataPixel;
                    #ifdef ARM_MATH_DSP
                    p = __USAT(p, 8);
                    #else
                    p = IM_MAX(p, 0);
                    #endif
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        int p = otherPixel - dataPixel;
                        #ifdef ARM_MATH_DSP
                        p = __USAT(p, 8);
                        #else
                        p = IM_MAX(p, 0);
                        #endif
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint16_t *) data->dst_row_override) + x));
                    int r_pixels = __USAT16(__SSUB16((otherPixels >> 11) & 0x1f001f, (dataPixels >> 11) & 0x1f001f), 5);
                    int g_pixels = __USAT16(__SSUB16((otherPixels >> 5) & 0x3f003f, (dataPixels >> 5) & 0x3f003f), 6);
                    int b_pixels = __USAT16(__SSUB16(otherPixels & 0x1f001f, dataPixels & 0x1f001f), 5);
                    *((uint32_t *) (row + x)) = (r_pixels << 11) | (g_pixels << 5) | b_pixels;
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int dR = COLOR_RGB565_TO_R5(dataPixel);
                    int dG = COLOR_RGB565_TO_G6(dataPixel);
                    int dB = COLOR_RGB565_TO_B5(dataPixel);
                    int oR = COLOR_RGB565_TO_R5(otherPixel);
                    int oG = COLOR_RGB565_TO_G6(otherPixel);
                    int oB = COLOR_RGB565_TO_B5(otherPixel);
                    int r = oR - dR;
                    int g = oG - dG;
                    int b = oB - dB;
                    #ifdef ARM_MATH_DSP
                    r = __USAT(r, 5);
                    g = __USAT(g, 6);
                    b = __USAT(b, 5);
                    #else
                    r = IM_MAX(r, 0);
                    g = IM_MAX(g, 0);
                    b = IM_MAX(b, 0);
                    #endif
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int dR = COLOR_RGB565_TO_R5(dataPixel);
                        int dG = COLOR_RGB565_TO_G6(dataPixel);
                        int dB = COLOR_RGB565_TO_B5(dataPixel);
                        int oR = COLOR_RGB565_TO_R5(otherPixel);
                        int oG = COLOR_RGB565_TO_G6(otherPixel);
                        int oB = COLOR_RGB565_TO_B5(otherPixel);
                        int r = oR - dR;
                        int g = oG - dG;
                        int b = oB - dB;
                        #ifdef ARM_MATH_DSP
                        r = __USAT(r, 5);
                        g = __USAT(g, 6);
                        b = __USAT(b, 5);
                        #else
                        r = IM_MAX(r, 0);
                        g = IM_MAX(g, 0);
                        b = IM_MAX(b, 0);
                        #endif
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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

// This is the multiply blend mode.
void imlib_mul_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_and_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    // p = dataPixel * otherPixel / 255
                    int p = (((dataPixel * otherPixel) + 1) * 257) >> 16;
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        // p = dataPixel * otherPixel / 255
                        int p = (((dataPixel * otherPixel) + 1) * 257) >> 16;
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int dR = COLOR_RGB565_TO_R5(dataPixel);
                    int dG = COLOR_RGB565_TO_G6(dataPixel);
                    int dB = COLOR_RGB565_TO_B5(dataPixel);
                    int oR = COLOR_RGB565_TO_R5(otherPixel);
                    int oG = COLOR_RGB565_TO_G6(otherPixel);
                    int oB = COLOR_RGB565_TO_B5(otherPixel);
                    // r = dr * oR / 31
                    int r = (((dR * oR) + 1) * 67) >> 11;
                    // g = dG * oG / 63
                    int g = (((dG * oG) + 1) * 65) >> 12;
                    // b = dB * oB / 31
                    int b = (((dB * oB) + 1) * 67) >> 11;
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int dR = COLOR_RGB565_TO_R5(dataPixel);
                        int dG = COLOR_RGB565_TO_G6(dataPixel);
                        int dB = COLOR_RGB565_TO_B5(dataPixel);
                        int oR = COLOR_RGB565_TO_R5(otherPixel);
                        int oG = COLOR_RGB565_TO_G6(otherPixel);
                        int oB = COLOR_RGB565_TO_B5(otherPixel);
                        // r = dr * oR / 31
                        int r = (((dR * oR) + 1) * 67) >> 11;
                        // g = dG * oG / 63
                        int g = (((dG * oG) + 1) * 65) >> 12;
                        // b = dB * oB / 31
                        int b = (((dB * oB) + 1) * 67) >> 11;
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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

// This is the screen blend mode.
void imlib_imul_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data) {
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_or_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    // p = 255 - (((255 - dataPixel) * (255 - otherPixel)) / 255)
                    int p = 255 - (((((255 - dataPixel) * (255 - otherPixel)) + 1) * 257) >> 16);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        // p = 255 - (((255 - dataPixel) * (255 - otherPixel)) / 255)
                        int p = 255 - (((((255 - dataPixel) * (255 - otherPixel)) + 1) * 257) >> 16);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int dR = COLOR_RGB565_TO_R5(dataPixel);
                    int dG = COLOR_RGB565_TO_G6(dataPixel);
                    int dB = COLOR_RGB565_TO_B5(dataPixel);
                    int oR = COLOR_RGB565_TO_R5(otherPixel);
                    int oG = COLOR_RGB565_TO_G6(otherPixel);
                    int oB = COLOR_RGB565_TO_B5(otherPixel);
                    // p = 31 - (((31 - dR) * (31 - oR)) / 31)
                    int r = 31 - (((((31 - dR) * (31 - oR)) + 1) * 67) >> 11);
                    // p = 63 - (((63 - dG) * (63 - oG)) / 63)
                    int g = 63 - (((((63 - dG) * (63 - oG)) + 1) * 65) >> 12);
                    // p = 31 - (((31 - dB) * (31 - oB)) / 31)
                    int b = 31 - (((((31 - dB) * (31 - oB)) + 1) * 67) >> 11);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int dR = COLOR_RGB565_TO_R5(dataPixel);
                        int dG = COLOR_RGB565_TO_G6(dataPixel);
                        int dB = COLOR_RGB565_TO_B5(dataPixel);
                        int oR = COLOR_RGB565_TO_R5(otherPixel);
                        int oG = COLOR_RGB565_TO_G6(otherPixel);
                        int oB = COLOR_RGB565_TO_B5(otherPixel);
                        // p = 31 - (((31 - dR) * (31 - oR)) / 31)
                        int r = 31 - (((((31 - dR) * (31 - oR)) + 1) * 67) >> 11);
                        // p = 63 - (((63 - dG) * (63 - oG)) / 63)
                        int g = 63 - (((((63 - dG) * (63 - oG)) + 1) * 65) >> 12);
                        // p = 31 - (((31 - dB) * (31 - oB)) / 31)
                        int b = 31 - (((((31 - dB) * (31 - oB)) + 1) * 67) >> 11);
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_and_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint8_t *) data->dst_row_override) + x));
                    __USUB8(dataPixels, otherPixels);
                    *((uint32_t *) (row + x)) = __SEL(otherPixels, dataPixels);
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    int p = IM_MIN(dataPixel, otherPixel);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        int p = IM_MIN(dataPixel, otherPixel);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint16_t *) data->dst_row_override) + x));
                    int dp_rb = dataPixels & 0xf81ff81f, op_rb = otherPixels & 0xf81ff81f;
                    __USUB8(dp_rb, op_rb);
                    int rb_pixels = __SEL(op_rb, dp_rb);
                    int dp_g = dataPixels & 0x07e007e0, op_g = otherPixels & 0x07e007e0;
                    __USUB16(dp_g, op_g);
                    int g_pixels = __SEL(op_g, dp_g);
                    *((uint32_t *) (row + x)) = rb_pixels | g_pixels;
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int r = IM_MIN(COLOR_RGB565_TO_R5(dataPixel), COLOR_RGB565_TO_R5(otherPixel));
                    int g = IM_MIN(COLOR_RGB565_TO_G6(dataPixel), COLOR_RGB565_TO_G6(otherPixel));
                    int b = IM_MIN(COLOR_RGB565_TO_B5(dataPixel), COLOR_RGB565_TO_B5(otherPixel));
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int r = IM_MIN(COLOR_RGB565_TO_R5(dataPixel), COLOR_RGB565_TO_R5(otherPixel));
                        int g = IM_MIN(COLOR_RGB565_TO_G6(dataPixel), COLOR_RGB565_TO_G6(otherPixel));
                        int b = IM_MIN(COLOR_RGB565_TO_B5(dataPixel), COLOR_RGB565_TO_B5(otherPixel));
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_or_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint8_t *) data->dst_row_override) + x));
                    __USUB8(dataPixels, otherPixels);
                    *((uint32_t *) (row + x)) = __SEL(dataPixels, otherPixels);
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    int p = IM_MAX(dataPixel, otherPixel);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        int p = IM_MAX(dataPixel, otherPixel);
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint16_t *) data->dst_row_override) + x));
                    int dp_rb = dataPixels & 0xf81ff81f, op_rb = otherPixels & 0xf81ff81f;
                    __USUB8(dp_rb, op_rb);
                    int rb_pixels = __SEL(dp_rb, op_rb);
                    int dp_g = dataPixels & 0x07e007e0, op_g = otherPixels & 0x07e007e0;
                    __USUB16(dp_g, op_g);
                    int g_pixels = __SEL(dp_g, op_g);
                    *((uint32_t *) (row + x)) = rb_pixels | g_pixels;
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    int r = IM_MAX(COLOR_RGB565_TO_R5(dataPixel), COLOR_RGB565_TO_R5(otherPixel));
                    int g = IM_MAX(COLOR_RGB565_TO_G6(dataPixel), COLOR_RGB565_TO_G6(otherPixel));
                    int b = IM_MAX(COLOR_RGB565_TO_B5(dataPixel), COLOR_RGB565_TO_B5(otherPixel));
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        int r = IM_MAX(COLOR_RGB565_TO_R5(dataPixel), COLOR_RGB565_TO_R5(otherPixel));
                        int g = IM_MAX(COLOR_RGB565_TO_G6(dataPixel), COLOR_RGB565_TO_G6(otherPixel));
                        int b = IM_MAX(COLOR_RGB565_TO_B5(dataPixel), COLOR_RGB565_TO_B5(otherPixel));
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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
    image_t *mask = data->callback_arg;

    switch (data->dst_img->pixfmt) {
        case PIXFORMAT_BINARY: {
            imlib_b_xor_line_op(x, x_end, y_row, data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 4; x += 4) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint8_t *) data->dst_row_override) + x));
                    int sub0 = __USUB8(dataPixels, otherPixels);
                    int sub1 = __USUB8(otherPixels, dataPixels);
                    *((uint32_t *) (row + x)) = __SEL(sub1, sub0);
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                    #if defined(ARM_MATH_DSP)
                    int p = __USAD8(dataPixel, otherPixel);
                    #else
                    int p = abs(dataPixel - otherPixel);
                    #endif
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(((uint8_t *) data->dst_row_override), x);
                        #if defined(ARM_MATH_DSP)
                        int p = __USAD8(dataPixel, otherPixel);
                        #else
                        int p = abs(dataPixel - otherPixel);
                        #endif
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row, x, p);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(data->dst_img, y_row);

            if (!mask) {
                #if defined(ARM_MATH_DSP)
                for (; (x_end - x) >= 2; x += 2) {
                    int dataPixels = *((uint32_t *) (row + x));
                    int otherPixels = *((uint32_t *) (((uint16_t *) data->dst_row_override) + x));
                    int dp_rb = dataPixels & 0xf81ff81f, op_rb = otherPixels & 0xf81ff81f;
                    int sub0 = __USUB8(dp_rb, op_rb);
                    int sub1 = __USUB8(op_rb, dp_rb);
                    int rb_pixels = __SEL(sub1, sub0);
                    int dp_g = dataPixels & 0x07e007e0, op_g = otherPixels & 0x07e007e0;
                    sub0 = __USUB16(dp_g, op_g);
                    sub1 = __USUB16(op_g, dp_g);
                    int g_pixels = __SEL(sub1, sub0);
                    *((uint32_t *) (row + x)) = rb_pixels | g_pixels;
                }
                #endif

                for (; x < x_end; x++) {
                    int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                    int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                    #if defined(ARM_MATH_DSP)
                    int r = __USAD8(COLOR_RGB565_TO_R5(dataPixel), COLOR_RGB565_TO_R5(otherPixel));
                    int g = __USAD8(COLOR_RGB565_TO_G6(dataPixel), COLOR_RGB565_TO_G6(otherPixel));
                    int b = __USAD8(COLOR_RGB565_TO_B5(dataPixel), COLOR_RGB565_TO_B5(otherPixel));
                    #else
                    int r = abs(COLOR_RGB565_TO_R5(dataPixel) - COLOR_RGB565_TO_R5(otherPixel));
                    int g = abs(COLOR_RGB565_TO_G6(dataPixel) - COLOR_RGB565_TO_G6(otherPixel));
                    int b = abs(COLOR_RGB565_TO_B5(dataPixel) - COLOR_RGB565_TO_B5(otherPixel));
                    #endif
                    IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
                }
            } else {
                for (; x < x_end; x++) {
                    if (image_get_mask_pixel(mask, x, y_row)) {
                        int dataPixel = IMAGE_GET_RGB565_PIXEL_FAST(row, x);
                        int otherPixel = IMAGE_GET_RGB565_PIXEL_FAST(((uint16_t *) data->dst_row_override), x);
                        #if defined(ARM_MATH_DSP)
                        int r = __USAD8(COLOR_RGB565_TO_R5(dataPixel), COLOR_RGB565_TO_R5(otherPixel));
                        int g = __USAD8(COLOR_RGB565_TO_G6(dataPixel), COLOR_RGB565_TO_G6(otherPixel));
                        int b = __USAD8(COLOR_RGB565_TO_B5(dataPixel), COLOR_RGB565_TO_B5(otherPixel));
                        #else
                        int r = abs(COLOR_RGB565_TO_R5(dataPixel) - COLOR_RGB565_TO_R5(otherPixel));
                        int g = abs(COLOR_RGB565_TO_G6(dataPixel) - COLOR_RGB565_TO_G6(otherPixel));
                        int b = abs(COLOR_RGB565_TO_B5(dataPixel) - COLOR_RGB565_TO_B5(otherPixel));
                        #endif
                        IMAGE_PUT_RGB565_PIXEL_FAST(row, x, COLOR_R5_G6_B5_TO_RGB565(r, g, b));
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

static void imlib_hat(image_t *img, int ksize, int threshold, image_t *mask, morph_op_t op) {
    image_t temp;
    temp.w = img->w;
    temp.h = img->h;
    temp.pixfmt = img->pixfmt;
    temp.data = fb_alloc(image_size(img), FB_ALLOC_CACHE_ALIGN);
    memcpy(temp.data, img->data, image_size(img));
    op(&temp, ksize, threshold, mask);
    void *dst_row_override = fb_alloc0(image_line_size(img), FB_ALLOC_CACHE_ALIGN);
    imlib_draw_image(img, &temp, 0, 0, 1.0f, 1.0f, NULL, -1, 256, NULL, NULL, 0,
                     imlib_difference_line_op, mask, dst_row_override);
    fb_free(); // dst_row_override
    fb_free(); // temp.data
}

void imlib_top_hat(image_t *img, int ksize, int threshold, image_t *mask) {
    imlib_hat(img, ksize, threshold, mask, imlib_open);
}

void imlib_black_hat(image_t *img, int ksize, int threshold, image_t *mask) {
    imlib_hat(img, ksize, threshold, mask, imlib_close);
}
#endif // defined(IMLIB_ENABLE_MATH_OPS) && defined(IMLIB_ENABLE_BINARY_OPS)
