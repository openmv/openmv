/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Ulab config file.
 *
 */
#ifndef __ULAB_CONFIG_H__
#define __ULAB_CONFIG_H__
// Override ulab defaults here.
#define NDARRAY_BINARY_USES_FUN_POINTER     (1)
#define ULAB_VECTORISE_USES_FUN_POINTER     (1)
#define ULAB_SCIPY_HAS_OPTIMIZE_MODULE      (1)
#define ULAB_SCIPY_HAS_SPECIAL_MODULE       (0)
#endif //__ULAB_CONFIG_H__
