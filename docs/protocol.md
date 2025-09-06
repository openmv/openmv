# OpenMV Protocol Specification

## 1. Overview

The OpenMV Protocol is a channel-based communication protocol for OpenMV camera devices. It provides reliable command/response communication with error detection, packet sequencing, and optimized data channel operations.

### 1.1 Key Features
- Channel-based architecture with 8-bit channel IDs (up to 32 channels)
- Transport-agnostic design (USB, UART, TCP/IP)
- Dual CRC error detection (CRC-16 for header, CRC-32 for payload)
- Packet fragmentation for large transfers
- Zero-copy data transmission with readp() interface
- Channel-specific operations (read, write, ioctl, lock/unlock, shape)
- Configurable CRC and sequence number validation
- Configurable async event notifications
- Retransmission support with exponential backoff

### 1.2 Channel Architecture

The protocol uses a channel-based model where every readable/writable resource is abstracted as a data channel (such as stdout, stdin, the frame buffer, etc.). Channels have flags that indicate if they're read-only, write-only, or read/write. The protocol provides these predefined channels, and it's possible to extend it with user-defined channels from Python. Each channel implements a standard interface with init, read, write, flush, ioctl, available, and locking operations as needed.

## 2. Packet Structure

### 2.1 Packet Format

All packets follow this structure:

| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0      | SYNC     | 2 bytes  | Pattern                 | Synchronization pattern (0xD5AA)  |
| 2      | SEQ      | 1 byte   | Number                  | Sequence number (0-255), wraps    |
| 3      | CHAN     | 1 byte   | ID                      | Channel ID for routing data ops   |
| 4      | FLAGS    | 1 byte   | Flags                   | Packet flags (see section 2.2)    |
| 5      | OPCODE   | 1 byte   | Code                    | Command/Response operation code   |
| 6      | LENGTH   | 2 bytes  | Length                  | Length of data field only         |
| 8      | CRC      | 2 bytes  | Checksum                | CRC-16 over header fields         |
| 10     | DATA     | Variable | Payload                 | Optional payload data             |
| 10+N   | DATA CRC | 4 bytes  | Checksum                | CRC-32 over data (if N > 0)       |

Total header size: 10 bytes (including header CRC)
Data CRC: 4 bytes (only present if LENGTH > 0)

### 2.2 Flags Field

```
Bit 7   6   5   4   3   2   1   0
    │   │   │   │   │   │   │   └─ ACK: Acknowledgment packet
    │   │   │   │   │   │   └───── NAK: Negative acknowledgment packet
    │   │   │   │   │   └───────── RTX: Retransmission packet
    │   │   │   │   └───────────── ACK_REQ: Packet requires acknowledgment
    │   │   │   └───────────────── FRAGMENT: More fragments follow
    │   │   └───────────────────── EVENT: Event packet
    │   └───────────────────────── Reserved
    └───────────────────────────── Reserved
```

- **ACK (bit 0)**: Set for acknowledgment packets
- **NAK (bit 1)**: Set for negative acknowledgment packets
- **RTX (bit 2)**: Set for retransmission packets
- **ACK_REQ (bit 3)**: Set when packet requires acknowledgment
- **FRAGMENT (bit 4)**: Set when more fragments follow, clear for last fragment
- **EVENT (bit 5)**: Set for event packets

## 3. Protocol Operations

### 3.1 Sequence Numbers

- Each endpoint maintains separate sequence counters for TX and RX
- Sequence numbers increment for each new packet (not retransmissions)
- Used to detect duplicates and missing packets
- Wraps from 255 to 0 (8-bit counter)

### 3.2 Acknowledgment Mechanism

All response packets automatically include ACK flag unless NAK is explicitly set:
1. ACK packets indicate successful processing
2. NAK packets include status code indicating the error
3. Sequence numbers increment with each packet sent

### 3.3 Fragmentation

The protocol supports fragmentation of large data packets. For data larger than maximum payload size, the fragmentation flag will be set to indicate more fragments to follow, and cleared on the last fragment.

## 4. Command Set

### 4.1 Command Categories

Commands are organized by functional area:

| Range | Category | Description |
|-------|----------|-------------|
| 0x00-0x0F | Protocol Control | Sync, capabilities |
| 0x10-0x1F | System Control | Reset, boot, system info, events |
| 0x20-0x2F | Channel Operations | Channel list, poll, lock, size, read, write, ioctl |
| 0x30-0xFF | Reserved/Extensions | Future use |

