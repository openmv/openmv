# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Histogram of Oriented Gradients (HoG) Example
#
# This example demonstrates HoG visualization.
#
# Note: Due to JPEG artifacts, the HoG visualization looks blurry. To see the
# image without JPEG artifacts, uncomment the lines that save the image to uSD.

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.contrast(1)
csi0.gainceiling(8)
csi0.framesize(csi.QVGA)
csi0.pixformat(csi.GRAYSCALE)
csi0.snapshot(time=2000)

clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()
    img = csi0.snapshot()
    img.find_hog()

    # Uncomment to save raw FB to file and exit the loop
    # img.save("hog.pgm")
    # break

    print(clock.fps())
