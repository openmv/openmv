"""
OpenMV Protocol V2 Packet Handling

This module provides packet construction, parsing, and validation
for the OpenMV Protocol V2.
"""

import struct
import binascii
import logging
from .exceptions import OMVPException
from .constants import OMVPOpcode, OMVPFlags, OMVProto, opcode_name, flags_name

# Precompiled struct objects for efficiency
_crc_struct = struct.Struct('<H')
_hdr_struct = struct.Struct('<HHHBBHH')

class Packet:
    """Handles packet construction, parsing, and validation"""
    
    def __init__(self, crc_enabled=True, seq_enabled=True, max_payload=None):
        self.crc_enabled = crc_enabled
        self.seq_enabled = seq_enabled
        self._max_payload = max_payload
        if max_payload is not None:
            self.packet = memoryview(bytearray(max_payload + OMVProto.HEADER_SIZE + OMVProto.CRC_SIZE))
        else:
            self.packet = None
    
    @property
    def max_payload(self):
        return self._max_payload
    
    @max_payload.setter
    def max_payload(self, value):
        self._max_payload = value
        if value is not None:
            # Reallocate packet buffer to accommodate new max_payload
            self.packet = memoryview(bytearray(value + OMVProto.HEADER_SIZE + OMVProto.CRC_SIZE))
        else:
            self.packet = None
    
    def _checksum(self, data):
        """Calculate CRC-16-CCITT for data using binascii.crc_hqx"""
        return 0 if not self.crc_enabled else binascii.crc_hqx(data, 0xFFFF)
    
    def _check_crc(self, crc_type, crc, buffer):
        """Check if CRC matches the calculated value or CRC is disabled"""
        exp = self._checksum(buffer)
        if not self.crc_enabled or crc == exp:
            return True
        logging.debug(f"Bad {crc_type} CRC: expected=0x{exp:04X}, actual=0x{crc:04X}, len={len(buffer)}")
        return False
    
    def _check_seq(self, sequence, expected_sequence, opcode):
        """Check if sequence is valid or sequence checking is disabled"""
        return (not self.seq_enabled or 
                sequence == expected_sequence or 
                opcode == OMVPOpcode.PROTO_SYNC)
    
    def build(self, sequence, opcode, channel=0, flags=0, data=b''):
        """Build a packet and return the bytes to send"""
        if self._max_payload is None:
            raise OMVPException("max_payload must be set before building packets")
        
        if len(data) > self.max_payload:
            raise OMVPException(f"Payload too large: {len(data)} > {self.max_payload}")
        
        length = len(data)
        
        # Pack header without CRC first (10 bytes)
        struct.pack_into('<HHHBBH', self.packet, 0, OMVProto.SYNC_WORD,
                         sequence, channel, flags, opcode, length)
        struct.pack_into('<H', self.packet, OMVProto.HEADER_SIZE - 2,
                         self._checksum(self.packet[:OMVProto.HEADER_SIZE - 2]))

        # Pack data if present
        if length > 0:
            self.packet[OMVProto.HEADER_SIZE:OMVProto.HEADER_SIZE + length] = data
            struct.pack_into('<H', self.packet, OMVProto.HEADER_SIZE + length, self._checksum(data))
        
        packet_size = OMVProto.HEADER_SIZE + length + (OMVProto.CRC_SIZE if length else 0)
        
        return {
            'sync': OMVProto.SYNC_WORD,
            'sequence': sequence,
            'channel': channel,
            'flags': flags,
            'opcode': opcode,
            'length': length,
            'header_crc': self._checksum(self.packet[:OMVProto.HEADER_SIZE - 2]),
            'data': data if data else None,
            'raw': bytes(self.packet[:packet_size])
        }
    
    def parse(self, packet_data, expected_sequence, max_payload):
        """Parse complete packet and validate header and payload"""
        if len(packet_data) < OMVProto.HEADER_SIZE:
            return None
            
        # Parse header
        sync, seq, chan, flags, opcode, length, crc = _hdr_struct.unpack(packet_data[:OMVProto.HEADER_SIZE])
        
        # Validate sync word
        if sync != OMVProto.SYNC_WORD:
            return None
            
        # Validate payload size
        if length > max_payload:
            return None
            
        # Validate sequence
        if not self._check_seq(seq, expected_sequence, opcode):
            return None
            
        # Validate header CRC
        if not self._check_crc("header", crc, packet_data[:OMVProto.HEADER_SIZE-2]):
            return None
        
        # Parse payload if present and we have enough data
        data = None
        if length > 0:
            payload_start = OMVProto.HEADER_SIZE
            payload_end = payload_start + length + OMVProto.CRC_SIZE
            
            # Only try to parse payload if we have enough data
            if len(packet_data) >= payload_end:
                payload_data = packet_data[payload_start:payload_end]
                
                if len(payload_data) >= OMVProto.CRC_SIZE:
                    data = payload_data[:-OMVProto.CRC_SIZE]
                    payload_crc = _crc_struct.unpack(payload_data[-OMVProto.CRC_SIZE:])[0]
                    
                    if not self._check_crc("payload", payload_crc, data):
                        return None
            
        return {
            'sync': sync,
            'sequence': seq,
            'channel': chan,
            'flags': flags,
            'opcode': opcode,
            'length': length,
            'header_crc': crc,
            'data': data
        }
    
    def log(self, packet_info, direction):
        """Log packet information for debugging"""
        opname = opcode_name(packet_info['opcode'])
        flags_str = flags_name(packet_info['flags'])
        logging.debug(f"{direction}: seq={packet_info['sequence']}, "
                      f"chan={packet_info['channel']}, "
                      f"opcode={opname}, flags={flags_str}, "
                      f"length={packet_info['length']}")