### 4.2 Command Summary

#### Protocol Control (0x00-0x0F)
| Opcode | Command | Payload Size | Response Size | Description |
|--------|---------|-------------|---------------|-------------|
| 0x00 | PROTO_SYNC | 0 bytes | 0 bytes (ACK) | Synchronization request |
| 0x01 | PROTO_GET_CAPS | 0 bytes | 16 bytes | Get protocol capabilities |
| 0x02 | PROTO_SET_CAPS | 16 bytes | 0/2 bytes | Set protocol capabilities |
| 0x03 | PROTO_STATS | 0 bytes | 32 bytes | Get protocol statistics |
| 0x04 | PROTO_VERSION | 0 bytes | 16 bytes | Get version information |

#### System Control (0x10-0x1F)
| Opcode | Command | Payload Size | Response Size | Description |
|--------|---------|-------------|---------------|-------------|
| 0x10 | SYS_RESET | 0 bytes | No response | System reset |
| 0x11 | SYS_BOOT | 0 bytes | No response | Jump to bootloader |
| 0x12 | SYS_INFO | 0 bytes | 76 bytes | Get system information |
| 0x13 | SYS_EVENT | 4 bytes | No response | System event notification |

#### Channel Operations (0x20-0x2F)
| Opcode | Command | Payload Size | Response Size | Description |
|--------|---------|-------------|---------------|-------------|
| 0x20 | CHANNEL_LIST | 0 bytes | Variable (16 bytes × channel count) | List registered channels |
| 0x21 | CHANNEL_POLL | 0 bytes | 4 bytes (flags) | Poll channels status |
| 0x22 | CHANNEL_LOCK | 0 bytes | 2 bytes (status) | Lock channel for exclusive access |
| 0x23 | CHANNEL_UNLOCK | 0 bytes | 2 bytes (status) | Unlock channel |
| 0x24 | CHANNEL_SHAPE | 0 bytes | Variable | Get data shape/dimensions |
| 0x25 | CHANNEL_SIZE | 0 bytes | 4 bytes | Get available data size |
| 0x26 | CHANNEL_READ | 8 bytes (offset + length) | Variable (requested data) | Read data from channel |
| 0x27 | CHANNEL_WRITE | 8+ bytes (offset + length + data) | 2 bytes (status) | Write data to channel |
| 0x28 | CHANNEL_IOCTL | 4+ bytes (request + data) | 2 bytes (status) | Channel-specific control operation |
| 0x29 | CHANNEL_EVENT | 4+ bytes (event + data) | No response | Channel-specific event notification |

### 4.3 Detailed Command Formats

#### Protocol Control Commands

**PROTO_SYNC (0x00)**

Request Format: (No payload)

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code (0x00 = success)      |

**PROTO_GET_CAPS (0x01)**

Request Format: (No payload)

Response Format: (16 bytes)
| Offset | Field         | Size     | Type                 | Description                       |
|--------|---------------|----------|----------------------|-----------------------------------|
| 0-3    | flags         | 4 bytes  | Bitfield             | Capability flags (bits 0-3)       |
|        |               |          |                      | bit 0: CRC enabled                |
|        |               |          |                      | bit 1: Sequence enabled           |
|        |               |          |                      | bit 2: ACK enabled                |
|        |               |          |                      | bit 3: Events enabled             |
| 4-5    | max_payload   | 2 bytes  | Size                 | Maximum payload size              |
| 6-15   | reserved      | 10 bytes | Padding              | Reserved for future use           |

**PROTO_SET_CAPS (0x02)**

Request Format: (16 bytes)
| Offset | Field         | Size     | Type                 | Description                       |
|--------|---------------|----------|----------------------|-----------------------------------|
| 0-3    | flags         | 4 bytes  | Bitfield             | Capability flags to set           |
| 4-5    | max_payload   | 2 bytes  | Size                 | Maximum payload size              |
| 6-15   | reserved      | 10 bytes | Padding              | Reserved for future use           |

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code (0x00 = success)      |

**PROTO_STATS (0x03)**

Request Format: (No payload)

