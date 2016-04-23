# Color Binary Filter Example
#
# This script shows off the binary image filter. This script was originally a
# test script... but, it can be useful for showing how to use binary.

import pyb, sensor, image, math

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.RGB565)

red_threshold = (0,100,   0,127,   0,127) # L A B
green_threshold = (0,100,   -128,0,   0,127) # L A B
blue_threshold = (0,100,   -128,127,   -128,0) # L A B

while(True):

    # Test red threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([red_threshold])
    # Test green threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([green_threshold])
    # Test blue threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([blue_threshold])
    # Test not red threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([red_threshold], invert = 1)
    # Test not green threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([green_threshold], invert = 1)
    # Test not blue threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([blue_threshold], invert = 1)
