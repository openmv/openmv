/**
  ******************************************************************************
  * @file    core_private.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   private header file of common private core private module defines
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef CORE_PRIVATE_H
#define CORE_PRIVATE_H

#include "ai_datatypes_format.h"
#include "ai_datatypes_internal.h"
#include "ai_math_helpers.h"

#include "core_assert.h"
#include "core_log.h"

/*!
 * @defgroup core_private Core Library Private macros and datatypes
 * @brief Common macros, datatypes and routines for core private rounites
 * @details This module contains the definitons and implementations of some
 * internal routines and datatypes that are supposed to not be exposed as
 * public headers. So usually this file should be include only on .c files or
 * headers that are private as well
 */

/***   Foreground Colors   ****************************************************/
#define CORE_COLOR_BLACK            "\x1b[30m"
#define CORE_COLOR_RED              "\x1b[31m"
#define CORE_COLOR_GREEN            "\x1b[32m"
#define CORE_COLOR_YELLOW           "\x1b[33m"
#define CORE_COLOR_BLUE             "\x1b[94m"
#define CORE_COLOR_MAGENTA          "\x1b[35m"
#define CORE_COLOR_CYAN             "\x1b[36m"
#define CORE_COLOR_WHYTE            "\x1b[37m"
#define CORE_COLOR_DEFAULT          "\x1b[39m"
#define CORE_COLOR_LGRAY            "\x1b[90m"
#define CORE_COLOR_LRED             "\x1b[91m"
#define CORE_COLOR_LGREEN           "\x1b[92m"
#define CORE_COLOR_LYELLOW          "\x1b[93m"
#define CORE_COLOR_LBLUE            "\x1b[94m"
#define CORE_COLOR_LMAGENTA         "\x1b[95m"
#define CORE_COLOR_LCYAN            "\x1b[96m"
#define CORE_COLOR_LWHITE           "\x1b[97m"

/***    Text Attributes  Colors   *********************************************/
#define CORE_COLOR_OFF              "\x1b[0m"
#define CORE_COLOR_BOLD             "\x1b[1m"
#define CORE_COLOR_UNDERLINE        "\x1b[4m"
#define CORE_COLOR_BLINK            "\x1b[5m"
#define CORE_COLOR_BOLD_OFF         "\x1b[21m"
#define CORE_COLOR_UNDERLINE_OFF    "\x1b[24m"
#define CORE_COLOR_BLINK_OFF        "\x1b[25m"

/***   Background Colors   ****************************************************/
#define CORE_COLOR_BG_BLACK         "\x1b[40m"
#define CORE_COLOR_BG_RED           "\x1b[41m"
#define CORE_COLOR_BG_GREEN         "\x1b[42m"
#define CORE_COLOR_BG_YELLOW        "\x1b[43m"
#define CORE_COLOR_BG_BLUE          "\x1b[44m"
#define CORE_COLOR_BG_MAGENTA       "\x1b[45m"
#define CORE_COLOR_BG_CYAN          "\x1b[46m"
#define CORE_COLOR_BG_WHITE         "\x1b[47m"
#define CORE_COLOR_BG_DEFAULT       "\x1b[49m"
#define CORE_COLOR_BG_LGRAY         "\x1b[100m"
#define CORE_COLOR_BG_LRED          "\x1b[101m"
#define CORE_COLOR_BG_LGREEN        "\x1b[102m"
#define CORE_COLOR_BG_LYELLOW       "\x1b[103m"
#define CORE_COLOR_BG_LBLUE         "\x1b[104m"
#define CORE_COLOR_BG_LMAGENTA      "\x1b[105m"
#define CORE_COLOR_BG_LCYAN         "\x1b[106m"
#define CORE_COLOR_BG_LWHITE        "\x1b[107m"


/*****************************************************************************/
#define CORE_ADDRESS_RANGE_INIT(start_, end_) \
  core_address_range_init(start_, end_)

#define CORE_GET_BUFFER_META_INFO(meta_info_, tensor_ptr_) \
  core_get_buffer_meta_info(meta_info_, tensor_ptr_)

#define CORE_ADDRESS_RANGE_END(range_) \
  ( (ai_ptr)(((range_)->start)+((range_)->size)) )