Response Format: (32 bytes)
| Offset | Field                | Size     | Type                 | Description                       |
|--------|----------------------|----------|----------------------|-----------------------------------|
| 0-3    | sent_packets         | 4 bytes  | Count                | Number of packets sent            |
| 4-7    | recv_packets         | 4 bytes  | Count                | Number of packets received        |
| 8-11   | checksum_errors      | 4 bytes  | Count                | Number of CRC errors detected     |
| 12-15  | sequence_errors      | 4 bytes  | Count                | Number of sequence errors         |
| 16-19  | retransmit           | 4 bytes  | Count                | Number of packet retransmissions  |
| 20-23  | transport_errors     | 4 bytes  | Count                | Number of transport layer errors  |
| 24-27  | sent_events          | 4 bytes  | Count                | Number of events sent             |
| 28-31  | reserved             | 4 bytes  | Reserved             | Reserved for future use           |

**PROTO_VERSION (0x04)**

Request Format: (No payload)

Response Format: (16 bytes)
| Offset | Field               | Size     | Type         | Description                       |
|--------|---------------------|----------|--------------|-----------------------------------|
| 0-2    | protocol_version    | 3 bytes  | Version      | Protocol version (Major, Minor, Patch) |
| 3-5    | bootloader_version  | 3 bytes  | Version      | Bootloader version (Major, Minor, Patch) |
| 6-8    | firmware_version    | 3 bytes  | Version      | Firmware version (Major, Minor, Patch) |
| 9-15   | reserved            | 7 bytes  | Padding      | Reserved for future expansion     |

#### System Control Commands

**SYS_RESET (0x10)**

Request Format: (No payload)  
Response: (No response - system resets immediately)

**SYS_BOOT (0x11)**

Request Format: (No payload)  
Response: (No response - system enters bootloader)

**SYS_EVENT (0x13)**

Request Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | event    | 4 bytes  | Event code              | Event type and data               |

Response: (No response - events are notifications)

Event types:
- 0x00: CHANNEL_REGISTERED - Channel dynamically registered
- 0x01: CHANNEL_UNREGISTERED - Channel unregistered
- 0x02: SOFT_REBOOT - System is going for a soft-reboot
- 0xFFFF: NOTIFY - Notification event (no payload)

Note: Events are only sent when the events_enabled capability is set to true.

**SYS_INFO (0x12)**

Request Format: (No payload)

Response Format: (76 bytes)
| Offset | Field               | Size     | Type         | Description                       |
|--------|---------------------|----------|--------------|-----------------------------------|
| 0-3    | cpu_id              | 4 bytes  | Register     | ARM CPUID register value          |
| 4-15   | dev_id              | 12 bytes | ID Array     | Unique device ID                  |
| 16-19  | usb_id              | 4 bytes  | ID           | USB VID/PID (VID in upper 16 bits) |
| 20-31  | chip_id             | 12 bytes | ID Array     | A list of detected sensors        |
| 32-39  | reserved            | 8 bytes  | Padding      | Reserved for future expansion     |
| 40-47  | hw_caps             | 8 bytes  | Bitfield     | Hardware capability flags:        |
|        |                     |          |              | bit 0: GPU,                       |
|        |                     |          |              | bit 1: NPU,                       |
|        |                     |          |              | bit 2: ISP,                       |
|        |                     |          |              | bit 3: Video encoder,             |
|        |                     |          |              | bit 4: JPEG encoder,              |
|        |                     |          |              | bit 5: DRAM,                      |
|        |                     |          |              | bit 6: Hardware CRC,              |
|        |                     |          |              | bit 7: PMU available,             |
|        |                     |          |              | bits 8-15: PMU event counter,     |
|        |                     |          |              | bit 16: WiFi,                     |
|        |                     |          |              | bit 17: Bluetooth,                |
|        |                     |          |              | bit 18: SD card,                  |
|        |                     |          |              | bit 19: Ethernet,                 |
|        |                     |          |              | bit 20: USB High-Speed,           |
|        |                     |          |              | bit 21: Multi-core                |
| 48-51  | flash_size_kb       | 4 bytes  | Size         | Flash memory size (KB)            |
| 52-55  | ram_size_kb         | 4 bytes  | Size         | RAM size (KB)                     |
| 56-59  | framebuffer_size_kb | 4 bytes  | Size         | Main framebuffer size (KB)        |
| 60-63  | stream_buffer_size_kb| 4 bytes | Size         | Stream framebuffer size (KB)      |
| 64-75  | reserved            | 12 bytes | Padding      | Reserved for future expansion     |

#### Channel Operations Commands

**CHANNEL_LIST (0x20)**

Request Format: (No payload)

