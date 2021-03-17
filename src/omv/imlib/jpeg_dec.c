/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * Copyright (c) 2013-2021 Larry Bank <bitbank@pobox.com>
 *
 * Copyright 2020 BitBank Software, Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>

#include "ff_wrapper.h"
#include "imlib.h"
#include "omv_boardconfig.h"

#define TIME_JPEG   (0)
#if (TIME_JPEG == 1)
#include "py/mphal.h"
#endif

// src -> Image object with data pointing to jpeg data and bpp equal to the data size. Width/Height
//        may be invalid and should be ignored...
// dst -> Output image object, all fields are invalid. jpeg_decompress() sets the width/height/bpp
//        and sets data=fb_alloc() of the image pixels. Callers fb_frees() alloced data.
// Returns True on failure and False on success. This function always fb_alloc()'s data for the
// caller to free.
bool jpeg_decompress(image_t *src, image_t *dst)
{
#if (TIME_JPEG==1)
    mp_uint_t start = mp_hal_ticks_ms();
#endif

#if (TIME_JPEG==1)
    printf("time: %u ms\n", mp_hal_ticks_ms() - start);
#endif

    return false;
}
