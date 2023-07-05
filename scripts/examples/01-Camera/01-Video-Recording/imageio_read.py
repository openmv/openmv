# Image Reader Example
#
# USE THIS EXAMPLE WITH A USD CARD!
#
# This example shows how to use the Image Reader object to replay snapshots of what your
# OpenMV Cam saw saved by the Image Writer object for testing machine vision algorithms.

# Altered to allow full speed reading from SD card for extraction of sequences to the network etc.
# Set the new pause parameter to false

import sensor
import image
import time

snapshot_source = False  # Set to true once finished to pull data from sensor.

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

stream = None
if snapshot_source is False:
    stream = image.ImageIO("/stream.bin", "r")

while True:
    clock.tick()
    if snapshot_source:
        img = sensor.snapshot()
    else:
        img = stream.read(copy_to_fb=True, loop=True, pause=True)
    # Do machine vision algorithms on the image here.
    print(clock.fps())
