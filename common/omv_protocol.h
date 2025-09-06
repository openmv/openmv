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

/***************************************************************************
 * Protocol Constants
 ***************************************************************************/

#define OMV_FIRMWARE_VERSION_MAJOR          (4)
#define OMV_FIRMWARE_VERSION_MINOR          (7)
#define OMV_FIRMWARE_VERSION_PATCH          (0)

#define OMV_PROTOCOL_VERSION_MAJOR          (1)
#define OMV_PROTOCOL_VERSION_MINOR          (0)
#define OMV_PROTOCOL_VERSION_PATCH          (0)

#define OMV_PROTOCOL_SYNC_SIZE              (2)
#define OMV_PROTOCOL_SYNC_WORD              (0xAA55)
#define OMV_PROTOCOL_CRC_SIZE               (2)
#define OMV_PROTOCOL_HEADER_SIZE            (12)    // SYNC[2] SEQ[2] CHAN[2] FLAGS[1] OPCODE[1] LEN[2] CRC[2]

#define OMV_PROTOCOL_MAX_CHANNELS           (32)
#define OMV_PROTOCOL_CHANNEL_NAME_SIZE      (14)
#define OMV_PROTOCOL_MIN_LOCK_WAIT_MS       (10)
#define OMV_PROTOCOL_DEFAULT_TIMEOUT_MS     (3000)
#define OMV_PROTOCOL_MAX_RETRIES            (3)
#define OMV_PROTOCOL_MAGIC_BAUDRATE         (921600)

// Rounds up packets to EP size.
#ifndef OMV_PROTOCOL_MAX_BUFFER_SIZE
#ifndef MICROPY_HW_USB_CDC_TX_DATA_SIZE
#define OMV_PROTOCOL_MAX_BUFFER_SIZE        (1024)
#else
#define OMV_PROTOCOL_MAX_BUFFER_SIZE        (MICROPY_HW_USB_CDC_TX_DATA_SIZE)
#endif
#endif

#define OMV_PROTOCOL_MIN_PAYLOAD_SIZE       (64   - OMV_PROTOCOL_HEADER_SIZE - 2)
#define OMV_PROTOCOL_MAX_PAYLOAD_SIZE       (OMV_PROTOCOL_MAX_BUFFER_SIZE - OMV_PROTOCOL_HEADER_SIZE - 2)
#define OMV_PROTOCOL_MAX_PACKET_SIZE        (OMV_PROTOCOL_HEADER_SIZE + OMV_PROTOCOL_MAX_PAYLOAD_SIZE + 2)
#define OMV_PROTOCOL_GET_PACKET_SIZE(pkt)   (OMV_PROTOCOL_HEADER_SIZE + (pkt->length + (!!pkt->length * 2)))

// Macro for protocol struct size validation
#define OMV_PROTOCOL_ASSERT_SIZE(type, size) \
    _Static_assert(sizeof(type) == (size), #type " must be exactly " #size " bytes")

/***************************************************************************
 * Packet Flags
 ***************************************************************************/

typedef enum {
    OMV_PROTOCOL_FLAG_ACK                   = (1 << 0),  // ACK packet
    OMV_PROTOCOL_FLAG_NAK                   = (1 << 1),  // NAK packet
    OMV_PROTOCOL_FLAG_FRAGMENT              = (1 << 2),  // More fragments to follow
    // Bits 2-7 reserved for future use
} omv_protocol_flags_t;

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
 * Protocol State Machine
 ***************************************************************************/

typedef enum {
    OMV_PROTOCOL_STATE_SYNC                 = 0x00,   // Wait for sync bytes
    OMV_PROTOCOL_STATE_HEADER               = 0x01,   // Reading header
    OMV_PROTOCOL_STATE_PAYLOAD              = 0x02,   // Reading data
} omv_protocol_state_t;

/***************************************************************************
 * Protocol Opcodes              
 ***************************************************************************/

typedef enum {
    // Protocol Control Commands (0x00-0x0F)
    OMV_PROTOCOL_OPCODE_PROTO_SYNC          = 0x00,  // Synchronization request
    OMV_PROTOCOL_OPCODE_PROTO_GET_CAPS      = 0x01,  // Get capabilities
    OMV_PROTOCOL_OPCODE_PROTO_SET_CAPS      = 0x02,  // Set capabilities
    
    // System Control Commands (0x10-0x1F)
    OMV_PROTOCOL_OPCODE_SYS_RESET           = 0x10,  // System reset
    OMV_PROTOCOL_OPCODE_SYS_BOOT            = 0x11,  // Jump to bootloader
    OMV_PROTOCOL_OPCODE_SYS_INFO            = 0x12,  // Get system info
       
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
} omv_protocol_opcode_t;

/***************************************************************************
 * Protocol Payloads
 ***************************************************************************/

// NAK respone structure
typedef struct __attribute__((packed)) {
    uint16_t status;    // Error status
} omv_protocol_response_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_response_t, 2);

