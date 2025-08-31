#!/usr/bin/env python
"""
OpenMV Protocol V2 Test Script

This module provides a test script for the OpenMV Protocol V2 implementation.
It demonstrates basic camera operations including script execution and data
channel communication.
"""

import sys
import os
import argparse
import time
import logging

# Add parent directories to path for openmv module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../..'))

from openmv.camera import OMVCamera

# Test script - 
test_script = """
import csi, image, time
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)

clock = time.clock()

while(True):
    clock.tick()
    img = csi0.snapshot()
    print(clock.fps(), " FPS")
""" # + "# " + "x" * 1000 # Test chunking

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

def main():
    parser = argparse.ArgumentParser(description='OpenMV Protocol V2 Test')
    parser.add_argument('--port', action='store', help='Serial port (dev/ttyACM0)', default='/dev/ttyACM0')
    parser.add_argument("--script", action="store", default=None, help="Script file")
    parser.add_argument('--timeout', action='store', type=float, default=1.0, help='Protocol timeout in seconds')
    parser.add_argument('--debug', action='store_true', help='Enable debug logging')
    
    # Camera configuration options
    parser.add_argument('--baudrate', type=int, default=921600, help='Serial baudrate (default: 921600)')
    parser.add_argument('--crc', type=str2bool, nargs='?', const=True, default=True, help='Enable CRC validation (default: true)')
    parser.add_argument('--seq', type=str2bool, nargs='?', const=True, default=True, help='Enable sequence number validation (default: true)')
    parser.add_argument('--ack', type=str2bool, nargs='?', const=True, default=False, help='Enable packet acknowledgment (default: false)')
    parser.add_argument('--frag', type=str2bool, nargs='?', const=True, default=True, help='Enable packet fragmentation (default: true)')
    parser.add_argument('--max-retry', type=int, default=3, help='Maximum number of retries (default: 3)')
    parser.add_argument('--max-payload', type=int, default=4096, help='Maximum payload size in bytes (default: 4096)')
    parser.add_argument('--quiet', action='store_true', help='Suppress script output text')
    
    args = parser.parse_args()
    
    # Configure logging
    if args.quiet:
        log_level = logging.WARNING  # Only show warnings, errors, and critical
    elif args.debug:
        log_level = logging.DEBUG
    else:
        log_level = logging.INFO
        
    logging.basicConfig(
        datefmt="%H:%M:%S",
        format="%(asctime)s.%(msecs)03d - %(message)s",
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
                       ack=args.ack, frag=args.frag, timeout=args.timeout, max_retry=args.max_retry,
                       max_payload=args.max_payload) as camera:
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
            camera.streaming(True)
            camera.exec(args.script)
            logging.info("Script executed successfully")
                
            # Let script run for a few seconds
            test_time = 1
            start_time = time.time()
            while time.time() - start_time < test_time:
                status = camera.read_status()

                if status['stdout'] and (text := camera.read_stdout()):
                    if not args.quiet:
                        print(text, end='')

                if status['stream'] and (frame := camera.read_frame()):
                    logging.info(f"Frame: {frame['width']}x{frame['height']} depth={frame['depth']} "
                                 f"format=0x{frame['format']:08X} length: {len(frame['data'])} bytes")

            # Stop the script
            logging.info("Stopping script...")
            camera.stop()
            
            # Statistics
            logging.critical(f"================Statistics================")
            logging.critical(f"{camera.stats()}")
            logging.critical(f"==========================================")
    except Exception as e:
        logging.error(f"Error: {e}")
        import traceback
        logging.error(f"{traceback.format_exc()}")
        sys.exit(1)

if __name__ == '__main__':
    main()
