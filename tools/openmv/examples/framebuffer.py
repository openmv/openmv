#!/usr/bin/env python
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Framebuffer Display Example
#
# This example demonstrates how to display live video from an OpenMV camera using pygame.
# It shows the basic functionality of connecting to a camera, executing a script, and 
# displaying the framebuffer data in real-time with FPS counter.
#
# Controls:
# - C key: Capture screenshot to 'capture.png'
# - ESC key: Exit
#
# Dependencies:
# - pygame
# - numpy

import sys
import os
import argparse
import time
import logging
import pygame
import numpy as np
import signal
import atexit

# Add parent directories to path for openmv module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../..'))

from openmv.camera import OMVCamera

def cleanup_and_exit():
    """Force cleanup pygame and exit"""
    try:
        pygame.quit()
    except:
        pass
    os._exit(0)

def signal_handler(signum, frame):
    cleanup_and_exit()

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

# Benchmark script for throughput testing
bench_script = """
import sensor, image, time
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
img = sensor.snapshot().compress()
while(True):
    img.flush()
"""

# Default test script for sensor-based cameras
test_script = """
import sensor, image, time
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()
    img = sensor.snapshot()
    print(clock.fps(), " FPS")
"""

def main():
    parser = argparse.ArgumentParser(description='OpenMV Framebuffer Display')
    parser.add_argument('--port', action='store', help='Serial port (default: /dev/ttyACM0)', default='/dev/ttyACM0')
    parser.add_argument("--script", action="store", default=None, help="Script file")
    parser.add_argument('--poll', action='store', help='Poll rate in ms (default: 4)', default=4, type=int)
    parser.add_argument('--scale', action='store', help='Display scaling factor (default: 4)', default=4, type=int)
    parser.add_argument('--bench', action='store_true', help='Run throughput benchmark', default=False)
    parser.add_argument('--timeout', action='store', type=float, default=1.0, help='Protocol timeout in seconds')
    parser.add_argument('--debug', action='store_true', help='Enable debug logging')
    
    # Camera configuration options
    parser.add_argument('--baudrate', type=int, default=921600, help='Serial baudrate (default: 921600)')
    parser.add_argument('--crc', type=str2bool, nargs='?', const=True, default=True, help='Enable CRC validation (default: true)')
    parser.add_argument('--seq', type=str2bool, nargs='?', const=True, default=True, help='Enable sequence number validation (default: true)')
    parser.add_argument('--ack', type=str2bool, nargs='?', const=True, default=True, help='Enable packet acknowledgment (default: false)')
    parser.add_argument('--events', type=str2bool, nargs='?', const=True, default=True, help='Enable event notifications (default: true)')
    parser.add_argument('--max-retry', type=int, default=3, help='Maximum number of retries (default: 3)')
    parser.add_argument('--max-payload', type=int, default=4096, help='Maximum payload size in bytes (default: 4096)')
    parser.add_argument('--drop-rate', type=float, default=0.0, help='Packet drop simulation rate (0.0-1.0, default: 0.0)')
    parser.add_argument('--quiet', action='store_true', help='Suppress script output text')

    args = parser.parse_args()

    # Register signal handlers for clean exit
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    atexit.register(cleanup_and_exit)

    # Configure logging
    if args.debug:
        log_level = logging.DEBUG
    else:
        log_level = logging.INFO
        
    logging.basicConfig(
        format="%(relativeCreated)010.3f - %(message)s",
        level=log_level,
    )

    # Load script
    if args.script is not None:
        with open(args.script, 'r') as f:
            script = f.read()
        logging.info(f"Loaded script from {args.script}")
    else:
        script = bench_script if args.bench else test_script
        logging.info("Using built-in script")

    # Initialize pygame
    pygame.init()
    
    screen = None
    clock = pygame.time.Clock()
    fps_clock = pygame.time.Clock()
    font = pygame.font.SysFont("monospace", 30)
    
    if not args.bench:
        pygame.display.set_caption("OpenMV Camera")
    else:
        pygame.display.set_caption("OpenMV Camera (Benchmark)")
        screen = pygame.display.set_mode((640, 120), pygame.DOUBLEBUF, 32)

    try:
        with OMVCamera(args.port, baudrate=args.baudrate, crc=args.crc, seq=args.seq,
                       ack=args.ack, events=args.events,
                       timeout=args.timeout, max_retry=args.max_retry,
                       max_payload=args.max_payload, drop_rate=args.drop_rate) as camera:
            logging.info(f"Connected to OpenMV camera on {args.port}")

            camera.stop()
            time.sleep(0.500) # Wait for soft-reboot (if script is running)
            camera.exec(script)
            camera.streaming(True, raw=False, res=(512, 512))
            logging.info("Script executed, starting display...")

            while True:
                # Handle pygame events first to keep UI responsive
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        raise KeyboardInterrupt
                    elif event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_ESCAPE:
                            raise KeyboardInterrupt
                        elif event.key == pygame.K_c and not args.bench:
                            if 'image' in locals():
                                pygame.image.save(image, "capture.png")
                                logging.info("Screenshot saved as capture.png")

                # Read camera status
                status = camera.read_status()

                # Handle text output
                if not args.bench and status and status.get('stdout'):
                    text = camera.read_stdout()
                    if text and not args.quiet:
                        print(text, end='')

                # Handle frame data
                if frame := camera.read_frame():
                    fps = fps_clock.get_fps()
                    w, h, data = frame['width'], frame['height'], frame['data']

                    # Create image from RGB888 data (always converted by camera module)
                    if not args.bench:
                        image = pygame.image.frombuffer(data, (w, h), 'RGB')
                        image = pygame.transform.smoothscale(image, (w * args.scale, h * args.scale))

                    # Create/resize screen if needed
                    if screen is None:
                        screen = pygame.display.set_mode((w * args.scale, h * args.scale), pygame.DOUBLEBUF, 32)

                    # Draw frame
                    if args.bench:
                        screen.fill((0, 0, 0))
                    else:
                        screen.blit(image, (0, 0))

                    # Draw FPS info with accurate data rate
                    current_mbps = (fps * frame['raw_size']) / 1024**2
                    if current_mbps < 1.0:
                        rate_text = f"{current_mbps * 1024:.2f} KB/s"
                    else:
                        rate_text = f"{current_mbps:.2f} MB/s"
                    fps_text = f"{fps:.2f} FPS {rate_text} {w}x{h} RGB888"
                    screen.blit(font.render(fps_text, True, (255, 0, 0)), (0, 0))

                    # Update display
                    pygame.display.flip()
                    fps_clock.tick()

                # Control main loop timing
                clock.tick(1000//args.poll)

    except KeyboardInterrupt:
        logging.info("Interrupted by user")
    except Exception as e:
        logging.error(f"Error: {e}")
        import traceback
        logging.error(f"{traceback.format_exc()}")
        sys.exit(1)
    finally:
        pygame.quit()

if __name__ == '__main__':
    main()
