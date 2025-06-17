
/**
  ******************************************************************************
  * @file    layers_generic.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform generic layers datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_GENERIC_H
#define LAYERS_GENERIC_H

#include "layers_common.h"

typedef enum {
    KTfLiteNone = 0,
    KTfLiteActRelu,
    KTfLiteActRelu1,
    KTfLiteActRelu6,
    KTfLiteActTanh,
    KTfLiteActSignBit,
    KTfLiteActSigmoid
} ai_tflitefused_activation;

/*!
 * @defgroup layers_generic Generic Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_time_delay
 * @ingroup layers_generic
 * @brief TimeDelay layer with sparse kernel
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_time_delay_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* mask;      /*!< sparse filter mask */
} ai_layer_time_delay;

/*!
 * @struct ai_layer_split
 * @ingroup layers_generic
 * @brief Split layer definition
 *
 * This layer defines the params of a splitting layer. It is intended to be used
 * by his associated forward function @ref forward_split
 */

typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_split_ {
    AI_LAYER_COMMON_FIELDS_DECLARE
    const ai_i32              outer_elems;
    const ai_i32              outer_elems_stride;
} ai_layer_split;


/*!
 * @struct ai_layer_topK
 * @ingroup layers_generic
 * @brief topK layer definition
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_topK_{
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_i16   axis;
  ai_i16   largest;
} ai_layer_topK;

typedef AI_ALIGNED_TYPE(struct,4)ai_layer_svdf_{
    AI_LAYER_COMMON_FIELDS_DECLARE
    ai_size rank;
    ai_tflitefused_activation activation;

} ai_layer_svdf;


/*!
 * @struct ai_layer_slice
 * @ingroup layers_generic
 * @brief Slice layer definition
 *
 * This layer defines the params of a slicing layer. It is intended to be used
 * by his associated forward function @ref forward_slice
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_slice_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* axes;    /*!< Axes that 'starts' and 'ends' apply to. It's optional*/
  AI_CONST ai_array* starts;  /*!< Starting indices of corrisponding axis in axes*/
  AI_CONST ai_array* ends;    /*!< Ending indices (exclusive) of corrisponding axis in axes*/
} ai_layer_slice;

/*!
 * @struct ai_layer_gather
 * @ingroup layers_generic
 * @brief Gather layer definition
 *
 * This layer defines the params of a gathering layer. It is intended to be used
 * by his associated forward function @ref forward_gather
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_gather_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_i16 axis;    /*!< Which axis to gather on It's optional*/
  ai_tensor* indices;  /*!< Indices of corrisponding axis in axes*/
  } ai_layer_gather;

/*!
 * @struct ai_layer_gather_nd
 * @ingroup layers_generic
 * @brief GatherND layer definition
 *
 * This layer defines the params of a gathering layer (ND). It is intended to be used
 * by his associated forward function @ref forward_gather_nd
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_gather_nd_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_tensor* indices;  /*!< Indices of corrisponding slices of inputs*/
  } ai_layer_gather_nd;

/*!
 * @struct ai_layer_tile
 * @ingroup layers generic
 * @brief Tile layer definition
 *
 * This layer defines the param of an tile layer. It constructs a tensor by tiling a
 * given tensor. It is intended to be used by its associated forward function
 * @ref forward_upsample
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_tile_{
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* repeats;  /*!< numbers of repeated copies along each dimension */
} ai_layer_tile;

/*!
 * @struct ai_layer_shape
 * @ingroup layers generic
 * @brief Shape layer definition
 *
 * This layer defines the param of a shape layer. It returns the shape of the
 * input tensor. It is intended to be used by its associated forward function
 * @ref forward_shape
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_shape_{
  AI_LAYER_COMMON_FIELDS_DECLARE
} ai_layer_shape;


/*!
 * @struct ai_layer_upsample
 * @ingroup layers generic
 * @brief Upsample layer definition
 *
 * This layer defines the param of an upsampling layer. It overloads its params
 * to allow zeros upsampling, helpful traspose convolutions, for instance.
 * It is intended to be used by its associated forward function @ref forward_upsample
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_upsample_{
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_upsample_mode    mode;          /*!< upsample mode */
  ai_bool             center;        /*!< center pixels */
  AI_CONST ai_array*  scales;        /*!< scale array along each dimension */
  ai_nearest_mode     nearest_mode;  /*!< used in nearest mode */
} ai_layer_upsample;

