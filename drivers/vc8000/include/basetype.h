/*
 * Copyright (c) 2015-2022, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 *
 ********************************************************************************
 *
 * This software is distributed under the terms of
 * BSD-3-Clause. The following provisions apply :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 *
 *  Description : Basic type definitions.
 *
 ********************************************************************************
 */

#ifndef BASETYPE_H_INCLUDED
#define BASETYPE_H_INCLUDED

#include <stdint.h>

#define VOLATILE    volatile

#ifdef __linux__    /* typedefs for Linux */

#include <stddef.h> /* for size_t, NULL, etc. */

typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef signed int i32;
typedef unsigned long long u64;
typedef int64_t   i64;

typedef size_t ptr_t;

#ifdef ADDRESS_WIDTH_64
#define PRT_PTR "lx"
#else
#define PRT_PTR "x"
#endif

#ifndef __cplusplus
#ifndef bool
typedef enum {
        false   = 0,
        true    = 1
} bool;
#endif
#endif

#else /* __symbian__ or __win__ or whatever, customize it to suit well */

#ifndef _SIZE_T_DEFINED
typedef uint32_t size_t;

#define _SIZE_T_DEFINED
#endif

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else /*  */
#define NULL    ((void *)0)
#endif /*  */
#endif

typedef uint32_t ptr_t;

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t   i64;


#ifndef __cplusplus
#ifndef bool
typedef enum {
        false   = 0,
        true    = 1
} bool;
#endif
#endif

#endif

#if defined(VC1SWDEC_16BIT) || defined(MP4ENC_ARM11)
typedef uint16_t u16x;
typedef int16_t i16x;
#else
typedef uint16_t u16x;
typedef int16_t i16x;
#endif

#endif /* BASETYPE_H_INCLUDED */
