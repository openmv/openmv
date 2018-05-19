/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CNN code.
 *
 */
#include "nn.h"
#include "imlib.h"
#include "ff_wrapper.h"
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
        printf("Layer: %s Shape: [%lu, %lu, %lu] ",
                layer_to_str(layer->type), layer->c, layer->h, layer->w);
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
    for (int i=0; i<net->n_layers - 1; i++) {
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

        // Read layer shape (c, h, w)
        read_data(&fp, &layer->c, 4);
        read_data(&fp, &layer->w, 4);
        read_data(&fp, &layer->h, 4);

        printf("Reading layer: %s Shape: [%lu, %lu, %lu] ",
                layer_to_str(layer->type), layer->c, layer->h, layer->w);

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
                read_data(&fp, &conv_layer->krn_str, 4);
                read_data(&fp, &conv_layer->krn_pad, 4);
                printf("l_shift: %lu r_shift:%lu k_size: %lu k_stride: %lu k_padding: %lu ",
                        conv_layer->l_shift, conv_layer->r_shift,
                        conv_layer->krn_dim, conv_layer->krn_str, conv_layer->krn_pad);
               
                // Alloc and read weights array
                read_data(&fp, &conv_layer->w_size, 4);
                printf("weights: %lu ", conv_layer->w_size);
                conv_layer->w = xalloc(conv_layer->w_size);
                read_data(&fp, conv_layer->w, conv_layer->w_size);

                // Alloc and read bias array
                read_data(&fp, &conv_layer->b_size, 4);
                printf("bias: %lu\n", conv_layer->b_size);
                conv_layer->b = xalloc(conv_layer->b_size);
                read_data(&fp, conv_layer->b, conv_layer->b_size);
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
                read_data(&fp, &pool_layer->type, 4);
                // Read krnel dim, stride and padding
                read_data(&fp, &pool_layer->krn_dim, 4);
                read_data(&fp, &pool_layer->krn_str, 4);
                read_data(&fp, &pool_layer->krn_pad, 4);
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
                ip_layer->w = xalloc(ip_layer->w_size);
                read_data(&fp, ip_layer->w, ip_layer->w_size);

                // Alloc and read bias array
                read_data(&fp, &ip_layer->b_size, 4);
                printf("bias %lu\n", ip_layer->b_size);
                ip_layer->b = xalloc(ip_layer->b_size);
                read_data(&fp, ip_layer->b, ip_layer->b_size);
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
            uint32_t im2col_buffer_size = 2 * 2 * layer->c * conv_layer->krn_dim * conv_layer->krn_dim;
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
    printf("max layer size: %lu max col buf size: %lu max scratch buf size: %lu\n",
            max_layer_size, max_colbuf_size, max_scrbuf_size);
error:
    file_buffer_off(&fp);
    file_close(&fp);
    return res;
}

int nn_run_network(nn_t *net, image_t *image)
{
    return 0;
}
#endif //IMLIB_ENABLE_CNN
