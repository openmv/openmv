/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library configuration.
 */
#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__

#include "imlib_config_default.h"

// Disabled features.
#undef IMLIB_ENABLE_LAB_LUT
#undef IMLIB_ENABLE_FLOOD_FILL
#undef IMLIB_ENABLE_MODE
#undef IMLIB_ENABLE_MIDPOINT
#undef IMLIB_ENABLE_BILATERAL
#undef IMLIB_ENABLE_DATAMATRICES
#undef IMLIB_ENABLE_FIND_LBP

#endif //__IMLIB_CONFIG_H__
