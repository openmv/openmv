# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Ellipse Drawing
#
# This example shows off drawing ellipses on the OpenMV Cam.

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
        x = randint(0, 2 * img.width()) - img.width() // 2
        y = randint(0, 2 * img.height()) - img.height() // 2
        rx = randint(0, max(img.height(), img.width()) // 2)
        ry = randint(0, max(img.height(), img.width()) // 2)
        rot = randint(0, 360)

        r = randint(0, 127) + 128
        g = randint(0, 127) + 128
        b = randint(0, 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x, y, radius x, and radius y.
        # Otherwise, it expects a (x,y,rx,ry) tuple.
        img.draw_ellipse(
            x, y, rx, ry, rot, color=(r, g, b), thickness=2, fill=False
        )

    print(clock.fps())
