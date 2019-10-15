/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef AI_LOG_H_
#define AI_LOG_H_
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

/*!
 * @defgroup log Core logger class definition and implementation
 * @brief Data structures and defines used to implementlogger module
 * functionalities
 */

#define LOG_VERSION       "0.3.0"
#define LOG_CR            "\r\n"

/***** Compilation options: define/undef as required **************************/
#define LOG_USE_COLOR
/* #define LOG_INFO_SOURCE_CODE */

#ifndef HAS_STM32
#define LOG_USE_FILE
#define LOG_INFO_TIME
#define LOG_INFO_SOURCE_CODE_STRIP_FILE_PATHS   '/'
#else
#define LOG_INFO_SOURCE_CODE_STRIP_FILE_PATHS   '\\'
#endif

/******************************************************************************/
#define LOG_SUDO          (0x0)
#define LOG_FATAL         (0x1)
#define LOG_ERROR         (0x2)
#define LOG_WARN          (0x3)
#define LOG_INFO          (0x4)
#define LOG_DEBUG         (0x5)
#define LOG_TRACE         (0x6)

/*!
 * @typedef log_LockFn
 * @ingroup ai_log
 * @brief callback function for locking implementation (e.g. mutexes, etc.)
 */
typedef void (*log_LockFn)(const void *udata, const bool lock);

/*!
 * @typedef log_MsgFn
 * @ingroup ai_log
 * @brief callback for listening at logged channels
 */
typedef void (*log_MsgFn)(
  const void *udata, const uint8_t level, 
  const char* msg, const uint32_t len);

/*!
 * @brief Get gloabal log context handle
 * @ingroup ai_log
 */
void* ai_log_acquire(void);

/*!
 * @brief Set global log level
 * @ingroup ai_log
 */
void ai_log_set_level(const uint8_t level);

/*!
 * @brief Set global log quiet mode (no messages are emitted)
 * @ingroup ai_log
 */
void ai_log_set_quiet(const bool enable);

/*!
 * @brief Set callback for log messages locking
 * @ingroup ai_log
 */
void ai_log_set_lock(log_LockFn fn, const void *udata);

/*!
 * @brief Push on log stack a new listener with given log level
 * @ingroup ai_log
 * @param[in] level the log level for this channel
 * @param[out] the callback function to emit when a message is available
 * @param[in] udata a pointer to the caller environment that is provided back 
 * when the callback is called
 * @return 0 if OK, value>0 that indicates the current size of the stack
 */
uint8_t ai_log_channel_push(const uint8_t level, log_MsgFn fn, const void *udata);

/*!
 * @brief Pop from log stack a pushed listener
 * @ingroup ai_log
 * @param[in] the callback function registered during @ref log_channel_push
 * @param[in] udata a pointer to the caller environment registered during @ref 
 * log_channel_push
 * @return 0 if OK, value>0 that indicates the max size of the callback stack
 */
uint8_t ai_log_channel_pop(log_MsgFn fn, const void *udata);

#ifdef LOG_USE_FILE
/*!
 * @brief Enable file dumping of all logged messages to a file as well. 
 * @details NB: the quiet option does not apply to file logging. file log 
 * messages are recorded also when the log is in quiet mode.
 * @ingroup ai_log
 * @param[out] fp the file pointer of the file used to log the massages
 */
void ai_log_set_fp(FILE *fp);
#endif

/*!
 * @brief Main Routine: PLEASE invoke always by using defined macros
 * @ingroup ai_log
 * @param[in] level the log level of the input message
 * @param[in] file the string containing the __FILE__ info about the source file
 * generating the message to log
 * @param[in] fmt the varargs format of the string to print 
 */
void ai_log_log(const uint8_t level, const char *file,
  const int line, const char *fmt, ...);

#endif    /*AI_LOG_H_*/
