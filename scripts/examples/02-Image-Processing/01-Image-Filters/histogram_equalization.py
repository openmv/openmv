# Histogram Equalization
#
# This example shows off how to use histogram equalization to improve
# the contrast in the image.

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot().histeq()

    print(clock.fps())
