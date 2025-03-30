/* This file was ported to work on Alif Semiconductor Ensemble family of devices. */

/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#if defined(RTE_Compiler_IO_STDIN)
#include "retarget_stdin.h"

#define RETARGET_STDIN_FUNC                 clang_fgetc
int RETARGET_STDIN_FUNC(FILE *file);

#endif  /* RTE_Compiler_IO_STDIN */

#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"

#define RETARGET_STDOUT_FUNC                 clang_fputc
int RETARGET_STDOUT_FUNC(char c, FILE *file);

#endif  /* RTE_Compiler_IO_STDOUT */

#if defined(RTE_Compiler_IO_STDERR)
#include "retarget_stderr.h"

#define RETARGET_STDERR_FUNC                 clang_ferrc
int RETARGET_STDERR_FUNC(char c, FILE *file);

#endif  /* RTE_Compiler_IO_STDERR */

#ifndef RETARGET_STDIN_FUNC
#define RETARGET_STDIN_FUNC                 NULL
#endif      /* RETARGET_STDIN_FUNC */

#ifndef RETARGET_STDOUT_FUNC
#define RETARGET_STDOUT_FUNC                NULL
#endif      /*  RETARGET_STDOUT_FUNC */

// Picolibc retarget
// https://github.com/picolibc/picolibc/blob/main/doc/os.md

#if ( ( defined(RTE_Compiler_IO_STDOUT)   || \
        defined(RTE_Compiler_IO_STDERR) ) && \
        defined(RTE_Compiler_IO_STDIN) )

    #define RETARGET_FLAG                       _FDEV_SETUP_RW

#elif ( defined(RTE_Compiler_IO_STDIN) && \
        !( defined(RTE_Compiler_IO_STDOUT)   || \
           defined(RTE_Compiler_IO_STDERR) ) )

    #define RETARGET_FLAG                       _FDEV_SETUP_READ

#elif ( !defined(RTE_Compiler_IO_STDIN) && \
        ( defined(RTE_Compiler_IO_STDOUT)   || \
          defined(RTE_Compiler_IO_STDERR) ) )

    #define RETARGET_FLAG                       _FDEV_SETUP_WRITE

#endif      /*  ((STDOUT || STDERR) && (STDIN)) */

#if (   defined(RTE_Compiler_IO_STDOUT_User) ||  \
        defined(RTE_Compiler_IO_STDERR_User) || \
        defined(RTE_Compiler_IO_STDIN_User) )

static FILE __stdio = FDEV_SETUP_STREAM(RETARGET_STDOUT_FUNC, \
                            RETARGET_STDIN_FUNC, NULL, RETARGET_FLAG);
FILE *const stdin   = &__stdio;
__strong_reference(stdin, stdout);      // Needed stdout

#endif      /*  ( STDOUT || STDERR || STDIN ) */

#if defined(RTE_Compiler_IO_STDIN_User)
int RETARGET_STDIN_FUNC(FILE *file)
{
    (void)file;
    return stdin_getchar();
}
#endif      /*  RTE_Compiler_IO_STDIN_User */

#if defined(RTE_Compiler_IO_STDOUT_User)

int RETARGET_STDOUT_FUNC(char c, FILE *file)
{
    (void)file;
    return stdout_putchar(c);
}
#endif      /*  RTE_Compiler_IO_STDOUT_User  */

#if defined(RTE_Compiler_IO_STDERR_User)
__strong_reference(stdin, stderr);

int RETARGET_STDERR_FUNC(char c, FILE *file)
{
    (void)file;
    return stderr_putchar(c);
}
#endif      /* RTE_Compiler_IO_STDERR_User */