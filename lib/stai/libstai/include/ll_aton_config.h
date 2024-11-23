/**
 ******************************************************************************
 * @file    ll_aton_config.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON runtime/library configuration.
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

#ifndef __LL_ATON_CONFIG_H
#define __LL_ATON_CONFIG_H

/* STEdgeAI runtime build info bit-field sizes (may not be modified) */
#define LL_ATON_CONFIG_PLAT_BSIZE    8
#define LL_ATON_CONFIG_OSAL_BSIZE    8
#define LL_ATON_CONFIG_RT_MODE_BSIZE 4
#define LL_ATON_CONFIG_BINFO_BSIZE   4
#define LL_ATON_CONFIG_CLKG_BSIZE    4
#define LL_ATON_CONFIG_SWF_BSIZE     1
#define LL_ATON_CONFIG_DD_BSIZE      1
#define LL_ATON_CONFIG_EBDBG_BSIZE   1

/*
 * Configuration defines:
 *      mandatory LL_ATON_PLATFORM
 *      mandatory LL_ATON_OSAL
 *      optional  LL_ATON_RT_MODE
 *      optional  LL_ATON_SW_FALLBACK               enable support for SW inference library integration
 *      optional  LL_ATON_DUMP_DEBUG_API            enable buffer dumping functions (for debug purposes only)
 *      optional  LL_ATON_EB_DBG_INFO               enable compilation of epoch block debug information
 *      optional  LL_ATON_DBG_BUFFER_INFO_EXCLUDED  exclude debug info from buffer info arrays
 *                                                  (to be defined as `0` or `1`)
 *      optional  LL_ATON_ENABLE_CLOCK_GATING       used to enable/disable clock gating of the ATON units not involved
 *                                                  during epoch execution (to be defined as `0` or `1`)
 *
 *      NOTE: `mandatory` means that these macros must be predefined using `-D` options in the command-line of the
 *            C compiler a/o preprocessor!
 *
 *      NOTE: the beyond macros MUST be set to the same values when compiling the different ATON runtime/library and
 *            AtoNN compiler generated files!
 */

/* Definition of ATON platforms */
#define LL_ATON_PLAT_NCSIM        1
#define LL_ATON_PLAT_STICE4       2
#define LL_ATON_PLAT_ZC706        3
#define LL_ATON_PLAT_SWEMUL       4
#define LL_ATON_PLAT_TLM_MCD      5
#define LL_ATON_PLAT_TSTXPL       6
#define LL_ATON_PLAT_NEUROMEM_SIM 7
#define LL_ATON_PLAT_IMAGING_SIM  8
#define LL_ATON_PLAT_CENTAURI     9
#define LL_ATON_PLAT_IWAVE        10
#define LL_ATON_PLAT_N64          11
#define LL_ATON_PLAT_STM32N6      12
#define LL_ATON_PLAT_BITTWARE     13
#define LL_ATON_PLAT_EC_TRACE     14
#define LL_ATON_PLAT_STM32H7P     15

/* Definition of ATON RTOS abstraction layers */
#define LL_ATON_OSAL_BARE_METAL 1
#define LL_ATON_OSAL_LINUX_UIO  2
#define LL_ATON_OSAL_LINUX_BW   3
#define LL_ATON_OSAL_THREADX    4
#define LL_ATON_OSAL_FREERTOS   5
#define LL_ATON_OSAL_ZEPHYR     6

#define LL_ATON_OSAL_USER_IMPL                                                                                         \
  ((1 << LL_ATON_CONFIG_OSAL_BSIZE) -                                                                                  \
   1) /* "Backdoor" for OSAL implementations of custom RTOSes not officially supported by our distribution */

/* Definition of the two different modes for ATON runtime */
/* `LL_ATON_RT_ASYNC` is RECOMMENDED
 */
#define LL_ATON_RT_POLLING 1
#define LL_ATON_RT_ASYNC   2

/* Check definition of LL_ATON_PLATFORM */
#ifndef LL_ATON_PLATFORM
#error "Needed define 'LL_ATON_PLATFORM' undefined"
#endif

/* Check definition of LL_ATON_OSAL */
#ifndef LL_ATON_OSAL
#error "Needed define 'LL_ATON_OSAL' undefined (old default was LL_ATON_OSAL_BARE_METAL)"
#endif

/* Set beyond macro to one of the above three modes */
#ifndef LL_ATON_RT_MODE
#define LL_ATON_RT_MODE LL_ATON_RT_ASYNC
#endif

/* Default values for the other remaining optional macros */
#ifndef LL_ATON_SW_FALLBACK
#define LL_ATON_SW_FALLBACK 0
#elif LL_ATON_SW_FALLBACK != 0 && LL_ATON_SW_FALLBACK != 1
#undef LL_ATON_SW_FALLBACK
#define LL_ATON_SW_FALLBACK 1
#endif

#ifndef LL_ATON_DUMP_DEBUG_API
// #define LL_ATON_DUMP_DEBUG_API
#endif

#ifndef LL_ATON_EB_DBG_INFO
// #define LL_ATON_EB_DBG_INFO
#endif

#ifndef LL_ATON_DBG_BUFFER_INFO_EXCLUDED
#define LL_ATON_DBG_BUFFER_INFO_EXCLUDED 0
#endif

#ifndef LL_ATON_ENABLE_CLOCK_GATING
#define LL_ATON_ENABLE_CLOCK_GATING 1
#endif

/* Check if selected values are valid */
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_NCSIM)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_STICE4)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_ZC706)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_SWEMUL)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_TLM_MCD)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_TSTXPL)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_NEUROMEM_SIM)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_IMAGING_SIM)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_CENTAURI)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_IWAVE)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_N64)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_BITTWARE)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_EC_TRACE)
#if (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32H7P)
#error "Wrong definition of `LL_ATON_PLATFORM`"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#if (LL_ATON_OSAL != LL_ATON_OSAL_BARE_METAL)
#if (LL_ATON_OSAL != LL_ATON_OSAL_LINUX_UIO)
#if (LL_ATON_OSAL != LL_ATON_OSAL_LINUX_BW)
#if (LL_ATON_OSAL != LL_ATON_OSAL_THREADX)
#if (LL_ATON_OSAL != LL_ATON_OSAL_FREERTOS)
#if (LL_ATON_OSAL != LL_ATON_OSAL_ZEPHYR)
#if (LL_ATON_OSAL != LL_ATON_OSAL_USER_IMPL)
#error "Wrong definition of `LL_ATON_OSAL`"
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#if (LL_ATON_RT_MODE != LL_ATON_RT_POLLING)
#if (LL_ATON_RT_MODE != LL_ATON_RT_ASYNC)
#error "Wrong definition of `LL_ATON_RT_MODE`"
#endif
#endif

#endif // __LL_ATON_CONFIG_H
