# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Grayscale Bilteral Filter Example
#
# This example shows off using the bilateral filter on grayscale images.

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

    # color_sigma controls how close color wise pixels have to be to each other to be
    # blurred together. A smaller value means they have to be closer.
    # A larger value is less strict.

    # space_sigma controls how close space wise pixels have to be to each other to be
    # blurred together. A smaller value means they have to be closer.
    # A larger value is less strict.

    # Run the kernel on every pixel of the image.
    img.bilateral(3, color_sigma=0.1, space_sigma=1)

    # Note that the bilateral filter can introduce image defects if you set
    # color_sigma/space_sigma to aggressively. Increase the sigma values until
    # the defects go away if you see them.

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
