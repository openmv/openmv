# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Histogram Equalization
#
# This example shows off how to use histogram equalization to improve
# the contrast in the image.

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QQVGA)
csi0.snapshot(time=2000)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot().histeq()

    print(clock.fps())
