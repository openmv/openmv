/**
  ******************************************************************************
  * @file    ai_layer_custom_interface.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of AI platform custom layers interface APIs
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef AI_LAYER_CUSTOM_INTERFACE_H
#define AI_LAYER_CUSTOM_INTERFACE_H

#include "ai_platform.h"
#include "ai_platform_interface.h"

#include "layers_custom.h"

#define INTQ_SCALE_FLOAT      (AI_BUFFER_META_FLAG_SCALE_FLOAT)
#define INTQ_ZEROPOINT_U8     (AI_BUFFER_META_FLAG_ZEROPOINT_U8)
#define INTQ_ZEROPOINT_S8     (AI_BUFFER_META_FLAG_ZEROPOINT_S8)
#define INTQ_ZEROPOINT_U16     (AI_BUFFER_META_FLAG_ZEROPOINT_U16)
#define INTQ_ZEROPOINT_S16     (AI_BUFFER_META_FLAG_ZEROPOINT_S16)

#define AI_TENSOR_HEIGHT      (3)
#define AI_TENSOR_WIDTH       (2)
#define AI_TENSOR_CHANNEL     (1)
#define AI_TENSOR_IN_CHANNEL  (0)


AI_API_DECLARE_BEGIN

typedef enum {
  TYPE_NONE = 0x0,
  TYPE_FLOAT,
  TYPE_BOOL,
  TYPE_INTEGER,
  TYPE_SIGNED,
  TYPE_UNSIGNED,
} ai_tensor_type;

typedef struct {
  ai_tensor_type      type;
  ai_i8               bits;
  ai_i8               fbits;
} ai_tensor_format;

typedef struct {
  ai_u16          flags;  /*!< optional flags to store intq info attributes */
  ai_u16          size;   /*!< number of elements in the the intq_info list  */
  ai_float*       scale;  /*!< array of scales factors */
  union {
    ai_u8*        zeropoint_u8;  /*!< array of zeropoints as unsigned */
    ai_i8*        zeropoint_s8;  /*!< array of zeropoints as signed */
  };
} ai_tensor_intq_info;


/****************************************************************************
 ** Layer Custom Interface APIs
 ****************************************************************************/
/*!
 * @brief acquire the custom layer from its handle
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the custom layer
 * @return a pointer to ai_layer_custom if found and valid, else NULL
 */
AI_INTERFACE_TYPE
ai_layer_custom* ai_layer_custom_get(
  ai_layer* layer);

/*!
 * @brief release the custom layer provided its handle
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the custom layer to release
 */
AI_INTERFACE_TYPE
void ai_layer_custom_release(
  ai_layer* layer);

/*!
 * @brief get the number of inputs tensors of a custom layer
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the custom layer
 * @return the number of input tensors of the layer. 0 if no input tensors or error
 */
AI_INTERFACE_TYPE
ai_size ai_layer_get_tensor_in_size(
  const ai_layer* layer);

/*!
 * @brief get the number of outputs tensors of a custom layer
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the custom layer
 * @return the number of outputs tensors of the layer. 0 if no outputs tensors or error
 */
AI_INTERFACE_TYPE
ai_size ai_layer_get_tensor_out_size(
  const ai_layer* layer);


/*!
 * @brief get the number of weights tensors of a custom layer
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the custom layer
 * @return the number of weights tensors of the layer. 0 if no weights tensors or error
 */
AI_INTERFACE_TYPE
ai_size ai_layer_get_tensor_weights_size(
  const ai_layer* layer);


/*!
 * @brief get the n-th (at index pos) input tensor pointer from a layer
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the layer
 * @param pos the index position in the tensor list
 * @return a pointer to a tensor if found, else, if invalid or out-of-range NULL
 */
AI_INTERFACE_TYPE
ai_tensor* ai_layer_get_tensor_in(
  const ai_layer* layer, const ai_u16 pos);

/*!
 * @brief get the n-th (at index pos) output tensor pointer from a layer
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the layer
 * @param pos the index position in the tensor list
 * @return a pointer to a tensor if found, else, if invalid or out-of-range NULL
 */
AI_INTERFACE_TYPE
ai_tensor* ai_layer_get_tensor_out(
  const ai_layer* layer, const ai_u16 pos);


/*!
 * @brief get the n-th (at index pos) weight tensor pointer from a layer
 * @ingroup ai_layer_custom_interface
 * @param layer an opaque handler to the layer
 * @param pos the index position in the tensor list
 * @return a pointer to a tensor if found, else, if invalid or out-of-range NULL
 */
AI_INTERFACE_TYPE
ai_tensor* ai_layer_get_tensor_weights(
  const ai_layer* layer, const ai_u16 pos);


/****  Layer Tensors APIs  ***************************************************/
/*!
 * @brief check if the tensor has integer quantization informations @ref ai_tensor_intq_info
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return true if tensot has integer quantization informations, false otherwise
 */
AI_INTERFACE_TYPE
ai_bool ai_tensor_has_intq(
  const ai_tensor* t);

/*!
 * @brief get the tensor integer quantization informations @ref ai_tensor_intq_info
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the integer quantization informations as a struct @ref ai_tensor_intq_info
 */
AI_INTERFACE_TYPE
ai_tensor_intq_info ai_tensor_get_intq(
  const ai_tensor* t);

/*!
 * @brief get the format of the tensor see @ref ai_tensor_format
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the tensor format
 */
AI_INTERFACE_TYPE
ai_tensor_format ai_tensor_get_format(
  const ai_tensor* t);

/****  Shapes Getters  ****/
/*!
 * @brief get the dimensionality of the tensor shapes
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the dimensionality of the tensor shape
 */
AI_INTERFACE_TYPE
ai_size ai_tensor_get_shape_size(
  const ai_tensor* t);

/*!
 * @brief get the value of the shape dimensionality pos
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the value of the shape dimensionality at pos of the tensor
 */
AI_INTERFACE_TYPE
ai_shape_dimension ai_tensor_get_shape(
  const ai_tensor* t, const ai_u16 pos);

/****  Strides Getters  ****/
/*!
 * @brief get the dimensionality of the tensor strides
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the dimensionality of the tensor strides @ref ai_stride
 */
AI_INTERFACE_TYPE
ai_size ai_tensor_get_stride_size(
  const ai_tensor* t);

/*!
 * @brief get the value of the stride dimensionality pos
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the value of the stride dimensionality at pos of the tensor
 */
AI_INTERFACE_TYPE
ai_stride_dimension ai_tensor_get_stride(
  const ai_tensor* t, const ai_u16 pos);

/****  Data Storage Getters  ****/
/*!
 * @brief get tensor storage data buffer pointer
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return a pointer to the tensor data buffer, set to NULL if error
 */
AI_INTERFACE_TYPE
ai_any_ptr ai_tensor_get_data(
  const ai_tensor* t);

/*!
 * @brief get number of tensor elements
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the number of tensor elements or 0 if error
 */
AI_INTERFACE_TYPE
ai_size ai_tensor_get_data_size(
  const ai_tensor* t);

/*!
 * @brief get the size in bytes of the tensor data buffer
 * @ingroup ai_layer_custom_interface
 * @param tensor a pointer to the tensor
 * @return the size in bytes of the tensor data buffer. 0 if error
 */
AI_INTERFACE_TYPE
ai_size ai_tensor_get_data_byte_size(
  const ai_tensor* t);


AI_API_DECLARE_END

#endif    /*AI_LAYER_CUSTOM_INTERFACE_H*/
