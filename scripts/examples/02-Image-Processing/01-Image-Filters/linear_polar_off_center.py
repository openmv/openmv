# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Linear Polar Mapping Off-Center Example
#
# This example shows off re-projecting the image using a linear polar
# transformation centered on a user-specified point rather than the
# image center. This is useful when the optical axis of the lens does
# not align with the sensor center (e.g. a fisheye lens mounted with
# an offset adapter), so the polar transform can be re-centered on the
# true optical center.
#
# Pass `x` and `y` keyword arguments to set the polar center. Both
# default to the image center when omitted.

import csi
import time

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.RGB565)  # or csi.GRAYSCALE
csi0.framesize(csi.QQVGA)  # or csi.QVGA (or others)
csi0.snapshot(time=2000)  # Let new settings take affect.

# Center the polar transform 20 pixels to the right and 10 pixels down
# from the sensor center. Tune these for your lens.
CENTER_X_OFFSET = 20
CENTER_Y_OFFSET = 10

clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()
    cx = (img.width() // 2) + CENTER_X_OFFSET
    cy = (img.height() // 2) + CENTER_Y_OFFSET
    img.linpolar(reverse=False, x=cx, y=cy)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
