/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CNN code.
 */
#include <stdio.h>
#include "nn.h"
#include "imlib.h"
#include "common.h"
#include "ff_wrapper.h"
#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "omv_boardconfig.h"
#ifdef IMLIB_ENABLE_CNN

static const char *layer_to_str(layer_type_t type)
{
    static const char *layers[] = {
        "DATA", "CONV", "RELU", "POOL", "IP"
    };
    if (type > sizeof(layers)/sizeof(layers[0])) {
        return "Unknown layer";
    } else {
        return layers[type];
    }
}

int nn_dump_network(nn_t *net)
{
    layer_t *layer = net->layers;
    
    printf("Net type: %4s Num layers: %lu Max layer: %lu Max col buf: %lu Max scratch buf: %lu\n",
            net->type, net->n_layers, net->max_layer_size, net->max_colbuf_size, net->max_scrbuf_size);

    while (layer != NULL) {
        printf("Layer: %s Shape: [%lu, %lu, %lu, %lu] ",
                layer_to_str(layer->type), layer->n, layer->c, layer->h, layer->w);
        switch (layer->type) {
            case LAYER_TYPE_DATA: {
                data_layer_t *data_layer = (data_layer_t *) layer;
                printf("r_mean: %lu g_mean: %lu b_mean: %lu scale: %lu\n",
                        data_layer->r_mean, data_layer->g_mean, data_layer->b_mean, data_layer->scale);
                break;
            }

            case LAYER_TYPE_CONV: {
                conv_layer_t *conv_layer = (conv_layer_t *) layer;
                printf("l_shift: %lu r_shift:%lu k_size: %lu k_stride: %lu k_padding: %lu\n",
                        conv_layer->l_shift, conv_layer->r_shift,
                        conv_layer->krn_dim, conv_layer->krn_str, conv_layer->krn_pad);
                break;
            }

            case LAYER_TYPE_RELU: {
                // Nothing to read for RELU layer
                printf("\n");
                //relu_layer_t *relu_layer = layer;
                break;
            }

            case LAYER_TYPE_POOL: {
                pool_layer_t *pool_layer = (pool_layer_t *) layer;
                printf("k_size: %lu k_stride: %lu k_padding: %lu\n",
                        pool_layer->krn_dim, pool_layer->krn_str, pool_layer->krn_pad);
                break;
            }

            case LAYER_TYPE_IP: {
                ip_layer_t *ip_layer = (ip_layer_t*) layer;
                printf("l_shift: %lu r_shift:%lu\n", ip_layer->l_shift, ip_layer->r_shift);
                break;
            }
        }
        layer = layer->next;
    }
    return 0;
}

