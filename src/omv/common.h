/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Common macros.
 *
 */
#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#ifndef BREAK
#define BREAK() __asm__ volatile ("BKPT")
#endif

#ifndef DISABLE_OPT
#define DISABLE_OPT __attribute__((optimize("O0")))
#endif

#ifdef DEBUG_PRINTF
#define debug_printf(fmt, ...) \
            do { printf("%s(): " fmt, __func__, ##__VA_ARGS__);} while (0)
#else
#define debug_printf(...)
#endif