// Get/Set capabilities structure
typedef struct __attribute__((packed)) {
    uint32_t crc_enabled : 1;
    uint32_t seq_enabled : 1;
    uint32_t ack_enabled : 1;
    uint32_t frag_enabled : 1;
    uint32_t reserved1 : 28;
    uint16_t max_payload;
    uint16_t max_retries;
    uint16_t timeout_ms;
    uint16_t lock_wait_ms;
    uint8_t reserved2[20];
} omv_protocol_caps_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_caps_t, 32);

// System information structure
typedef struct __attribute__((packed)) {
    // Hardware identification
    uint32_t cpu_id;                    // CPUID register value
    uint32_t dev_id[3];                 // Unique device ID register
    uint32_t chip_id;                   // Camera sensor chip ID
    uint32_t id_reserved[1];            // Reserved for future expansion
       
    // Hardware capabilities
    union {
        struct {
            uint32_t gpu_present:1;     // Graphics Processing Unit
            uint32_t npu_present:1;     // Neural Processing Unit
            uint32_t isp_present:1;     // Image Signal Processor
            uint32_t venc_present:1;    // Video encoder present
            uint32_t jpeg_present:1;    // JPEG encoder present
            uint32_t dram_present:1;    // DRAM present
            uint32_t pmu_present:1;     // Performance Monitoring Unit
            uint32_t pmu_eventcnt:6;    // PMU number of event counters
            uint32_t wifi_present:1;    // WiFi module present
            uint32_t bt_present:1;      // Bluetooth available
            uint32_t sd_present:1;      // SD card slot available
            uint32_t eth_present:1;     // Ethernet interface
            uint32_t usb_highspeed:1;   // USB High-Speed capable
            uint32_t :12;               // Reserved bits
        };
        uint32_t hw_caps;
    };
    
    // Memory information
    uint32_t flash_size_kb;             // Flash memory size in KB
    uint32_t ram_size_kb;               // RAM size in KB
    uint32_t framebuffer_size_kb;       // Framebuffer size in KB
    uint32_t stream_buffer_size_kb;     // Streaming buffer size in KB
    uint32_t memory_reserved[2];        // Reserved for future expansion
    
    // Version information
    uint8_t firmware_version[3];        // Major, Minor, Patch
    uint8_t protocol_version[3];        // Major, Minor, Patch
    uint8_t bootloader_version[3];      // Bootloader version
    
    // Pad struct to 64 bytes
    uint8_t reserved[3];
} omv_protocol_sys_info_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_sys_info_t, 64);

// Channel size structure
typedef struct __attribute__((packed)) {
    uint32_t size;
} omv_protocol_channel_size_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_size_t, 4);

// Channel request structure
typedef struct __attribute__((packed)) {
    uint32_t offset;
    uint32_t length;
    uint8_t payload[];
} omv_protocol_channel_request_t;

// Channel ioctl structure
typedef struct __attribute__((packed)) {
    uint32_t request;
    uint8_t payload[];
} omv_protocol_channel_ioctl_t;

// Channel list entry structure
typedef struct __attribute__((packed)) {
    uint8_t id;
    uint8_t flags;
    char name[OMV_PROTOCOL_CHANNEL_NAME_SIZE];
} omv_protocol_channel_entry_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_entry_t, 16);

// Channel poll response structure
typedef struct __attribute__((packed)) {
    uint32_t flags;
} omv_protocol_channel_poll_t;
OMV_PROTOCOL_ASSERT_SIZE(omv_protocol_channel_poll_t, 4);

// Packet structure
typedef struct __attribute__((packed)) {
    uint16_t sync;          // Synchronization word (0xAA55)
    uint16_t sequence;      // Sequence number
    uint16_t channel;       // Channel ID
    uint8_t  flags;         // Packet flags
    uint8_t  opcode;        // Command/response opcode
    uint16_t length;        // Length of data field only
    uint16_t crc;           // CRC of header fields
    uint8_t payload[];      // Flexible array member
} omv_protocol_packet_t;

/***************************************************************************
 * Channel Interface
 ***************************************************************************/

// Channel flags
typedef enum {
    OMV_PROTOCOL_CHANNEL_FLAG_READ     = (1 << 0),  // Channel supports read operations
    OMV_PROTOCOL_CHANNEL_FLAG_WRITE    = (1 << 1),  // Channel supports write operations  
    OMV_PROTOCOL_CHANNEL_FLAG_LOCK     = (1 << 2),  // Channel requires and supports locking
    OMV_PROTOCOL_CHANNEL_FLAG_PHYSICAL = (1 << 3),  // Physical transport channel (not accessible via protocol)
} omv_protocol_channel_flags_t;

