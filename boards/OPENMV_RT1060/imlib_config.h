/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library configuration.
 */
#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__

#include "imlib_config_default.h"

// Additional features.

// AprilTag families, find_apriltags() needs at least one
#define IMLIB_ENABLE_APRILTAGS_TAG16H5
#define IMLIB_ENABLE_APRILTAGS_TAG25H9
#define IMLIB_ENABLE_APRILTAGS_TAG36H10

// Enable the fast debayer
#define IMLIB_ENABLE_DEBAYER_OPTIMIZATION

#endif //__IMLIB_CONFIG_H__
