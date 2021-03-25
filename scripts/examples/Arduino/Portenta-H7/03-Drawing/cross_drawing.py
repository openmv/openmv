# Cross Drawing
#
# This example shows off drawing crosses on the OpenMV Cam.

import sensor, image, time, pyb

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA) # or QQVGA...
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()

    img = sensor.snapshot()

    for i in range(10):
        x = (pyb.rng() % (2*img.width())) - (img.width()//2)
        y = (pyb.rng() % (2*img.height())) - (img.height()//2)
        r = (pyb.rng() % 127) + 128
        g = (pyb.rng() % 127) + 128
        b = (pyb.rng() % 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x and y. Otherwise, it expects a (x,y) tuple.
        img.draw_cross(x, y, color = (r, g, b), size = 10, thickness = 2)

    print(clock.fps())