Response Format: (Variable length - 16 bytes per channel)
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| N×0    | id       | 1 byte   | ID                      | Channel ID                        |
| N×1    | flags    | 1 byte   | Bitfield                | Channel capability flags          |
| N×2-15 | name     | 14 bytes | String                  | Channel name (null-terminated)    |

(Repeat for each registered channel)

**CHANNEL_POLL (0x21)**

Request Format: (No payload)

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | flags    | 4 bytes  | Bitfield                | Channel status flags              |

**CHANNEL_LOCK (0x22) / CHANNEL_UNLOCK (0x23)**

Request Format: (No payload)

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code                       |

**CHANNEL_SIZE (0x25)**

Request Format: (No payload)

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | size     | 4 bytes  | Count                   | Available bytes in channel        |

**CHANNEL_READ (0x26)**

Request Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | offset   | 4 bytes  | Position                | Starting position to read         |
| 4-7    | length   | 4 bytes  | Count                   | Number of bytes to read           |

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-N    | data     | N bytes  | Payload                 | Requested channel data            |

**CHANNEL_WRITE (0x27)**

Request Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | offset   | 4 bytes  | Position                | Starting position to write        |
| 4-7    | length   | 4 bytes  | Count                   | Number of bytes to write          |
| 8-N    | data     | N bytes  | Payload                 | Data to write to channel          |

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code                       |

**CHANNEL_IOCTL (0x28)**

Request Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | request  | 4 bytes  | Code                    | IOCTL request code                |
| 4-N    | data     | N bytes  | Payload                 | Request-specific data             |

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code                       |

**CHANNEL_SHAPE (0x24)**

Request Format: (No payload)

Response Format: (Variable length - array of size_t values)
| Offset | Field      | Size     | Type                  | Description                       |
|--------|------------|----------|-----------------------|-----------------------------------|
| 0-3    | dimension0 | 4 bytes  | size_t                | First dimension (e.g., count)     |
| 4-7    | dimension1 | 4 bytes  | size_t                | Second dimension (e.g., size)     |
| ...    | ...        | 4 bytes  | size_t                | Additional dimensions (if any)    |

The response contains 1-4 size_t values depending on the channel type:
- Profile channel: 2 values (record_count, record_size)
- Stream channel: 1 value (total_size) for compressed, 3 values (width, height, bpp) for uncompressed
- Other channels: Channel-specific format


### 4.4 Async Events

The protocol supports asynchronous event notifications that can be sent from device to host at any time when the `events_enabled` capability is set to true. There are two types of events:

1. **System Events**: Use the SYS_EVENT opcode (0x13) on channel 0 and are marked with the EVENT flag (bit 3)
2. **Channel Events**: Use the CHANNEL_EVENT opcode (0x29) on the specific channel that generates the event

Events do not require acknowledgment and do not affect sequence numbering. The device will only send events if:
1. The protocol transport is active
2. The ACK queue is not full  
3. The events_enabled capability is set to true

**System Events (SYS_EVENT - 0x13):**
- **CHANNEL_REGISTERED (0x00)**: Sent when a dynamic channel is registered
- **CHANNEL_UNREGISTERED (0x01)**: Sent when a dynamic channel is removed
- **SOFT_REBOOT (0x02)**: Sent when system is going for a soft-reboot

**Channel Events (CHANNEL_EVENT - 0x29):**
Channel events are sent on the specific channel that generates the event and can include channel-specific event data in the payload.

### 4.5 Status Codes

Protocol status responses use these codes:

| Code | Name     | Value    | Type                      | Description                       |
|------|----------|----------|---------------------------|-----------------------------------|
| 0x00 | SUCCESS  | 0        | Status                    | Operation completed successfully  |
| 0x01 | FAILED   | 1        | Error                     | Command failed                    |
| 0x02 | INVALID  | 2        | Error                     | Invalid command/argument          |
| 0x03 | TIMEOUT  | 3        | Error                     | Operation timeout                 |
| 0x04 | BUSY     | 4        | Status                    | Device busy                       |
| 0x05 | CHECKSUM | 5        | Error                     | CRC error                         |
| 0x06 | SEQUENCE | 6        | Error                     | Sequence error                    |
| 0x07 | OVERFLOW | 7        | Error                     | Buffer overflow                   |
| 0x08 | FRAGMENT | 8        | Error                     | Fragmentation error               |
| 0x09 | UNKNOWN  | 9        | Error                     | Unknown error                     |

