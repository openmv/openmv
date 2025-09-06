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
 * OpenMV Protocol - Transport-agnostic communication protocol
 * This protocol provides reliable communication between host and
 * device with support for multiple transport layers (USB, UART, TCP/IP).
 */
#ifndef __OMV_PROTOCOL_H__
#define __OMV_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "omv_buffer.h"
#include "omv_protocol_channel.h"
#include "omv_boardconfig.h"

/***************************************************************************
* Protocol Constants
***************************************************************************/

#define OMV_FIRMWARE_VERSION_MAJOR          (5)
#define OMV_FIRMWARE_VERSION_MINOR          (0)
#define OMV_FIRMWARE_VERSION_PATCH          (0)

#define OMV_PROTOCOL_VERSION_MAJOR          (1)
#define OMV_PROTOCOL_VERSION_MINOR          (0)
#define OMV_PROTOCOL_VERSION_PATCH          (0)

#define OMV_PROTOCOL_SYNC_SIZE              (2)
#define OMV_PROTOCOL_SYNC_WORD              (0xD5AA)
#define OMV_PROTOCOL_HEADER_SIZE            (10)    // SYNC[2] SEQ[1] CHAN[1] FLAGS[1] OPCODE[1] LEN[2] CRC[2]

#define OMV_PROTOCOL_MAX_CHANNELS           (32)
#define OMV_PROTOCOL_DEF_RTX_RETRIES        (3)
#define OMV_PROTOCOL_DEF_RTX_TIMEOUT_MS     (500)   // Doubled after each timeout
#define OMV_PROTOCOL_MIN_LOCK_INTERVAL_MS   (10)
#define OMV_PROTOCOL_MAGIC_BAUDRATE         (921600)

#ifndef OMV_PROTOCOL_DEFAULT_CHANNELS
#define OMV_PROTOCOL_DEFAULT_CHANNELS       (1)
#endif

#ifndef OMV_PROTOCOL_MAX_BUFFER_SIZE
#define OMV_PROTOCOL_MAX_BUFFER_SIZE        (4096)
#endif
#define OMV_PROTOCOL_MIN_PAYLOAD_SIZE       (64 - OMV_PROTOCOL_HEADER_SIZE - 4) // HEADER[10] PAYLOAD[n] CRC[4]
#define OMV_PROTOCOL_MAX_PAYLOAD_SIZE       (OMV_PROTOCOL_MAX_BUFFER_SIZE - OMV_PROTOCOL_HEADER_SIZE - 4)
#define OMV_PROTOCOL_MAX_PACKET_SIZE        (OMV_PROTOCOL_HEADER_SIZE + OMV_PROTOCOL_MAX_PAYLOAD_SIZE + 4)

#define OMV_PROTOCOL_PACKET_GET_SIZE(pkt) \
    (OMV_PROTOCOL_HEADER_SIZE + (pkt->length + (!!pkt->length * 4)))