int nn_load_network(nn_t *net, const char *path)
{
    FIL fp;
    int res = 0;

    file_read_open(&fp, path);
    file_buffer_on(&fp);

    // Read network type
    read_data(&fp, net->type, 4);

    // Read number of layers
    read_data(&fp, &net->n_layers, 4);

    layer_t *prev_layer = NULL;
    for (int i=0; i<net->n_layers; i++) {
        layer_t *layer;
        uint32_t layer_type;

        // Read layer type
        read_data(&fp, &layer_type, 4);
        switch (layer_type) {
            case LAYER_TYPE_DATA:
                layer = xalloc0(sizeof(data_layer_t));
                break;
            case LAYER_TYPE_CONV:
                layer = xalloc0(sizeof(conv_layer_t));
                break;
            case LAYER_TYPE_RELU:
                layer = xalloc0(sizeof(relu_layer_t));
                break;
            case LAYER_TYPE_POOL:
                layer = xalloc0(sizeof(pool_layer_t));
                break;
            case LAYER_TYPE_IP:
                layer = xalloc0(sizeof(ip_layer_t));
                break;
            default:
                res = -1;
                goto error;
        }

        if (prev_layer == NULL) { // First layer
            net->layers = layer;
        } else {
            layer->prev = prev_layer;
            prev_layer->next = layer;
        }
        prev_layer = layer;

        // Set type
        layer->type = layer_type;

        // Read layer shape (NCHW)
        read_data(&fp, &layer->n, 4);
        read_data(&fp, &layer->c, 4);
        read_data(&fp, &layer->h, 4);
        read_data(&fp, &layer->w, 4);

        switch (layer_type) {
            case LAYER_TYPE_DATA: {
                data_layer_t *data_layer = (data_layer_t *) layer;
                // Read data layer R, G, B mean and input scale
                read_data(&fp, &data_layer->r_mean, 4);
                read_data(&fp, &data_layer->g_mean, 4);
                read_data(&fp, &data_layer->b_mean, 4);
                read_data(&fp, &data_layer->scale, 4);
                break;
            }

            case LAYER_TYPE_CONV: {
                conv_layer_t *conv_layer = (conv_layer_t *) layer;
                // Read layer l_shift, r_shift
                read_data(&fp, &conv_layer->l_shift, 4);
                read_data(&fp, &conv_layer->r_shift, 4);
                // Read krnel dim, stride and padding
                read_data(&fp, &conv_layer->krn_dim, 4);
                read_data(&fp, &conv_layer->krn_pad, 4);
                read_data(&fp, &conv_layer->krn_str, 4);

                // Alloc and read weights array
                read_data(&fp, &conv_layer->w_size, 4);
                conv_layer->wt = xalloc(conv_layer->w_size);
                read_data(&fp, conv_layer->wt, conv_layer->w_size);

                // Alloc and read bias array
                read_data(&fp, &conv_layer->b_size, 4);
                conv_layer->bias = xalloc(conv_layer->b_size);
                read_data(&fp, conv_layer->bias, conv_layer->b_size);
                break;
            }

            case LAYER_TYPE_RELU: {
                // Nothing to read for RELU layer
                break;
            }

            case LAYER_TYPE_POOL: {
                pool_layer_t *pool_layer = (pool_layer_t *) layer;
                // Read pooling layer type
                read_data(&fp, &pool_layer->ptype, 4);
                // Read krnel dim, stride and padding
                read_data(&fp, &pool_layer->krn_dim, 4);
                read_data(&fp, &pool_layer->krn_pad, 4);
                read_data(&fp, &pool_layer->krn_str, 4);
                break;
            }

            case LAYER_TYPE_IP: {
                ip_layer_t *ip_layer = (ip_layer_t *) layer;
                // Read layer l_shift, r_shift
                read_data(&fp, &ip_layer->l_shift, 4);
                read_data(&fp, &ip_layer->r_shift, 4);

                // Alloc and read weights array
                read_data(&fp, &ip_layer->w_size, 4);
                ip_layer->wt = xalloc(ip_layer->w_size);
                read_data(&fp, ip_layer->wt, ip_layer->w_size);

                // Alloc and read bias array
                read_data(&fp, &ip_layer->b_size, 4);
                ip_layer->bias = xalloc(ip_layer->b_size);
                read_data(&fp, ip_layer->bias, ip_layer->b_size);
                break;
            }
        }
    }

    layer_t *layer = net->layers;
    while (layer != NULL) {
        // First layer is DATA will be skipped, so prev_layer *should* not be NULL.
        prev_layer = layer->prev;

        if (layer->type == LAYER_TYPE_IP) {
            uint32_t fc_buffer_size = 2 * prev_layer->c * prev_layer->w * prev_layer->h;
            net->max_colbuf_size = IM_MAX(net->max_colbuf_size, fc_buffer_size);
        }

        if (layer->type == LAYER_TYPE_CONV) {
            conv_layer_t *conv_layer = (conv_layer_t *) layer;
            uint32_t im2col_buffer_size = 2 * 2 * conv_layer->c * conv_layer->krn_dim * conv_layer->krn_dim;
            net->max_colbuf_size = IM_MAX(net->max_colbuf_size, im2col_buffer_size);
        }

        if (layer->type == LAYER_TYPE_IP) {
            uint32_t buffer_size = layer->c;
            if (prev_layer->type == LAYER_TYPE_IP) {
                buffer_size = buffer_size + prev_layer->c;
            } else if (prev_layer->type == LAYER_TYPE_CONV || prev_layer->type == LAYER_TYPE_POOL) {
                buffer_size = buffer_size + prev_layer->c * prev_layer->h * prev_layer->w;
            }
            net->max_scrbuf_size = IM_MAX(net->max_scrbuf_size, buffer_size);
        }

        if (layer->type == LAYER_TYPE_CONV || layer->type == LAYER_TYPE_POOL) {
            uint32_t buffer_size = layer->c * layer->h * layer->w + prev_layer->c * prev_layer->h * prev_layer->w;
            net->max_scrbuf_size = IM_MAX(net->max_scrbuf_size, buffer_size);
        }

        uint32_t layer_size = layer->c * layer->h * layer->w;
        net->max_layer_size = IM_MAX(net->max_layer_size, layer_size);
        if (layer->next == NULL) {
            net->output_size = layer->c;
        }
        layer = layer->next;
    }

    // Alloc output buffer.
    net->output_data = xalloc(net->output_size);
error:
    file_buffer_off(&fp);
    file_close(&fp);
    return res;
}