## 5. Channel System

The OpenMV Protocol uses a channel-based architecture where each channel represents a specialized data pipeline. Channels are identified by numeric IDs and provide a uniform interface for accessing different types of data and functionality.

### 5.1 Channel Types

**Channel 0 (transport)**: The physical transport layer (USB, UART, TCP/IP) that handles low-level data transmission. Not directly accessible via protocol commands.

**Channel 1 (stdin)**: Script input channel (write-only) for sending Python code to the device for execution.

**Channel 2 (stdout)**: Text output channel (read-only) for receiving console output and script results from the device.

**Channel 3 (stream)**: Read-only channel for high-bandwidth data like image frames. Supports exclusive locking to prevent concurrent access conflicts.

**Channel 4 (profile)**: Optional read-only channel providing performance metrics and diagnostic information. Profiling availability is determined by the presence of this channel rather than a capability flag. The channel is only registered when OMV_PROFILER_ENABLE is defined at compile time. The number of PMU event counters is embedded in the hardware capabilities bitfield (bits 8-15) in the system information.

### 5.2 Channel Capabilities

Each channel declares its supported operations through capability flags:
- **READ**: Channel supports read operations
- **WRITE**: Channel supports write operations
- **EXEC**: Channel supports script execution
- **LOCK**: Channel supports exclusive locking
- **STREAM**: Streaming channel
- **DYNAMIC**: Channel was dynamically created
- **PHYSICAL**: Transport-only channel (not accessible via protocol)

### 5.3 Channel Operations

**Discovery**: The CHANNEL_LIST command returns available channels with their IDs, capabilities, and names.

**Data Access**: Read operations specify offset and length parameters. The offset meaning depends on the channel - it might be a byte position, frame number, or other logical addressing scheme.

**Zero-Copy Reads**: The readp operation returns direct pointers to channel data, eliminating copy overhead for large transfers.

**Control**: The ioctl operation handles channel-specific commands like configuration changes or status queries.

**Flow Control**: Channels return appropriate status codes when data isn't ready, and use locking mechanisms to coordinate access between host and device.

### 5.4 Channel IOCTL Commands

Each channel type supports specific IOCTL commands for configuration and control:

#### Stdin Channel IOCTLs
| Request | Name | Payload Size | Description |
|---------|------|--------------|-------------|
| 0x01 | STDIN_STOP | 0 bytes | Stop running script |
| 0x02 | STDIN_EXEC | 0 bytes | Execute buffered script |
| 0x03 | STDIN_RESET | 0 bytes | Reset script buffer |

#### Stream Channel IOCTLs
| Request | Name | Payload Size | Description |
|---------|------|--------------|-------------|
| 0x00 | STREAM_CTRL | 4 bytes | Enable/disable streaming (uint32_t enable) |
| 0x01 | STREAM_RAW_CTRL | 4 bytes | Enable/disable raw streaming (uint32_t enable) |
| 0x02 | STREAM_RAW_CFG | 8 bytes | Set raw stream resolution (uint32_t width, uint32_t height) |

#### Profile Channel IOCTLs
| Request | Name | Payload Size | Description |
|---------|------|--------------|-------------|
| 0x00 | PROFILE_MODE | 4 bytes | Set profiling mode (uint32_t mode) |
| 0x01 | PROFILE_SET_EVENT | 8 bytes | Set event type to profile |
| 0x02 | PROFILE_RESET | 0 bytes | Reset profiler data |
| 0x03 | PROFILE_STATS | 0 bytes | Get profiler statistics |

## 6. Error Handling

### 6.1 CRC Verification

The protocol uses custom CRC polynomials optimized for Hamming distance (HD=6):

**Header CRC (CRC-16)**
- Polynomial: 0xF94F
- Initial value: 0xFFFF
- HD=6 up to 114 bits (covers 10-byte header)
- Calculated over header fields (excluding the CRC field itself)

**Payload CRC (CRC-32)**
- Polynomial: 0xFA567D89
- Initial value: 0xFFFFFFFF
- HD=6 up to 32738 bits (~4KB payload)
- Only present if payload length > 0
- Calculated over payload data (excluding the CRC field itself)

Hardware CRC acceleration is available on supported platforms (indicated by HW_CAPS bit 6).

### 6.2 Stream Recovery
- On sync loss, scan for SYNC pattern (0xD5AA)
- Validate packet header after SYNC
- Verify CRC before processing

