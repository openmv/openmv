# OpenMV Protocol Specification

## 1. Overview

The OpenMV Protocol is a channel-based communication protocol for OpenMV camera devices. It provides reliable command/response communication with error detection, packet sequencing, and optimized data channel operations.

### 1.1 Key Features
- Channel-based architecture with 8-bit channel IDs (up to 10 channels)
- Transport-agnostic design (USB, UART, TCP/IP)
- Dual CRC-16-CCITT error detection (separate header and payload CRCs)
- Packet fragmentation for large transfers
- Zero-copy data transmission with readp() interface
- Channel-specific operations (read, write, ioctl, lock/unlock, shape)
- Configurable CRC and sequence number validation
- Real-time stream processing with ring buffer

### 1.2 Channel Architecture

The protocol uses a channel-based model where every readable/writable resource is abstracted as a data channel (such as stdout, stdin, the frame buffer, etc.). Channels have flags that indicate if they're read-only, write-only, or read/write. The protocol provides these predefined channels, and it's possible to extend it with user-defined channels from Python. Each channel implements a standard interface with init, read, write, flush, ioctl, available, and locking operations as needed.

## 2. Packet Structure

### 2.1 Packet Format

All packets follow this structure:

| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0      | SYNC     | 2 bytes  | Pattern                 | Synchronization pattern (0xAA55)  |
| 2      | SEQ      | 2 bytes  | Number                  | Sequence number (0-65535), wraps  |
| 4      | CHAN     | 2 bytes  | ID                      | Channel ID for routing data ops   |
| 6      | FLAGS    | 1 byte   | Flags                   | Packet flags (see section 2.2)    |
| 7      | OPCODE   | 1 byte   | Code                    | Command/Response operation code   |
| 8      | LENGTH   | 2 bytes  | Length                  | Length of data field only         |
| 10     | CRC      | 2 bytes  | Checksum                | CRC-16-CCITT over header fields   |
| 12     | DATA     | Variable | Payload                 | Optional payload data             |
| 12+N   | D_CRC    | 2 bytes  | Checksum                | CRC-16-CCITT over data (if N > 0) |

Total header size: 12 bytes (including header CRC)
Data CRC: 2 bytes (only present if LENGTH > 0)

### 2.2 Flags Field

```
Bit 7   6   5   4   3   2   1   0
    │   │   │   │   │   │   │   └─ ACK: Acknowledgment packet
    │   │   │   │   │   │   └───── NAK: Negative acknowledgment packet
    │   │   │   │   │   └───────── FRAGMENT: More fragments follow
    │   │   │   │   └───────────── Reserved
    │   │   │   └───────────────── Reserved
    │   │   └───────────────────── Reserved
    │   └───────────────────────── Reserved
    └───────────────────────────── Reserved
```

- **ACK (bit 0)**: Set for acknowledgment packets
- **NAK (bit 1)**: Set for negative acknowledgment packets
- **FRAGMENT (bit 2)**: Set when more fragments follow, clear for last fragment

## 3. Protocol Operations

### 3.1 Sequence Numbers

- Each endpoint maintains separate sequence counters for TX and RX
- Sequence numbers increment for each new packet (not retransmissions)
- Used to detect duplicates and missing packets
- Wraps from 65535 to 0

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
| 0x10-0x1F | System Control | Reset, boot, system info |
| 0x20-0x2F | Channel Operations | Channel list, lock, size, read, write, ioctl |
| 0x30-0xFF | Reserved/Extensions | Future use |

### 4.2 Command Summary

#### Protocol Control (0x00-0x0F)
| Opcode | Command | Payload Size | Response Size | Description |
|--------|---------|-------------|---------------|-------------|
| 0x00 | PROTO_SYNC | 0 bytes | 2 bytes (status) | Synchronization request |
| 0x01 | PROTO_GET_CAPS | 0 bytes | 32 bytes | Get protocol capabilities |
| 0x02 | PROTO_SET_CAPS | 32 bytes | 32 bytes | Set protocol capabilities |

#### System Control (0x10-0x1F)
| Opcode | Command | Payload Size | Response Size | Description |
|--------|---------|-------------|---------------|-------------|
| 0x10 | SYS_RESET | 0 bytes | No response | System reset |
| 0x11 | SYS_BOOT | 0 bytes | No response | Jump to bootloader |
| 0x12 | SYS_INFO | 0 bytes | 64 bytes | Get system information |

