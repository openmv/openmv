#ifndef __OMV_BUFFER_H__
#define __OMV_BUFFER_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    size_t size;          // Total buffer size
    uint8_t *read_ptr;    // Where to read/process from
    uint8_t *write_ptr;   // Where to write new data
    size_t used;          // Bytes currently in buffer (unprocessed)
    uint8_t *data;        // The actual buffer memory
} omv_buffer_t;

// Initialize buffer with pre-allocated memory
static inline void omv_buffer_init(omv_buffer_t *buf, uint8_t *buffer, size_t size) {
    buf->data = buffer;
    buf->size = size;
    buf->read_ptr = buffer;
    buf->write_ptr = buffer;
    buf->used = 0;
}

// Reset buffer to empty state
static inline void omv_buffer_clear(omv_buffer_t *buf) {
    buf->read_ptr = buf->data;
    buf->write_ptr = buf->data;
    buf->used = 0;
}

// Get number of bytes available for reading
static inline size_t omv_buffer_avail(omv_buffer_t *buf) {
    return buf->used;
}

// Get number of bytes available for writing
static inline size_t omv_buffer_free(omv_buffer_t *buf) {
    return buf->size - buf->used;
}

// Get pointer to readable data
// Returns data pointer or NULL if buffer is empty
static inline void *omv_buffer_data(omv_buffer_t *buf) {
    return (buf->used > 0) ? buf->read_ptr : NULL;
}

// Peek at 16-bit value at start of buffer
// Caller must ensure omv_buffer_avail(buf) >= 2
static inline uint16_t omv_buffer_peek16(omv_buffer_t *buf) {
    return *((uint16_t *) buf->read_ptr);
}

// Mark data as consumed and advance read pointer
// Resets buffer to beginning when empty
static inline void omv_buffer_consume(omv_buffer_t *buf, size_t consumed) {
    buf->read_ptr += consumed;
    buf->used -= consumed;

    // Reset to beginning if buffer is empty
    if (buf->used == 0) {
        buf->read_ptr = buf->data;
        buf->write_ptr = buf->data;
    }
}

// Claim contiguous space for writing, compacting buffer if needed
static inline void *omv_buffer_claim(omv_buffer_t *buf, size_t requested_size) {
    // Check if we have enough total space
    if (buf->used + requested_size > buf->size) {
        return NULL; // Buffer full
    }

    size_t space_to_end = buf->size - (buf->write_ptr - buf->data);

    // If not enough contiguous space at the end, move unprocessed data to beginning
    if (space_to_end < requested_size && buf->used > 0) {
        // Move unprocessed data to the beginning
        memmove(buf->data, buf->read_ptr, buf->used);
        buf->read_ptr = buf->data;
        buf->write_ptr = buf->data + buf->used;
    }

    return buf->write_ptr;
}

// Commit written data to the buffer
// Call this after successfully writing to the claimed space
static inline void omv_buffer_commit(omv_buffer_t *buf, size_t written) {
    buf->write_ptr += written;
    buf->used += written;
}
#endif // __OMV_BUFFER_H__
