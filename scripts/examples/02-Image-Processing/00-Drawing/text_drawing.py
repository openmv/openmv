# Text Drawing
#
# This example shows off drawing text on the OpenMV Cam.

import sensor
import time
import pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # or GRAYSCALE...
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
        # to see x, y, and text. Otherwise, it expects a (x,y,text) tuple.

        # Character and string rotation can be done at 0, 90, 180, 270, and etc. degrees.
        img.draw_string(x, y, "Hello World!", color = (r, g, b), scale = 2, mono_space = False,
                        char_rotation = 0, char_hmirror = False, char_vflip = False,
                        string_rotation = 0, string_hmirror = False, string_vflip = False)

    print(clock.fps())
