#ifndef CORE_COMMON_H
#define CORE_COMMON_H
/**
  ******************************************************************************
  * @file    core_common.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of common core datatypes
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
#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "core_datatypes.h"

/*!
 * @defgroup core_common Common Core Library Routines
 * @brief Common macros, datatypes and routines of core common module
 * @details This module contains the definitons and handling of the @ref ai_node
 * datastructures. An ai_node is a generic abstraction for a network node that
 * could be either a fixed function layer or an operator. Ideally the platform
 * interface defined in api module should handle an process generic nodes in the
 * network, not relying on the fact that they are layers or operators datastructs
 * Specific implementative details should be kept inside layers and operators
 * modules. The core module implements additionally common routines used in the
 * layers and operators modules.
 */

/******************************************************************************/
#ifdef HAS_AI_ASSERT
  #define ASSERT_ARRAY_SANITY(a_) \
    AI_ASSERT((a_) && (a_)->size>0)

  #define ASSERT_ARRAY_DATA_SANITY(a_) \
    ASSERT_ARRAY_SANITY(a_) \
    AI_ASSERT((a_)->data && (a_)->data_start)

  #define ASSERT_TENSOR_SANITY(t_) \
    AI_ASSERT((t_) && (t_)->data) \
    AI_ASSERT(CORE_TENSOR_GET_SHAPE_SIZE(t_)>0) \
    ASSERT_ARRAY_SANITY((t_)->data)

  #define ASSERT_TENSOR_LIST_SANITY(tlist_) \
    AI_ASSERT((tlist_) && (GET_TENSOR_LIST_SIZE(tlist_)>0)) \

  #define ASSERT_TENSOR_DATA_SANITY(t_) \
    ASSERT_TENSOR_SANITY(t_) \
    ASSERT_ARRAY_DATA_SANITY((t_)->data)

  #define ASSERT_NODE_SANITY(node_) \
    do { \
      AI_ASSERT(AI_NODE_OBJ(node_)->tensors && AI_NODE_OBJ(node_)->tensors->chain) \
      ASSERT_TENSOR_SANITY(GET_TENSOR_IN(AI_NODE_OBJ(node_)->tensors, 0)) \
      ASSERT_TENSOR_SANITY(GET_TENSOR_OUT(AI_NODE_OBJ(node_)->tensors, 0)) \
    } while (0);
#else
  #define ASSERT_ARRAY_SANITY(a_)            /* ASSERT_ARRAY_SANITY */
  #define ASSERT_ARRAY_DATA_SANITY(a_)       /* ASSERT_ARRAY_DATA_SANITY */
  #define ASSERT_TENSOR_SANITY(t_)           /* ASSERT_TENSOR_SANITY */
  #define ASSERT_TENSOR_LIST_SANITY(tlist_)  /* ASSERT_TENSOR_LIST_SANITY */
  #define ASSERT_TENSOR_DATA_SANITY(t_)      /* ASSERT_TENSOR_DATA_SANITY */
  #define ASSERT_NODE_SANITY(node_)          /* ASSERT_NODE_SANITY */
#endif  /*HAS_AI_ASSERT*/


#if defined(__GNUC__) || defined(__clang__)
  /* Suppress unused function warnings */
  #define AI_UNUSED_FUNCTION __attribute__((unused))
  /* Manage false positives in address sanitizer */
  #define AI_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#else
  #define AI_UNUSED_FUNCTION             /* AI_UNUSED_FUNCTION */
  #define AI_NO_SANITIZE_ADDRESS         /* AI_NO_SANITIZE_ADDRESS */
#endif


/******************************************************************************/
#define AI_NODE_TYPE(type_)  \
  ((ai_node_type)((ai_u32)(type_)&0xFFFF))

#define AI_NODE_OBJ(obj_) \
  ((ai_node*)(obj_))

#define AI_NODE_FUNC(func_) \
  ((node_func)(func_))

#define AI_NODE_COMMON_FIELDS_DECLARE \
  ai_node_type type;              /*!< node type id (see @ref ai_node_type) */ \
  ai_id_obj id;                   /*!< node object instance id (see @ref ai_id_obj) */ \
  ai_flags flags;                 /*!< node object flags */ \
  ai_klass_obj klass;             /*!< opaque handler to specific layer implementations */ \
  ai_network* network;            /*!< handle to global network context */ \
  struct ai_node_s* next;         /*!< the next node object in the sequence */ \
  node_func forward;              /*!< forward function for the node */ \
  AI_CONST ai_tensor_chain* tensors; /*!< pointer to node tensor chain */

#define AI_NODE_STATEFUL_FIELDS_DECLARE \
  AI_NODE_COMMON_FIELDS_DECLARE \
  ai_handle         state; \
  node_func         init; \
  node_func         update; \
  node_func         destroy;

#define AI_NODE_COMMON_INIT(type_, id_, flags_, klass_, network_, next_, forward_) \
  .type = AI_NODE_TYPE(type_), \
  .id   = AI_ID_OBJ(id_), \
  .flags = (flags_), \
  .klass = AI_KLASS_OBJ(klass_), \
  .network = AI_NETWORK_OBJ(network_), \
  .next = AI_NODE_OBJ(next_), \
  .forward = AI_NODE_FUNC(forward_)

/*****************************************************************************/
/** Network Tensors Chains / Lists Handlers                                 **/
/*****************************************************************************/
#define AI_FOR_EACH_TENSOR_CHAIN_DO(tlist_ptr_, chain_) \
  ai_tensor_list* tlist_ptr_ = (chain_)->chain; \
  for (; tlist_ptr_<(((chain_)->chain)+((chain_)->size)); tlist_ptr_++)

