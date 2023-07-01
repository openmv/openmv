/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2007-2017 Ralph Hempel
 * Copyright (c) 2017-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2017-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * UMM memory allocator.
 */
#ifndef __UMM_MALLOC_H__
#define __UMM_MALLOC_H__
#include <stdlib.h>
void umm_alloc_fail();
void  umm_init_x(size_t size);   // Min of 2.5KB - Max of 640 KB.
void *umm_malloc(size_t size);
void *umm_calloc(size_t num, size_t size);
void *umm_realloc(void *ptr, size_t size);
void  umm_free(void *ptr);
#endif /* __UMM_MALLOC_H__ */