#define CORE_ADDRESS_RANGE_OVERLAP(overlap_) \
  ( ((overlap_)->start) && (((overlap_)->size)>0) )

#define CORE_ADDRESS_RANGE_OVERLAP_PARTIAL(overlap_, ref_) \
  ( ((overlap_)->start) && (((overlap_)->size)<((ref_)->size)) )

#define CORE_MEMORY_OVERLAP_INIT(partial_, range_, chain_id_, tensor_id_) { \
  .partial = (partial_), .range = AI_PACK(range_), \
  .chain_id = (chain_id_), .tensor_id = (tensor_id_) \
}

#define CORE_OFFSET(offset_, max_) \
  ((ai_i32)(((offset_)<0) ? AI_MAX((max_) - (offset_), 0) : AI_MIN(offset_, max_)))

/*****************************************************************************/
/** Network Context Handlers                                                **/
/*****************************************************************************/

/*****************************************************************************/
/** Network Tensors Handlers                                                **/
/*****************************************************************************/
#define AI_TENSOR_HAS_INTQ_INFO \
  AI_BUFFER_META_HAS_INTQ_INFO

#define CORE_TENSOR_GET_SHAPE_SIZE(tensor_) \
  ai_shape_get_size(AI_TENSOR_SHAPE(tensor_))


#define CORE_ASSERT_SHAPE_MATCH(x, y) \
do { \
  AI_ASSERT(AI_SHAPE_H(y) == 1 || AI_SHAPE_H(x)==1 || AI_SHAPE_H(y)==AI_SHAPE_H(x)) \
  AI_ASSERT(AI_SHAPE_W(y) == 1 || AI_SHAPE_W(x)==1 || AI_SHAPE_W(y)==AI_SHAPE_W(x)) \
  AI_ASSERT(AI_SHAPE_D(y) == 1 || AI_SHAPE_D(x)==1 || AI_SHAPE_D(y)==AI_SHAPE_D(x)) \
  AI_ASSERT(AI_SHAPE_E(y) == 1 || AI_SHAPE_E(x)==1 || AI_SHAPE_E(y)==AI_SHAPE_E(x)) \
  AI_ASSERT(AI_SHAPE_CH(y) == 1 || AI_SHAPE_CH(x)==1|| AI_SHAPE_CH(y)==AI_SHAPE_CH(x)) \
  AI_ASSERT(AI_SHAPE_IN_CH(y) == 1 || AI_SHAPE_IN_CH(x)==1|| AI_SHAPE_IN_CH(y)==AI_SHAPE_IN_CH(x)) \
} while(0);


#define AI_TENSOR_ARRAY_BYTE_SIZE(t_) \
    AI_ARRAY_OBJ_BYTE_SIZE(AI_ARRAY_OBJ(t_->data))

#define AI_TENSOR_ARRAY_GET_DATA_ADDR(t_) \
    AI_HANDLE_PTR(AI_ARRAY_OBJ_DATA_START(t_->data, void))

#define AI_TENSOR_ARRAY_UPDATE_DATA_ADDR(t_, addr_) \
    { ai_array *arr_ = AI_ARRAY_OBJ(t_->data); \
      const uintptr_t off_ = (uintptr_t)arr_->data - (uintptr_t)arr_->data_start; \
      arr_->data_start = AI_PTR(addr_); \
      arr_->data = AI_PTR((uintptr_t)addr_ + off_); \
    }

#define AI_TENSOR_INTEGER_GET_SIZE(t_) \
    ((t_->klass) ? (AI_KLASS_GET_INTQ_INFO_LIST(t_))->size : 0)

#define AI_TENSOR_INTEGER_GET_SCALE(t_, idx_) \
    AI_INTQ_INFO_LIST_SCALE(AI_KLASS_GET_INTQ_INFO_LIST(t_), ai_float, idx_)

#define AI_TENSOR_INTEGER_GET_ZEROPOINT_I8(t_, idx_) \
    AI_INTQ_INFO_LIST_ZEROPOINT(AI_KLASS_GET_INTQ_INFO_LIST(t_), ai_i8, idx_)

#define AI_TENSOR_INTEGER_GET_ZEROPOINT_U8(t_, idx_) \
    AI_INTQ_INFO_LIST_ZEROPOINT(AI_KLASS_GET_INTQ_INFO_LIST(t_), ai_u8, idx_)