// Macro for protocol struct size validation
#define OMV_PROTOCOL_ASSERT_SIZE(type, size) \
    _Static_assert(sizeof(type) == (size), #type " must be exactly " #size " bytes")

#ifndef OMV_PROTOCOL_TICKS_MS
#define OMV_PROTOCOL_TICKS_MS()             mp_hal_ticks_ms()
#endif

#ifndef OMV_PROTOCOL_EVENT_POLL
#define OMV_PROTOCOL_EVENT_POLL()           mp_event_handle_nowait()
#endif

#define omv_protocol_check_timeout(start_ms, timeout) \
    ((OMV_PROTOCOL_TICKS_MS() - start_ms) > timeout)

/***************************************************************************
* Packet Flags
***************************************************************************/

typedef enum {
    OMV_PROTOCOL_FLAG_ACK                   = (1 << 0),  // ACK packet
    OMV_PROTOCOL_FLAG_NAK                   = (1 << 1),  // NAK packet
    OMV_PROTOCOL_FLAG_RTX                   = (1 << 2),  // RTX packet
    OMV_PROTOCOL_FLAG_ACK_REQ               = (1 << 3),  // Packet requires an ACK
    OMV_PROTOCOL_FLAG_FRAGMENT              = (1 << 4),  // More fragments to follow
    OMV_PROTOCOL_FLAG_EVENT                 = (1 << 5),  // Event packet
    // Bits 6-7 reserved for future use
} omv_protocol_flags_t;

#define OMV_PROTOCOL_FLAG_NO_ACK    (OMV_PROTOCOL_FLAG_ACK | OMV_PROTOCOL_FLAG_NAK | OMV_PROTOCOL_FLAG_EVENT)

/***************************************************************************
* System Events (Channel 0)
***************************************************************************/

typedef enum {
    OMV_PROTOCOL_EVENT_CHANNEL_REGISTERED   = 0x00,  // Channel dynamically registered
    OMV_PROTOCOL_EVENT_CHANNEL_UNREGISTERED = 0x01,  // Channel unregistered
    OMV_PROTOCOL_EVENT_SOFT_REBOOT          = 0x02,  // System is going for a soft-reboot
    OMV_PROTOCOL_EVENT_NOTIFY               = 0xFFFF, // Notification event (no payload)
} omv_protocol_event_type_t;

/***************************************************************************
* Status Codes
***************************************************************************/

typedef enum {
    OMV_PROTOCOL_STATUS_SUCCESS             = 0x00,  // Success
    OMV_PROTOCOL_STATUS_FAILED              = 0x01,  // Command failed
    OMV_PROTOCOL_STATUS_INVALID             = 0x02,  // Invalid command/arg
    OMV_PROTOCOL_STATUS_TIMEOUT             = 0x03,  // Operation timeout
    OMV_PROTOCOL_STATUS_BUSY                = 0x04,  // Device busy
    OMV_PROTOCOL_STATUS_CHECKSUM            = 0x05,  // CRC error
    OMV_PROTOCOL_STATUS_SEQUENCE            = 0x06,  // Sequence error
    OMV_PROTOCOL_STATUS_OVERFLOW            = 0x07,  // Buffer overflow
    OMV_PROTOCOL_STATUS_FRAGMENT            = 0x08,  // Fragmentation error
    OMV_PROTOCOL_STATUS_UNKNOWN             = 0x09,  // Unknown error
} omv_protocol_status_t;

/***************************************************************************
* Protocol Opcodes
***************************************************************************/

typedef enum {
    // Protocol Level Commands (0x00-0x0F)
    OMV_PROTOCOL_OPCODE_PROTO_SYNC          = 0x00,  // Synchronization request
    OMV_PROTOCOL_OPCODE_PROTO_GET_CAPS      = 0x01,  // Get capabilities
    OMV_PROTOCOL_OPCODE_PROTO_SET_CAPS      = 0x02,  // Set capabilities
    OMV_PROTOCOL_OPCODE_PROTO_STATS         = 0x03,  // Get protocol statistics
    OMV_PROTOCOL_OPCODE_PROTO_VERSION       = 0x04,  // Get version info

    // System Level Commands (0x10-0x1F)
    OMV_PROTOCOL_OPCODE_SYS_RESET           = 0x10,  // System reset
    OMV_PROTOCOL_OPCODE_SYS_BOOT            = 0x11,  // Jump to bootloader
    OMV_PROTOCOL_OPCODE_SYS_INFO            = 0x12,  // Get system info
    OMV_PROTOCOL_OPCODE_SYS_EVENT           = 0x13,  // System event
    OMV_PROTOCOL_OPCODE_SYS_LAST            = 0x13,  // Last system command

    // Data Channels Commands (0x20-0x2F)
    OMV_PROTOCOL_OPCODE_CHANNEL_LIST        = 0x20,  // List registered channels
    OMV_PROTOCOL_OPCODE_CHANNEL_POLL        = 0x21,  // Poll channels status
    OMV_PROTOCOL_OPCODE_CHANNEL_LOCK        = 0x22,  // Lock the data channel
    OMV_PROTOCOL_OPCODE_CHANNEL_UNLOCK      = 0x23,  // Lock the data channel
    OMV_PROTOCOL_OPCODE_CHANNEL_SHAPE       = 0x24,  // Available data size.
    OMV_PROTOCOL_OPCODE_CHANNEL_SIZE        = 0x25,  // Available data size.
    OMV_PROTOCOL_OPCODE_CHANNEL_READ        = 0x26,  // Dump the data channel
    OMV_PROTOCOL_OPCODE_CHANNEL_WRITE       = 0x27,  // Write the data channel
    OMV_PROTOCOL_OPCODE_CHANNEL_IOCTL       = 0x28,  // Perform ioctl on channel
    OMV_PROTOCOL_OPCODE_CHANNEL_EVENT       = 0x29,  // System event
} omv_protocol_opcode_t;

/***************************************************************************
* Packet structure
***************************************************************************/

typedef struct __attribute__((packed)) {
    uint16_t sync;          // Synchronization word (0xAA55)
    uint8_t sequence;       // Sequence number
    uint8_t channel;        // Channel ID
    uint8_t flags;          // Packet flags
    uint8_t opcode;         // Command/response opcode
    uint16_t length;        // Length of data field only
    uint16_t crc;           // CRC of header fields
    uint8_t payload[];      // Flexible array member
} omv_protocol_packet_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_packet_t, OMV_PROTOCOL_HEADER_SIZE);

