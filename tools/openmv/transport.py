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
from .constants import OMVProto, OMVPState, OMVPFlags, OMVPStatus
from .packet import Packet
from .buffer import OMVRingBuffer
from .exceptions import OMVPTimeoutException, OMVPException, OMVPChecksumException, OMVPSequenceException

class OMVTransport:
    """Low-level transport layer with state machine"""
    
    def __init__(self, serial, crc, seq, ack, max_payload, timeout, event_callback=None, drop_rate=0.0):
        self.serial = serial
        self.timeout = timeout
        self.ack = ack
        self.max_payload = max_payload
        
        # Protocol state
        self.sequence = 0
        self.state = OMVPState.SYNC
        self.packet = Packet(crc, seq, max_payload)
        self.buffer = OMVRingBuffer(max(max_payload * 4, 4 * 1024 * 1024))
        
        # Event callback
        self.event_callback = event_callback
        
        # Packet simulation
        self.drop_rate = drop_rate
        
        # Statistics
        self.stats = {
            'sent': 0,
            'received': 0,
            'checksum': 0,
            'sequence': 0,
            'frames': 0,
            'avg_fps': 0.0
        }
    
    def reset_sequence(self):
        """Reset sequence counter to 0"""
        self.sequence = 0
    
    def update_caps(self, crc, seq, ack, max_payload):
        """Update transport capabilities"""
        self.ack = ack
        self.max_payload = max_payload
        # Recreate packet processor with new settings
        self.packet = Packet(crc, seq, max_payload)
        self.buffer = OMVRingBuffer(max(max_payload * 4, 4 * 1024 * 1024))
    
    def update_frame_stats(self, first_frame_time):
        """Update frame statistics including FPS calculation"""
        current_time = time.time()
        self.stats['frames'] += 1
        
        # Calculate average FPS
        if self.stats['frames'] > 1 and first_frame_time is not None:
            elapsed = current_time - first_frame_time
            if elapsed > 0:
                self.stats['avg_fps'] = round((self.stats['frames'] - 1) / elapsed, 2)

    def send_packet(self, opcode, channel=0, flags=0, data=b''):
        """Send a packet to the camera"""
        if not self.serial or not self.serial.is_open:
            raise OMVPTimeoutException("Serial connection not open")
        packet = self.packet.build(self.sequence, opcode, channel, flags, data)
        self.packet.log(packet, "Send")
        self.serial.write(packet['raw'])
        self.stats['sent'] += 1
    
    def recv_packet(self):
        """Receive and parse a packet from the camera with NAK handling"""
        if not self.serial or not self.serial.is_open:
            raise OMVPException("Serial connection not open")
        
        start_time = time.time()
        fragments = bytearray()  # Collect fragment payloads
        
        while time.time() - start_time < self.timeout:
            if self.serial.in_waiting > 0:
                data = self.serial.read(self.serial.in_waiting)
                self.buffer.extend(data)
            
            # Process state machine
            packet = self._process_state_machine()
            if not packet:
                continue

            # Simulate packet drops by randomly dropping parsed packets
            if self.drop_rate > 0.0 and random.random() < self.drop_rate:
                self.packet.log(packet, "Drop")
                continue

            self.stats['received'] += 1
            self.packet.log(packet, "Recv")
            
            # ACK the received packet if ACK is enabled (unless it's already an ACK or NAK)
            if self.ack and not (packet['flags'] & (OMVPFlags.ACK | OMVPFlags.NAK)):
                self.send_packet(packet['opcode'], packet['channel'], OMVPFlags.ACK)

            # Handle event packets
            if packet['flags'] & OMVPFlags.EVENT:
                if self.event_callback:
                    self.event_callback(packet['channel'], 0xFFFF if not packet['length']
                                        else struct.unpack('<H', packet['payload'])[0])
                continue  # Don't process as normal packet

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
                status = struct.unpack('<H', packet['payload'][:2])[0]
                logging.debug(f"Command failed with status: {OMVPStatus(status).name}")
                # Raise specific exception for all NAK statuses except BUSY
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

        raise OMVPTimeoutException("Packet receive timeout")
    
    def _process_state_machine(self):
        """Process the protocol state machine"""
        while len(self.buffer) > 2:
            if self.state == OMVPState.SYNC:
                # Find sync pattern
                while len(self.buffer) > 2:
                    sync = self.buffer.peek16()
                    if sync == OMVProto.SYNC_WORD:
                        self.state = OMVPState.HEADER
                        break
                    self.buffer.consume(1)
            elif self.state == OMVPState.HEADER:
                # Check if we have complete header
                if len(self.buffer) < OMVProto.HEADER_SIZE:
                    return None

                self.state = OMVPState.SYNC
                # Try to parse header only to get length
                header_data = self.buffer.peek(OMVProto.HEADER_SIZE)
                header = self.packet.parse(header_data, self.sequence, self.max_payload)
                
                if not header:
                    # Invalid header, try next byte
                    self.buffer.consume(1)
                    continue
                
                self.state = OMVPState.PAYLOAD
                self.plength = OMVProto.HEADER_SIZE + header['length']
                self.plength += OMVProto.CRC_SIZE if header['length'] else 0
                
            elif self.state == OMVPState.PAYLOAD:
                # Check if we have complete packet
                if len(self.buffer) < self.plength:
                    return None
                
                # Get complete packet data
                packet_data = self.buffer.peek(self.plength)
                
                # Parse complete packet
                packet = self.packet.parse(packet_data, self.sequence, self.max_payload)
                
                if packet is None:
                    self.stats['checksum'] += 1
                    self.buffer.consume(1)  # Try next byte
                    self.state = OMVPState.SYNC
                    continue
    
                self.state = OMVPState.SYNC
                self.buffer.consume(self.plength) 

                return {
                    'sequence': packet['sequence'],
                    'channel': packet['channel'],
                    'flags': packet['flags'],
                    'opcode': packet['opcode'],
                    'length': packet['length'],
                    'payload': packet['data'] if packet['data'] is not None else b'',
                }
        
        return None
