# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Perspective Correction
#
# This example shows off how to use the rotation_corr() to fix perspective
# issues related to how your OpenMV Cam is mounted.

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

clock = time.clock()

# The image will be warped such that the following points become the new:
#
#   (0,   0)
#   (w-1, 0)
#   (w-1, h-1)
#   (0,   h-1)
#
# Try setting the points below to the corners of a quadrilateral
# (in clock-wise order) in the field-of-view. You can get points
# on the image by clicking and dragging on the frame buffer and
# recording the values shown in the histogram widget.

w = csi0.width()
h = csi0.height()

TARGET_POINTS = [
    (0, 0),  # (x, y) CHANGE ME!
    (w - 1, 0),  # (x, y) CHANGE ME!
    (w - 1, h - 1),  # (x, y) CHANGE ME!
    (0, h - 1),
]  # (x, y) CHANGE ME!

while True:
    clock.tick()

    img = csi0.snapshot().rotation_corr(corners=TARGET_POINTS)

    print(clock.fps())
