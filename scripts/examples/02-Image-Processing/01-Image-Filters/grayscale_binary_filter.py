# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Grayscale Binary Filter Example
#
# This script shows off the binary image filter. You may pass binary any
# number of thresholds to segment the image by.

import sensor
import time

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.skip_frames(time=2000)
clock = time.clock()

low_threshold = (0, 50)
high_threshold = (205, 255)

while True:
    # Test low threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([low_threshold])
        print(clock.fps())

    # Test high threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([high_threshold])
        print(clock.fps())

    # Test not low threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([low_threshold], invert=1)
        print(clock.fps())

    # Test not high threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([high_threshold], invert=1)
        print(clock.fps())
