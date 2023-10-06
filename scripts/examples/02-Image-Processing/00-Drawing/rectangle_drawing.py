# Rectangle Drawing
#
# This example shows off drawing rectangles on the OpenMV Cam.

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
        w = randint(0, img.width() // 2)
        h = randint(0, img.height() // 2)

        r = randint(0, 127) + 128
        g = randint(0, 127) + 128
        b = randint(0, 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x, y, w, and h. Otherwise, it expects a (x,y,w,h) tuple.
        img.draw_rectangle(x, y, w, h, color=(r, g, b), thickness=2, fill=False)

    print(clock.fps())