#define AI_FOR_EACH_TENSOR_LIST_DO(idx_, t_ptr_, tlist_ptr_) \
  ai_tensor* t_ptr_ = NULL; \
  for (ai_size idx_ = 0; (idx_ < GET_TENSOR_LIST_SIZE(tlist_ptr_)) && \
    ((t_ptr_ = GET_TENSOR_LIST_ITEM(tlist_ptr_, idx_)) != NULL); ++idx_)


#define GET_TENSOR_LIST_INFO(list_) \
  ((list_)->info)

#define GET_TENSOR_LIST_META(list_, pos_) \
  (&(GET_TENSOR_LIST_INFO(list_)->meta[pos_]))

#define GET_TENSOR_LIST_STATE(list_, pos_) \
  (&(GET_TENSOR_LIST_INFO(list_)->state[pos_]))

#define GET_TENSOR_LIST_BUFFER(list_, pos_) \
  (&(GET_TENSOR_LIST_INFO(list_)->buffer[pos_]))

#define GET_TENSOR_LIST_ITEM(list_, pos_) \
  ((NULL!=GET_TENSOR_LIST_ITEMS(list_)) \
   ? GET_TENSOR_LIST_ITEMS(list_)[(pos_)] : NULL)

#define GET_TENSOR_LIST_ITEMS(list_) \
  ((list_)->tensor)

#define GET_TENSOR_LIST_SIZE(list_) \
  ((NULL!=(list_)) ? (list_)->size : 0)

#define GET_TENSOR_CHAIN_SIZE(chain_) \
  ((NULL!=(chain_)) ? (chain_)->size : 0)

#define GET_TENSOR_LIST(chain_, type_) \
  ((AI_CONCAT(AI_TENSOR_CHAIN_, type_)<(chain_)->size) \
     ? &(chain_)->chain[AI_CONCAT(AI_TENSOR_CHAIN_, type_)] : NULL)

#define GET_TENSOR_LIST_IN(chain_) \
  (GET_TENSOR_LIST(chain_, INPUT))

#define GET_TENSOR_LIST_OUT(chain_) \
  (GET_TENSOR_LIST(chain_, OUTPUT))

#define GET_TENSOR_LIST_WEIGTHS(chain_) \
  (GET_TENSOR_LIST(chain_, WEIGHTS))

#define GET_TENSOR_LIST_SCRATCH(chain_) \
  (GET_TENSOR_LIST(chain_, SCRATCH))

#define GET_TENSOR_IN(chain_, pos_) \
  (GET_TENSOR_LIST_ITEM(GET_TENSOR_LIST_IN(chain_), (pos_)))

#define GET_TENSOR_OUT(chain_, pos_) \
  (GET_TENSOR_LIST_ITEM(GET_TENSOR_LIST_OUT(chain_), (pos_)))

#define GET_TENSOR_WEIGHTS(chain_, pos_) \
  (GET_TENSOR_LIST_ITEM(GET_TENSOR_LIST_WEIGTHS(chain_), (pos_)))

#define GET_TENSOR_SCRATCH(chain_, pos_) \
  (GET_TENSOR_LIST_ITEM(GET_TENSOR_LIST_SCRATCH(chain_), (pos_)))

/******************************************************************************/

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_node_type
 * @ingroup core_common
 * @brief generic network node numeric type ID
 *
 */
typedef uint16_t ai_node_type;

/*!
 * @typedef void (*node_func)(struct ai_node_s* node)
 * @ingroup core_common
 * @brief Callback signatures for all forward functions
 */
typedef void (*node_func)(struct ai_node_s* node);

/*!
 * @typedef ai_float (*func_nl_el)(const ai_float x)
 * @ingroup core_common
 * @brief Fuction pointer for generic elementwise transforms
 *
 * This function pointer abstracts a generic nonlinear function applied to a
 * single element. See @ref ai_math_sqrt in @ref math_helpers as examples.
 */
typedef ai_float (*func_nl_el)(const ai_float x);

/*!
 * @struct ai_node
 * @ingroup core_common
 * @brief Structure encoding a generic node of the network
 *
 * The node struct includes information about the network it belong to, the
 * next node in a sequential network and the forward function. The forward
 * functions are implemented in the @ref layers module.
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_node_s {
  AI_NODE_COMMON_FIELDS_DECLARE
} ai_node;

/*!
 * @struct ai_node_stateful
 * @ingroup core_common
 * @brief Structure encoding a stateful node of the network
 *
 * The node struct includes information about the network it belong to, the
 * next node in a sequential network and the init, update and forward functions.
 * The node functions are implemented in the @ref layers module.
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_node_stateful_s {
  AI_NODE_STATEFUL_FIELDS_DECLARE
} ai_node_stateful;

/*!
 * @brief initialize core module
 * @ingroup core_common
 * @return false if initialization fails, false otherwise
 */
AI_INTERNAL_API
ai_bool core_init(void);

/*!
 * @brief get 1st error raised during processing
 * @ingroup core_common
 * @param[out] error the @ref ai_error recorded during processing
 * @return the 1st error generated during processing. If no errors AI_ERROR_NONE
 */
AI_INTERNAL_API
ai_error core_get_error(ai_error* error);

/*!
 * @brief set error recorded during processing
 * @ingroup core_common
 * @param[out] error the @ref ai_error to set
 * @param[in] type the specific error type to set
 * @param[in] code the specific error code to set
 * @return true if the error is set, false in case a precedent error was already
 */
AI_INTERNAL_API
ai_bool core_set_error(
  ai_error* error, const ai_error_type type, const ai_error_code code);

AI_API_DECLARE_END

#endif    /*CORE_COMMON_H*/