/***************************************************************************
* Packet Payloads
***************************************************************************/

// NAK respone structure
typedef struct __attribute__((packed)) {
    uint16_t status;    // Error status
} omv_protocol_response_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_response_t, 2);

// Channel size structure
typedef struct __attribute__((packed)) {
    uint32_t size;
} omv_protocol_channel_size_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_size_t, 4);

// Channel poll response structure
typedef struct __attribute__((packed)) {
    uint32_t flags;
} omv_protocol_channel_poll_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_poll_t, 4);

// Channel io structure
typedef struct __attribute__((packed)) {
    uint32_t offset;
    uint32_t length;
    uint8_t payload[];
} omv_protocol_channel_io_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_io_t, 8);

// Channel ioctl structure
typedef struct __attribute__((packed)) {
    uint32_t request;
    uint8_t payload[];
} omv_protocol_channel_ioctl_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_ioctl_t, 4);

// Channel list entry structure
typedef struct __attribute__((packed)) {
    uint8_t id;
    uint8_t flags;
    char name[OMV_PROTOCOL_CHANNEL_NAME_SIZE];
} omv_protocol_channel_entry_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_entry_t, 16);

// Protocol capabilities structure
typedef struct __attribute__((packed)) {
    uint32_t crc_enabled : 1;
    uint32_t seq_enabled : 1;
    uint32_t ack_enabled : 1;
    uint32_t event_enabled : 1;
    uint32_t reserved1 : 28;
    uint16_t max_payload;
    uint8_t reserved2[10];
} omv_protocol_caps_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_caps_t, 16);

// Protocol statistics structure
typedef struct __attribute__((packed)) {
    uint32_t sent_packets;
    uint32_t recv_packets;
    uint32_t checksum_errors;
    uint32_t sequence_errors;
    uint32_t retransmit;
    uint32_t transport_errors;
    uint32_t sent_events;
    uint32_t reserved;
} omv_protocol_stats_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_stats_t, 32);

// Version information structure
typedef struct __attribute__((packed)) {
    uint8_t protocol_version[3];        // Protocol version
    uint8_t bootloader_version[3];      // Bootloader version
    uint8_t firmware_version[3];        // Firmware version
    uint8_t reserved[7];                // Reserved for future expansion
} omv_protocol_version_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_version_t, 16);

