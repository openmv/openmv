/**
  ******************************************************************************
  * @file    core_log.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of core log interfaces
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
#ifndef CORE_LOG_H
#define CORE_LOG_H

#include "ai_platform.h"
#include "ai_datatypes_defines.h"

/*!
 * @defgroup core_log Logger core routines wrapper interface
 * @brief Common macros, datatypes and routines of ai logger module
 * @details This header defines the wrapping macros interfaces to handle the
 * global logger module. These macro are defined when the macro HAS_LOG is
 * defined, otherwise they are all set to NOP routines and no logger code is
 * compiled at all. When the macro HAS_LOG is defined, only the log messages
 * having an enum id >= the value of the macro are compiled. Thus to include in
 * compilation only log messages up to the error level the value of HAS_LOG must
 * be equal the the enum value of LOG_ERROR macro (i.e. 3). a value of 6 means
 * to include all log messages up to the lower LOG_TRACE level.
 */

#if defined HAS_LOG && (HAS_LOG>=0)
#include "ai_log.h"
  #define AI_LOG_SECTION(...) \
    { __VA_ARGS__ }

  #define AI_LOG_ACQUIRE() \
    ai_log_acquire()
  #define AI_LOG_SET_LEVEL(level_) \
    AI_WRAP_FUNC(ai_log_set_level(level_);)
  #define AI_LOG_SET_QUIET(onoff_) \
    AI_WRAP_FUNC(ai_log_set_quiet(onoff_);)
  #define AI_LOG_SET_LOCK_FN(fn_, udata_) \
    AI_WRAP_FUNC(ai_log_set_lock(fn_, udata_);)
  #define AI_LOG_CHANNEL_PUSH(level_, fn_, udata_) \
    AI_WRAP_FUNC(ai_log_channel_push(level_, fn_, udata_);)
  #define AI_LOG_CHANNEL_POP(fn_, udata_) \
    AI_WRAP_FUNC(ai_log_channel_pop(fn_, udata_);)
  #ifdef LOG_USE_FILE
    #define AI_LOG_SET_FILE_POINTER(fp_) \
      AI_WRAP_FUNC(ai_log_set_fp(fp_);)
  #else
    #define AI_LOG_SET_FILE_POINTER(fp_) \
      AI_WRAP_FUNC(/*AI_LOG_SET_FILE_POINTER()*/)
  #endif
#else
  #define AI_LOG_SECTION(...)                         AI_WRAP_FUNC(/*AI_LOG_SECTION()*/)

  #define AI_LOG_ACQUIRE()                            (NULL)
  #define AI_LOG_SET_LEVEL(level_)                    AI_WRAP_FUNC(/*AI_LOG_SET_LEVEL()*/)
  #define AI_LOG_SET_QUIET(onoff_)                    AI_WRAP_FUNC(/*AI_LOG_SET_QUIET()*/)
  #define AI_LOG_SET_LOCK_FN(fn_, udata_)             AI_WRAP_FUNC(/*AI_LOG_SET_LOCK_FN()*/)
  #define AI_LOG_CHANNEL_PUSH(level_, fn_, udata_)    AI_WRAP_FUNC(/*AI_LOG_CHANNEL_PUSH()*/)
  #define AI_LOG_CHANNEL_POP(fn_, udata_)             AI_WRAP_FUNC(/*AI_LOG_CHANNEL_POP()*/)
  #define AI_LOG_SET_FILE_POINTER(fp_)                AI_WRAP_FUNC(/*AI_LOG_SET_FILE_POINTER()*/)
#endif

#if defined HAS_LOG
  #define AI_LOG_PRINT(level, fmt, ...) \
    AI_WRAP_FUNC(ai_log_print(level, fmt, ##__VA_ARGS__);)
#else
  #define AI_LOG_PRINT(level, fmt, ...) \
    AI_WRAP_FUNC(/*AI_LOG_PRINT(...)*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_SUDO)
  #define AI_LOG_SUDO(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_SUDO, __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_SUDO(fmt, ...)         AI_WRAP_FUNC(/*AI_LOG_SUDO()*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_TRACE)
  #define AI_LOG_TRACE(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_TRACE, __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_TRACE(fmt, ...)        AI_WRAP_FUNC(/*AI_LOG_TRACE()*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_DEBUG)
  #define AI_LOG_DEBUG(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_DEBUG, __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_DEBUG(fmt, ...)        AI_WRAP_FUNC(/*AI_LOG_DEBUG()*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_INFO)
  #define AI_LOG_INFO(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_INFO,  __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_INFO(fmt, ...)         AI_WRAP_FUNC(/*AI_LOG_INFO()*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_WARN)
  #define AI_LOG_WARN(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_WARN,  __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_WARN(fmt, ...)         AI_WRAP_FUNC(/*AI_LOG_WARN()*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_ERROR)
  #define AI_LOG_ERROR(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_ERROR, __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_ERROR(fmt, ...)        AI_WRAP_FUNC(/*AI_LOG_ERROR()*/)
#endif

#if defined HAS_LOG && (HAS_LOG>=LOG_FATAL)
  #define AI_LOG_FATAL(fmt, ...) \
    AI_WRAP_FUNC(ai_log_log(LOG_FATAL, __FILE__, __LINE__, fmt LOG_CR, ##__VA_ARGS__);)
#else
  #define AI_LOG_FATAL(fmt, ...)        AI_WRAP_FUNC(/*AI_LOG_FATAL()*/)
#endif

#endif    /*CORE_LOG_H*/
