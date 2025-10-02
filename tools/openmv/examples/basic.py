#!/usr/bin/env python
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Protocol Test Script
#
# This module provides a test script for the OpenMV Protocol implementation.
# It demonstrates basic camera operations including script execution and data
# channel communication.

import sys
import os
import argparse
import time
import logging
import random

# Add parent directories to path for openmv module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../..'))

from openmv.camera import OMVCamera

# Test script - 
test_script = """
import time
import csi
import image

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)

clock = time.clock()

while(True):
    clock.tick()
    img = csi0.snapshot()
    print(clock.fps(), " FPS")
"""


def str2bool(v):
    """Convert string to boolean for argparse"""
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

def signal_handler(sig, frame):
    """Handle interrupt signals"""
    logging.info("Interrupt signal received, stopping...")
    sys.exit(0)

def main():
    parser = argparse.ArgumentParser(description='OpenMV Protocol Test')
    parser.add_argument('--port', action='store', help='Serial port (dev/ttyACM0)', default='/dev/ttyACM0')
    parser.add_argument("--script", action="store", default=None, help="Script file")
    parser.add_argument('--timeout', action='store', type=float, default=1.0, help='Protocol timeout in seconds')
    parser.add_argument('--debug', action='store_true', help='Enable debug logging')
    
    # Camera configuration options
    parser.add_argument('--baudrate', type=int, default=921600, help='Serial baudrate (default: 921600)')
    parser.add_argument('--crc', type=str2bool, nargs='?', const=True, default=True, help='Enable CRC validation (default: true)')
    parser.add_argument('--seq', type=str2bool, nargs='?', const=True, default=True, help='Enable sequence number validation (default: true)')
    parser.add_argument('--ack', type=str2bool, nargs='?', const=True, default=False, help='Enable packet acknowledgment (default: false)')
    parser.add_argument('--events', type=str2bool, nargs='?', const=True, default=True, help='Enable event notifications (default: true)')
    parser.add_argument('--max-retry', type=int, default=3, help='Maximum number of retries (default: 3)')
    parser.add_argument('--max-payload', type=int, default=4096, help='Maximum payload size in bytes (default: 4096)')
    parser.add_argument('--drop-rate', type=float, default=0.0, help='Packet drop simulation rate (0.0-1.0, default: 0.0)')
    parser.add_argument('--quiet', action='store_true', help='Suppress script output text')
    
    args = parser.parse_args()
    
    # Set up signal handler for graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Configure logging
    if args.quiet:
        log_level = logging.WARNING
    elif args.debug:
        log_level = logging.DEBUG
    else:
        log_level = logging.INFO
       
    logging.basicConfig(
        format="%(relativeCreated)010.3f - %(message)s",
        level=log_level,
    )

    if args.script is None:
        args.script = test_script
        logging.info("Using built-in test script...")
    else:
        with open(args.script, 'r') as f:
            args.script = f.read()
        logging.info("Loaded script from file")

    try:
        with OMVCamera(args.port, baudrate=args.baudrate, crc=args.crc, seq=args.seq, 
                       ack=args.ack, events=args.events, timeout=args.timeout, max_retry=args.max_retry,
                       max_payload=args.max_payload, drop_rate=args.drop_rate) as camera:
            logging.critical(f"Connected to OpenMV camera on {args.port}")

            data = camera.read_profile()
            if data:
                logging.info(f"Profile data ({len(data)} entries):")
                # Print first 5 entries to avoid overwhelming output
                for i, entry in enumerate(data[:min(5, len(data))]):
                    logging.info(f"Entry {i}: address=0x{entry['address']:08X}, "
                                f"caller=0x{entry['caller']:08X}, calls={entry['call_count']}, "
                                f"ticks={entry['total_ticks']}")
                if len(data) > 5:
                    logging.info(f"... and {len(data) - min(5, len(data))} more entries")

            # Execute script
            logging.info("Executing script...")
            camera.streaming(True, raw=False, res=(256, 256))
            camera.exec(args.script)
            logging.info("Script executed successfully")

            # Let script run for a few seconds
            test_time = 1
            start_time = time.time()
            while time.time() - start_time < test_time:
                if not (status := camera.read_status()):
                    continue

                if status['stdout'] and (text := camera.read_stdout()):
                    if not args.quiet:
                        print(text, end='')

                if status['stream'] and (frame := camera.read_frame()):
                    logging.info(f"Frame: {frame['width']}x{frame['height']} depth={frame['depth']} "
                                 f"format=0x{frame['format']:08X} length: {len(frame['data'])} bytes")

                # Test custom channel
                if data := camera.channel_read("time"):
                    logging.info("Channel output: " + bytes(data).decode('utf-8', errors='ignore'))

                if camera.has_channel("buffer"):
                    random_data = bytes([random.randint(0, 255) for _ in range(10)])
                    logging.info(f"Writing random data: {random_data.hex()}")
                    camera.channel_write("buffer", random_data)
                    if data := camera.channel_read("buffer"):
                        logging.info(f"Read back data: {bytes(data).hex()}")
                        if bytes(data) == random_data:
                            logging.info("✓ Data verification successful!")
                        else:
                            logging.warning("✗ Data mismatch!")

            # Stop the script
            logging.info("Stopping script...")
            camera.stop()
            
            # Read output for a while
            start_time = time.time()
            while time.time() - start_time < 1:
                if status['stdout'] and (text := camera.read_stdout()):
                    if not args.quiet:
                        print(text, end='')

            print("")
            host_stats = camera.host_stats()
            logging.critical("=========== Host Statistics ===========")
            logging.critical(f"{host_stats}")
            logging.critical(f"==========================================")

            device_stats = camera.device_stats()
            logging.critical("=========== Device Statistics ===========")
            logging.critical(f"{device_stats}")
            logging.critical(f"==========================================")

    except KeyboardInterrupt:
        logging.info("Interrupted by user")
        sys.exit(0)
    except Exception as e:
        logging.error(f"Error: {e}")
        import traceback
        logging.error(f"{traceback.format_exc()}")
        sys.exit(1)

if __name__ == '__main__':
    main()
