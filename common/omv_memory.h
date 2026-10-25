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
 * Memory attributes abstraction layer.
 */
#ifndef __OMV_MEMORY_H__
#define __OMV_MEMORY_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Returns true if any part of [addr, addr + size) is cached by the data cache.
// Returns false if the data cache is not present or disabled, or if the range
// is in TCM or non-cacheable memory. The result reflects the MPU configuration
// at the first call.
bool omv_memory_is_cacheable(const void *addr, size_t size);

// Returns true if any part of [addr, addr + size) is cached write-back, i.e.
// may hold dirty cache lines that need to be cleaned. Write-through memory is
// cacheable but never dirty.
bool omv_memory_is_write_back(const void *addr, size_t size);

// Returns true if all of [addr, addr + size) is accessible by the bus master
// `master`, which is the base address of a DMA controller, DMA stream/channel,
// or of a peripheral with its own DMA (e.g. DMA2D, SDMMC1). If master is NULL,
// the port's general purpose DMA is assumed. Alignment is not checked.
bool omv_memory_is_dmable(const void *master, const void *addr, size_t size);
#endif // __OMV_MEMORY_H__
