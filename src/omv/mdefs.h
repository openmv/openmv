/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Misc macros.
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

#ifndef PRINT_LINE
#define PRINT_LINE printf("%s:%d\n", __FILE__, __LINE__)
#endif
