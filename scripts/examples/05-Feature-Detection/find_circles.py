# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Find Circles Example
#
# This example shows off how to find circles in the image using the Hough
# Transform. https://en.wikipedia.org/wiki/Circle_Hough_Transform
#
# Note that the find_circles() method will only find circles which are completely
# inside of the image. Circles which go outside of the image/roi are ignored...

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)  # grayscale is faster
csi0.framesize(csi.QQVGA)
csi0.snapshot(time=2000)

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot().lens_corr(1.8)

    # Circle objects have four values: x, y, r (radius), and magnitude. The
    # magnitude is the strength of the detection of the circle. Higher is
    # better...

    # `threshold` controls how many circles are found. Increase its value
    # to decrease the number of circles detected...

    # `x_margin`, `y_margin`, and `r_margin` control the merging of similar
    # circles in the x, y, and r (radius) directions.

    # r_min, r_max, and r_step control what radiuses of circles are tested.
    # Shrinking the number of tested circle radiuses yields a big performance boost.

    for c in img.find_circles(
        threshold=2000,
        x_margin=10,
        y_margin=10,
        r_margin=10,
        r_min=2,
        r_max=100,
        r_step=2,
    ):
        img.draw_circle(c.x(), c.y(), c.r(), color=(255, 0, 0))
        print(c)

    print("FPS %f" % clock.fps())
