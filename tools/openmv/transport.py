# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Protocol Transport Layer
#
# This module provides the low-level transport layer with the protocol state 
# machine for packet parsing and communication management.

import time
import logging
import struct
import random
from .constants import *
from .exceptions import *
from .crc import crc16, crc32
from .buffer import OMVRingBuffer

# Precompiled struct objects for efficiency
_crc16_struct = struct.Struct('<H')
_crc32_struct = struct.Struct('<L')
_hdr_struct = struct.Struct('<HBBBBHH')

class OMVTransport:
    """Low-level transport layer with state machine"""

    def __init__(self, serial, crc, seq, max_payload, timeout, event_callback, drop_rate=0.0):
        self.serial = serial
        self.timeout = timeout
        self.max_payload = max_payload
        
        # Protocol state
        self.sequence = 0
        self.state = OMVPState.SYNC
        self.crc_enabled = crc
        self.seq_enabled = seq
        
        # Event callback
        self.event_callback = event_callback
        
        # Packet simulation
        self.drop_rate = drop_rate
 
        # Packet buffers for send/recv
        self.buf = OMVRingBuffer(max(max_payload * 4, 4 * 1024 * 1024))
        self.pbuf = memoryview(bytearray(max_payload + OMVProto.HEADER_SIZE + OMVProto.CRC_SIZE))
       
        # Statistics
        self.stats = {
            'sent': 0,
            'received': 0,
            'checksum': 0,
            'sequence': 0,
        }
    
    def reset_sequence(self):
        """Reset sequence counter to 0"""
        self.sequence = 0
    
    def update_caps(self, crc, seq, ack, max_payload):
        """Update transport capabilities"""
        self.crc_enabled = crc
        self.seq_enabled = seq
        self.max_payload = max_payload
        # Reallocate buffers to accommodate new max_payload
        self.buf = OMVRingBuffer(max(max_payload * 4, 4 * 1024 * 1024))
        self.pbuf = memoryview(bytearray(max_payload + OMVProto.HEADER_SIZE + OMVProto.CRC_SIZE))

    def _crc(self, data, crc_size=16):
        """Calculate CRC with specified size (16 or 32)"""
        if not self.crc_enabled:
            return 0
        return crc16(data) if crc_size == 16 else crc32(data)
    
    def _check_crc(self, crc, buffer, crc_size=16):
        """Check if CRC matches the calculated value or CRC is disabled"""
        return not self.crc_enabled or crc == self._crc(buffer, crc_size)
    
    def _check_seq(self, sequence, expected_sequence, opcode, flags):
        """Check if sequence is valid or sequence checking is disabled"""
        return (not self.seq_enabled or
                (flags & OMVPFlags.EVENT) or
                (flags & OMVPFlags.RTX) or
                sequence == expected_sequence or
                opcode == OMVPOpcode.PROTO_SYNC)
    
    def _format_flags(self, flags):
        """Get human-readable name for packet flags"""
        if flags == 0:
            return "0x00"

        flag_parts = []
        if flags & OMVPFlags.ACK:
            flag_parts.append("ACK")
        if flags & OMVPFlags.NAK:
            flag_parts.append("NAK")
        if flags & OMVPFlags.RTX:
            flag_parts.append("RTX")
        if flags & OMVPFlags.FRAGMENT:
            flag_parts.append("FRAG")
        if flags & OMVPFlags.EVENT:
            flag_parts.append("EVT")
        if flags & OMVPFlags.ACK_REQ:
            flag_parts.append("ACK_REQ")
        
        return "|".join(flag_parts) if flag_parts else f"0x{flags:02X}"
    
    def log(self, seq=None, ch=None, opcode=None, flags=None, length=None, direction=None, packet=None):
        """Log packet information for debugging"""
        if packet is not None:
            seq = packet['sequence']
            ch = packet['channel']
            opcode = packet['opcode']
            flags = packet['flags']
            length = packet['length']

        opcode_str = OMVPOpcode(opcode).name if opcode in OMVPOpcode else f"0x{opcode:02X}"
        flags_str = self._format_flags(flags)

        # Add emoji based on packet type and direction
        if direction == "Drop":
            emoji = "🎲"
        elif direction == "Rjct":
            emoji = "🚫"
        elif flags & OMVPFlags.ACK:
            emoji = "✅"
        elif flags & OMVPFlags.NAK:
            emoji = "❌"
        elif direction == "Send":
            emoji = "➡️"
        else:
            emoji = "⬅️"

        logging.debug(f"{emoji} {direction}: seq={seq:03d},"
                      f" chan={ch}, opcode={opcode_str},"
                      f" flags={flags_str}, length={length}")

    def send_packet(self, opcode, channel, flags, data=None, sequence=None):
        """Send a packet to the camera"""
        if not self.serial or not self.serial.is_open:
            raise OMVPTimeoutException("Serial connection not open")

        sequence = self.sequence if sequence is None else sequence
        length = 0 if data is None else len(data)
        
        if length > self.max_payload:
            raise OMVPException(f"Payload too large: {length} > {self.max_payload}")
        
        # Pack header without CRC first (10 bytes)
        struct.pack_into('<HBBBBH', self.pbuf, 0, OMVProto.SYNC_WORD,
                         sequence, channel, flags, opcode, length)
        struct.pack_into('<H', self.pbuf, OMVProto.HEADER_SIZE - 2,
                         self._crc(self.pbuf[:OMVProto.HEADER_SIZE - 2], 16))

        # Pack data if present
        if length > 0:
            self.pbuf[OMVProto.HEADER_SIZE:OMVProto.HEADER_SIZE + length] = data
            struct.pack_into('<L', self.pbuf, OMVProto.HEADER_SIZE + length, self._crc(data, 32))

        packet_size = OMVProto.HEADER_SIZE + length + (OMVProto.CRC_SIZE if length else 0)

        self.log(sequence, channel, opcode, flags, length, "Send")
        self.serial.write(self.pbuf[:packet_size])
        self.stats['sent'] += 1

    def recv_packet(self, poll_events=False):
        """Receive and parse a packet from the camera with NAK handling"""
        if not self.serial or not self.serial.is_open:
            raise OMVPException("Serial connection not open")

        fragments = bytearray()  # Collect fragment payloads
        start_time = time.time()

        while time.time() - start_time < self.timeout:
            if self.serial.in_waiting > 0:
                data = self.serial.read(self.serial.in_waiting)
                self.buf.extend(data)

            # Process state machine
            if not (packet := self._process()):
                if poll_events:
                    return
                time.sleep(0.001)
                continue

            # Simulate packet drops by randomly dropping parsed packets
            if self.drop_rate > 0.0 and random.random() < self.drop_rate:
                self.log(packet=packet, direction="Drop")
                continue

            self.stats['received'] += 1
            self.log(packet=packet, direction="Recv")
            
            # Handle retransmission
            if (packet['flags'] & OMVPFlags.RTX) and (self.sequence != packet['sequence']):
                if packet['flags'] & OMVPFlags.ACK_REQ:
                    self.send_packet(packet['opcode'], packet['channel'],
                                     OMVPFlags.ACK, sequence=packet['sequence'])
                continue    # Skip further processing of duplicate packet

            # ACK the received packet
            if packet['flags'] & OMVPFlags.ACK_REQ:
                # Simulate packet ACK drops
                if self.drop_rate > 0.0 and random.random() < self.drop_rate:
                    status = struct.pack('<H', OMVPStatus.CHECKSUM)
                    self.log(packet['sequence'], packet['channel'], packet['opcode'], OMVPFlags.ACK, 0, "Drop")
                    # Enable to simulate rejecting packets
                    #self.send_packet(packet['opcode'], packet['channel'], OMVPFlags.NAK, data=status)
                    #continue
                else:
                    self.send_packet(packet['opcode'], packet['channel'], OMVPFlags.ACK)

            # Handle event packets
            if packet['flags'] & OMVPFlags.EVENT:
                self.event_callback(packet['channel'], 0xFFFF if not packet['length']
                                    else struct.unpack('<H', packet['payload'])[0])
                start_time = time.time()
                continue

            # Update sequence after each packet (including fragments)
            self.sequence = (self.sequence + 1) & 0xFF

            # Check if this is a fragmented packet
            if packet['flags'] & OMVPFlags.FRAGMENT:
                fragments.extend(packet['payload'])
                start_time = time.time()
                continue    # Continue collecting fragments

            # Either last fragment or non-fragmented packet
            if fragments:
                # This is the last fragment - combine all
                fragments.extend(packet['payload'])
                packet['payload'] = bytes(fragments)
                packet['length'] = len(fragments)

            # Handle NAK flags
            if packet['flags'] & OMVPFlags.NAK:
                # Raise specific exception for all NAK statuses except BUSY
                status = struct.unpack('<H', packet['payload'][:2])[0]
                if status == OMVPStatus.CHECKSUM:
                    raise OMVPChecksumException("")
                elif status == OMVPStatus.SEQUENCE:
                    raise OMVPSequenceException("")
                elif status == OMVPStatus.TIMEOUT:
                    raise OMVPTimeoutException("")
                elif status != OMVPStatus.BUSY:
                    raise OMVPException(f"Command failed with status: {OMVPStatus(status).name}")
                return False

            # Return payload or True for ACK
            return True if not packet['length'] else bytes(packet['payload'])

        if not poll_events:
            raise OMVPTimeoutException("Packet receive timeout")

    def _process(self):
        """Process the protocol state machine"""
        while len(self.buf) > 2:
            if self.state == OMVPState.SYNC:
                # Find sync pattern
                while len(self.buf) > 2:
                    sync = self.buf.peek16()
                    if sync == OMVProto.SYNC_WORD:
                        self.state = OMVPState.HEADER
                        break
                    self.buf.consume(1)

            elif self.state == OMVPState.HEADER:
                # Wait for complete header
                if len(self.buf) < OMVProto.HEADER_SIZE:
                    return None

                # Parse header
                header = self.buf.peek(OMVProto.HEADER_SIZE)
                sync, seq, chan, flags, opcode, length, crc = _hdr_struct.unpack(header[:OMVProto.HEADER_SIZE])

                self.state = OMVPState.SYNC
                if length > self.max_payload:
                    self.log(seq, chan, opcode, flags, length, "Rjct")
                    self.buf.consume(1)
                elif not self._check_seq(seq, self.sequence, opcode, flags):
                    self.log(seq, chan, opcode, flags, length, "Rjct")
                    self.buf.consume(1)
                elif not self._check_crc(crc, header[:OMVProto.HEADER_SIZE-2], 16):
                    self.log(seq, chan, opcode, flags, length, "Rjct")
                    self.buf.consume(1)
                else:
                    self.state = OMVPState.PAYLOAD
                    self.plength = OMVProto.HEADER_SIZE + length
                    self.plength += OMVProto.CRC_SIZE if length else 0

            elif self.state == OMVPState.PAYLOAD:
                # Wait for a complete packet
                if len(self.buf) < self.plength:
                    return None

                payload = None
                self.state = OMVPState.SYNC

                # Parse packet
                packet = self.buf.peek(self.plength)
                sync, seq, chan, flags, opcode, length, crc = _hdr_struct.unpack(packet[:OMVProto.HEADER_SIZE])

                # Parse payload
                if length > 0:
                    payload = packet[OMVProto.HEADER_SIZE:-OMVProto.CRC_SIZE]
                    payload_crc = _crc32_struct.unpack(packet[-OMVProto.CRC_SIZE:])[0]

                    if not self._check_crc(payload_crc, payload, 32):
                        self.stats['checksum'] += 1
                        self.log(seq, chan, opcode, flags, length, "Rjct")
                        self.buf.consume(1)  # Try next byte
                        continue

                self.buf.consume(self.plength)

                return {
                    'sync': sync,
                    'sequence': seq,
                    'channel': chan,
                    'flags': flags,
                    'opcode': opcode,
                    'length': length,
                    'header_crc': crc,
                    'payload': payload
                }