/*!
 * @struct ai_layer_resize
 * @ingroup layers generic
 * @brief Resize layer definition
 *
 * This layer defines the param of a resize layer.
 * It is intended to be used by its associated forward function @ref forward_resize
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_resize_{
  AI_LAYER_COMMON_FIELDS_DECLARE

  ai_coord_transf_mode  coord_transf_mode;  /*!< coordinate tranformation mode */
  ai_float              cubic_coeff_a;      /*!< the coefficient 'a' used in cubic interpolation */
  ai_bool               exclude_outside;    /*!< exclude outside pixels flag */
  ai_float              extrapol_val;       /*!< used in tf_crop_and_resize cas */
  ai_resize_mode        mode;               /*!< resize mode */
  ai_nearest_mode       nearest_mode;       /*!< used in nearest mode */
  AI_CONST ai_array*    scales;             /*!< scale array along each dimension */
  AI_CONST ai_array*    roi;                /*!< roi array, used in tf_crop_and_resize case */
} ai_layer_resize;

/*!
 * @struct ai_layer_instanceNormalization
 * @ingroup layers generic
 * @brief instance normalization layer definition
 *
 * This layer defines the params of an instance normalization layer.
 * It is intended to be used by its associated forward function @ref forward_instanceNormalization
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_instanceNormaization_{
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_float            eps;      /*!< epsilon value, to avoid by zero division */
} ai_layer_instanceNormalization;

/*!
 * @struct ai_layer_mode
 * @ingroup layers generic
 * @brief Pad layer definition
 *
 * This layer defines the param of an pad layer. It pad a tensor.
 * It is intended to be used by its associated forward function @ref forward_pad
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_pad_{
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_pad_mode  mode;   /*!< pad mode */
  ai_shape     pads;   /*!< Number of padding to add or remove at the beginning and end of each axis */
  const ai_array*    value;  /*!< Indicates the value to be filled */
} ai_layer_pad;

/*!
 * @struct ai_layer_mode
 * @ingroup layers generic
 * @brief ConstantOfShape layer definition
 *
 * This layer defines the param of an constantofshape layer. It constantofshape a tensor.
 * It is intended to be used by its associated forward function @ref forward_constantofshape
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_constantofshape_{
  AI_LAYER_COMMON_FIELDS_DECLARE
  const ai_array*    value;  /*!< Indicates the value to be filled */
} ai_layer_constantofshape;
/*!
 * @struct ai_layer_add
 * @ingroup layers_generic
 * @brief Add layer definition
 *
 * This layer defines the params of an add layer.
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_add_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_u16             in_layers_count; /*!< number of input layers to concat */
  ai_u16             in_layer_curr;   /*!< current layer to concat  */
  ai_tensor**        in_tensors;  /*!< input tensors list (if NULL==no copy) */
  ai_tensor*         out_tensor;  /*!< output tensor (if NULL==no copy) */
  func_copy_tensor   copy_to_out_tensor; /*!< pointer to copy tensor func
                                         (NULL = no copy) */
  ai_layer_base*     split_layer; /*!< pointer to associated split layer */
  ai_layer_base*     next_layer;  /*!< pointer to next layer to process */
} ai_layer_add;

typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_argminmax_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_i16   axis;
  ai_i16   select_last_index;
} ai_layer_argminmax;

/*!
 * @struct ai_layer_transpose
 * @ingroup layers_generic
 * @brief Transpose layer datastruct declaration. This defines the params of a
 * transpose layer. It is intended to be used by his associated forward function
 * @ref forward_transpose
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_transpose_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape out_mapping;       /*!< transpose output mapping order. I.e. tt is a
                                   permutation of the input tensor shape */
} ai_layer_transpose;

/*!
 * @struct ai_layer_transpose_batch
 * @ingroup layers_generic
 * @brief Transpose batch layer datastruct declaration. This defines the params of a
 * transpose layer. It is intended to be used by his associated forward function
 * @ref forward_transpose_batch
 */
typedef ai_layer_base ai_layer_transpose_batch;


#define AI_TIME_DISTRIBUTED_AXIS    (AI_SHAPE_HEIGHT)

