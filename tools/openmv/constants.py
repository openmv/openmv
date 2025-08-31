"""
OpenMV Protocol V2 Constants

This module defines all the constants used in the OpenMV Protocol V2
including opcodes, status codes, flags, and other protocol-level definitions.
"""

from enum import IntEnum

class OMVPFlags(IntEnum):
    """OpenMV Protocol V2 Packet Flags"""
    ACK = (1 << 0)
    NAK = (1 << 1)
    FRAGMENT = (1 << 2)


class OMVPState(IntEnum):
    """OpenMV Protocol V2 Parser State Machine States"""
    SYNC = 0
    HEADER = 1
    PAYLOAD = 2


class OMVProto(IntEnum):
    """OpenMV Protocol V2 Constants"""
    SYNC_WORD = 0xAA55
    HEADER_SIZE = 12
    CRC_SIZE = 2
    MIN_PAYLOAD_SIZE = 64 - 12 - 2  # 64 - OMV_PROTOCOL_HEADER_SIZE - 2 = 50


class OMVPStatus(IntEnum):
    """OpenMV Protocol V2 Status Codes"""
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
    """OpenMV Protocol V2 Operation Codes"""
    # Protocol commands
    PROTO_SYNC = 0x00
    PROTO_GET_CAPS = 0x01
    PROTO_SET_CAPS = 0x02
    
    # System commands
    SYS_RESET = 0x10
    SYS_BOOT = 0x11
    SYS_INFO = 0x12
    
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


def opcode_name(opcode):
    """Get human-readable name for opcode"""
    try:
        return OMVPOpcode(opcode).name
    except ValueError:
        return f'UNKNOWN (0x{opcode:02X})'


def status_name(status):
    """Get human-readable name for status code"""
    try:
        return OMVPStatus(status).name
    except ValueError:
        return f'UNKNOWN_0x{status:02X}'


def flags_name(flags):
    """Get human-readable name for packet flags"""
    if flags == 0:
        return "0x00"
    
    flag_parts = []
    if flags & OMVPFlags.ACK:
        flag_parts.append("ACK")
    if flags & OMVPFlags.NAK:
        flag_parts.append("NAK")
    if flags & OMVPFlags.FRAGMENT:
        flag_parts.append("FRAG")
    
    # Handle any unknown bits
    known_flags = OMVPFlags.ACK | OMVPFlags.NAK | OMVPFlags.FRAGMENT
    unknown = flags & ~known_flags
    if unknown:
        flag_parts.append(f"0x{unknown:02X}")
    
    return "|".join(flag_parts) if flag_parts else f"0x{flags:02X}"
