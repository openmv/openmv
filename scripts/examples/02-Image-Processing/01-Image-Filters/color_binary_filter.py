# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Color Binary Filter Example
#
# This script shows off the binary image filter. You may pass binary any
# number of thresholds to segment the image by.

import sensor
import time

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.RGB565)
sensor.skip_frames(time=2000)
clock = time.clock()

# Use the Tools -> Machine Vision -> Threshold Edtor to pick better thresholds.
red_threshold = (0, 100, 0, 127, 0, 127)  # L A B
green_threshold = (0, 100, -128, 0, 0, 127)  # L A B
blue_threshold = (0, 100, -128, 127, -128, 0)  # L A B

while True:
    # Test red threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([red_threshold])
        print(clock.fps())

    # Test green threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([green_threshold])
        print(clock.fps())

    # Test blue threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([blue_threshold])
        print(clock.fps())

    # Test not red threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([red_threshold], invert=1)
        print(clock.fps())

    # Test not green threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([green_threshold], invert=1)
        print(clock.fps())

    # Test not blue threshold
    for i in range(100):
        clock.tick()
        img = sensor.snapshot()
        img.binary([blue_threshold], invert=1)
        print(clock.fps())