/*!
 * @struct ai_layer_time_distributed
 * @ingroup layers_generic
 * @brief Time distributed layer datastruct declaration. This defines the params
 * of a time distributed layer. It is intended to be used by his associated
 * forward function @ref forward_time_distributed
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_time_distributed_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_layer_base*  inner_layer;       /*!< inner layer to process */
} ai_layer_time_distributed;

/*!
 * @struct ai_layer_concat
 * @ingroup layers_generic
 * @brief Concatenation layer
 *
 * Concat Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_concat_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape_dimension axis;       /*!< which axis to concatenate on */
} ai_layer_concat;

/*!
 * @struct ai_layer_pack
 * @ingroup layers_generic
 * @brief pack layer
 *
 * Pack Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_pack_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape_dimension axis;       /*!< which axis to concatenate on */
} ai_layer_pack;

/*!
 * @struct ai_layer_unpack
 * @ingroup layers_generic
 * @brief unpack layer
 *
 * Unpack Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_unpack_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape_dimension axis;       /*!< which axis to concatenate on */
} ai_layer_unpack;

typedef void (*func_binary)(ai_handle out,const ai_handle a, const ai_handle b);
typedef void (*func_buffer_binary)(ai_handle out,const ai_handle a, const ai_handle b, const ai_size loop);
typedef void (*func_buffer_binary_integer)(ai_handle out,const ai_handle a, const ai_handle b, const ai_size loop,
                                        const ai_handle scale1, const ai_handle zp1, const ai_handle scale2, const ai_handle zp2,
                                        const ai_handle scaleout, const ai_handle zpout, const ai_i32 scalar_op);

/*!
 * @struct ai_layer_eltwise
 * @ingroup layers_generic
 * @brief General element-wise transformation layer
 *
 * Elementwise Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_eltwise_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  func_binary operation;       /*!< operation to apply elementwise */
  func_buffer_binary buffer_operation; /*!< operation to apply elementwise */
} ai_layer_eltwise;

/*!
 * @struct ai_layer_eltwise_integer
 * @ingroup layers_generic
 * @brief General element-wise transformation layer for integer data
 *
 * Elementwise Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_eltwise_integer_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  func_binary operation;       /*!< operation to apply elementwise */
  func_buffer_binary_integer buffer_operation; /*!< operation to apply elementwise */
} ai_layer_eltwise_integer;

/*!
 * @struct ai_layer_scatter_nd
 * @ingroup layers_generic
 * @brief ScatterND layer definition
 *
 * This layer defines the params of a scattering layer (ND). It is intended to be used
 * by his associated forward function @ref forward_scatter_nd
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_scatter_nd_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_tensor* indices;  /*!< Indices of corrisponding slices of inputs*/
  ai_tensor* updates;  /*!< Updates of corrisponding slices of inputs*/
  func_binary operation;    /*!< operation to apply elementwise */
  ai_scatter_nd_reduction reduction; /*!< Reduction operation in ScatterND layer*/
} ai_layer_scatter_nd;

/*!
 * @struct ai_layer_reduce
 * @ingroup layers_generic
 * @brief General dimension reduction layer
 *
 * reduction Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_reduce_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  const ai_array* neutral_value;   /*!< Initialization value for operation */
  func_binary operation;    /*!< operation to apply elementwise */
} ai_layer_reduce;

/*!
 * @struct ai_layer_reduce_log_sum_exp
 * @ingroup layers_generic
 * @brief General dimension reduction layer
 *
 * reduction Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_reduce_log_sum_exp_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape_dimension axis;
} ai_layer_reduce_log_sum_exp;

/*!
 * @struct ai_layer_reduce l1
 * @ingroup layers_generic
 * @brief General dimension reduction layer
 *
 * reduction Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_reduce_l1_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* axes;
} ai_layer_reduce_l1;


/*!
 * @struct ai_layer_reduce l2
 * @ingroup layers_generic
 * @brief General dimension reduction layer
 *
 * reduction Layer.
 * It is a sequential layer. see @ref ai_layer_sequential
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_reduce_l2_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* axes;
} ai_layer_reduce_l2;


/*!
 * @struct ai_layer_where
 * @ingroup layers generic
 * @brief Where layer definition
 *
 * This layer operates on 3 input tensors: condition, X and Y.
 * It return elements, either from X or Y, depending on condition
 * (with Numpy-style broadcasting support).
 * @ref forward_where
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_where_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  const ai_array *shapes_len;
  ai_bool channel_first;
} ai_layer_where;


/*!
 * @struct ai_layer_reverse
 * @ingroup layers_reverse
 * @brief Reverse layer
 *
 * The type of reverse function is handled by the specific forward function
 * @ref forward_svm_regressor
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_reverse_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_i32 axis;         /*!< selected axis to perform the operation */
} ai_layer_reverse;