#### Channel Operations (0x20-0x2F)
| Opcode | Command | Payload Size | Response Size | Description |
|--------|---------|-------------|---------------|-------------|
| 0x20 | CHANNEL_LIST | 0 bytes | Variable (16 bytes × channel count) | List registered channels |
| 0x21 | CHANNEL_LOCK | 0 bytes | 2 bytes (status) | Lock channel for exclusive access |
| 0x22 | CHANNEL_UNLOCK | 0 bytes | 2 bytes (status) | Unlock channel |
| 0x23 | CHANNEL_SHAPE | 0 bytes | Variable | Get data shape/dimensions |
| 0x24 | CHANNEL_SIZE | 0 bytes | 4 bytes | Get available data size |
| 0x25 | CHANNEL_READ | 8 bytes (offset + length) | Variable (requested data) | Read data from channel |
| 0x26 | CHANNEL_WRITE | 8+ bytes (offset + length + data) | 2 bytes (status) | Write data to channel |
| 0x27 | CHANNEL_IOCTL | 4+ bytes (request + data) | 2 bytes (status) | Channel-specific control operation |

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

Response Format: (32 bytes)
| Offset | Field       | Size     | Type                 | Description                       |
|--------|-------------|----------|----------------------|-----------------------------------|
| 0-3    | flags       | 4 bytes  | Bitfield             | Capability flags (bits 0-2)       |
|        |             |          |                      | bit 0: CRC enabled                |
|        |             |          |                      | bit 1: Sequence enabled           |
|        |             |          |                      | bit 2: Fragmentation enabled      |
| 4-5    | max_payload | 2 bytes  | Size                 | Maximum payload size              |
| 6-31   | reserved    | 26 bytes | Padding              | Reserved for future use           |



**PROTO_SET_CAPS (0x02)**

Request Format: (32 bytes)
| Offset | Field       | Size     | Type                 | Description                       |
|--------|-------------|----------|----------------------|-----------------------------------|
| 0-3    | flags       | 4 bytes  | Bitfield             | Capability flags to set           |
| 4-5    | max_payload | 2 bytes  | Size                 | Maximum payload size              |
| 6-31   | reserved    | 26 bytes | Padding              | Reserved for future use           |

Response Format: (Same as request - echoes back the set values)



#### System Control Commands

**SYS_RESET (0x10)**

Request Format: (No payload)  
Response: (No response - system resets immediately)

**SYS_BOOT (0x11)**

Request Format: (No payload)  
Response: (No response - system enters bootloader)

**SYS_INFO (0x12)**

Request Format: (No payload)

Response Format: (64 bytes)
| Offset | Field               | Size     | Type         | Description                       |
|--------|---------------------|----------|--------------|-----------------------------------|
| 0-3    | cpu_id              | 4 bytes  | Register     | ARM CPUID register value          |
| 4-15   | dev_id              | 12 bytes | ID Array     | Unique device ID (3×uint32)       |
| 16-19  | chip_id             | 4 bytes  | ID           | Camera sensor chip ID             |
| 20-23  | reserved1           | 4 bytes  | Padding      | Reserved for expansion            |
| 24-27  | hw_capabilities     | 4 bytes  | Bitfield     | Hardware capability flags:        |
|        |                     |          |              | bit 0: GPU,                       |
|        |                     |          |              | bit 1: NPU,                       |
|        |                     |          |              | bit 2: ISP,                       |
|        |                     |          |              | bit 3: Video encoder,             |
|        |                     |          |              | bit 4: JPEG encoder,              |
|        |                     |          |              | bit 5: DRAM,                      |
|        |                     |          |              | bit 6: PMU,                       |
|        |                     |          |              | bits 7-12: PMU event count,      |
|        |                     |          |              | bit 13: WiFi,                     |
|        |                     |          |              | bit 14: Bluetooth,                |
|        |                     |          |              | bit 15: SD,                       |
|        |                     |          |              | bit 16: Ethernet,                 |
|        |                     |          |              | bit 17: USB-HS                    |
| 28-31  | flash_size_kb       | 4 bytes  | Size         | Flash memory size (KB)            |
| 32-35  | ram_size_kb         | 4 bytes  | Size         | RAM size (KB)                     |
| 36-39  | framebuffer_size_kb | 4 bytes  | Size         | Framebuffer size (KB)             |
| 40-43  | streambuffer_size_kb| 4 bytes  | Size         | Stream buffer size (KB)           |
| 44-51  | reserved2           | 8 bytes  | Padding      | Reserved for expansion            |
| 52-54  | firmware_version    | 3 bytes  | Version      | Firmware version (M.m.p)          |
| 55-57  | protocol_version    | 3 bytes  | Version      | Protocol version (M.m.p)          |
| 58-60  | bootloader_version  | 3 bytes  | Version      | Bootloader version (M.m.p)        |
| 61-63  | reserved3           | 3 bytes  | Padding      | Padding to 64 bytes               |

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