#define AI_TENSOR_FMT_GET_SIGN(t_) \
    AI_BUFFER_FMT_GET_SIGN(AI_ARRAY_OBJ(t_->data)->format)

#define AI_TENSOR_FMT_GET_BITS(t_) \
    AI_BUFFER_FMT_GET_BITS(AI_ARRAY_OBJ(t_->data)->format)

#define AI_TENSOR_FMT_GET_FBITS(t_) \
    AI_BUFFER_FMT_GET_FBITS(AI_ARRAY_OBJ(t_->data)->format)

#define AI_TENSOR_FMT_GET_TYPE(t_) \
    AI_BUFFER_FMT_GET_TYPE(AI_ARRAY_OBJ(t_->data)->format)

#define AI_TENSOR_GET_FMT(t_) \
    AI_FMT_OBJ(AI_ARRAY_OBJ(t_->data)->format)


/*****************************************************************************/
/** Network Buffers Handlers                                                 **/
/*****************************************************************************/
#define AI_FOR_EACH_BUFFER_ARRAY_ITEM(buffer_ptr_, buffer_array_ptr_, start_pos_, end_pos_) \
  ai_buffer* buffer_ptr_ = AI_BUFFER_ARRAY_ITEM(buffer_array_ptr_, \
                                                CORE_OFFSET(end_pos_, AI_BUFFER_ARRAY_SIZE(buffer_array_ptr_))); \
  for ( ; buffer_ptr_ && AI_BUFFER_ARRAY_SIZE(buffer_array_ptr_) && \
          (buffer_ptr_>=AI_BUFFER_ARRAY_ITEM(buffer_array_ptr_, \
                          CORE_OFFSET(start_pos_, AI_BUFFER_ARRAY_SIZE(buffer_array_ptr_)))); buffer_ptr_--)

/*****************************************************************************/
/** Network Arrays Handlers                                                 **/
/*****************************************************************************/
#define AI_ARRAY_OBJ_FMT(array_) \
  AI_FMT_OBJ(AI_ARRAY_OBJ(array_)->format)

#define AI_ARRAY_OBJ_FMT_GET(array_) \
  AI_FMT_GET(AI_ARRAY_OBJ_FMT(array_))

#define AI_ARRAY_OBJ_SIZE(array_) \
  (AI_ARRAY_OBJ(array_)->size)

#define AI_ARRAY_OBJ_BYTE_SIZE(array_) \
  AI_SIZE(AI_ARRAY_GET_BYTE_SIZE(AI_ARRAY_OBJ_FMT(array_), \
                         AI_ARRAY_OBJ_SIZE(array_)))

#define AI_ARRAY_OBJ_DATA_SIZE(array_) \
  AI_ARRAY_GET_DATA_BYTE_SIZE(AI_ARRAY_OBJ_FMT(array_), \
                              AI_ARRAY_OBJ_SIZE(array_))

#define AI_ARRAY_OBJ_DATA(array_, type_) \
  AI_CAST(type_*, AI_ARRAY_OBJ(array_)->data)

#define AI_ARRAY_OBJ_DATA_START(array_, type_) \
  AI_CAST(type_*, AI_ARRAY_OBJ(array_)->data_start)

#define AI_ARRAY_OBJ_ELEM(array_, type_, pos_) \
  AI_ARRAY_OBJ_DATA(array_, type_)[(pos_)]


/*****************************************************************************/
/** Network Tensors Chains / Lists Handlers                                 **/
/*****************************************************************************/
#define SET_TENSOR_IN(chain_, pos_) \
  (GET_TENSOR_LIST_IN(chain_)->tensor[(pos_)])

#define SET_TENSOR_OUT(chain_, pos_) \
  (GET_TENSOR_LIST_OUT(chain_)->tensor[(pos_)])

#define AI_NODE_IO_GET(node_, in_, out_) \
  ASSERT_NODE_SANITY(node_) \
  ai_tensor* in_  = GET_TENSOR_IN((node_)->tensors, 0); \
  ai_tensor* out_ = GET_TENSOR_OUT((node_)->tensors, 0); \
  ASSERT_TENSOR_SANITY(in_) \
  ASSERT_TENSOR_SANITY(out_)


