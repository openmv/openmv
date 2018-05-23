/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CNN code.
 *
 */
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
                printf("r_mean: %lu g_mean: %lu b_mean: %lu\n",
                        data_layer->r_mean, data_layer->g_mean, data_layer->b_mean);
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

    printf("Net type: %4s Num layers: %lu\n", net->type, net->n_layers);

    layer_t *prev_layer = NULL;
    for (int i=0; i<net->n_layers; i++) {
        layer_t *layer;
        layer_type_t layer_type;

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

        printf("Reading layer: %s Shape: [%lu, %lu, %lu, %lu] ",
                layer_to_str(layer->type), layer->n, layer->c, layer->h, layer->w);

        switch (layer_type) {
            case LAYER_TYPE_DATA: {
                data_layer_t *data_layer = (data_layer_t *) layer;
                // Read data layer R, G, B mean
                read_data(&fp, &data_layer->r_mean, 4);
                read_data(&fp, &data_layer->g_mean, 4);
                read_data(&fp, &data_layer->b_mean, 4);
                printf("r_mean: %lu g_mean: %lu b_mean: %lu\n",
                        data_layer->r_mean, data_layer->g_mean, data_layer->b_mean);
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
                printf("l_shift: %lu r_shift:%lu k_size: %lu k_stride: %lu k_padding: %lu ",
                        conv_layer->l_shift, conv_layer->r_shift,
                        conv_layer->krn_dim, conv_layer->krn_str, conv_layer->krn_pad);
               
                // Alloc and read weights array
                read_data(&fp, &conv_layer->w_size, 4);
                printf("weights: %lu ", conv_layer->w_size);
                conv_layer->wt = xalloc(conv_layer->w_size);
                read_data(&fp, conv_layer->wt, conv_layer->w_size);

                // Alloc and read bias array
                read_data(&fp, &conv_layer->b_size, 4);
                printf("bias: %lu\n", conv_layer->b_size);
                conv_layer->bias = xalloc(conv_layer->b_size);
                read_data(&fp, conv_layer->bias, conv_layer->b_size);
                break;
            }

            case LAYER_TYPE_RELU: {
                // Nothing to read for RELU layer
                printf("\n");
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
                printf("k_size: %lu k_stride: %lu k_padding: %lu\n",
                        pool_layer->krn_dim, pool_layer->krn_str, pool_layer->krn_pad);
                break;
            }

            case LAYER_TYPE_IP: {
                ip_layer_t *ip_layer = (ip_layer_t *) layer;
                // Read layer l_shift, r_shift
                read_data(&fp, &ip_layer->l_shift, 4);
                read_data(&fp, &ip_layer->r_shift, 4);
                printf("l_shift: %lu r_shift:%lu ",
                        ip_layer->l_shift, ip_layer->r_shift);

                // Alloc and read weights array
                read_data(&fp, &ip_layer->w_size, 4);
                printf("weights: %lu ", ip_layer->w_size);
                ip_layer->wt = xalloc(ip_layer->w_size);
                read_data(&fp, ip_layer->wt, ip_layer->w_size);

                // Alloc and read bias array
                read_data(&fp, &ip_layer->b_size, 4);
                printf("bias %lu\n", ip_layer->b_size);
                ip_layer->bias = xalloc(ip_layer->b_size);
                read_data(&fp, ip_layer->bias, ip_layer->b_size);
                break;
            }
        }
    }

    uint32_t max_layer_size  = 0;
    uint32_t max_colbuf_size = 0;
    uint32_t max_scrbuf_size = 0;
    layer_t *layer = net->layers;
    while (layer != NULL) {
        // First layer is DATA will be skipped, so prev_layer *should* not be NULL.
        prev_layer = layer->prev;

        if (layer->type == LAYER_TYPE_IP) {
            uint32_t fc_buffer_size = 2 * layer->c;
            max_colbuf_size = IM_MAX(max_colbuf_size, fc_buffer_size);
        }

        if (layer->type == LAYER_TYPE_CONV) {
            conv_layer_t *conv_layer = (conv_layer_t *) layer;
            uint32_t im2col_buffer_size = 2 * 2 * conv_layer->c * conv_layer->krn_dim * conv_layer->krn_dim;
            max_colbuf_size = IM_MAX(max_colbuf_size, im2col_buffer_size);
        }

        if (layer->type == LAYER_TYPE_IP) {
            uint32_t buffer_size = layer->c;
            if (prev_layer->type == LAYER_TYPE_IP) {
                buffer_size = buffer_size + prev_layer->c; 
            } else if (prev_layer->type == LAYER_TYPE_CONV || prev_layer->type == LAYER_TYPE_POOL) {
                buffer_size = buffer_size + prev_layer->c * prev_layer->h * prev_layer->w;
            }
            max_scrbuf_size = IM_MAX(max_scrbuf_size, buffer_size);
        }

        if (layer->type == LAYER_TYPE_CONV || layer->type == LAYER_TYPE_POOL) {
            uint32_t buffer_size = layer->c * layer->h * layer->w + prev_layer->c * prev_layer->h * prev_layer->w;
            max_scrbuf_size = IM_MAX(max_scrbuf_size, buffer_size);
        }

        uint32_t layer_size = layer->c * layer->h * layer->w;
        max_layer_size = IM_MAX(max_layer_size, layer_size);
        layer = layer->next;
    }

    net->max_layer_size  = max_layer_size;
    net->max_colbuf_size = max_colbuf_size;
    net->max_scrbuf_size = max_scrbuf_size;
    printf("Max layer: %lu Max col buf: %lu Max scratch buf: %lu\n\n",
            max_layer_size, max_colbuf_size, max_scrbuf_size);
