#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# An example script using pyopenmv to grab the framebuffer.

import sys
import numpy as np
import pygame
import pyopenmv
import argparse
import time

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

bench_script = """
import sensor, image, time
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.VGA)
img = sensor.snapshot().compress()
while(True):
    img.flush()
"""

def pygame_test(port, poll_rate, scale, benchmark):
    # init pygame
    pygame.init()
    pyopenmv.disconnect()

    connected = False
    for i in range(10):
        try:
            # opens CDC port.
            # Set small timeout when connecting
            pyopenmv.init(port, baudrate=921600, timeout=0.050)
            connected = True
            break
        except Exception as e:
            connected = False
            time.sleep(0.100)
    
    if not connected:
        print("Failed to connect to OpenMV's serial port.\n"
              "Please install OpenMV's udev rules first:\n"
              "sudo cp openmv/udev/50-openmv.rules /etc/udev/rules.d/\n"
              "sudo udevadm control --reload-rules\n\n")
        sys.exit(1)
    
    # Set higher timeout after connecting for lengthy transfers.
    pyopenmv.set_timeout(1*2) # SD Cards can cause big hicups.
    pyopenmv.stop_script()
    pyopenmv.enable_fb(True)
    pyopenmv.exec_script(bench_script if benchmark else test_script)
    
    # init screen
    running = True
    screen = None
    
    clock = pygame.time.Clock()
    fps_clock = pygame.time.Clock()
    font = pygame.font.SysFont("monospace", 30)

    if benchmark:
        screen = pygame.display.set_mode((640, 120), pygame.DOUBLEBUF, 32)

    try:
        while running:
            # Read state
            w, h, data, size, text, fmt = pyopenmv.read_state()
    
            if text is not None:
                print(text, end="")
    
            if data is not None:
                fps = fps_clock.get_fps()
    
                # Create image from RGB888
                if not benchmark:
                    image = pygame.image.frombuffer(data.flat[0:], (w, h), 'RGB')
                    image = pygame.transform.smoothscale(image, (w * scale, h * scale))
    
                if screen is None:
                    screen = pygame.display.set_mode((w * scale, h * scale), pygame.DOUBLEBUF, 32)
    
                # blit stuff
                if benchmark:
                    screen.fill((0, 0, 0))
                else:
                    screen.blit(image, (0, 0))
                screen.blit(font.render(f"{fps:.2f} FPS {fps * size / 1024**2:.2f} MB/s {w}x{h} {fmt}", 5, (255, 0, 0)), (0, 0))
    
                # update display
                pygame.display.flip()
                fps_clock.tick(1000//poll_rate)
    
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                     running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                    if event.key == pygame.K_c:
                        pygame.image.save(image, "capture.png")
    
            clock.tick(1000//poll_rate)
    except KeyboardInterrupt:
        pass
    
    pygame.quit()
    pyopenmv.stop_script()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='pyopenmv module')
    parser.add_argument('--port', action = 'store', help='OpenMV camera port (default /dev/ttyACM0)', default='/dev/ttyACM0', )
    parser.add_argument('--poll', action = 'store', help='Poll rate (default 4ms)', default=4, type=int)
    parser.add_argument('--bench', action = 'store_true', help='Run throughput benchmark.', default=False)
    parser.add_argument('--scale', action = 'store', help='Set frame scaling factor (default 4x).', default=4, type=int)
    args = parser.parse_args()
    pygame_test(args.port, args.poll, args.scale, args.bench)