### 6.3 Sequence Numbers
- Increment after successful packet transmission
- Reset to 0 on PROTO_SYNC command
- Used for duplicate detection

## 7. Protocol State Machine

The state machine is driven by the transport layer when data is available:

```
┌──────┐
│ IDLE │◄──────────────────────┐
└──┬───┘                       │
   │ Data available            │
   ▼                           │
┌──────┐                       │
│ SYNC │───── Timeout ─────────┤
└──┬───┘                       │
   │ SYNC found                │
   ▼                           │
┌────────┐                     │
│ HEADER │──── Invalid ────────┤
└──┬─────┘                     │
   │ Valid header              │
   ▼                           │
┌──────┐                       │
│ DATA │───── Timeout ─────────┤
└──┬───┘                       │
   │ Data complete             │
   ▼                           │
┌─────┐                        │
│ CRC │────── Invalid ─────────┤
└──┬──┘                        │
   │ Valid CRC                 │
   ▼                           │
┌─────────┐                    │
│ PROCESS │────────────────────┘
└─────────┘
```

## 8. Implementation Notes

### 8.1 Zero-Copy Optimization
- readp() interface allows direct pointer access to data without copying
- Header, payload, and data CRC sent as separate transport writes
- Incremental CRC calculation supports non-contiguous data

### 8.2 Buffer Requirements
- Maximum buffer size: configurable via OMV_PROTOCOL_MAX_BUFFER_SIZE (default 4096 bytes)
- Maximum payload: buffer size - 14 bytes (e.g., 4082 bytes with 4096 buffer)
- Minimum payload: 50 bytes (64 - 10 header - 4 data CRC)
- Contiguous buffer implementation that compacts data as needed for zero-copy packet access

### 8.3 Channel Registration
- Maximum 32 channels supported (indices 0-31)
- Channels registered at protocol initialization
- Channel 0 is transport layer (not accessible via protocol)
- Protocol responses use channel 0 in packet header but are routed through transport

## 9. Example Communication Flow

### 9.1 Channel Read Operation
```
Host                           Device
  │                              │
  ├─── CHANNEL_READ(CHAN=2) ────►│  // Read frame data
  │    offset=0, length=1024     │
  │                              │
  │◄─ Response(FRAGMENTED) ──────┤  // More fragments follow
  │                              │
  │◄─ Response(no flags) ────────┤  // Final fragment
  │                              │
```

### 9.2 Channel Discovery
```
Host                           Device
  │                              │
  ├─── CHANNEL_LIST ────────────►│
  │                              │
  │◄─ Channel entries ───────────┤  // Array of channel info
  │                              │
  ├─── CHANNEL_SIZE(CHAN=2) ────►│
  │                              │
  │◄─ Size response ─────────────┤  // Available bytes
  │                              │
```

## 10. Protocol Configuration

### 10.1 Protocol Capabilities

The protocol supports negotiation of these capabilities between host and device:

- **crc_enabled**: Enable/disable CRC error checking
- **seq_enabled**: Enable/disable sequence number validation
- **ack_enabled**: Enable/disable ACK packet requirement
- **events_enabled**: Enable/disable async event notifications
- **max_payload**: Maximum payload size supported

### 10.2 Default Capabilities

The protocol uses these default settings:

- **CRC enabled**: true
- **Sequence validation**: true
- **ACK enabled**: true
- **Events enabled**: true
- **Max payload**: 4082 bytes (4096 - 10 header - 4 data CRC)
- **Soft reboot**: true
- **RTX retries**: 3
- **RTX timeout**: 500ms (doubled after each timeout)
- **Lock interval**: 10ms

Fragmentation is always supported and handled automatically by the protocol when data exceeds the maximum payload size.

### 10.3 Capability Negotiation

The `PROTO_GET_CAPS` and `PROTO_SET_CAPS` commands allow dynamic configuration. See the detailed command formats above for the exact 32-byte payload structure.

## 11. System Information

The `SYS_INFO` command returns hardware and memory information in a 76-byte response, while `PROTO_VERSION` returns version information in a 16-byte response. See the detailed command formats above for the complete field layouts.

## 12. Version History

| Version | Date     | Status   | Type                    | Description                            |
|---------|----------|----------|-------------------------|----------------------------------------|
| 1.0.0   | 2025     | Current  | Specification           | OpenMV Protocol specification          |