error:
    file_buffer_off(&fp);
    file_close(&fp);
    return res;
}

#define BUFFER_2STR(buffer)\
        (buffer == buffer1)     ? "buffer1":\
        (buffer == buffer2)     ? "buffer2":\
        (buffer == input_data)  ? "input_data":\
        (buffer == output_data) ? "output_data": "???"

#define CONV_FUNC_2STR(conv_func)\
        (conv_func == arm_convolve_HWC_q7_basic) ? "arm_convolve_HWC_q7_basic" :\
        (conv_func == arm_convolve_HWC_q7_fast ) ? "arm_convolve_HWC_q7_fast":"arm_convolve_HWC_q7_RGB"

#define POOL_FUNC_2STR(pool_func)\
        (pool_func == arm_maxpool_q7_HWC) ? "arm_maxpool_q7_HWC" : "arm_avepool_q7_HWC"

int nn_run_network(nn_t *net, image_t *img, int8_t *output_data)
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

    q7_t *buffer1     = fb_alloc0(net->max_scrbuf_size);
    q7_t *buffer2     = buffer1 + net->max_layer_size;
    q7_t *col_buffer  = fb_alloc0(net->max_colbuf_size);

    q7_t *input_data    = NULL;
    q7_t *input_buffer  = NULL;
    q7_t *output_buffer = NULL;

    while (layer != NULL) {
        layer_t *prev_layer = layer->prev;

        switch (layer->type) {
            case LAYER_TYPE_DATA: {
                data_layer_t *data_layer = (data_layer_t *) layer;
                input_data = fb_alloc(data_layer->c * data_layer->h * data_layer->w);
                // Scale, convert, remove mean image and load input data.
                int x_ratio = (int)((img->w<<16)/layer->w)+1;
                int y_ratio = (int)((img->h<<16)/layer->h)+1;
                for (int y=0, i=0; y<layer->h; y++) {
                    int sy = (y*y_ratio)>>16;
                    for (int x=0; x<layer->w; x++, i+=3) {
                        int sx = (x*x_ratio)>>16;
                        uint16_t p = IM_GET_RGB565_PIXEL(img, sx, sy);
                        input_data[i+0] = (int8_t) (((int) COLOR_RGB565_TO_R8(p)) - (int) data_layer->r_mean);
                        input_data[i+1] = (int8_t) (((int) COLOR_RGB565_TO_G8(p)) - (int) data_layer->g_mean);
                        input_data[i+2] = (int8_t) (((int) COLOR_RGB565_TO_B8(p)) - (int) data_layer->b_mean);
                    }
                }
                // Set image data as input buffer for the next layer.
                input_buffer = input_data;
                output_buffer = buffer1;
                break;
            }

            case LAYER_TYPE_CONV: {
                conv_func_t conv_func = NULL;
                conv_layer_t *conv_layer = (conv_layer_t *) layer;
                if (prev_layer->c % 4 != 0 ||
                    conv_layer->n % 2 != 0 || prev_layer->h % 2 != 0) {
                    conv_func = arm_convolve_HWC_q7_basic;
                    if (prev_layer->c == 3) {
                        conv_func = arm_convolve_HWC_q7_RGB;
                    }
                } else {
                    conv_func = arm_convolve_HWC_q7_fast;
                }
                debug_printf("forward: %s(%s, %lu, %lu, %s, %lu, %lu, %lu, %lu, %s, %lu, %lu, %s, %lu, %s, %p);\n",
                        CONV_FUNC_2STR(conv_func), BUFFER_2STR(input_buffer),
                        prev_layer->h, prev_layer->c, "conv_wt", conv_layer->c, 
                        conv_layer->krn_dim, conv_layer->krn_pad, conv_layer->krn_str,
                        "conv_bias", conv_layer->l_shift, conv_layer->r_shift,
                        BUFFER_2STR(output_buffer), conv_layer->h, "col_buffer", NULL);
                conv_func(input_buffer, prev_layer->h, prev_layer->c, conv_layer->wt, conv_layer->c, 
                        conv_layer->krn_dim, conv_layer->krn_pad, conv_layer->krn_str, conv_layer->bias,
                        conv_layer->l_shift, conv_layer->r_shift, output_buffer, conv_layer->h, (q15_t*)col_buffer, NULL); 
                break;
            }

            case LAYER_TYPE_RELU: {
                relu_layer_t *relu_layer = (relu_layer_t *) layer;
                debug_printf("forward: arm_relu_q7(%s, %lu*%lu*%lu);\n",
                        BUFFER_2STR(input_buffer), relu_layer->h, relu_layer->w, relu_layer->c);
                arm_relu_q7(input_buffer, relu_layer->h * relu_layer->w * relu_layer->c);
                break;
            }

            case LAYER_TYPE_POOL: {
                pool_func_t pool_func = NULL;
                pool_layer_t *pool_layer = (pool_layer_t *) layer;
                if (pool_layer->ptype == POOL_TYPE_MAX) {
                    pool_func = arm_maxpool_q7_HWC;
                } else {
                    pool_func = arm_avepool_q7_HWC;
                }
                debug_printf("forward: %s(%s, %lu, %lu, %lu, %lu, %lu, %lu, %s, %s);\n",
                        POOL_FUNC_2STR(pool_func), BUFFER_2STR(input_buffer),
                        prev_layer->h, prev_layer->c, pool_layer->krn_dim,
                        pool_layer->krn_pad, pool_layer->krn_str, layer->w, "col_buffer", BUFFER_2STR(output_buffer));
                pool_func(input_buffer, prev_layer->h, prev_layer->c, pool_layer->krn_dim,
                        pool_layer->krn_pad, pool_layer->krn_str, layer->w, col_buffer, output_buffer);
                break;
            }

            case LAYER_TYPE_IP: {
                ip_layer_t *ip_layer = (ip_layer_t*) layer;
                debug_printf("forward: arm_fully_connected_q7_opt(%s, %s, %lu, %lu, %lu, %lu, %s, %s, %s);\n",
                        BUFFER_2STR(input_buffer), "ip_wt", prev_layer->c * prev_layer->h * prev_layer->w,
                        ip_layer->c, ip_layer->l_shift, ip_layer->r_shift, "ip_bias", BUFFER_2STR(output_buffer), "col_buffer");
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
                output_buffer = output_data;
            }
        }
        
        layer = layer->next;
    }

    fb_free_all();
    debug_printf("\n");
    return 0;
}
#endif //IMLIB_ENABLE_CNN
