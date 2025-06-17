/**
 ******************************************************************************
 * @file    ll_aton_attributes.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON library attributes handling.
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

#ifndef __LL_ATON_ATTRIBUTES_H
#define __LL_ATON_ATTRIBUTES_H

/*
 * Exported attributes handling:
 *      LL_ATON_API_ENTRY
 *      LL_ATON_ALIGNED(x)
 *      LL_ATON_LIB_UNUSED(x)
 *      LL_ATON_CONCAT(a, b)
 *      LL_ATON_CONCAT3(a, b, c)
 *      LL_ATON_WEAK
 *
 */

/* Exported attributes handling */

#if defined(__clang__)
#undef __weak
#define __weak __attribute__((weak))
#endif
#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak))
#endif /* __weak */
#endif /* __GNUC__ */
#define LL_ATON_WEAK __weak

#define LL_ATON_LIB_UNUSED(x) ((void)(x)) // prevent from eventual compiler warnings due to unused variables

#define __LL_ATON_CONCAT_ARG(a, b) a##b
#define LL_ATON_CONCAT(a, b)       __LL_ATON_CONCAT_ARG(a, b)
#define LL_ATON_CONCAT3(a, b, c)   LL_ATON_CONCAT(a, LL_ATON_CONCAT(b, c))

/* Alignment macros borrowed from ST.AI (file `stai.h`) */
#if defined(_MSC_VER)
#define LL_ATON_API_ENTRY  __declspec(dllexport)
#define LL_ATON_ALIGNED(x) __declspec(align(x))
#elif defined(__ICCARM__) || defined(__IAR_SYSTEMS_ICC__)
#define LL_ATON_API_ENTRY  /* LL_ATON_API_ENTRY */
#define LL_ATON_ALIGNED(x) LL_ATON_CONCAT(LL_ATON_ALIGNED_, x)
#define LL_ATON_ALIGNED_1  _Pragma("data_alignment = 1")
#define LL_ATON_ALIGNED_2  _Pragma("data_alignment = 2")
#define LL_ATON_ALIGNED_4  _Pragma("data_alignment = 4")
#define LL_ATON_ALIGNED_8  _Pragma("data_alignment = 8")
#define LL_ATON_ALIGNED_16 _Pragma("data_alignment = 16")
#define LL_ATON_ALIGNED_32 _Pragma("data_alignment = 32")
#define LL_ATON_ALIGNED_64 _Pragma("data_alignment = 64")
#elif defined(__CC_ARM)
#define LL_ATON_API_ENTRY  __attribute__((visibility("default")))
#define LL_ATON_ALIGNED(x) __attribute__((aligned(x)))
/* Keil disallows anonymous union initialization by default */
#pragma anon_unions
#elif defined(__GNUC__)
// #define LL_ATON_API_ENTRY          __attribute__((visibility("default")))
#define LL_ATON_API_ENTRY  /* LL_ATON_API_ENTRY */
#define LL_ATON_ALIGNED(x) __attribute__((aligned(x)))
#else
/* Dynamic libraries are not supported by the compiler */
#define LL_ATON_API_ENTRY  /* LL_ATON_API_ENTRY */
#define LL_ATON_ALIGNED(x) /* LL_ATON_ALIGNED(x) */
#endif

#endif
