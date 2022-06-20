# Line Drawing
#
# This example shows off drawing lines on the OpenMV Cam.

import sensor, image, time, pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA) # or QQVGA...
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()

    img = sensor.snapshot()

    for i in range(10):
        x0 = (pyb.rng() % (2*img.width())) - (img.width()//2)
        y0 = (pyb.rng() % (2*img.height())) - (img.height()//2)
        x1 = (pyb.rng() % (2*img.width())) - (img.width()//2)
        y1 = (pyb.rng() % (2*img.height())) - (img.height()//2)
        r = (pyb.rng() % 127) + 128
        g = (pyb.rng() % 127) + 128
        b = (pyb.rng() % 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x0, y0, x1, and y1. Otherwise, it expects a (x0,y0,x1,y1) tuple.
        img.draw_line(x0, y0, x1, y1, color = (r, g, b), thickness = 2)

    print(clock.fps())
