
/**
 ******************************************************************************
 * @file    ll_sw.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ll_sw low level software library module.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __LL_SW_H__
#define __LL_SW_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#include "ll_sw_float.h"
#include "ll_sw_integer.h"

  // ############################ ############################ ###########################
  // ############################     SUPPORTED NODES TYPES    ###########################
  // ############################ ############################ ###########################

  typedef enum
  {
    LL_SW_CONV = 0,
    LL_SW_RELU,
    LL_SW_PRELU,
    LL_SW_THRESHOLDEDRELU,
    LL_SW_ELU,
    LL_SW_SELU,
    LL_SW_MAXPOOL,
    LL_SW_MINPOOL,
    LL_SW_AVGPOOL,
    LL_SW_SOFTMAX,
    LL_SW_BATCHNORM,
    LL_SW_LRN,
    LL_SW_ARITHADD,
    LL_SW_ARITHSUM,
    LL_SW_ARITHSUB,
    LL_SW_ARITHMUL,
    LL_SW_ARITHDIV,
    LL_SW_CONCAT,
    LL_SW_SPLIT,
    LL_SW_SLICE,
    LL_SW_SIGMOID,
    LL_SW_ATAN,
    LL_SW_ATANH,
    LL_SW_ACOS,
    LL_SW_ACOSH,
    LL_SW_ASIN,
    LL_SW_ASINH,
    LL_SW_TAN,
    LL_SW_TANH,
    LL_SW_COS,
    LL_SW_COSH,
    LL_SW_SIN,
    LL_SW_SINH,
    LL_SW_SQRT,
    LL_SW_CEIL,
    LL_SW_GATHER,
    LL_SW_FLOOR,
    LL_SW_EXP,
    LL_SW_LOG,
    LL_SW_QLINEARCONV,
    LL_SW_QUANTIZELINEAR,
    LL_SW_REQUANTIZELINEAR,
    LL_SW_DEQUANTIZELINEAR,
    LL_SW_QLINEARMATMUL,
    LL_SW_GEMM,
    LL_SW_RESIZE,
    LL_SW_ERF,
    LL_SW_POW,
    LL_SW_AND,
    LL_SW_OR,
    LL_SW_XOR,
    LL_SW_EQUAL,
    LL_SW_GREATER,
    LL_SW_LESS,
    LL_SW_GREATEROREQUAL,
    LL_SW_LESSOREQUAL,
    LL_SW_CLIP,
    LL_SW_HARDSIGMOID,
    LL_SW_HARDMAX,
    LL_SW_HARDSWISH,
    LL_SW_SWISH,
    LL_SW_ABS,
    LL_SW_INSTANCENORM,
    LL_SW_REDUCESUM,
    LL_SW_REDUCEMIN,
    LL_SW_REDUCEMEAN,
    LL_SW_REDUCEMAX,
    LL_SW_REDUCEPROD,
    LL_SW_SOFTSIGN,
    LL_SW_SOFTPLUS,
    LL_SW_LPNORM,
    LL_SW_ARGMIN,
    LL_SW_ARGMAX,
    LL_SW_MATMUL,
    LL_SW_MIN,
    LL_SW_MAX,
    LL_SW_NEG,
    LL_SW_RECIPROCAL,
    LL_SW_MOD,
    LL_SW_ROUND,
    LL_SW_REDUCELOGSUMEXP,
    LL_SW_SIGN,
    LL_SW_TILE,
    LL_SW_GELU
  } NodeType;

  // ############################ ############################ ###########################
  // ############################ GENERAL HELPER NODES STRUCTS ###########################
  // ############################ ############################ ###########################

  typedef struct LL_SW_State_Monitor
  {
    long unsigned int id[16];
  } LL_SW_State_Monitor;

  typedef struct Tensor_dim_info
  {
    uint32_t tensor_h;
    uint32_t tensor_w;
    uint32_t tensor_c;
    uint32_t tensor_b;
    uint32_t num_elem;
  } Tensor_dim_info;

  typedef struct Tensor_stride_info
  {
    uint32_t h;
    uint32_t w;
    uint32_t c;
    uint32_t b;
  } Tensor_stride_info;

  typedef struct
  {
    unsigned char *start_offset;
  } Tensor_mem_info;

  typedef struct
  {
    bool is_signed;
  } Tensor_format_info;

  typedef struct
  {
    unsigned char *scale;
    unsigned char *offset;
  } Tensor_int_metadata_info;

  typedef struct Tensor_info
  {
    Tensor_dim_info dim;
    Tensor_stride_info stride;
    Tensor_mem_info mem;
    Tensor_format_info format;
  } Tensor_info;

  typedef struct General
  {
    NodeType type;
    // info for the input tensor
    Tensor_info input;
    Tensor_info output;

  } General;

  // tensor quantization info
  typedef struct intq_info_
  {
    const float *scale;
    const void *zeropoint;
  } intq_info;

  typedef struct intq_info_list_
  {
    uint16_t flags;        /**< optional flags to store intq info attributes */
    uint16_t size;         /**< number of elements in the the intq_info list  */
    const intq_info *info; /**< pointer to an array of metainfo associated to the intq_info list */
  } intq_info_list;

  // ############################ ############################ ###########################
  // ############################ FLOATING POINT NODES STRUCTS ###########################
  // ############################ ############################ ###########################

  typedef struct Conv_sw_info
  {
    General general;
    Tensor_info weights;
    Tensor_info scratch;
    Tensor_info bias;
    Tensor_info weights_permuted;
    int ngroup; // should it change?
    long int pads[4];
    long int strides[4];
    long int dilations[2];
  } Conv_sw_info;

  typedef struct Bn_sw_info
  {
    General general;
    Tensor_info bias;
    Tensor_info scale;
    Tensor_info mean;
    Tensor_info var;
  } Bn_sw_info;

  typedef struct Instance_normalization_sw_info
  {
    General general;
    Tensor_info bias;
    Tensor_info scale;
    float epsilon;
  } Instance_normalization_sw_info;

  typedef struct Argmin_sw_info
  {
    General general;
    int axis;
    int keepdims;
    int select_last_index;
  } Argmin_sw_info;
  typedef struct Argmax_sw_info
  {
    General general;
    int axis;
    int keepdims;
    int select_last_index;
  } Argmax_sw_info;

  typedef struct Arith_sw_info
  {
    General general;
    Tensor_info operand;
    Tensor_info operand1;
    Tensor_info operand2;
    unsigned int num_of_inputs;
  } Arith_sw_info;

  typedef struct Pool_sw_info
  {
    General general;
    int ngroup; // should it change?
    long int pads[4];
    long int strides[4];
    long int k_shape[2];
    bool count_include_pad;
  } Pool_sw_info;

  typedef struct Lrn_sw_info
  {
    General general;
    uint32_t size; // should it change?
    float alpha;
    float beta;
    float bias;
  } Lrn_sw_info;

  typedef struct Lpnormalization_sw_info
  {
    General general;
    int axis;
    int p;
  } Lpnormalization_sw_info;

  typedef struct Sign_sw_info
  {
    General general;
  } Sign_sw_info;

  typedef enum
  {
    RESIZE_ZEROS = 0x0,
    RESIZE_NEAREST,
    RESIZE_LINEAR,
    RESIZE_CUBIC
  } resize_mode;

  typedef enum
  {
    HALF_PIXEL = 0x0,
    PYTORCH_HALF_PIXEL,
    ALIGN_CORNERS,
    ASYMMETRIC,
    TF_HALF_PIXEL_FOR_NN,
    TF_CROP_AND_RESIZE
  } coord_transf_mode;

  typedef enum
  {
    ROUND_PREFER_FLOOR = 0x0,
    ROUND_PREFER_CEIL,
    FLOOR,
    CEIL
  } nearest_mode;

  typedef struct Resize_sw_info
  {
    General general;
    Tensor_info scales;
    Tensor_info roi;
    Tensor_info sizes;
    float cubic_coeff_a;                 /**< the coefficient 'a' used in cubic interpolation */
    bool exclude_outside;                /**< exclude outside pixels flag */
    float extrapol_val;                  /**< used in tf_crop_and_resize cas */
    resize_mode mode;                    /**< resize mode */
    nearest_mode nearest_mode;           /**< used in nearest mode */
    coord_transf_mode coord_transf_mode; /**< coordinate transformation mode */
  } Resize_sw_info;

  typedef struct Activ_sw_info
  {
    General general;
    Tensor_info operand;
    float alpha;
    float beta;
    float gamma;
    float min;
    float max;
  } Activ_sw_info;

  typedef struct Softmax_sw_info
  {
    General general;
    Tensor_info scratch;
    int axis;
  } Softmax_sw_info;

  typedef struct Concat_sw_info
  {
    General general;
    unsigned int concat_axis;
    unsigned int num_of_inputs;
    Tensor_info operand;
    Tensor_info operand1;
    Tensor_info operand2;
  } Concat_sw_info;

  typedef struct Gather_sw_info
  {
    General general;
    Tensor_info operand;
    int axis;
  } Gather_sw_info;

  typedef struct Global_pool_sw_info
  {
    General general;
  } Global_pool_sw_info;

  typedef struct Reduce_sw_info
  {
    General general;
    int axis;
  } Reduce_sw_info;

  typedef struct Gemm_sw_info
  {
    General general;
    Tensor_info operand_b;
    Tensor_info operand_c;
    float alpha; /**< alpha coefficient */
    float beta;  /**< beta coefficient */
    uint8_t tA;  /**< transpose A flag */
    uint8_t tB;  /**< transpose B flag */
  } Gemm_sw_info;

  typedef struct Matmul_sw_info
  {
    General general;
    Tensor_info operand_b;
  } Matmul_sw_info;
  typedef struct Tile_sw_info
  {
    General general;
    int16_t repeats[4];
  } Tile_sw_info;

  // ############################ ############################ ###########################
  // ############################  SCALE OFFSET NODES STRUCTS  ###########################
  // ############################ ############################ ###########################

  typedef enum
  {
    LL_SW_SSSA_PW_CONV,
    LL_SW_SSSA_RGB_CONV,
    LL_SW_SSSA_DW_CONV,
    LL_SW_SSSA_DILATED_CONV,
    LL_SW_SSSA_GENERIC_CONV,
    LL_SW_GENERIC_CONV
  } conv_fwd_case;

  typedef struct Qlinearconv_sw_info
  {
    General general;
    Tensor_info weights;
    Tensor_info scratch;
    Tensor_info bias;
    Tensor_info ws;
    Tensor_info wzp;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    int ngroup;
    long int pads[4];
    long int strides[4];
    long int dilations[2];
    conv_fwd_case fwd_func;
  } Qlinearconv_sw_info;

  typedef struct Qlinearmatmul_sw_info
  {
    General general;
    Tensor_info weights;
    Tensor_info scratch;
    Tensor_info bias;
    Tensor_info ws;
    Tensor_info wzp;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
  } Qlinearmatmul_sw_info;

  typedef struct Gemm_integer_sw_info
  {
    General general;
    Tensor_info weights;
    Tensor_info scratch;
    Tensor_info bias;
    Tensor_info ws;
    Tensor_info wzp;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    float alpha; /**< alpha coefficient */
    float beta;  /**< beta coefficient */
    uint8_t tA;  /**< transpose A flag */
    uint8_t tB;  /**< transpose B flag */
  } Gemm_integer_sw_info;

  typedef struct Conv_integer_sw_info
  {
    General general;
    Tensor_info weights;
    Tensor_info scratch;
    Tensor_info bias;
    Tensor_info ws;
    Tensor_info wzp;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    int ngroup; // should it change?
    long int pads[4];
    long int strides[4];
    long int dilations[2];
    conv_fwd_case fwd_func;
  } Conv_integer_sw_info;

  typedef struct Pool_integer_sw_info
  {
    General general;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    int ngroup;
    long int pads[4];
    long int strides[4];
    long int k_shape[2];
    bool count_include_pad;
  } Pool_integer_sw_info;

  typedef struct Global_pool_integer_sw_info
  {
    General general;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    int ngroup;
    long int pads[4];
    long int strides[4];
    long int k_shape[2];
  } Global_pool_integer_sw_info;

  typedef struct Activ_integer_sw_info
  {
    General general;
    Tensor_info operand;
    Tensor_info operand_s;
    Tensor_info operand_zp;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    float alpha;
    float gamma;
    float min;
    float max;
  } Activ_integer_sw_info;

  typedef struct Softmax_integer_sw_info
  {
    General general;
    Tensor_info scratch;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    int32_t quantized_multiplier;
    int32_t left_shift;
    int32_t diff_min;
    int32_t axis;
  } Softmax_integer_sw_info;

  typedef struct Eltwise_integer_sw_info
  {
    General general;
    Tensor_info operand;
    Tensor_info operand_s;
    Tensor_info operand_zp;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    unsigned int num_of_inputs;
  } Eltwise_integer_sw_info;

  typedef struct Resize_integer_sw_info
  {
    General general;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
    Tensor_info scales;
    Tensor_info roi;
    Tensor_info sizes;
    float cubic_coeff_a;                 /**< the coefficient 'a' used in cubic interpolation */
    bool exclude_outside;                /**< exclude outside pixels flag */
    float extrapol_val;                  /**< used in tf_crop_and_resize cas */
    resize_mode mode;                    /**< resize mode */
    nearest_mode nearest_mode;           /**< used in nearest mode */
    coord_transf_mode coord_transf_mode; /**< coordinate transformation mode */
  } Resize_integer_sw_info;

  // ############################ ########################### ###########################
  // ############################ QUANT/DEQUANT NODES STRUCTS ###########################
  // ############################ ########################### ###########################

  typedef struct Quantizelinear_sw_info
  {
    General general;
    Tensor_info os;
    Tensor_info ozp;
  } Quantizelinear_sw_info;

  typedef struct Dequantizelinear_sw_info
  {
    General general;
    Tensor_info is;
    Tensor_info izp;
  } Dequantizelinear_sw_info;

  typedef struct Requantizelinear_sw_info
  {
    General general;
    Tensor_info is;
    Tensor_info izp;
    Tensor_info os;
    Tensor_info ozp;
  } Requantizelinear_sw_info;
  typedef struct Argmax_integer_sw_info
  {
    General general;
    int axis;
    int keepdims;
    int select_last_index;
  } Argmax_integer_sw_info;
  typedef struct Argmin_integer_sw_info
  {
    General general;
    int axis;
    int keepdims;
    int select_last_index;
  } Argmin_integer_sw_info;

#ifdef __cplusplus
}
#endif

#endif //__LL_SW_H__