/******************************************************************************/
/* Forward Functions Section                                                  */
/******************************************************************************/

/*!
 * @brief Dummy forward routine with no processing.
 * @ingroup layers_generic
 * @param generic layer handle
 */
AI_INTERNAL_API
void forward_nop(ai_layer* layer);

/*!
 * @brief Computes the activations of a TimeDelay layer.
 * @ingroup layers_generic
 * @param layer the time delay layer
 */
AI_INTERNAL_API
void forward_time_delay(ai_layer* layer);

/*!
 * @brief Split network computation in N parallel branches.
 * @ingroup layers_generic
 * @param layer the split layer
 */
AI_INTERNAL_API
void forward_split(ai_layer* layer);

/*!
 * @brief Add network computation from N parallel branches.
 * @ingroup layers_generic
 * @param layer the add layer
 */
AI_INTERNAL_API
void forward_add(ai_layer* layer);

/*!
 * @brief Compute the indices of the max elements of the input tensor's element along the provided axis.
 * @ingroup layers_generic
 * @param layer argminmax layer
 */
AI_INTERNAL_API
void forward_argmax(ai_layer* layer);

/*!
 * @brief Compute the indices of the min elements of the input tensor's element along the provided axis.
 * @ingroup layers_generic
 * @param layer argminmax layer
 */
AI_INTERNAL_API
void forward_argmin(ai_layer* layer);

/*!
 * @brief Svdf layer.
 * @ingroup layers_generic
 * @param layer svdf layer
 */
AI_INTERNAL_API
void forward_svdf(ai_layer* layer);

/*!
 * @brief Transpose a tensor along a pivot and save transposed values into an output
 * tensor
 * @ingroup layers_generic
 * @param layer the transpose layer
 */
AI_INTERNAL_API
void forward_transpose(ai_layer* layer);

/*!
 * @brief Transpose batch and save transposed values of a determinate batch into an output
 * tensor
 * @ingroup layers_generic
 * @param layer the transpose batch layer
 */
AI_INTERNAL_API
void forward_transpose_batch(ai_layer* layer);

/*!
 * @brief TimeDistrubuted forward layer function. This forward function
 * implements the timedistributed layer.
 * @ingroup layers_generic
 * @param layer the time distributed layer
 */
AI_INTERNAL_API
void forward_time_distributed(ai_layer* layer);

/*!
 * @brief Packing a list of tensors in a single tensor
 * @ingroup layers generic
 * @param layer the packing layer
 */
AI_INTERNAL_API
void forward_pack(ai_layer* layer);

/*!
 * @brief Unpacking a single of tensors in a list tensor
 * @ingroup layers generic
 * @param layer the unpacking layer
 */
AI_INTERNAL_API
void forward_unpack(ai_layer* layer);

/*!
 * @brief Concatenates a list of tensors into a single tensor.
 * @ingroup layers_generic
 * @param layer the concatenation layer
 */
AI_INTERNAL_API
void forward_concat(ai_layer* layer);

/*!
 * @brief Gather an input tensor
 * @ingroup layers_generic
 * @param layer the gathered layer
 */
AI_INTERNAL_API
void forward_gather(ai_layer* layer);

/*!
 * @brief GatherND an input tensor
 * @ingroup layers_generic
 * @param layer the gathered layer (ND)
 */
AI_INTERNAL_API
void forward_gather_nd(ai_layer* layer);

/*!
 * @brief GatherND channel first an input tensor
 * @ingroup layers_generic
 * @param layer the gathered layer (ND)
 */
AI_INTERNAL_API
void forward_gather_nd_channel_first(ai_layer* layer);

/*!
 * @brief ScatterND an input tensor
 * @ingroup layers_generic
 * @param layer the scattered layer (ND)
 */
AI_INTERNAL_API
void forward_scatter_nd(ai_layer* layer);

/*!
 * @brief Slice an input tensors
 * @ingroup layers_generic
 * @param layer the sliced layer
 */
AI_INTERNAL_API
void forward_slice(ai_layer* layer);

