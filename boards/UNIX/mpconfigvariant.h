/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * MicroPython Unix port variant for OpenMV.
 */
// Enable all features, as in the coverage variant.
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EVERYTHING)

#include "variants/mpconfigvariant_common.h"

// Deliver SIGINT at VM or imlib polls.
#undef MICROPY_ASYNC_KBD_INTR
#define MICROPY_ASYNC_KBD_INTR (0)

// OpenMV calls m_free(ptr) and m_realloc(ptr, size) without size tracking.
#undef MICROPY_MALLOC_USES_ALLOCATED_SIZE
#undef MICROPY_MEM_STATS
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (0)
#define MICROPY_MEM_STATS (0)

void omv_unix_init(void);
void omv_unix_deinit(void);
#define MICROPY_PORT_INIT_FUNC omv_unix_init()
#define MICROPY_PORT_DEINIT_FUNC omv_unix_deinit()

// Free umalloc blocks left by aborted code when the VM catches an exception, as embedded ports do.
#define MICROPY_VM_HOOK_EXC            \
    do {                               \
        extern void uma_collect(void); \
        uma_collect();                 \
    } while (0);
