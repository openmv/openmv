# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Protocol Constants
#
# This module defines all the constants used in the OpenMV Protocol
# including opcodes, status codes, flags, and other definitions.

from enum import IntEnum

class OMVPFlags(IntEnum):
    """OpenMV Protocol Packet Flags"""
    ACK = (1 << 0)
    NAK = (1 << 1)
    RTX = (1 << 2)
    ACK_REQ = (1 << 3)
    FRAGMENT = (1 << 4)
    EVENT = (1 << 5)


class OMVPState(IntEnum):
    """OpenMV Protocol Parser State Machine States"""
    SYNC = 0
    HEADER = 1
    PAYLOAD = 2


class OMVProto(IntEnum):
    """OpenMV Protocol Constants"""
    SYNC_WORD = 0xD5AA
    HEADER_SIZE = 10
    CRC_SIZE = 4
    MIN_PAYLOAD_SIZE = 64 - 10 - 2  # 64 - OMV_PROTOCOL_HEADER_SIZE - 2 = 52


class OMVPStatus(IntEnum):
    """OpenMV Protocol Status Codes"""
    SUCCESS = 0x00
    FAILED = 0x01
    INVALID = 0x02
    TIMEOUT = 0x03
    BUSY = 0x04
    CHECKSUM = 0x05
    SEQUENCE = 0x06
    OVERFLOW = 0x07
    FRAGMENT = 0x08
    UNKNOWN = 0x09

class OMVPOpcode(IntEnum):
    """OpenMV Protocol Operation Codes"""
    # Protocol commands
    PROTO_SYNC = 0x00
    PROTO_GET_CAPS = 0x01
    PROTO_SET_CAPS = 0x02
    PROTO_STATS = 0x03
    
    # System commands
    SYS_RESET = 0x10
    SYS_BOOT = 0x11
    SYS_INFO = 0x12
    SYS_EVENT = 0x13
    
    # Channel commands
    CHANNEL_LIST = 0x20
    CHANNEL_POLL = 0x21
    CHANNEL_LOCK = 0x22
    CHANNEL_UNLOCK = 0x23
    CHANNEL_SHAPE = 0x24
    CHANNEL_SIZE = 0x25
    CHANNEL_READ = 0x26
    CHANNEL_WRITE = 0x27
    CHANNEL_IOCTL = 0x28
    CHANNEL_EVENT = 0x29


class OMVPEventType(IntEnum):
    """OpenMV Protocol Event Types"""
    CHANNEL_REGISTERED = 0x00
    CHANNEL_UNREGISTERED = 0x01
    SOFT_REBOOT = 0x02


class OMVPChannelIOCTL(IntEnum):
    """OpenMV Protocol Channel IOCTL Commands"""
    # Stdin channel IOCTLs
    STDIN_STOP = 0x01    # Stop running script
    STDIN_EXEC = 0x02    # Execute script
    STDIN_RESET = 0x03   # Reseet script buffer
    
    # Stream channel IOCTLs  
    STREAM_CTRL = 0x00      # Enable/disable streaming
    STREAM_RAW_CTRL = 0x01  # Enable/disable raw streaming
    STREAM_RAW_CFG = 0x02   # Set raw stream resolution
    
    # Profile channel IOCTLs
    PROFILE_MODE = 0x00      # Set profiling mode
    PROFILE_SET_EVENT = 0x01 # Set event type to profile
    PROFILE_RESET = 0x02     # Reset profiler data


