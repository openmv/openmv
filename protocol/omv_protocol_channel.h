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
 * OpenMV Protocol - Channel Interface
 * This header defines the channel abstraction layer for the OpenMV Protocol.
 * Channels provide a unified interface for accessing different types of data
 * and functionality through the protocol.
 */
#ifndef __OMV_PROTOCOL_CHANNEL_H__
#define __OMV_PROTOCOL_CHANNEL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/***************************************************************************
* Channel Constants
***************************************************************************/

#define OMV_PROTOCOL_CHANNEL_NAME_SIZE  (14)

/***************************************************************************
* Channel IOCTL Commands
***************************************************************************/

// Standard IOCTL commands for stdin channel
typedef enum {
    OMV_CHANNEL_IOCTL_STDIN_STOP        = 0x01,     // Stop running script
    OMV_CHANNEL_IOCTL_STDIN_EXEC        = 0x02,     // Execute script
    OMV_CHANNEL_IOCTL_STDIN_RESET       = 0x03,     // Reset script buffer
} omv_channel_ioctl_stdin_t;

// Standard IOCTL commands for stream channel
typedef enum {
    OMV_CHANNEL_IOCTL_STREAM_CTRL       = 0x00, // Enable/disable streaming
    OMV_CHANNEL_IOCTL_STREAM_RAW_CTRL   = 0x01, // Enable/disable raw streaming
    OMV_CHANNEL_IOCTL_STREAM_RAW_CFG    = 0x02, // Set raw stream resolution
} omv_channel_ioctl_stream_t;

// Standard IOCTL commands for profile channel
typedef enum {
    OMV_CHANNEL_IOCTL_PROFILE_MODE      = 0x00, // Set profiling mode
    OMV_CHANNEL_IOCTL_PROFILE_SET_EVENT = 0x01, // Set event type to profile
    OMV_CHANNEL_IOCTL_PROFILE_RESET     = 0x02, // Reset profiler data
    OMV_CHANNEL_IOCTL_PROFILE_STATS     = 0x03, // Get profiler statistics
} omv_channel_ioctl_profile_t;

// IOCTL sizes lookup table entries
#define OMV_PROTOCOL_CHANNEL_IOCTL_TABLE                                       \
    {OMV_PROTOCOL_CHANNEL_ID_STDIN, OMV_CHANNEL_IOCTL_STDIN_STOP, 0},          \
    {OMV_PROTOCOL_CHANNEL_ID_STDIN, OMV_CHANNEL_IOCTL_STDIN_EXEC, 0},          \
    {OMV_PROTOCOL_CHANNEL_ID_STDIN, OMV_CHANNEL_IOCTL_STDIN_RESET, 0},         \
                                                                               \
    {OMV_PROTOCOL_CHANNEL_ID_STREAM, OMV_CHANNEL_IOCTL_STREAM_CTRL, 4},        \
    {OMV_PROTOCOL_CHANNEL_ID_STREAM, OMV_CHANNEL_IOCTL_STREAM_RAW_CTRL, 4},    \
    {OMV_PROTOCOL_CHANNEL_ID_STREAM, OMV_CHANNEL_IOCTL_STREAM_RAW_CFG, 8},     \
                                                                               \
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_MODE, 4},      \
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_SET_EVENT, 8}, \
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_RESET, 0},     \
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_STATS, 0}

/***************************************************************************
* Channel Interface
***************************************************************************/

// Reserved/predefined channel IDs (usable as array indices)
typedef enum {
    OMV_PROTOCOL_CHANNEL_ID_TRANSPORT   = 0,          // Transport layer (not accessible via protocol)
    OMV_PROTOCOL_CHANNEL_ID_STDIN       = 1,          // Script input (write-only)
    OMV_PROTOCOL_CHANNEL_ID_STDOUT      = 2,          // Text output (read-only)
    OMV_PROTOCOL_CHANNEL_ID_STREAM      = 3,          // Stream data (read-only)
    OMV_PROTOCOL_CHANNEL_ID_PROFILE     = 4,          // Profiling data (when enabled, read-only)
} omv_protocol_channel_id_t;

// Channel flags
typedef enum {
    OMV_PROTOCOL_CHANNEL_FLAG_READ      = (1 << 0),  // Channel supports read operations
    OMV_PROTOCOL_CHANNEL_FLAG_WRITE     = (1 << 1),  // Channel supports write operations
    OMV_PROTOCOL_CHANNEL_FLAG_EXEC      = (1 << 2),  // Executable channel
    OMV_PROTOCOL_CHANNEL_FLAG_LOCK      = (1 << 3),  // Channel requires locking before read/write
    OMV_PROTOCOL_CHANNEL_FLAG_STREAM    = (1 << 4),  // Streaming channel
    OMV_PROTOCOL_CHANNEL_FLAG_DYNAMIC   = (1 << 5),  // Channel was dynamically created
    OMV_PROTOCOL_CHANNEL_FLAG_PHYSICAL  = (1 << 6),  // Physical transport channel (not accessible via protocol)
} omv_protocol_channel_flags_t;

// Helper macros
#define OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, flag) \
    (((channel)->flags & OMV_PROTOCOL_CHANNEL_FLAG_##flag) != 0)

#define OMV_PROTOCOL_CHANNEL_IS_TRANSPORT(channel)             \
    (OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, PHYSICAL) &&       \
     (channel)->read && (channel)->write && (channel)->size && \
     (channel)->is_active)

// Forward declaration
typedef struct omv_protocol_channel omv_protocol_channel_t;

// Channel interface (used for both transport and logical channels)
struct omv_protocol_channel {
    void *priv;
    uint8_t id;
    uint32_t flags;
    char name[OMV_PROTOCOL_CHANNEL_NAME_SIZE];
    int (*init) (const omv_protocol_channel_t *channel);
    int (*deinit) (const omv_protocol_channel_t *channel);
    bool (*poll) (const omv_protocol_channel_t *channel);
    int (*lock) (const omv_protocol_channel_t *channel);
    int (*unlock) (const omv_protocol_channel_t *channel);
    size_t (*size) (const omv_protocol_channel_t *channel);
    size_t (*shape) (const omv_protocol_channel_t *channel, size_t shape[4]);
    int (*read) (const omv_protocol_channel_t *channel, uint32_t offset, size_t size, void *data);
    int (*write) (const omv_protocol_channel_t *channel, uint32_t offset, size_t size, const void *data);
    const void *(*readp) (const omv_protocol_channel_t *channel, uint32_t offset, size_t size);
    int (*flush) (const omv_protocol_channel_t *channel);
    int (*ioctl) (const omv_protocol_channel_t *channel, uint32_t cmd, size_t len, void *arg);
    // Stdin-specific function to execute scripts internall (not exposed via ioctl).
    bool (*exec) (const omv_protocol_channel_t *channel);
    // Transport-specific functions (for channel ID 0 only)
    bool (*is_active) (const omv_protocol_channel_t *channel);
};

// Default channels.
extern const omv_protocol_channel_t omv_usb_channel;
extern const omv_protocol_channel_t omv_stdin_channel;
extern const omv_protocol_channel_t omv_stdout_channel;
extern const omv_protocol_channel_t omv_stream_channel;
extern const omv_protocol_channel_t omv_profile_channel;

#endif // __OMV_PROTOCOL_CHANNEL_H__
