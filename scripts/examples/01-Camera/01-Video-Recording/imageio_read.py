# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Reader Example
#
# NOTE: This example requires an SD card.
#
# This example shows how to use the Image Reader object to replay a raw video file.

import image
import time

stream = image.ImageIO("stream.bin", "r")

clock = time.clock()  # Create a clock object to track the FPS.
while True:
    clock.tick()
    img = stream.read(copy_to_fb=True, loop=True, pause=True)
    # Do machine vision algorithms on the image here.
    print(clock.fps())
