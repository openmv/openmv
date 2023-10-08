# Arrow Drawing
#
# This example shows off drawing arrows on the OpenMV Cam.

import sensor
import time
from random import randint

sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA)  # or QQVGA...
sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot()

    for i in range(10):
        x0 = randint(0, 2 * img.width()) - img.width() // 2
        y0 = randint(0, 2 * img.height()) - img.height() // 2
        x1 = randint(0, 2 * img.width()) - img.width() // 2
        y1 = randint(0, 2 * img.height()) - img.height() // 2

        r = randint(0, 127) + 128
        g = randint(0, 127) + 128
        b = randint(0, 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x0, y0, x1, and y1. Otherwise, it expects a (x0,y0,x1,y1) tuple.
        img.draw_arrow(x0, y0, x1, y1, color=(r, g, b), size=30, thickness=2)

    print(clock.fps())