/*!
 * @brief Tile an input tensors
 * @ingroup layers_generic
 * @param layer the tiled layer
 */
AI_INTERNAL_API
void forward_tile(ai_layer* layer);

/*!
 * @brief Returns the shape of an input tensors
 * @ingroup layers_generic
 * @param layer the Shape layer
 */
AI_INTERNAL_API
void forward_shape(ai_layer* layer);

/*!
 * @brief TopK an input tensors
 * @ingroup layers_generic
 * @param layer the Topked layer
 */
AI_INTERNAL_API
void forward_topK(ai_layer* layer);

/*!
 * @brief Pad an input tensors
 * @ingroup layers_generic
 * @param layer the pad layer
 */
AI_INTERNAL_API
void forward_pad(ai_layer* layer);

/*!
 * @brief ConstantofShape an input tensors
 * @ingroup layers_generic
 * @param layer the constantofshape layer
 */
AI_INTERNAL_API
void forward_constantofshape(ai_layer* layer);

/*!
 * @brief Upsample an input tensors
 * @ingroup layers_generic
 * @param layer the upsampled layer
 */
AI_INTERNAL_API
void forward_upsample(ai_layer* layer);

/*!
 * @brief Resize an input tensors
 * @ingroup layers_generic
 * @param layer the resized layer
 */
AI_INTERNAL_API
void forward_resize(ai_layer* layer);

/*!
 * @brief Instance Normalization on an input tensors
 * @ingroup layers_generic
 * @param layer the instance normalization layer
 */
AI_INTERNAL_API
void forward_instanceNormalization(ai_layer* layer);

/*!
 * @brief Apply an elementwise transformation to the input tensors
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_eltwise(ai_layer* layer);

/*!
 * @brief Apply an elementwise transformation to the integer input tensors
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_eltwise_integer(ai_layer* layer);

/*!
 * @brief Apply an elementwise transformation to the signed integer input tensors
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_eltwise_integer_INT8(ai_layer* layer);

/*!
 * @brief Apply an elementwise transformation to the unsigned integer input tensors
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_eltwise_integer_UINT8(ai_layer* layer);

/*!
 * @brief Apply a reduce transformation to the input tensors
 * @ingroup layers_generic
 * @param layer the reduce layer
 */
AI_INTERNAL_API
void forward_reduce(ai_layer* layer);

/*!
 * @brief Apply a reduce transformation to the input tensors
 * @ingroup layers_generic
 * @param layer the reduce layer
 */
AI_INTERNAL_API
void forward_reduce_log_sum_exp(ai_layer* layer);

/*!
 * @brief Apply a reduce transformation to the input tensors
 * @ingroup layers_generic
 * @param layer the reduce layer
 */
AI_INTERNAL_API
void forward_reduce_l1(ai_layer* layer);

/*!
 * @brief Apply a reduce transformation to the input tensors
 * @ingroup layers_generic
 * @param layer the reduce layer
 */
AI_INTERNAL_API
void forward_reduce_l2(ai_layer* layer);


/*!
 * @brief Behave like numpy.where with Numpy-style broadcasting support
 * @ingroup layers_generic
 * @param layer the where layer
 */
AI_INTERNAL_API
void forward_where(ai_layer* layer);

/*!
 * @brief Apply an elementwise addition to the input tensors
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_add_integer(ai_layer* layer);

/*!
 * @brief Apply an elementwise addition to the input tensors
 *        with int8 I/O
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_add_integer_INT8(ai_layer* layer);

/*!
 * @brief Apply an elementwise addition to the input tensors
 *        with uint8 I/O
 * @ingroup layers_generic
 * @param layer the elementwise layer
 */
AI_INTERNAL_API
void forward_add_integer_UINT8(ai_layer* layer);


/*!
 * @brief Reverse layer.
 * @ingroup layers_generic
 * @param layer reverse layer
 */
AI_INTERNAL_API
void forward_reverse(ai_layer *pLayer);


/*!
 * @brief Upsample an input tensors with unsigned 8-bit integer input,.
 *        It is to be used also for other formats, since the function only
 *        performs memory copy.
 * @ingroup layers_generic
 * @param layer the upsampled layer
 */
AI_INTERNAL_API
void forward_upsample_generic(ai_layer* layer);


AI_API_DECLARE_END

#endif    /*LAYERS_GENERIC_H*/
