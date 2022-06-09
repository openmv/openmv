# Erode and Dilate Example
#
# This example shows off the erode and dilate functions which you can run on
# a binary image to remove noise. This example was originally a test but its
# useful for showing off how these functions work.

import pyb, sensor, image

sensor.reset()
sensor.set_framesize(sensor.QVGA)

grayscale_thres = (170, 255)
rgb565_thres = (70, 100, -128, 127, -128, 127)

while(True):

    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(20):
        img = sensor.snapshot()
        img.binary([grayscale_thres])
        img.erode(2)
    for i in range(20):
        img = sensor.snapshot()
        img.binary([grayscale_thres])
        img.dilate(2)

    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(20):
        img = sensor.snapshot()
        img.binary([rgb565_thres])
        img.erode(2)
    for i in range(20):
        img = sensor.snapshot()
        img.binary([rgb565_thres])
        img.dilate(2)