#ifndef __SSAT
#define __SSAT(a, b) ({ __typeof__ (a) _a = (a); \
                        __typeof__ (b) _b = (b); \
                        _b = 1 << (_b - 1); \
                        _a = _a < (_b - 1) ? _a : (_b - 1); \
                        _a > (-_b) ? _a : (-_b); })
#endif

void nn_transform_input(data_layer_t *data_layer, image_t *img, q7_t *input_data, rectangle_t *roi)
{
    int input_scale = data_layer->scale;
    // Scale, convert and normalize input image.
    int x_ratio = (int)((roi->w<<16)/data_layer->w)+1;
    int y_ratio = (int)((roi->h<<16)/data_layer->h)+1;

    if ((img->bpp == 2) && (data_layer->c == 3)) { // RGB565 to RGB888
        for (int y=0, i=0; y<data_layer->h; y++) {
            int sy = (y*y_ratio)>>16;
            for (int x=0; x<data_layer->w; x++, i+=3) {
                int sx = (x*x_ratio)>>16;
                uint16_t p = IM_GET_RGB565_PIXEL(img, sx+roi->x, sy+roi->y);
                input_data[i+0] = (q7_t)__SSAT((((((int) COLOR_RGB565_TO_R8(p))
                                  - (int) data_layer->r_mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
                input_data[i+1] = (q7_t)__SSAT((((((int) COLOR_RGB565_TO_G8(p))
                                  - (int) data_layer->g_mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
                input_data[i+2] = (q7_t)__SSAT((((((int) COLOR_RGB565_TO_B8(p))
                                  - (int) data_layer->b_mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
            }
        }
    } else if ((img->bpp == 2) && (data_layer->c == 1)) { // RGB565 to GS
        for (int y=0, i=0; y<data_layer->h; y++) {
            int sy = (y*y_ratio)>>16;
            for (int x=0; x<data_layer->w; x++, i++) {
                int sx = (x*x_ratio)>>16;
                uint16_t p = IM_GET_RGB565_PIXEL(img, sx+roi->x, sy+roi->y);
                input_data[i] = (q7_t)__SSAT((((((int) COLOR_RGB565_TO_GRAYSCALE(p))
                                - (int) data_layer->r_mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
            }
        }
    } else if ((img->bpp == 1) && (data_layer->c == 3)) { // GS to RGB88
        int mean = (int) ((0.30f * data_layer->r_mean) +
                          (0.59f * data_layer->g_mean) +
                          (0.11f * data_layer->b_mean));
        for (int y=0, i=0; y<data_layer->h; y++) {
            int sy = (y*y_ratio)>>16;
            for (int x=0; x<data_layer->w; x++, i+=3) {
                int sx = (x*x_ratio)>>16;
                int p = (int) IMAGE_GET_GRAYSCALE_PIXEL(img, sx+roi->x, sy+roi->y);
                input_data[i+0] = (q7_t)__SSAT((((p - (int) mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
                input_data[i+1] = (q7_t)__SSAT((((p - (int) mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
                input_data[i+2] = (q7_t)__SSAT((((p - (int) mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
            }
        }
    } else if ((img->bpp == 1) && (data_layer->c == 1)) { // GS to GS
        for (int y=0, i=0; y<data_layer->h; y++) {
            int sy = (y*y_ratio)>>16;
            for (int x=0; x<data_layer->w; x++, i++) {
                int sx = (x*x_ratio)>>16;
                int p = (int) IMAGE_GET_GRAYSCALE_PIXEL(img, sx+roi->x, sy+roi->y);
                input_data[i] = (q7_t)__SSAT((((p - (int) data_layer->r_mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
            }
        }
    } else if ((img->bpp == 0) && (data_layer->c == 3)) { // BINARY to RGB88
        int mean = (int) ((0.30f * data_layer->r_mean) +
                          (0.59f * data_layer->g_mean) +
                          (0.11f * data_layer->b_mean));
        for (int y=0, i=0; y<data_layer->h; y++) {
            int sy = (y*y_ratio)>>16;
            for (int x=0; x<data_layer->w; x++, i+=3) {
                int sx = (x*x_ratio)>>16;
                int p = (int) COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL(img, sx+roi->x, sy+roi->y));
                input_data[i+0] = (q7_t)__SSAT((((p - (int) mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
                input_data[i+1] = (q7_t)__SSAT((((p - (int) mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
                input_data[i+2] = (q7_t)__SSAT((((p - (int) mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
            }
        }
    } else if ((img->bpp == 0) && (data_layer->c == 1)) { // BINARY to GS
        for (int y=0, i=0; y<data_layer->h; y++) {
            int sy = (y*y_ratio)>>16;
            for (int x=0; x<data_layer->w; x++, i++) {
                int sx = (x*x_ratio)>>16;
                int p = (int) COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL(img, sx+roi->x, sy+roi->y));
                input_data[i] = (q7_t)__SSAT((((p - (int) data_layer->r_mean)<<7) + (1<<(input_scale-1))) >> input_scale, 8);
            }
        }
    }
}

int nn_run_network(nn_t *net, image_t *img, rectangle_t *roi, bool softmax)
{
    uint32_t layer_idx = 0;
    layer_t *layer = net->layers;

    if (layer == NULL) {
        printf("First layer is NULL!\n");
        return -1;
    }

    if (layer->type != LAYER_TYPE_DATA) {
        printf("First layer is not a DATA layer!\n");
        return -1;
    }

    q7_t *input_data    = NULL;
    q7_t *input_buffer  = NULL;
    q7_t *output_buffer = NULL;

    fb_alloc_mark();

    q7_t *buffer1     = fb_alloc(net->max_scrbuf_size, FB_ALLOC_NO_HINT);
    q7_t *buffer2     = buffer1 + net->max_layer_size;
    q7_t *col_buffer  = fb_alloc(net->max_colbuf_size, FB_ALLOC_NO_HINT);

    while (layer != NULL) {
        layer_t *prev_layer = layer->prev;

        switch (layer->type) {
            case LAYER_TYPE_DATA: {
                data_layer_t *data_layer = (data_layer_t *) layer;
                input_data = fb_alloc(data_layer->c * data_layer->h * data_layer->w, FB_ALLOC_NO_HINT);
                nn_transform_input(data_layer, img, input_data, roi);
                // Set image data as input buffer for the next layer.
                input_buffer = input_data;
                output_buffer = buffer1;
                break;
            }

            case LAYER_TYPE_CONV: {
                conv_func_t conv_func = NULL;
                conv_func_nonsquare_t conv_func_nonsquare = NULL;
                conv_layer_t *conv_layer = (conv_layer_t *) layer;
                if (prev_layer->c % 4 != 0 ||
                    conv_layer->n % 2 != 0 || prev_layer->h % 2 != 0) {
                    if (prev_layer->c == 3) {
                        conv_func = arm_convolve_HWC_q7_RGB;
                    } else if (prev_layer->w == prev_layer->h) {
                        conv_func = arm_convolve_HWC_q7_basic;
                    } else {
                        conv_func_nonsquare = arm_convolve_HWC_q7_basic_nonsquare;
                    }
                } else {
                    if (prev_layer->w == prev_layer->h) {
                        conv_func = arm_convolve_HWC_q7_fast;
                    } else {
                        conv_func_nonsquare = arm_convolve_HWC_q7_fast_nonsquare;
                    }
                }
                if (conv_func) {
                    conv_func(input_buffer, prev_layer->h, prev_layer->c, conv_layer->wt, conv_layer->c,
                            conv_layer->krn_dim, conv_layer->krn_pad, conv_layer->krn_str, conv_layer->bias,
                            conv_layer->l_shift, conv_layer->r_shift, output_buffer, conv_layer->h, (q15_t*)col_buffer, NULL);
                } else {
                    conv_func_nonsquare(input_buffer, prev_layer->w, prev_layer->h, prev_layer->c, conv_layer->wt, conv_layer->c,
                            conv_layer->krn_dim, conv_layer->krn_dim, conv_layer->krn_pad, conv_layer->krn_pad, conv_layer->krn_str,
                            conv_layer->krn_str, conv_layer->bias, conv_layer->l_shift, conv_layer->r_shift, output_buffer,
                            conv_layer->w, conv_layer->h, (q15_t*)col_buffer, NULL);
                }
                break;
            }

            case LAYER_TYPE_RELU: {
                relu_layer_t *relu_layer = (relu_layer_t *) layer;
                arm_relu_q7(input_buffer, relu_layer->h * relu_layer->w * relu_layer->c);
                break;
            }

            case LAYER_TYPE_POOL: {
                pool_func_t pool_func = NULL;
                pool_func_nonsquare_t pool_func_nonsquare = NULL;
                pool_layer_t *pool_layer = (pool_layer_t *) layer;
                if (pool_layer->ptype == POOL_TYPE_MAX) {
                    if (prev_layer->w == prev_layer->h) {
                        pool_func = arm_maxpool_q7_HWC;
                    } else {
                        pool_func_nonsquare = arm_maxpool_q7_HWC_nonsquare;
                    }
                } else {
                    if (prev_layer->w == prev_layer->h) {
                        pool_func = arm_avepool_q7_HWC;
                    } else {
                        pool_func_nonsquare = arm_avepool_q7_HWC_nonsquare;
                    }
                }
                if (pool_func) {
                    pool_func(input_buffer, prev_layer->h, prev_layer->c, pool_layer->krn_dim,
                            pool_layer->krn_pad, pool_layer->krn_str, layer->w, col_buffer, output_buffer);
                } else {
                    pool_func_nonsquare(input_buffer, prev_layer->w, prev_layer->h, prev_layer->c, pool_layer->krn_dim,
                            pool_layer->krn_pad, pool_layer->krn_str, layer->w, layer->h, col_buffer, output_buffer);
                }
                break;
            }

            case LAYER_TYPE_IP: {
                ip_layer_t *ip_layer = (ip_layer_t*) layer;
                arm_fully_connected_q7_opt(input_buffer, ip_layer->wt, prev_layer->c * prev_layer->h * prev_layer->w,
                        ip_layer->c, ip_layer->l_shift, ip_layer->r_shift, ip_layer->bias, output_buffer, (q15_t*)col_buffer);
                break;
            }
        }

        if (layer_idx++ > 0) {
            if (input_buffer == input_data) {
                // Image data has been processed
                input_buffer = buffer2;
            }

            if (layer->type != LAYER_TYPE_RELU) {
                // Switch buffers
                q7_t *tmp_buffer = input_buffer;
                input_buffer  = output_buffer;
                output_buffer = tmp_buffer;
            }

            // Last layer
            if (layer->next && layer->next->next == NULL) {
                output_buffer = net->output_data;
            }
        }

        layer = layer->next;
    }

    // Softmax output
    if (softmax) {
        arm_softmax_q7(net->output_data, net->output_size, net->output_data);
    }

    fb_alloc_free_till_mark();
    return 0;
}

#define BUFFER_2STR(buffer)\
        (buffer == buffer1)     ? "buffer1":\
        (buffer == buffer2)     ? "buffer2":\
        (buffer == input_data)  ? "input_data":\
        (buffer == net->output_data) ? "output_data": "???"

#define CONV_FUNC_2STR(conv_func)\
        (conv_func == arm_convolve_HWC_q7_basic) ? "arm_convolve_HWC_q7_basic" :\
        (conv_func == arm_convolve_HWC_q7_fast ) ? "arm_convolve_HWC_q7_fast":"arm_convolve_HWC_q7_RGB"

#define POOL_FUNC_2STR(pool_func)\
        (pool_func == arm_maxpool_q7_HWC) ? "arm_maxpool_q7_HWC" : "arm_avepool_q7_HWC"

#define CONV_FUNC_NONSQ_2STR(conv_func)\
        (conv_func == arm_convolve_HWC_q7_basic_nonsquare) ? "arm_convolve_HWC_q7_basic_nonsquare":\
        "arm_convolve_HWC_q7_fast_nonsquare"

#define POOL_FUNC_NONSQ_2STR(pool_func)\
        (pool_func == arm_maxpool_q7_HWC_nonsquare) ? "arm_maxpool_q7_HWC_nonsquare" : "arm_avepool_q7_HWC_nonsquare"

int nn_dry_run_network(nn_t *net, image_t *img, bool softmax)
{
    uint32_t layer_idx = 0;
    layer_t *layer = net->layers;

    if (layer == NULL) {
        printf("First layer is NULL!\n");
        return -1;
    }

    if (layer->type != LAYER_TYPE_DATA) {
        printf("First layer is not a DATA layer!\n");
        return -1;
    }

    q7_t *input_data    = NULL;
    q7_t *input_buffer  = NULL;
    q7_t *output_buffer = NULL;

    fb_alloc_mark();

    q7_t *buffer1     = fb_alloc(net->max_scrbuf_size, FB_ALLOC_NO_HINT);
    q7_t *buffer2     = buffer1 + net->max_layer_size;

    while (layer != NULL) {
        layer_t *prev_layer = layer->prev;
        switch (layer->type) {
            case LAYER_TYPE_DATA: {
                data_layer_t *data_layer = (data_layer_t *) layer;
                // Set image data as input buffer for the next layer.
                input_buffer = input_data = fb_alloc(data_layer->c * data_layer->h * data_layer->w, FB_ALLOC_NO_HINT);
                output_buffer = buffer1;
                break;
            }

            case LAYER_TYPE_CONV: {
                conv_func_t conv_func = NULL;
                conv_func_nonsquare_t conv_func_nonsquare = NULL;
                conv_layer_t *conv_layer = (conv_layer_t *) layer;
                if (prev_layer->c % 4 != 0 ||
                    conv_layer->n % 2 != 0 || prev_layer->h % 2 != 0) {
                    if (prev_layer->c == 3) {
                        conv_func = arm_convolve_HWC_q7_RGB;
                    } else if (prev_layer->w == prev_layer->h) {
                        conv_func = arm_convolve_HWC_q7_basic;
                    } else {
                        conv_func_nonsquare = arm_convolve_HWC_q7_basic_nonsquare;
                    }
                } else {
                    if (prev_layer->w == prev_layer->h) {
                        conv_func = arm_convolve_HWC_q7_fast;
                    } else {
                        conv_func_nonsquare = arm_convolve_HWC_q7_fast_nonsquare;
                    }
                }

                if (conv_func) {
                    printf("forward: %s(%s, %lu, %lu, %s, %lu, %lu, %lu, %lu, %s, %lu, %lu, %s, %lu, %s, %p);\n",
                            CONV_FUNC_2STR(conv_func), BUFFER_2STR(input_buffer),
                            prev_layer->h, prev_layer->c, "conv_wt", conv_layer->c,
                            conv_layer->krn_dim, conv_layer->krn_pad, conv_layer->krn_str,
                            "conv_bias", conv_layer->l_shift, conv_layer->r_shift,
                            BUFFER_2STR(output_buffer), conv_layer->h, "col_buffer", NULL);
                } else {
                    printf("forward: %s(%s, %lu, %lu, %lu, %s, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %s, %lu, %lu, \
                        %s, %lu, %lu, %s, %p);\n",
                            CONV_FUNC_NONSQ_2STR(conv_func_nonsquare), BUFFER_2STR(input_buffer),
                            prev_layer->w, prev_layer->h, prev_layer->c, "conv_wt", conv_layer->c,
                            conv_layer->krn_dim, conv_layer->krn_dim, conv_layer->krn_pad, conv_layer->krn_pad,
                            conv_layer->krn_str, conv_layer->krn_str, "conv_bias", conv_layer->l_shift, conv_layer->r_shift,
                            BUFFER_2STR(output_buffer), conv_layer->w, conv_layer->h, "col_buffer", NULL);
                }
                break;
            }

            case LAYER_TYPE_RELU: {
                relu_layer_t *relu_layer = (relu_layer_t *) layer;
                printf("forward: arm_relu_q7(%s, %lu*%lu*%lu);\n",
                        BUFFER_2STR(input_buffer), relu_layer->h, relu_layer->w, relu_layer->c);
                break;
            }

            case LAYER_TYPE_POOL: {
                pool_func_t pool_func = NULL;
                pool_func_nonsquare_t pool_func_nonsquare = NULL;
                pool_layer_t *pool_layer = (pool_layer_t *) layer;
                if (pool_layer->ptype == POOL_TYPE_MAX) {
                    if (prev_layer->w == prev_layer->h) {
                        pool_func = arm_maxpool_q7_HWC;
                    } else {
                        pool_func_nonsquare = arm_maxpool_q7_HWC_nonsquare;
                    }
                } else {
                    if (prev_layer->w == prev_layer->h) {
                        pool_func = arm_avepool_q7_HWC;
                    } else {
                        pool_func_nonsquare = arm_avepool_q7_HWC_nonsquare;
                    }
                }
                if (pool_func) {
                    printf("forward: %s(%s, %lu, %lu, %lu, %lu, %lu, %lu, %s, %s);\n",
                            POOL_FUNC_2STR(pool_func), BUFFER_2STR(input_buffer),
                            prev_layer->h, prev_layer->c, pool_layer->krn_dim,
                            pool_layer->krn_pad, pool_layer->krn_str, layer->w, "col_buffer", BUFFER_2STR(output_buffer));
                } else {
                    printf("forward: %s(%s, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %s, %s);\n",
                            POOL_FUNC_NONSQ_2STR(pool_func_nonsquare), BUFFER_2STR(input_buffer),
                            prev_layer->w, prev_layer->h, prev_layer->c, pool_layer->krn_dim,
                            pool_layer->krn_pad, pool_layer->krn_str, layer->w, layer->h, "col_buffer", BUFFER_2STR(output_buffer));
                }
                break;
            }

            case LAYER_TYPE_IP: {
                ip_layer_t *ip_layer = (ip_layer_t*) layer;
                printf("forward: arm_fully_connected_q7_opt(%s, %s, %lu, %lu, %lu, %lu, %s, %s, %s);\n",
                        BUFFER_2STR(input_buffer), "ip_wt", prev_layer->c * prev_layer->h * prev_layer->w,
                        ip_layer->c, ip_layer->l_shift, ip_layer->r_shift, "ip_bias", BUFFER_2STR(output_buffer), "col_buffer");
                break;
            }
        }

        if (layer_idx++ > 0) {
            if (input_buffer == input_data) {
                // Image data has been processed
                input_buffer = buffer2;
            }

            if (layer->type != LAYER_TYPE_RELU) {
                // Switch buffers
                q7_t *tmp_buffer = input_buffer;
                input_buffer  = output_buffer;
                output_buffer = tmp_buffer;
            }

            // Last layer
            if (layer->next && layer->next->next == NULL) {
                output_buffer = net->output_data;
            }
        }
        
        layer = layer->next;
    }

    fb_alloc_free_till_mark();
    printf("\n");
    return 0;
}
#endif //IMLIB_ENABLE_CNN
