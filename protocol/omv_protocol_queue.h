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
 * OpenMV Protocol Queue Management
 */
#ifndef __OMV_PROTOCOL_QUEUE_H__
#define __OMV_PROTOCOL_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#define OMV_PROTOCOL_QUEUE_SIZE         (16)

// Queue entry
typedef struct {
    uint8_t opcode;
    uint8_t sequence;
} omv_protocol_queue_entry_t;

// Queue structure
typedef struct {
    omv_protocol_queue_entry_t entries[OMV_PROTOCOL_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} omv_protocol_queue_t;

// Queue management functions
void omv_protocol_queue_reset(omv_protocol_queue_t *queue);
bool omv_protocol_queue_check(omv_protocol_queue_t *queue, uint8_t opcode, uint8_t sequence);
bool omv_protocol_queue_push(omv_protocol_queue_t *queue, uint8_t opcode, uint8_t sequence);
bool omv_protocol_queue_pop(omv_protocol_queue_t *queue, uint8_t opcode, uint8_t sequence, bool seq_check);
bool omv_protocol_queue_is_full(omv_protocol_queue_t *queue);

#endif // __OMV_PROTOCOL_QUEUE_H__