// Reserved/predefined channel IDs (usable as array indices)
typedef enum {
    OMV_PROTOCOL_CHANNEL_ID_TRANSPORT = 0,      // Transport layer (not accessible via protocol)
    OMV_PROTOCOL_CHANNEL_ID_STDIN     = 1,      // Script input (write-only)
    OMV_PROTOCOL_CHANNEL_ID_STDOUT    = 2,      // Text output (read-only)
    OMV_PROTOCOL_CHANNEL_ID_STREAM    = 3,      // Stream data (read-only)
    OMV_PROTOCOL_CHANNEL_ID_PROFILE   = 4,      // Profiling data (read-only)
} omv_protocol_channel_id_t;

// Forward declaration
typedef struct omv_protocol_channel omv_protocol_channel_t;

// Channel interface (used for both transport and logical channels)
struct omv_protocol_channel {
    uint8_t id;     // Channel identifier (0=transport, 1+=logical channels)
    uint32_t flags;
    void *priv;
    char name[OMV_PROTOCOL_CHANNEL_NAME_SIZE];
    int (*init)(const omv_protocol_channel_t *channel);
    int (*deinit)(const omv_protocol_channel_t *channel);
    bool (*poll)(const omv_protocol_channel_t *channel);
    int (*lock)(const omv_protocol_channel_t *channel);
    int (*unlock)(const omv_protocol_channel_t *channel);
    size_t (*size)(const omv_protocol_channel_t *channel);
    size_t (*shape)(const omv_protocol_channel_t *channel, size_t shape[4]);
    int (*read)(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, void *data);
    int (*write)(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, const void *data);
    const void *(*readp)(const omv_protocol_channel_t *channel, uint32_t offset, size_t size);
    int (*flush)(const omv_protocol_channel_t *channel);
    int (*ioctl)(const omv_protocol_channel_t *channel, uint32_t cmd, void *arg);
    // Transport-specific functions (for channel ID 0 only)
    int (*phy_read)(const omv_protocol_channel_t *channel, size_t size, void *data, uint32_t timeout_ms);
    int (*phy_write)(const omv_protocol_channel_t *channel, size_t size, const void *data, uint32_t timeout_ms);
};

/***************************************************************************
 * Protocol Context
 ***************************************************************************/

// Protocol statistics
typedef struct {
    uint32_t sent_packets;
    uint32_t recv_packets;
    uint32_t checksum_errors;
    uint32_t sequence_errors;
    uint32_t retransmit;
} omv_protocol_stats_t;


// Protocol context
typedef struct {   
    // State machine
    bool active;                // Protocol is active
    uint16_t sequence;          // Sequence number
    bool wait_for_ack;          // Waiting for ACK of last packet
    uint32_t rtx_size;
    const uint8_t *rtx_data;
    uint8_t rtx_opcode;
    uint32_t last_lock_ms;      // Timestamp of the last successful lock
    omv_protocol_state_t state;

    // Protocol capabilities/configuration
    omv_protocol_caps_t caps;

    // Buffer for received data
    omv_buffer_t buffer;
    uint8_t rawbuf[OMV_PROTOCOL_MAX_BUFFER_SIZE];
    
    // Protocol physical/logical channels
    uint8_t channels_count;
    const omv_protocol_channel_t *channels[OMV_PROTOCOL_MAX_CHANNELS];
    
    // Protocol Statistics
    omv_protocol_stats_t stats;
} omv_protocol_context_t;

// Built-in Channels
extern const omv_protocol_channel_t omv_usb_channel;
extern const omv_protocol_channel_t omv_stdin_channel;
extern const omv_protocol_channel_t omv_stdout_channel;
extern const omv_protocol_channel_t omv_stream_channel;
extern const omv_protocol_channel_t omv_profile_channel;

// Initialize protocol context
int omv_protocol_init(const omv_protocol_channel_t *transport, const omv_protocol_caps_t *caps);

// Deinitialize protocol context
int omv_protocol_deinit(void);

// Helper function to check if the transport is active.
bool omv_protocol_active(void);

// Helper function to exec scripts in stdio buffer.
// Returns false, if no script is ready, true on executing the script.
bool omv_protocol_exec_script(void);

// Register a channel
int omv_protocol_register_channel(const omv_protocol_channel_t *channel);

// Find channel by ID.
const omv_protocol_channel_t *omv_protocol_get_channel(uint8_t channel_id);

// Send ACK/NAK packet
void omv_protocol_send_status(uint8_t opcode, omv_protocol_status_t status);

// Send response packet
int omv_protocol_send_packet(uint8_t opcode, size_t size, const void *data, uint8_t flags);

// Call on events or periodically to process events
int omv_protocol_task();

// Process assembled packet
void omv_protocol_process(const omv_protocol_packet_t *packet);
#endif // __OMV_PROTOCOL_H__
