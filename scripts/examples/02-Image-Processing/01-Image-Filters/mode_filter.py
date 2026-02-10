# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Mode Filter Example
#
# This example shows off mode filtering. Mode filtering is a highly non-linear
# operation which replaces each pixel with the mode of the NxN neighborhood
# of pixels around it. Avoid using the mode filter on RGB565 images. It will
# cause artifacts on image edges...

import csi
import time

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.GRAYSCALE)  # or csi.RGB565
csi0.framesize(csi.QQVGA)  # or csi.QVGA (or others)
csi0.snapshot(time=2000)  # Let new settings take affect.

clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.

    # The only argument to the median filter is the kernel size, it can be
    # either 0, 1, or 2 for a 1x1, 3x3, or 5x5 kernel respectively.
    img.mode(1)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
