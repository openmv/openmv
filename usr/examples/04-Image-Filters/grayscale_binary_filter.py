# Grayscale Binary Filter Example
#
# This script shows off the binary image filter. This script was originally a
# test script... but, it can be useful for showing how to use binary.

import pyb, sensor, image, math

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

low_threshold = (0, 50)
high_threshold = (205, 255)

while(True):
    # Test low threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([low_threshold])
    # Test high threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([high_threshold])
    # Test not low threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([low_threshold], invert = 1)
    # Test not high threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([high_threshold], invert = 1)
