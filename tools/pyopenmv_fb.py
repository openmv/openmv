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
from time import sleep

script = """
# Hello World Example
#
# Welcome to the OpenMV IDE! Click on the green run arrow button below to run the script!

import sensor, image, time

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

while(True):
    clock.tick()
    img = sensor.snapshot()         # Take a picture and return the image.
    sensor.flush()
    print(clock.fps(), " FPS")
"""

# init pygame
pygame.init()

if len(sys.argv)!= 2:
    print ('usage: pyopenmv_fb.py <serial port>')
    sys.exit(1)

connected = False
portname = sys.argv[1]

pyopenmv.disconnect()
for i in range(10):
    try:
        # opens CDC port.
        # Set small timeout when connecting
        pyopenmv.init(portname, baudrate=921600, timeout=0.050)
        connected = True
        break
    except Exception as e:
        connected = False
        sleep(0.100)

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
pyopenmv.exec_script(script)

# init screen
running = True
screen = None
IMAGE_SCALE = 4

Clock = pygame.time.Clock()
font = pygame.font.SysFont("monospace", 25)

try:
    while running:
        Clock.tick(30)
        # Read state
        w, h, data, text = pyopenmv.read_state()

        if text is not None:
            print(text, end="")

        if data is not None:
            fps = Clock.get_fps()
            # Create image from RGB888
            image = pygame.image.frombuffer(data.flat[0:], (w, h), 'RGB')
            image = pygame.transform.scale(image, (w * IMAGE_SCALE, h * IMAGE_SCALE))

            if screen is None:
                screen = pygame.display.set_mode((w * IMAGE_SCALE, h * IMAGE_SCALE), pygame.DOUBLEBUF, 32)

            # blit stuff
            screen.blit(image, (0, 0))
            screen.blit(font.render("FPS %.2f"%(fps), 1, (255, 0, 0)), (0, 0))
            # update display
            pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                 running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                if event.key == pygame.K_c:
                    pygame.image.save(image, "capture.png")
except KeyboardInterrupt:
    pass

pygame.quit()
pyopenmv.stop_script()