/*****************************************************************************/
#define AI_BITS_TO_BYTES(bits_) \
  (((bits_)+0x7) >> 3)

#define AI_BYTES_TO_BITS(bytes_) \
  ((bytes_) << 3)


/*****************************************************************************/
/** Network Nodes Handlers                                                  **/
/*****************************************************************************/
#define AI_NODE_IS_FIRST(node) \
  (AI_NODE_OBJ(node)==AI_NODE_OBJ(AI_NODE_OBJ(node)->network->input_node))

#define AI_NODE_IS_LAST(node_) \
  ((AI_NODE_OBJ(node_)==AI_NODE_OBJ(node_)->next) || \
   (AI_NODE_OBJ(node_)->next==NULL))

#define AI_FOR_EACH_NODE_DO(node_, nodes_) \
  for (ai_node* node_ = AI_NODE_OBJ(nodes_); (node_); \
       node_ = ((AI_NODE_IS_LAST(node_)) ? NULL : (node_)->next))

/*****************************************************************************/
typedef struct {
  ai_ptr         start;
  ai_size        size;
} ai_address_range;

typedef struct {
  ai_address_range   range;
  ai_u16             chain_id;
  ai_u16             tensor_id;
  ai_bool            partial;
} ai_memory_overlap;

/*****************************************************************************/
AI_DECLARE_STATIC
ai_address_range core_address_range_init(
  const ai_handle start, const ai_handle end)
{
  ai_address_range r;

  r.start = (ai_ptr)((start<end) ? start : end);
  r.size  = (ai_size) ((start<end)
    ? ((ai_uptr)end-(ai_uptr)start) : ((ai_uptr)start-(ai_uptr)end));
  return r;
}

AI_DECLARE_STATIC
ai_buffer_meta_info* core_get_buffer_meta_info(
  ai_buffer_meta_info* meta,
  const ai_tensor* t)
{
  if (!meta) return NULL;
  AI_ASSERT(t && t->data)
  ai_bool ok;

  meta->flags     = 0x0;
  meta->intq_info = AI_KLASS_GET_INTQ_INFO_LIST(t);
  ok = (meta->intq_info && (meta->intq_info->size>0));
  meta->flags |= (ok) ? AI_BUFFER_META_HAS_INTQ_INFO : 0x0;
  return (ok) ? meta : NULL;
}


#if 0
#include <stdio.h>
#include <stdarg.h>

AI_DECLARE_STATIC
void _dump_file_print(
  const char* fname, const char* fmt, ...)
{
  static FILE* fp = NULL;
  if (fname) {
    if (!fp) {
      fp = fopen(fname, "a");
    }
  }

  if (fp) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    fflush(fp);
  }
}


AI_DECLARE_STATIC
void _dump_bytearray(
  const char* fname,
  const ai_handle src, const ai_size src_size, const ai_u8 src_id,
  const char* name)
{
  static FILE* fp = NULL;
  if (fname && src && (src_size>0)) {
    if (!fp) {
      fp = fopen(fname, "a");
    }
  }

  if (fp) {
    switch (src_id) {
      case 1:
      {
        const ai_float* src_value = (const ai_float*)src;
        fprintf(fp, "ai_float %s[%u] = {%f", name, src_size, src_value[0]);
        for (ai_size i=1; i<src_size; i++) { fprintf(fp, ", %f", src_value[i]); }
      } break;
      case 2:
      {
        const ai_i8* src_value = (const ai_i8*)src;
        fprintf(fp, "ai_i8 %s[%u] = {%d", name, src_size, src_value[0]);
        for (ai_size i=1; i<src_size; i++) { fprintf(fp, ", %d", src_value[i]); }
      } break;
      case 3:
      {
        const ai_u8* src_value = (const ai_u8*)src;
        fprintf(fp, "ai_u8 %s[%u] = {%u", name, src_size, src_value[0]);
        for (ai_size i=1; i<src_size; i++) { fprintf(fp, ", %u", src_value[i]); }
      } break;
      default:
        fprintf(fp, "format not supported: %u {", src_id);
        break;
    }
    fprintf(fp, "};\n");
    fflush(fp);
  }
}
#endif

#endif  /* CORE_PRIVATE_H */
