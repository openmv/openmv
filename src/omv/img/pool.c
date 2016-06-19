/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image pooling.
 *
 */
#include "imlib.h"

void imlib_midpoint_pool(image_t *img_i, image_t *img_o, int x_div, int y_div, const int bias)
{
    int min_bias = (256-bias);
    int max_bias = bias;
    if (IM_IS_GS(img_i)) {
        for (int y = 0, yy = img_i->h / y_div; y < yy; y++) {
            for (int x = 0, xx = img_i->w / x_div; x < xx; x++) {
                int min = 255, max = 0;
                for (int i = 0; i < y_div; i++) {
                    for (int j = 0; j < x_div; j++) {
                        const uint8_t pixel = IM_GET_GS_PIXEL(img_i, (x * x_div) + j, (y * y_div) + i);
                        min = IM_MIN(min, pixel);
                        max = IM_MAX(max, pixel);
                    }
                }
                IM_SET_GS_PIXEL(img_o, x, y,
                    ((min*min_bias)+(max*max_bias))>>8);
            }
        }
    } else {
        for (int y = 0, yy = img_i->h / y_div; y < yy; y++) {
            for (int x = 0, xx = img_i->w / x_div; x < xx; x++) {
                int r_min = 255, r_max = 0;
                int g_min = 255, g_max = 0;
                int b_min = 255, b_max = 0;
                for (int i = 0; i < y_div; i++) {
                    for (int j = 0; j < x_div; j++) {
                        const uint16_t pixel = IM_GET_RGB565_PIXEL(img_i, (x * x_div) + j, (y * y_div) + i);
                        int red = IM_R565(pixel);
                        int green = IM_G565(pixel);
                        int blue = IM_B565(pixel);
                        r_min = IM_MIN(r_min, red);
                        r_max = IM_MAX(r_max, red);
                        g_min = IM_MIN(g_min, green);
                        g_max = IM_MAX(g_max, green);
                        b_min = IM_MIN(b_min, blue);
                        b_max = IM_MAX(b_max, blue);
                    }
                }
                IM_SET_RGB565_PIXEL(img_o, x, y,
                    IM_RGB565(((r_min*min_bias)+(r_max*max_bias))>>8,
                              ((g_min*min_bias)+(g_max*max_bias))>>8,
                              ((b_min*min_bias)+(b_max*max_bias))>>8));
            }
        }
    }
}

void imlib_mean_pool(image_t *img_i, image_t *img_o, int x_div, int y_div)
{
    int n = x_div * y_div;
    if (IM_IS_GS(img_i)) {
        for (int y = 0, yy = img_i->h / y_div; y < yy; y++) {
            for (int x = 0, xx = img_i->w / x_div; x < xx; x++) {
                int acc = 0;
                for (int i = 0; i < y_div; i++) {
                    for (int j = 0; j < x_div; j++) {
                        const uint8_t pixel = IM_GET_GS_PIXEL(img_i, (x * x_div) + j, (y * y_div) + i);
                        acc += pixel;
                    }
                }
                IM_SET_GS_PIXEL(img_o, x, y,
                    acc / n);
            }
        }
    } else {
        for (int y = 0, yy = img_i->h / y_div; y < yy; y++) {
            for (int x = 0, xx = img_i->w / x_div; x < xx; x++) {
                int r_acc = 0;
                int g_acc = 0;
                int b_acc = 0;
                for (int i = 0; i < y_div; i++) {
                    for (int j = 0; j < x_div; j++) {
                        uint16_t pixel = IM_GET_RGB565_PIXEL(img_i, (x * x_div) + j, (y * y_div) + i);
                        r_acc += IM_R565(pixel);
                        g_acc += IM_G565(pixel);
                        b_acc += IM_B565(pixel);
                    }
                }
                IM_SET_RGB565_PIXEL(img_o, x, y,
                    IM_RGB565(r_acc / n, g_acc / n, b_acc / n));
            }
        }
    }
}
