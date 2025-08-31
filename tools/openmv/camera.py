"""
OpenMV Protocol V2 Camera Interface

This module provides the high-level OMVCamera class that handles all
camera operations and channel communications using the OpenMV Protocol V2.
"""

import struct
import time
import serial
import logging
from functools import reduce
from operator import mul
from .constants import OMVPOpcode, OMVPStatus, OMVPFlags, OMVProto, status_name
from .exceptions import OMVPException, OMVPTimeoutException, OMVPChecksumException, OMVPSequenceException
from .transport import OMVTransport

class OMVCamera:
    """OpenMV Camera Protocol V2 Implementation"""
    
    def __init__(self, port, baudrate=921600, crc=True, seq=True, ack=False,
                 frag=True, timeout=1.0, max_retry=3, max_payload=4096):
        """
        Initialize the OMV Camera connection
        
        Args:
            port: Serial port name (e.g., '/dev/ttyACM0', 'COM3')
            baudrate: Serial communication baud rate
            crc: Enable CRC validation
            seq: Enable sequence number validation
            ack: Enable ACK/NAK acknowledgments
            frag: Enable packet fragmentation
            timeout: Default timeout for operations in seconds
            max_retry: Maximum number of retries for failed operations
            max_payload: Maximum payload size in bytes
        """
        # Serial connection
        self._serial = None
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        
        # Configuration
        self.crc_enabled = crc
        self.seq_enabled = seq
        self.ack_enabled = ack
        self.frag_enabled = frag
        self.max_payload = max_payload
        self.max_retry = max_retry
        
        # Frame timing
        self.first_frame_time = None
        
        # Protocol components
        self.channels = {}
        self.sysinfo = {}
        self.transport = None
        
    def __enter__(self):
        """Context manager entry"""
        self.connect()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.disconnect()

    def _send_cmd_wait_resp(self, opcode, channel=0, data=b''):
        """Send a command and wait for response (ACK/NAK or data)"""
        if not self.is_connected():
            raise OMVPException("Not connected")
        
        # Special handling for reset commands - they never return
        if opcode in [OMVPOpcode.SYS_RESET, OMVPOpcode.SYS_BOOT]:
            self.transport.send_packet(opcode, channel, 0, data)
            self.disconnect()  # Device will reset, connection lost
            return None
        
        for retry in range(self.max_retry):
            try:
                self.transport.send_packet(opcode, channel, 0, data)
                return self.transport.recv_packet()  # Returns payload/None or raises exception
            except OMVPTimeoutException:
                if retry < self.max_retry - 1:
                    continue
                raise
        raise OMVPTimeoutException(f"Command failed after {self.max_retry} retries")

    def connect(self):
        """Establish connection to the OpenMV camera"""
        try:
            self._serial = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
            
            # Always start with safe minimal settings for initial connection
            self.transport = OMVTransport(self._serial, crc_enabled=True, seq_enabled=True, 
                                          ack_enabled=False, max_payload=OMVProto.MIN_PAYLOAD_SIZE, 
                                          timeout=self.timeout)
            
            # Sync with device
            self.sync()
            
            # Set protocol configuration to user-requested values
            caps = self.get_capabilities()
            self.max_payload = min(self.max_payload, caps['max_payload'])
            self.set_capabilities(self.crc_enabled, self.seq_enabled, self.ack_enabled, 
                                 self.frag_enabled, self.max_payload, caps['max_retries'], 
                                 caps['timeout_ms'], caps['lock_wait_ms'])

            # Update transport with final negotiated capabilities
            self.transport.update_caps(self.crc_enabled, self.seq_enabled, 
                                      self.ack_enabled, self.max_payload)

            # Cache system info
            self.sysinfo = self.system_info()

            # Cache channel list
            self.chaninfo = self.channel_list()
            self.channels = {ch['name']: ch['id'] for ch in self.chaninfo}

            # Print system information
            self.print_system_info()
        except Exception as e:
            self.disconnect()
            raise OMVPException(f"Failed to connect: {e}")
        
    def disconnect(self):
        """Close connection to the OpenMV camera"""
        if self._serial:
            self._serial.close()
            self._serial = None
        self.transport = None
    
    def is_connected(self):
        """Check if connected to camera"""
        return self._serial is not None and self._serial.is_open
    
    def stats(self):
        """Get transport statistics"""
        return self.transport.stats if self.transport else {}
    
    # Protocol commands
    def sync(self):
        """Synchronize with the camera"""
        self._send_cmd_wait_resp(OMVPOpcode.PROTO_SYNC)
        # Reset sequence after sync - next command will use sequence 0
        self.transport.reset_sequence()

    def reset(self):
        """Reset the camera"""
        self._send_cmd_wait_resp(OMVPOpcode.SYS_RESET)

    def boot(self):
        """Jump to bootloader"""
        self._send_cmd_wait_resp(OMVPOpcode.SYS_BOOT)
    
    def get_capabilities(self):
        """Get device capabilities"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.PROTO_GET_CAPS)
        flags, max_payload, max_retries, timeout_ms, lock_wait_ms = struct.unpack('<IHHHH', payload[:12])
        return {
            'crc_enabled': bool(flags & 0x1),
            'seq_enabled': bool(flags & 0x2),
            'ack_enabled': bool(flags & 0x4),
            'frag_enabled': bool(flags & 0x8),
            'max_payload': max_payload,
            'max_retries': max_retries,
            'timeout_ms': timeout_ms,
            'lock_wait_ms': lock_wait_ms
        }
    
    def set_capabilities(self, crc_enabled, seq_enabled, ack_enabled, frag_enabled, 
                        max_payload, max_retries, timeout_ms, lock_wait_ms):
        """Set device capabilities"""
        flags = 0
        if crc_enabled:
            flags |= 0x1
        if seq_enabled:
            flags |= 0x2
        if ack_enabled:
            flags |= 0x4
        if frag_enabled:
            flags |= 0x8
        caps = struct.pack('<IHHHH20x', flags, max_payload, max_retries, timeout_ms, lock_wait_ms)
        response = self._send_cmd_wait_resp(OMVPOpcode.PROTO_SET_CAPS, 0, caps)
        self.crc_enabled = crc_enabled
        self.seq_enabled = seq_enabled
        self.ack_enabled = ack_enabled
        self.frag_enabled = frag_enabled
        self.max_payload = max_payload
        logging.debug(f"Updated capabilities: CRC={crc_enabled}, SEQ={seq_enabled}, "
                      f"ACK={ack_enabled}, FRAG={frag_enabled}, max_payload={max_payload}")
  
    # Channel operations
    def channel_list(self):
        """List registered channels on the device"""
        channels = []
        entry_size = 16  # bytes: 1 (id) + 1 (flags) + 14 (name)
        
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_LIST)
        num_channels = len(payload) // entry_size
    
        for i in range(num_channels):
            offset = i * entry_size
            # Unpack id, flags, and raw name bytes in one go
            cid, flags, raw_name = struct.unpack_from("<BB14s", payload, offset)
            # Strip at first null byte, then decode
            name = raw_name.split(b"\x00", 1)[0].decode("utf-8", errors="ignore")
            channels.append({"id": cid, "flags": flags, "name": name })

        return channels
 
    def channel_lock(self, channel_id):
        """Lock a data channel"""
        return self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_LOCK, channel_id)
    
    def channel_unlock(self, channel_id):
        """Lock a data channel"""
        return self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_UNLOCK, channel_id)

    def channel_size(self, channel_id):
        """Get available data size for a channel"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_SIZE, channel_id)
        return struct.unpack('<I', payload[:4])[0]
    
    def channel_shape(self, channel_id):
        """Get available data size for a channel"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_SHAPE, channel_id)
        return struct.unpack(f'<{len(payload)//4}I', payload)

    def channel_read(self, channel_id, offset, length):
        """Read data from a channel with automatic packet splitting"""
        data = bytearray()
        chunk_size = length if self.frag_enabled else self.max_payload
        
        for start in range(0, length, chunk_size):
            payload = struct.pack('<II', offset + start, min(chunk_size, length - start))
            chunk = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_READ, channel_id, payload)
            data.extend(chunk)
        return bytes(data)

    def channel_write(self, channel_id, data, offset=0):
        """Write data to a channel with automatic packet splitting"""
        # Maximum payload size - 8-byte header (offset + length)
        chunk_size = self.max_payload - 8

        for start in range(0, len(data), chunk_size):
            chunk = data[start:start + chunk_size]
            # Payload: offset + length + data
            payload = struct.pack('<II', offset + start, len(chunk)) + chunk
            self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_WRITE, channel_id, payload)

    def channel_ioctl(self, channel_id, cmd, fmt=None, *args):
        """Perform ioctl operation on a channel"""
        payload = struct.pack('<I', int(cmd))
        if fmt and args:
            payload += struct.pack('<' + fmt, *args)
        return self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_IOCTL, channel_id, payload)
    
    def stop(self):
        """Stop running script"""
        # Execute the script with ioctl request=0x01
        stdin_id = self.channels["stdin"]
        self.channel_ioctl(stdin_id, 0x01)
 
    def exec(self, script):
        """Write and execute a script"""
        stdin_id = self.channels["stdin"]
        script = memoryview(script.encode('utf-8'))

        # Upload script data (handles chunking)
        self.channel_write(stdin_id, script)
        # Execute the script with ioctl request=0x02
        self.channel_ioctl(stdin_id, 0x02)
   
    def streaming(self, enable):
        """Enable or disable streaming"""
        stream_id = self.channels.get('stream')
        if stream_id:
            self.channel_ioctl(stream_id, 0x00, 'I', int(enable))

    def read_status(self):
        """Poll channels status and return a dictionary of channel readiness"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_POLL)
        flags = struct.unpack('<I', payload[:4])[0]
        
        # Build a dictionary mapping channel names to their poll status
        result = {}
        for name, channel_id in self.channels.items():
            result[name] = bool(flags & (1 << channel_id))
        
        return result

    def read_profile(self):
        """Read profiler data from the profile channel"""           
        # Check if profile channel is available (replaces profile_enabled check)
        profile_id = self.channels.get('profile')
        if not profile_id:
            return None

        # TODO just subtract the known record size
        # Get event count from cached system info (pmu_eventcnt field)
        event_count = self.sysinfo['pmu_eventcnt']
        
        # Lock the profile channel
        if not self.channel_lock(profile_id):
            return None

        try:
            # Get profile data shape and calculate size
            shape = self.channel_shape(profile_id)
            if len(shape) < 2:
                return None
            
            profile_size = reduce(mul, shape)
            if profile_size == 0:
                return None
                
            record_count, record_size = shape[0], shape[1]
            
            # Read raw profile data using calculated size
            data = self.channel_read(profile_id, 0, profile_size)
            if len(data) == 0:
                return None
                
            # Parse profile records
            records = []
            record_format = f"<5I2Q{event_count}QI"
            
            for i in range(record_count):
                offset = i * record_size
                if offset + record_size > len(data):
                    break
                    
                # Unpack the record
                profile = struct.unpack(record_format, data[offset:offset + record_size])
                
                # Parse the profile data
                records.append({
                    'address': profile[0],
                    'caller': profile[1],
                    'call_count': profile[2],
                    'min_ticks': profile[3],
                    'max_ticks': profile[4],
                    'total_ticks': profile[5],
                    'total_cycles': profile[6],
                    'events': profile[7:7 + event_count]
                })
                
            return records
        finally:
            # Always unlock the profile channel
            self.channel_unlock(profile_id)

    def read_stdout(self):
        """Read text output buffer"""
        stdout_id = self.channels["stdout"]
        if size := self.channel_size(stdout_id):
            data = self.channel_read(stdout_id, 0, size)
            return bytes(data).decode('utf-8', errors='ignore')

    def read_frame(self):
        """Read stream buffer data with header at the beginning"""
        stream_id = self.channels.get('stream')

        # Lock the stream buffer
        if not self.channel_lock(stream_id):
            return None

        try:
            # Get total size (16-byte header + stream data)
            if (size := self.channel_size(stream_id)) <= 16:
                return None
            
            # Read all data (header + stream data)
            data = self.channel_read(stream_id, 0, size)
            if len(data) < 16:
                return None
                
            # Parse stream header: width(4), height(4), pixformat(4), depth/size(4)
            width, height, pixformat, depth = struct.unpack('<IIII', data[:16])
            
            # Update frame timing
            if self.first_frame_time is None:
                self.first_frame_time = time.time()
            
            self.transport.update_frame_stats(self.first_frame_time)
        
            return {
                'width': width,
                'height': height, 
                'format': pixformat,
                'depth': depth,
                'data': data[16:]
            }
        finally:
            # Always unlock the stream buffer
            self.channel_unlock(stream_id)

    def system_info(self):
        """Get system information"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.SYS_INFO)
        
        if len(payload) < 64:
            raise OMVPException(f"Invalid SYS_INFO payload size: {len(payload)}")
        
        # Unpack the structure:
        data = struct.unpack('<I12sII I6I 3s3s3s 3x', payload)
        
        # Extract capability bitfield
        capabilities = data[4]
        
        return {
            'cpu_id': data[0],
            'device_id': data[1],
            'sensor_chip_id': data[2],
            'gpu_present': bool(capabilities & (1 << 0)),
            'npu_present': bool(capabilities & (1 << 1)),
            'isp_present': bool(capabilities & (1 << 2)),
            'venc_present': bool(capabilities & (1 << 3)),
            'jpeg_present': bool(capabilities & (1 << 4)),
            'dram_present': bool(capabilities & (1 << 5)),
            'pmu_present': bool(capabilities & (1 << 6)),
            'pmu_eventcnt': (capabilities >> 7) & 0x3F,  # 6 bits starting at bit 7
            'wifi_present': bool(capabilities & (1 << 13)),
            'bt_present': bool(capabilities & (1 << 14)),
            'sd_present': bool(capabilities & (1 << 15)),
            'eth_present': bool(capabilities & (1 << 16)),
            'usb_highspeed': bool(capabilities & (1 << 17)),
            'flash_size_kb': data[5],
            'ram_size_kb': data[6],
            'framebuffer_size_kb': data[7],
            'stream_buffer_size_kb': data[8],
            'firmware_version': data[11],
            'protocol_version': data[12],
            'bootloader_version': data[13]
        }
    
    def print_system_info(self):
        """Print formatted system information"""
        logging.info("=== OpenMV System Information ===")

        # Print registered channels
        logging.info(f"Registered channels ({len(self.chaninfo)}):")
        for ch in self.chaninfo:
            logging.info(f"  ID: {ch['id']}, Flags: 0x{ch['flags']:02X}, Name: {ch['name']}")

        logging.info(f"CPU ID: 0x{self.sysinfo['cpu_id']:08X}")
        logging.info(f"Device ID: {self.sysinfo['device_id'].hex().upper()}")
        logging.info(f"Sensor Chip ID: 0x{self.sysinfo['sensor_chip_id']:08X}")
        
        # Memory info
        if self.sysinfo['flash_size_kb'] > 0:
            logging.info(f"Flash: {self.sysinfo['flash_size_kb']} KB")
        if self.sysinfo['ram_size_kb'] > 0:
            logging.info(f"RAM: {self.sysinfo['ram_size_kb']} KB")
        if self.sysinfo['framebuffer_size_kb'] > 0:
            logging.info(f"Framebuffer: {self.sysinfo['framebuffer_size_kb']} KB")
        if self.sysinfo['stream_buffer_size_kb'] > 0:
            logging.info(f"Stream Buffer: {self.sysinfo['stream_buffer_size_kb']} KB")
        
        # Hardware capabilities
        logging.info("Hardware capabilities:")
        logging.info(f"  GPU: {'Yes' if self.sysinfo['gpu_present'] else 'No'}")
        logging.info(f"  NPU: {'Yes' if self.sysinfo['npu_present'] else 'No'}")
        logging.info(f"  ISP: {'Yes' if self.sysinfo['isp_present'] else 'No'}")
        logging.info(f"  Video Encoder: {'Yes' if self.sysinfo['venc_present'] else 'No'}")
        logging.info(f"  JPEG Encoder: {'Yes' if self.sysinfo['jpeg_present'] else 'No'}")
        logging.info(f"  DRAM: {'Yes' if self.sysinfo['dram_present'] else 'No'}")
        logging.info(f"  PMU: {'Yes' if self.sysinfo['pmu_present'] else 'No'} ({self.sysinfo['pmu_eventcnt']} counters)")
        logging.info(f"  WiFi: {'Yes' if self.sysinfo['wifi_present'] else 'No'}")
        logging.info(f"  Bluetooth: {'Yes' if self.sysinfo['bt_present'] else 'No'}")
        logging.info(f"  SD Card: {'Yes' if self.sysinfo['sd_present'] else 'No'}")
        logging.info(f"  Ethernet: {'Yes' if self.sysinfo['eth_present'] else 'No'}")
        logging.info(f"  USB High-Speed: {'Yes' if self.sysinfo['usb_highspeed'] else 'No'}")
        
        # Profiler info - check if profile channel is available
        profile_available = 'profile' in self.channels
        logging.info(f"Profiler: {'Available' if profile_available else 'Not available'}")
        
        # Version info
        fw = self.sysinfo['firmware_version']
        proto = self.sysinfo['protocol_version'] 
        boot = self.sysinfo['bootloader_version']
        logging.info(f"Firmware version: {fw[0]}.{fw[1]}.{fw[2]}")
        logging.info(f"Protocol version: {proto[0]}.{proto[1]}.{proto[2]}")
        logging.info(f"Bootloader version: {boot[0]}.{boot[1]}.{boot[2]}")
        logging.info("=================================")

