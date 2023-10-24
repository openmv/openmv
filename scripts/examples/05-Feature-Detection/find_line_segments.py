# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Find Line Segments Example
#
# This example shows off how to find line segments in the image. For each line object
# found in the image a line object is returned which includes the line's rotation.

# find_line_segments() finds finite length lines (but is slow).
# Use find_line_segments() to find non-infinite lines (and is fast).

import sensor
import time

ENABLE_LENS_CORR = False  # turn on for straighter lines...

sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # grayscale is faster
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

# All lines also have `x1()`, `y1()`, `x2()`, and `y2()` methods to get their end-points
# and a `line()` method to get all the above as one 4 value tuple for `draw_line()`.

while True:
    clock.tick()
    img = sensor.snapshot()
    if ENABLE_LENS_CORR:
        img.lens_corr(1.8)  # for 2.8mm lens...

    # `merge_distance` controls the merging of nearby lines. At 0 (the default), no
    # merging is done. At 1, any line 1 pixel away from another is merged... and so
    # on as you increase this value. You may wish to merge lines as line segment
    # detection produces a lot of line segment results.

    # `max_theta_diff` controls the maximum amount of rotation difference between
    # any two lines about to be merged. The default setting allows for 15 degrees.

    for l in img.find_line_segments(merge_distance=0, max_theta_diff=5):
        img.draw_line(l.line(), color=(255, 0, 0))
        # print(l)

    print("FPS %f" % clock.fps())
