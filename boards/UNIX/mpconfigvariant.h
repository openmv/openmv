/*
 * MicroPython Unix port variant for OpenMV integration
 *
 * This variant disables MICROPY_MALLOC_USES_ALLOCATED_SIZE to match
 * OpenMV's MicroPython expectations where m_free(ptr) and m_realloc(ptr, size)
 * are used without size tracking parameters.
 */

// Set base feature level
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES)

// Include common Unix variant features
#include "../../lib/micropython/ports/unix/variants/mpconfigvariant_common.h"

// OpenMV-specific overrides: disable size tracking in malloc/free
#undef MICROPY_MALLOC_USES_ALLOCATED_SIZE
#undef MICROPY_MEM_STATS
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (0)
#define MICROPY_MEM_STATS (0)
