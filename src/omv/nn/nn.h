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

typedef struct _layer {
    layer_type_t type;
    uint32_t c, h, w;
    struct _layer *prev;
    struct _layer *next;
} layer_t;

typedef struct {
    layer_t base;
    uint32_t r_mean;
    uint32_t g_mean;
    uint32_t b_mean;
} data_layer_t;

typedef struct {
    layer_t base;
    uint32_t l_shift;
    uint32_t r_shift;
    uint32_t krn_dim;
    uint32_t krn_str;
    uint32_t krn_pad;
    uint32_t w_size;
    uint32_t b_size;
    int8_t *w, *b;
} conv_layer_t;

typedef struct {
    layer_t base;
} relu_layer_t;

typedef struct {
    layer_t base;
    pool_type_t type;
    uint32_t krn_dim;
    uint32_t krn_str;
    uint32_t krn_pad;
} pool_layer_t;

typedef struct {
    layer_t base;
    uint32_t l_shift;
    uint32_t r_shift;
    uint32_t w_size;
    uint32_t b_size;
    int8_t *w, *b;
} ip_layer_t;

typedef struct {
    uint8_t type[4];
    uint32_t n_layers;
    uint32_t max_layer_size;
    uint32_t max_colbuf_size;
    uint32_t max_scrbuf_size;
    layer_t *layers;
} nn_t;

int nn_dump_network(nn_t *net);
int nn_load_network(nn_t *net, const char *path);
int nn_run_network(nn_t *net, image_t *image);
#endif //#define __CNN_H__
