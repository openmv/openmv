/**
 ******************************************************************************
 * @file    ll_aton_util.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON utility functions
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

#ifndef __LL_ATON_UTIL_H
#define __LL_ATON_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#ifndef NDEBUG

#define LL_ATON_PRINTF(...) printf(__VA_ARGS__)
#define LL_ATON_PUTS(...)   puts(__VA_ARGS__)
#define LL_ATON_FFLUSH(...) fflush(__VA_ARGS__)

#define LL_ATON_ASSERT(...) assert(__VA_ARGS__)

#define LL_ATON_PROFILER_PRINTF(...) printf(__VA_ARGS__)
#define LL_ATON_PROFILER_ASSERT(...) assert(__VA_ARGS__)

#else

#define LL_ATON_PRINTF(...)
#define LL_ATON_PUTS(...)
#define LL_ATON_FFLUSH(...)

#define LL_ATON_ASSERT(...)
#define LL_ATON_ASSERTF(...)

#define LL_ATON_PROFILER_PRINTF(...)
#define LL_ATON_PROFILER_ASSERT(...)

#endif

#define __clean_errno() (errno == 0 ? "None" : strerror(errno))
#define __log_error(M, ...)                                                                                            \
  LL_ATON_PRINTF("[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, __clean_errno(), ##__VA_ARGS__)

/**
 *  @brief Cheks a condition: if it evaluates to false, prints an error message and asserts(false).
 *  @param COND the condition to check.
 *  @param MSG the message to print if the condition evaluates to false (can be a formatted string as well)
 *  @param ASSERT_COND boolean parameter: if 1, this command will LL_ATON_ASSERT(COND), otherwise il will
 * LL_ATON_ASSERT(0)
 *  @param variadic_args the (potential) variables to print in the message's formatted string, if any
 */
#define assertf(COND, ASSERT_COND, MSG, ...)                                                                           \
  if (!(COND))                                                                                                         \
  {                                                                                                                    \
    __log_error(MSG, ##__VA_ARGS__);                                                                                   \
    if (ASSERT_COND)                                                                                                   \
      LL_ATON_ASSERT(COND);                                                                                            \
    else                                                                                                               \
      LL_ATON_ASSERT(0);                                                                                               \
  }

  /**
   * @brief  Helper function. Extracts bitfield from a buffer
   * @param  bits Pointer of the buffer to extract bits from
   * @param  pos Global bit position of the bitfield to extract
   * @retval nbits Number of bits to extract
   */
  uint32_t LL_ATON_getbits(uint32_t *bits, unsigned int pos, int nbits);

  /**
   * @brief  Helper function. Packs bitfield into a buffer
   * @param  bits Pointer of the buffer to pack bits into
   * @param  pos Global position of the bitfield to pack
   * @retval nbits Number of bits to extract
   */
  void LL_ATON_setbits(uint32_t *bits, unsigned int pos, int nbits, unsigned int val);

#ifdef __cplusplus
}
#endif

#endif