**CHANNEL_LOCK (0x21) / CHANNEL_UNLOCK (0x22)**

Request Format: (No payload)

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code                       |

**CHANNEL_SIZE (0x24)**

Request Format: (No payload)

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | size     | 4 bytes  | Count                   | Available bytes in channel        |

**CHANNEL_READ (0x25)**

Request Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | offset   | 4 bytes  | Position                | Starting position to read         |
| 4-7    | length   | 4 bytes  | Count                   | Number of bytes to read           |

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-N    | data     | N bytes  | Payload                 | Requested channel data            |

**CHANNEL_WRITE (0x26)**

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

**CHANNEL_IOCTL (0x27)**

Request Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-3    | request  | 4 bytes  | Code                    | IOCTL request code                |
| 4-N    | data     | N bytes  | Payload                 | Request-specific data             |

Response Format:
| Offset | Field    | Size     | Type                    | Description                       |
|--------|----------|----------|-------------------------|-----------------------------------|
| 0-1    | status   | 2 bytes  | Code                    | Status code                       |

**CHANNEL_SHAPE (0x23)**

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


### 4.4 Status Codes

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

**Channel 1 (stdio)**: Bidirectional channel for script I/O. The host can send Python code for execution, and the device sends back console output and results.

**Channel 2 (stream)**: Read-only channel for high-bandwidth data like image frames. Supports exclusive locking to prevent concurrent access conflicts.

**Channel 3 (profile)**: Optional read-only channel providing performance metrics and diagnostic information. Profiling availability is determined by the presence of this channel rather than a capability flag. The number of PMU event counters is embedded in the hardware capabilities bitfield (bits 7-12) in the system information.

### 5.2 Channel Capabilities

Each channel declares its supported operations through capability flags:
- **READ**: Channel supports read operations
- **WRITE**: Channel supports write operations  
- **LOCK**: Channel supports exclusive locking
- **PHYSICAL**: Transport-only channel (not accessible via protocol)

### 5.3 Channel Operations

**Discovery**: The CHANNEL_LIST command returns available channels with their IDs, capabilities, and names.

**Data Access**: Read operations specify offset and length parameters. The offset meaning depends on the channel - it might be a byte position, frame number, or other logical addressing scheme.

**Zero-Copy Reads**: The readp operation returns direct pointers to channel data, eliminating copy overhead for large transfers.

**Control**: The ioctl operation handles channel-specific commands like configuration changes or status queries.

**Flow Control**: Channels return appropriate status codes when data isn't ready, and use locking mechanisms to coordinate access between host and device.

## 6. Error Handling

### 6.1 CRC Verification
- CRC-16-CCITT polynomial (0x1021) with initial value 0xFFFF
- Header CRC: calculated over header fields (excluding CRC field itself)
- Data CRC: calculated over payload data (only if payload length > 0)
- Hardware acceleration available on STM32 and Alif platforms

### 6.2 Stream Recovery
- On sync loss, scan for SYNC pattern (0xAA55)
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
- Maximum buffer size: 1024 bytes (configurable via OMV_PROTOCOL_MAX_BUFFER_SIZE)
- Maximum payload: configurable (default: max buffer - header - data CRC)
- Minimum payload: 54 bytes (64 - 12 header - 2 data CRC)
- Ring buffer implementation for received data

### 8.3 Channel Registration
- Maximum 10 channels supported (indices 0-9)
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

### 10.1 Configurable Parameters

The protocol supports runtime configuration of key parameters:

- **crc_enabled**: Enable/disable CRC error checking
- **seq_enabled**: Enable/disable sequence number validation
- **frag_enabled**: Enable/disable packet fragmentation support
- **lock_rate_hz**: Maximum rate for channel lock operations
- **timeout_ms**: Command response timeout in milliseconds

### 10.2 Default Configuration

- **CRC enabled**: true
- **Sequence validation**: true
- **Fragmentation enabled**: true
- **Lock rate**: 50 Hz (20ms minimum between locks)
- **Timeout**: 100ms
- **Max payload**: buffer size dependent

### 10.3 Capability Negotiation

The `PROTO_GET_CAPS` and `PROTO_SET_CAPS` commands allow dynamic configuration. See the detailed command formats above for the exact 32-byte payload structure.

## 11. System Information

The `SYS_INFO` command returns comprehensive system information in a 64-byte response. See the detailed command format above for the complete field layout including hardware identification, capabilities, memory information, and version data.

## 12. Version History

| Version | Date     | Status   | Type                    | Description                       |
|---------|----------|----------|-------------------------|-----------------------------------|
| 1.0     | 2025     | Released | Specification           | Initial specification             |
