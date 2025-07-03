/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * STM32 MDMA utils.
 */
#ifndef __STM_MDMA_H__
#define __STM_MDMA_H__

#include STM32_HAL_H
#include "omv_csi.h"

#ifdef MDMA
#define MDMA_CHAN_TO_INSTANCE(x) \
    (MDMA_Channel_TypeDef *) (MDMA_Channel0_BASE + ((MDMA_Channel1_BASE - MDMA_Channel0_BASE) * x))
#endif

#define MDMA_BUFFER_SIZE    (64)

void stm_mdma_init(omv_csi_t *csi, uint32_t bytes_per_pixel, uint32_t x_crop);
void stm_mdma_start(omv_csi_t *csi, uint32_t src, uint32_t dst, uint32_t line_width, uint32_t line_count);
#endif // __STM_MDMA_H__
