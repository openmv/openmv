# Circle Drawing
#
# This example shows off drawing circles on the OpenMV Cam.

import sensor
import time
import pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA)  # or QQVGA...
sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot()

    for i in range(10):
        x = (pyb.rng() % (2 * img.width())) - (img.width() // 2)
        y = (pyb.rng() % (2 * img.height())) - (img.height() // 2)
        radius = pyb.rng() % (max(img.height(), img.width()) // 2)

        r = (pyb.rng() % 127) + 128
        g = (pyb.rng() % 127) + 128
        b = (pyb.rng() % 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x, y, and radius. Otherwise, it expects a (x,y,radius) tuple.
        img.draw_circle(x, y, radius, color=(r, g, b), thickness=2, fill=False)

    print(clock.fps())
