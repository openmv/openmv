#!/usr/bin/env python2
import sys
# import usb.core
# import usb.util
import numpy as np
import pygame
import openmv
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
    img = sensor.snapshot()         # Take a picture and return the image.
    sensor.flush()
"""

# init pygame
pygame.init()

# init openmv
if 'darwin' in sys.platform:
    portname = "/dev/cu.usbmodem14221"
else:
    portname = "/dev/openmvcam"

connected = False
openmv.disconnect()
for i in range(10):
    try:
        # opens CDC port.
        # Set small timeout when connecting
        openmv.init(portname, baudrate=921600, timeout=0.050)
        connected = True
        break
    except Exception as e:
        connected = False
        sleep(0.100)

if not connected:
    print ( "Failed to connect to OpenMV's serial port.\n"
            "Please install OpenMV's udev rules first:\n"
            "sudo cp openmv/udev/50-openmv.rules /etc/udev/rules.d/\n"
            "sudo udevadm control --reload-rules\n\n")
    sys.exit(1)

# Set higher timeout after connecting for lengthy transfers.
openmv.set_timeout(1*2) # SD Cards can cause big hicups.
openmv.stop_script()
openmv.enable_fb(True)
openmv.exec_script(script)

# init screen
running = True
Clock = pygame.time.Clock()
font = pygame.font.SysFont("monospace", 15)

while running:
    Clock.tick(100)

    # read framebuffer
    fb = openmv.fb_dump()

    if fb == None:
        continue

    # create image from RGB888
    image = pygame.image.frombuffer(fb[2].flat[0:], (fb[0], fb[1]), 'RGB')
    # TODO check if res changed
    screen = pygame.display.set_mode((fb[0], fb[1]), pygame.DOUBLEBUF, 32)

    fps = Clock.get_fps()
    if fps < 50.0:
        sys.stderr.write("WARNING: fps drop\n")

    # blit stuff
    screen.blit(image, (0, 0))
    screen.blit(font.render("FPS %.2f"%(fps), 1, (255, 0, 0)), (0, 0))

    # update display
    pygame.display.flip()

    event = pygame.event.poll()
    if event.type == pygame.QUIT:
         running = False
    elif event.type == pygame.KEYDOWN:
        if event.key == pygame.K_ESCAPE:
            running = False
        if event.key == pygame.K_c:
            pygame.image.save(image, "capture.png")


pygame.quit()
openmv.stop_script()