// System information structure
typedef struct __attribute__((packed)) {
    // Hardware identification
    uint32_t cpu_id;                    // CPUID register value
    uint32_t dev_id[3];                 // Unique device ID register
    uint32_t usb_id;                    // USB VID/PID (VID<<16|PID)
    uint32_t chip_id[3];                // Camera sensor chip ID
    uint32_t reserved_id[2];            // Reserved for future expansion

    // Hardware capabilities
    uint32_t hw_caps[2];                // Defined in hw_caps.h

    // Memory information
    uint32_t flash_size_kb;             // Flash memory size in KB
    uint32_t ram_size_kb;               // RAM size in KB
    uint32_t frame_buffer_size_kb;      // Framebuffer size in KB
    uint32_t stream_buffer_size_kb;     // Streaming buffer size in KB
    uint32_t reserved_memory[3];        // Reserved for future expansion
} omv_protocol_sys_info_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_sys_info_t, 76);

/***************************************************************************
* Protocol Context
***************************************************************************/

// Protocol State Machine
typedef enum {
    OMV_PROTOCOL_STATE_SYNC                 = 0x00,   // Wait for sync bytes
    OMV_PROTOCOL_STATE_HEADER               = 0x01,   // Reading header
    OMV_PROTOCOL_STATE_PAYLOAD              = 0x02,   // Reading data
    OMV_PROTOCOL_STATE_SYNC_RECOVERY        = 0x03,   // Scan for SYNC commands when stuck
    OMV_PROTOCOL_STATE_WAIT_ACK             = 0x04,   // Wait for ACK response
} omv_protocol_state_t;

// Protocol configuration structure (internal use)
typedef struct {
    // Protocol capabilities (negotiated)
    bool crc_enabled;
    bool seq_enabled;
    bool ack_enabled;
    bool event_enabled;
    uint16_t max_payload;
    // Transport configuration (local only)
    bool soft_reboot;
    uint16_t rtx_retries;
    uint16_t rtx_timeout_ms;
    uint16_t lock_intval_ms;
} omv_protocol_config_t;

// Protocol context
typedef struct {
    // State machine
    uint8_t sequence;           // Next sequence number
    uint32_t last_lock_ms;      // Timestamp of the last successful lock
    int32_t scan_offset;
    bool wait_for_ack;
    omv_protocol_state_t state;

    // ACK waiting context
    uint8_t ack_opcode;     // Opcode of waiting ACK
    uint8_t ack_sequence;   // Sequence of waiting ACK
    uint8_t ack_status;     // Status of waiting ACK

    // Protocol configuration (capabilities + transport config)
    omv_protocol_config_t config;

    // Buffer for received data
    omv_buffer_t buffer;
    uint8_t rawbuf[OMV_PROTOCOL_MAX_BUFFER_SIZE];

    // Protocol physical/logical channels
    uint8_t channels_count;
    const omv_protocol_channel_t *channels[OMV_PROTOCOL_MAX_CHANNELS];

    // Protocol Statistics
    omv_protocol_stats_t stats;
} omv_protocol_context_t;

// Initialize the protocol context
int omv_protocol_init(const omv_protocol_config_t *config);

// Initialize the protocol using defaults (default config + default channels)
int omv_protocol_init_default(void);

// Deinitialize protocol context
void omv_protocol_deinit(void);

// Helper function to check if the transport is active.
bool omv_protocol_is_active(void);

// Helper function to exec scripts in stdio buffer.
// Returns false, if no script is ready, true on executing the script.
bool omv_protocol_exec_script(void);

// Register a channel
int omv_protocol_register_channel(const omv_protocol_channel_t *channel);

// Find channel by ID.
const omv_protocol_channel_t *omv_protocol_find_channel(uint8_t channel_id);

// Find transport channel.
const omv_protocol_channel_t *omv_protocol_find_transport(void);

// Send event packet
int omv_protocol_send_event(uint8_t channel_id, uint16_t event, bool wait_ack);

// Send response packet
int omv_protocol_send_packet(uint8_t opcode, uint8_t channel_id, size_t size, const void *data, uint8_t flags);

// Call on events or periodically to process events
int omv_protocol_task(void);

// Process assembled packet
void omv_protocol_process(const omv_protocol_packet_t *packet);
#endif // __OMV_PROTOCOL_H__
