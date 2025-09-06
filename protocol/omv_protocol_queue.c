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
#include "omv_protocol_queue.h"

void omv_protocol_queue_reset(omv_protocol_queue_t *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

bool omv_protocol_queue_check(omv_protocol_queue_t *queue, uint8_t opcode, uint8_t sequence) {
    for (int i = 0; i < queue->count; i++) {
        int index = (queue->head + i) % OMV_PROTOCOL_QUEUE_SIZE;
        if (queue->entries[index].opcode == opcode &&
            queue->entries[index].sequence == sequence) {
            return true;
        }
    }
    return false;
}

bool omv_protocol_queue_push(omv_protocol_queue_t *queue, uint8_t opcode, uint8_t sequence) {
    if (queue->count >= OMV_PROTOCOL_QUEUE_SIZE) {
        return false;
    }
    
    queue->entries[queue->tail].opcode = opcode;
    queue->entries[queue->tail].sequence = sequence;
    queue->tail = (queue->tail + 1) % OMV_PROTOCOL_QUEUE_SIZE;
    queue->count++;
    return true;
}

bool omv_protocol_queue_pop(omv_protocol_queue_t *queue, uint8_t opcode, uint8_t sequence, bool seq_check) {
    for (int i = 0; i < queue->count; i++) {
        int index = (queue->head + i) % OMV_PROTOCOL_QUEUE_SIZE;
        if (queue->entries[index].opcode == opcode && 
            (!seq_check || queue->entries[index].sequence == sequence)) {
            // Remove by shifting elements
            for (int j = i; j < queue->count - 1; j++) {
                int curr = (queue->head + j) % OMV_PROTOCOL_QUEUE_SIZE;
                int next = (queue->head + j + 1) % OMV_PROTOCOL_QUEUE_SIZE;
                queue->entries[curr] = queue->entries[next];
            }
            queue->count--;
            queue->tail = (queue->tail - 1 + OMV_PROTOCOL_QUEUE_SIZE) % OMV_PROTOCOL_QUEUE_SIZE;
            return true;
        }
    }
    return false;
}

bool omv_protocol_queue_is_full(omv_protocol_queue_t *queue) {
    return queue->count >= OMV_PROTOCOL_QUEUE_SIZE;
}
