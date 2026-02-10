# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Midpoint Adaptive Threshold Filter Example
#
# This example shows off midpoint filtering with adaptive thresholding.
# When midpoint(threshold=True) the midpoint() method adaptive thresholds the image
# by comparing the midpoint of the pixels around a pixel, minus an offset, with that pixel.

import csi
import time

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.RGB565)  # or csi.GRAYSCALE
csi0.framesize(csi.QQVGA)  # or csi.QVGA (or others)
csi0.snapshot(time=2000)  # Let new settings take affect.

clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.

    # The first argument is the kernel size. N corresponds to a ((N*2)+1)^2
    # kernel size. E.g. 1 == 3x3 kernel, 2 == 5x5 kernel, etc. Note: You
    # shouldn't ever need to use a value bigger than 2. The "bias" argument
    # lets you select between min and max blending. 0.5 == midpoint filter,
    # 0.0 == min filter, and 1.0 == max filter. Note that the min filter
    # makes images darker while the max filter makes images lighter.
    img.midpoint(1, bias=0.5, threshold=True, offset=5, invert=True)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
