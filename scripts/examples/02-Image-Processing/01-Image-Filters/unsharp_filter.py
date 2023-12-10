# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Unsharp Filter Example
#
# This example shows off using the gaussian filter to unsharp mask filter images.

import sensor
import time

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE)  # or sensor.RGB565
sensor.set_framesize(sensor.QQVGA)  # or sensor.QVGA (or others)
sensor.skip_frames(time=2000)  # Let new settings take affect.
clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()  # Take a picture and return the image.

    # Run the kernel on every pixel of the image.
    img.gaussian(1, unsharp=True)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
