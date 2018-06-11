/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CNN code.
 *
 */
#ifndef __NN_H__
#define __NN_H__
#include <stdint.h>
#include <imlib.h>
typedef enum {
    LAYER_TYPE_DATA = 0,
    LAYER_TYPE_CONV,
    LAYER_TYPE_RELU,
    LAYER_TYPE_POOL,
    LAYER_TYPE_IP,
} layer_type_t;

typedef enum {
    POOL_TYPE_MAX,
    POOL_TYPE_AVE,
} pool_type_t;

typedef enum {
    NETWORK_TYPE_CAFFE = 0,
} network_type_t;

#define NN_LAYER_BASE   \
    layer_type_t type;  \
    uint32_t n, c, h, w;\
    struct _layer *prev;\
    struct _layer *next \

typedef struct _layer {
    NN_LAYER_BASE;
} layer_t;

typedef struct {
    NN_LAYER_BASE;
    uint32_t r_mean;
    uint32_t g_mean;
    uint32_t b_mean;
    uint32_t scale;
} data_layer_t;

typedef struct {
    NN_LAYER_BASE;
    uint32_t l_shift;
    uint32_t r_shift;
    uint32_t krn_dim;
    uint32_t krn_str;
    uint32_t krn_pad;
    uint32_t w_size;
    uint32_t b_size;
    int8_t *wt, *bias;
} conv_layer_t;

typedef struct {
    NN_LAYER_BASE;
} relu_layer_t;

typedef struct {
    NN_LAYER_BASE;
    pool_type_t ptype;
    uint32_t krn_dim;
    uint32_t krn_str;
    uint32_t krn_pad;
} pool_layer_t;

typedef struct {
    NN_LAYER_BASE;
    uint32_t l_shift;
    uint32_t r_shift;
    uint32_t w_size;
    uint32_t b_size;
    int8_t *wt, *bias;
} ip_layer_t;

typedef struct {
    uint8_t  type[4];
    uint32_t n_layers;
    int8_t  *output_data;
    uint32_t output_size;
    uint32_t max_layer_size;
    uint32_t max_colbuf_size;
    uint32_t max_scrbuf_size;
    layer_t *layers;
} nn_t;

typedef arm_status (*conv_func_t) (const q7_t * Im_in, const uint16_t dim_im_in, const uint16_t ch_im_in,
        const q7_t * wt, const uint16_t ch_im_out, const uint16_t dim_kernel, const uint16_t padding,
        const uint16_t stride, const q7_t * bias, const uint16_t bias_shift, const uint16_t out_shift,
        q7_t * Im_out,  const uint16_t dim_im_out,  q15_t * bufferA,  q7_t * bufferB);

typedef void (*pool_func_t)(q7_t * Im_in, const uint16_t dim_im_in, const uint16_t ch_im_in,
        const uint16_t dim_kernel, const uint16_t padding, const uint16_t stride,
        const uint16_t dim_im_out, q7_t * bufferA, q7_t * Im_out);


int nn_dump_network(nn_t *net);
int nn_load_network(nn_t *net, const char *path);
int nn_run_network(nn_t *net, image_t *img, rectangle_t *roi, bool softmax);
int nn_dry_run_network(nn_t *net, image_t *img, bool softmax);
#endif //#define __CNN_H__
