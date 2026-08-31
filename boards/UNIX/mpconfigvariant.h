/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * MicroPython unix variant configuration for the OpenMV unix port.
 */

#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EVERYTHING)

#include "../../lib/micropython/ports/unix/variants/mpconfigvariant_common.h"

// OpenMV's imlib / common code calls m_free(ptr) and m_realloc(ptr, size)
// without size tracking; the unix variant default would otherwise enable it.
#undef MICROPY_MALLOC_USES_ALLOCATED_SIZE
#undef MICROPY_MEM_STATS
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (0)
#define MICROPY_MEM_STATS (0)
