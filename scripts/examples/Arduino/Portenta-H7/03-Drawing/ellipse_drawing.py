# Ellipse Drawing
#
# This example shows off drawing ellipses on the OpenMV Cam.

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
        radius_x = pyb.rng() % (max(img.height(), img.width())//2)
        radius_y = pyb.rng() % (max(img.height(), img.width())//2)
        rot = pyb.rng()

        r = (pyb.rng() % 127) + 128
        g = (pyb.rng() % 127) + 128
        b = (pyb.rng() % 127) + 128

        # If the first argument is a scaler then this method expects
        # to see x, y, radius x, and radius y.
        # Otherwise, it expects a (x,y,radius_x,radius_y) tuple.
        img.draw_ellipse(x, y, radius_x, radius_y, rot,
                         color = (r, g, b), thickness = 2, fill = False)

    print(clock.fps())
