# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Color Light Removal
#
# This example shows off how to remove bright lights from the image.
# You can do this using the binary() method with the "zero=" argument.
#
# Removing bright lights from the image allows you to now use
# histeq() on the image without outliers from oversaturated
# parts of the image breaking the algorithm...

import csi
import time

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.RGB565)  # or csi.GRAYSCALE
csi0.framesize(csi.QQVGA)  # or csi.QVGA (or others)
csi0.snapshot(time=2000)  # Let new settings take affect.

clock = time.clock()  # Tracks FPS.

thresholds = (90, 100, -128, 127, -128, 127)

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot().binary([thresholds], invert=False, zero=True)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
