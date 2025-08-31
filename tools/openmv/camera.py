# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Protocol Camera Interface
#
# This module provides the high-level OMVCamera class that handles all
# camera operations and channel communications using the OpenMV Protocol.

import struct
import time
import sys
import serial
import logging
from functools import reduce
from operator import mul
from .constants import OMVPOpcode, OMVProto, OMVPEventType, OMVPChannelIOCTL
from .exceptions import OMVPException, OMVPTimeoutException, OMVPResyncException
from .transport import OMVTransport
from . import image as omv_image

class OMVCamera:
    """OpenMV Camera Protocol Implementation"""
    
    def __init__(self, port, baudrate=921600,
                 crc=True, seq=True, ack=True, events=True,
                 timeout=1.0, max_retry=3, max_payload=4096,
                 drop_rate=0.0):
        # Serial connection
        self._serial = None
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.max_retry = max_retry
        self.drop_rate = drop_rate
        
        # Configuration stored in dictionary
        self.caps = {
            'crc': crc,
            'seq': seq,
            'ack': ack,
            'events': events,
            'max_payload': max_payload,
        }
        
        # Protocol components
        self.channels_by_name = {}
        self.channels_by_id = {}
        self.pending_channel_events = 0
        self.sysinfo = {}
        self.transport = None
        self.frame_event = False

    def __enter__(self):
        """Context manager entry"""
        self.connect()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.disconnect()

    @staticmethod
    def retry_if_failed(func):
        """Decorator to automatically retry on resync"""
        # Note all held locks are released on resync in the firmware.
        def wrapper(self, *args, **kwargs):
            try:
                return func(self, *args, **kwargs)
            except OMVPResyncException:
                return func(self, *args, **kwargs)
        wrapper.__name__ = func.__name__
        wrapper.__doc__ = func.__doc__
        return wrapper

    def poll_events(self):
        self.transport.recv_packet(poll_events=True)

    def _send_cmd_wait_resp(self, opcode, channel=0, data=b''):
        """Send a command and wait for response (ACK/NAK or data)"""
        if not self.is_connected():
            raise OMVPException("Not connected")
        
        # Special handling for reset commands - they never return
        if opcode in [OMVPOpcode.SYS_RESET, OMVPOpcode.SYS_BOOT]:
            self.transport.send_packet(opcode, channel, 0, data)
            self.disconnect()  # Device will reset, connection lost
            return None

        try:
            self.transport.send_packet(opcode, channel, 0, data)
            # Returns payload/None or raises exception
            return self.transport.recv_packet()
        except OMVPException: 
            self._resync()
            raise OMVPResyncException()
        except Exception as e:
            logging.error(e)
            sys.exit(0)

    def _handle_event(self, channel_id, event):
        """Handle events from the device"""
        if channel_id == 0:
            # System events
            event_name = OMVPEventType(event).name if event in OMVPEventType else f"0x{event:04X}"
            logging.info(f"🔔 System Event: channel=system, event={event_name}")
            
            # Handle system events
            if event == OMVPEventType.SOFT_REBOOT:
                logging.info("🔥 Soft Reboot triggered")
            elif event == OMVPEventType.CHANNEL_REGISTERED:
                self.pending_channel_events += 1
        elif channel_id in self.channels_by_id:
            # Channel events
            event_type = ""
            channel = self.channels_by_id[channel_id]["name"]

            if channel == "stream":
                self.frame_event = True
                event_type = " (Frame Ready)"
            elif channel == "stdin":
                event_type = " (Script Started)" if event == 1 else " (Script Stopped)"

            logging.info(f"🔔 Channel Event: channel={channel}, event=0x{event:04X}{event_type}")
        else:
            # Unknown channel
            logging.warning(f"⚠️ Unknown Event: channel={channel_id}, event=0x{event:04X}")

    def _resync(self):
        logging.info("🔁 Resynchronizing")

        # Use the protocol defaults for the initial connection
        self.transport = OMVTransport(self._serial, crc=True, seq=True,
                                      max_payload=OMVProto.MIN_PAYLOAD_SIZE, timeout=self.timeout,
                                      event_callback=self._handle_event, drop_rate=self.drop_rate)

        # Perform resync sequence on timeout
        for attempt in range(self.max_retry):
            try:
                # Send SYNC command and wait for response
                self.transport.reset_sequence()
                self.transport.send_packet(OMVPOpcode.PROTO_SYNC, 0, 0)
                if self.transport.recv_packet():
                    self.transport.reset_sequence()
                    break
            except OMVPException:
                if attempt < self.max_retry - 1:
                    logging.warning(f"⚠️ Sync attempt {attempt + 1} failed, retrying...")
                    continue
                else:
                    logging.error("❌ Failed to resync after maximum attempts")
                    raise OMVPTimeoutException("Resync failed - unable to synchronize with device")

         # Set protocol configuration to user-requested values
        self.update_capabilities()

        # Update transport with final negotiated capabilities
        self.transport.update_caps(self.caps['crc'], self.caps['seq'],
                                   self.caps['ack'], self.caps['max_payload'])

    def _channel_lock(self, channel_id):
        """Lock a data channel"""
        return self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_LOCK, channel_id)
    
    def _channel_unlock(self, channel_id):
        """Lock a data channel"""
        return self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_UNLOCK, channel_id)

    def _channel_size(self, channel_id):
        """Get available data size for a channel"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_SIZE, channel_id)
        return struct.unpack('<I', payload[:4])[0]
    
    def _channel_shape(self, channel_id):
        """Get available data size for a channel"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_SHAPE, channel_id)
        return struct.unpack(f'<{len(payload)//4}I', payload)

    def _channel_read(self, channel_id, offset, length):
        """Read data from a channel (protocol handles fragmentation automatically)"""
        payload = struct.pack('<II', offset, length)
        data = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_READ, channel_id, payload)
        return bytes(data)

    def _channel_write(self, channel_id, data, offset=0):
        """Write data to a channel with automatic packet splitting"""
        # Maximum payload size - 8-byte header (offset + length)
        chunk_size = self.caps['max_payload'] - 8

        for start in range(0, len(data), chunk_size):
            chunk = data[start:start + chunk_size]
            # Payload: offset + length + data
            payload = struct.pack('<II', offset + start, len(chunk)) + chunk
            self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_WRITE, channel_id, payload)

    def _channel_ioctl(self, channel_id, cmd, fmt=None, *args):
        """Perform ioctl operation on a channel"""
        payload = struct.pack('<I', cmd)
        if fmt and args:
            payload += struct.pack('<' + fmt, *args)
        return self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_IOCTL, channel_id, payload)
    
    def _channel_list(self):
        """List registered channels on the device"""
        channels = {}
        entry_size = 16  # bytes: 1 (id) + 1 (flags) + 14 (name)
        
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_LIST)
        num_channels = len(payload) // entry_size
    
        for i in range(num_channels):
            offset = i * entry_size
            # Unpack id, flags, and raw name bytes in one go
            cid, flags, raw_name = struct.unpack_from("<BB14s", payload, offset)
            # Strip at first null byte, then decode
            name = raw_name.split(b"\x00", 1)[0].decode("utf-8", errors="ignore")
            channels[cid] = {"name": name, "flags": flags}

        return channels

    def update_channels(self):
        """Update channel list from device"""
        if self.pending_channel_events > 0:
            self.pending_channel_events -= 1
        self.channels_by_id = self._channel_list()
        self.channels_by_name = {ch['name']: cid for cid, ch in self.channels_by_id.items()}
        logging.info(f"Registered channels ({len(self.channels_by_id)}):")
        for cid, ch in self.channels_by_id.items():
            logging.info(f"  ID: {cid}, Flags: 0x{ch['flags']:02X}, Name: {ch['name']}")
    
    def get_channel(self, name=None, channel_id=None):
        """Get channel ID by name or channel name by ID with lazy loading"""
        if self.pending_channel_events > 0:
            self.update_channels()
        
        if name is not None:
            # Return channel ID for given name
            return self.channels_by_name.get(name)
        elif channel_id is not None:
            # Return channel name given ID
            return self.channels_by_id.get(channel_id)["name"]
        else:
            raise ValueError("Must specify either name or channel_id")

    def connect(self):
        """Establish connection to the OpenMV camera"""
        try:
            self._serial = serial.Serial(self.port, self.baudrate, timeout=self.timeout)#, rtscts=True)
            # Perform resync
            self._resync()

            # Cache channel list
            self.update_channels()

            # Cache system info
            self.sysinfo = self.system_info()

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
    
    def host_stats(self):
        """Get transport statistics"""
        return self.transport.stats

    @retry_if_failed
    def device_stats(self):
        """Get protocol statistics"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.PROTO_STATS)
        
        if len(payload) < 32:
            raise OMVPException(f"Invalid PROTO_STATS payload size: {len(payload)}")
        
        # Unpack the structure: 8 uint32_t fields (32 bytes total)
        data = struct.unpack('<8I', payload)
        
        return {
            'sent': data[0],
            'received': data[1],
            'checksum': data[2],
            'sequence': data[3],
            'retransmit': data[4],
            'transport': data[5],
            'sent_events': data[6],
            'max_ack_queue_depth': data[7]
        }

    @retry_if_failed
    def reset(self):
        """Reset the camera"""
        self._send_cmd_wait_resp(OMVPOpcode.SYS_RESET)

    @retry_if_failed
    def boot(self):
        """Jump to bootloader"""
        self._send_cmd_wait_resp(OMVPOpcode.SYS_BOOT)
    
    @retry_if_failed
    def update_capabilities(self):
        """Set device capabilities"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.PROTO_GET_CAPS)
        flags, max_payload = struct.unpack('<IH', payload[:6])

        flags = (self.caps['crc']   << 0 |
                 self.caps['seq']   << 1 |
                 self.caps['ack']   << 2 |
                 self.caps['events'] << 3)

        self.caps['max_payload'] = min(max_payload, self.caps['max_payload'])

        payload = struct.pack('<IH10x', flags, self.caps['max_payload'])
        response = self._send_cmd_wait_resp(OMVPOpcode.PROTO_SET_CAPS, 0, payload)

    @retry_if_failed
    def stop(self):
        """Stop running script"""
        # Stop running script
        stdin_id = self.get_channel(name="stdin")
        self._channel_ioctl(stdin_id, OMVPChannelIOCTL.STDIN_STOP)
 
    @retry_if_failed
    def exec(self, script):
        """Write and execute a script"""
        stdin_id = self.get_channel(name="stdin")
        # Reset script buffer
        self._channel_ioctl(stdin_id, OMVPChannelIOCTL.STDIN_RESET)
        # Upload script data
        self._channel_write(stdin_id, memoryview(script.encode('utf-8')))
        # Execute the script
        self._channel_ioctl(stdin_id, OMVPChannelIOCTL.STDIN_EXEC)
   
    @retry_if_failed
    def streaming(self, enable, raw=False, res=None):
        """Enable or disable streaming"""
        stream_id = self.get_channel(name="stream")
        if raw:
            self._channel_ioctl(stream_id, OMVPChannelIOCTL.STREAM_RAW_CFG, 'II', *res)
        self._channel_ioctl(stream_id, OMVPChannelIOCTL.STREAM_RAW_CTRL, 'I', raw)
        self._channel_ioctl(stream_id, OMVPChannelIOCTL.STREAM_CTRL, 'I', enable)

    @retry_if_failed
    def read_status(self):
        """Poll channels status and return a dictionary of channel readiness"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.CHANNEL_POLL)
        flags = struct.unpack('<I', payload[:4])[0]
        
        # Build a dictionary mapping channel names to their poll status
        result = {}
        if self.pending_channel_events > 0:
            self.update_channels()
        for name, channel_id in self.channels_by_name.items():
            result[name] = bool(flags & (1 << channel_id))
        
        return result

    @retry_if_failed
    def profiler_reset(self):
        """Reset the profiler data"""
        if profile_id := self.get_channel(name="profile"):
            self._channel_ioctl(profile_id, OMVPChannelIOCTL.PROFILE_RESET)
            logging.debug("Profiler reset")

    @retry_if_failed
    def profiler_mode(self, exclusive=False):
        """Set profiler mode (exclusive=True for exclusive, False for inclusive)"""
        if profile_id := self.get_channel(name="profile"):
            mode = 1 if exclusive else 0
            self._channel_ioctl(profile_id, OMVPChannelIOCTL.PROFILE_MODE, 'I', mode)
            logging.debug(f"Profile mode set to {'exclusive' if exclusive else 'inclusive'}")

    @retry_if_failed
    def profiler_event_type(self, counter_num, event_id):
        """Configure an event counter to monitor a specific event"""
        if profile_id := self.get_channel(name="profile"):
            self._channel_ioctl(profile_id, OMVPChannelIOCTL.PROFILE_SET_EVENT, 'II', counter_num, event_id)
            logging.debug(f"Event counter {counter_num} set to event 0x{event_id:04X}")

    @retry_if_failed
    def read_profile(self):
        """Read profiler data from the profile channel"""           
        # Check if profile channel is available (replaces profile_enabled check)
        profile_id = self.get_channel(name="profile")
        if not profile_id:
            return None

        # TODO just subtract the known record size
        # Get event count from cached system info (pmu_eventcnt field)
        event_count = self.sysinfo['pmu_eventcnt']
        
        # Lock the profile channel
        if not self._channel_lock(profile_id):
            return None

        try:
            # Get profile data shape and calculate size
            shape = self._channel_shape(profile_id)
            if len(shape) < 2:
                return None
            
            profile_size = reduce(mul, shape)
            if profile_size == 0:
                return None
                
            record_count, record_size = shape[0], shape[1]
            
            # Read raw profile data using calculated size
            data = self._channel_read(profile_id, 0, profile_size)
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
            self._channel_unlock(profile_id)

    @retry_if_failed
    def read_stdout(self):
        """Read text output buffer"""
        stdout_id = self.get_channel(name="stdout")
        if size := self._channel_size(stdout_id):
            data = self._channel_read(stdout_id, 0, size)
            return bytes(data).decode('utf-8', errors='ignore')

    @retry_if_failed
    def read_frame(self):
        """Read stream buffer data with header at the beginning and convert to RGB888"""
        stream_id = self.get_channel(name="stream")

        # Lock the stream buffer
        if not self._channel_lock(stream_id):
            return None

        self.frame_event = False
        try:
            # Get total size (16-byte header + stream data)
            if (size := self._channel_size(stream_id)) <= 16:
                return None
            
            # Read all data (header + stream data)
            data = self._channel_read(stream_id, 0, size)
            if len(data) < 16:
                return None
                
            # Parse stream header: width(4), height(4), pixformat(4), depth/size(4)
            width, height, pixformat, depth = struct.unpack('<IIII', data[:16])
            
            # Extract raw frame data
            raw_data = data[16:]
            
            # Convert to RGB888 using image module
            rgb_data, fmt_str = omv_image.convert_to_rgb888(raw_data, width, height, pixformat)
            if rgb_data is None:
                return None
            
            return {
                'width': width,
                'height': height, 
                'format': pixformat,
                'depth': depth,
                'data': rgb_data,
                'raw_size': len(raw_data)
            }
        finally:
            self._channel_unlock(stream_id)
    
    @retry_if_failed
    def channel_size(self, channel):
        """Get size of data available in a custom channel"""
        channel_id = self.get_channel(name=channel)
        return 0 if channel_id is None else self._channel_size(channel_id)
    
    @retry_if_failed
    def channel_read(self, channel, size=None):
        """Read data from a custom channel"""
        if channel_id := self.get_channel(name=channel):
            if size is None:
                size = self._channel_size(channel_id)
            return self._channel_read(channel_id, 0, size)
        return None

    @retry_if_failed
    def channel_write(self, channel, data):
        """Write data to a custom channel"""
        channel_id = self.get_channel(name=channel)
        if channel_id:
            self._channel_write(channel_id, data)
        return channel_id is not None

    def has_channel(self, channel):
        """Check if a channel exists"""
        return self.get_channel(name=channel) is not None

    @retry_if_failed
    def system_info(self):
        """Get system information"""
        payload = self._send_cmd_wait_resp(OMVPOpcode.SYS_INFO)
        
        if len(payload) < 80:
            raise OMVPException(f"Invalid SYS_INFO payload size: {len(payload)}")
        
        # Unpack the structure:
        # cpu_id[1] + dev_id[3] + chip_id[3] + id_reserved[2] + hw_caps[2] + memory[6] + versions[9] + padding[3]
        data = struct.unpack('<I 3I 3I 2I 2I 6I 3s3s3s 3x', payload)
        
        # Extract capability bitfield (always 2 words)
        capabilities = data[9]  # First hw_caps word
        capabilities2 = data[10]  # Second hw_caps word
        
        return {
            'cpu_id': data[0],
            'device_id': data[1:4],  # Now 3 words
            'sensor_chip_id': data[4:7],  # Now 3 words
            'gpu_present': bool(capabilities & (1 << 0)),
            'npu_present': bool(capabilities & (1 << 1)),
            'isp_present': bool(capabilities & (1 << 2)),
            'venc_present': bool(capabilities & (1 << 3)),
            'jpeg_present': bool(capabilities & (1 << 4)),
            'dram_present': bool(capabilities & (1 << 5)),
            'crc_present': bool(capabilities & (1 << 6)),
            'pmu_present': bool(capabilities & (1 << 7)),
            'pmu_eventcnt': (capabilities >> 8) & 0xFF,  # 8 bits starting at bit 8
            'wifi_present': bool(capabilities & (1 << 16)),
            'bt_present': bool(capabilities & (1 << 17)),
            'sd_present': bool(capabilities & (1 << 18)),
            'eth_present': bool(capabilities & (1 << 19)),
            'usb_highspeed': bool(capabilities & (1 << 20)),
            'multicore_present': bool(capabilities & (1 << 21)),
            'flash_size_kb': data[11],
            'ram_size_kb': data[12],
            'framebuffer_size_kb': data[13],
            'stream_buffer_size_kb': data[14],
            'firmware_version': data[17],
            'protocol_version': data[18],
            'bootloader_version': data[19]
        }
    
    def print_system_info(self):
        """Print formatted system information"""
        logging.info("=== OpenMV System Information ===")

        # Print registered channels
        logging.info(f"CPU ID: 0x{self.sysinfo['cpu_id']:08X}")
        
        # Device ID is now an array of 3 words
        dev_id_hex = ''.join(f"{word:08X}" for word in self.sysinfo['device_id'])
        logging.info(f"Device ID: {dev_id_hex}")
        
        # Sensor Chip IDs are now an array of 3 words
        for i, chip_id in enumerate(self.sysinfo['sensor_chip_id']):
            if chip_id != 0:  # Only show non-zero chip IDs
                logging.info(f"CSI{i}: 0x{chip_id:08X}")
        
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
        logging.info(f"  CRC Hardware: {'Yes' if self.sysinfo['crc_present'] else 'No'}")
        logging.info(f"  PMU: {'Yes' if self.sysinfo['pmu_present'] else 'No'} "
                     f"({self.sysinfo['pmu_eventcnt']} counters)")
        logging.info(f"  Multi-core: {'Yes' if self.sysinfo['multicore_present'] else 'No'}")
        logging.info(f"  WiFi: {'Yes' if self.sysinfo['wifi_present'] else 'No'}")
        logging.info(f"  Bluetooth: {'Yes' if self.sysinfo['bt_present'] else 'No'}")
        logging.info(f"  SD Card: {'Yes' if self.sysinfo['sd_present'] else 'No'}")
        logging.info(f"  Ethernet: {'Yes' if self.sysinfo['eth_present'] else 'No'}")
        logging.info(f"  USB High-Speed: {'Yes' if self.sysinfo['usb_highspeed'] else 'No'}")
        
        # Profiler info - check if profile channel is available
        profile_available = self.get_channel(name="profile") is not None
        logging.info(f"Profiler: {'Available' if profile_available else 'Not available'}")
        
        # Version info
        fw = self.sysinfo['firmware_version']
        proto = self.sysinfo['protocol_version'] 
        boot = self.sysinfo['bootloader_version']
        logging.info(f"Firmware version: {fw[0]}.{fw[1]}.{fw[2]}")
        logging.info(f"Protocol version: {proto[0]}.{proto[1]}.{proto[2]}")
        logging.info(f"Bootloader version: {boot[0]}.{boot[1]}.{boot[2]}")
        logging.info(f"Protocol capabilities: CRC={self.caps['crc']}, SEQ={self.caps['seq']}, "
                     f"ACK={self.caps['ack']}, EVENTS={self.caps['events']}, "
                     f"PAYLOAD={self.caps['max_payload']}")
        logging.info("=================================")
