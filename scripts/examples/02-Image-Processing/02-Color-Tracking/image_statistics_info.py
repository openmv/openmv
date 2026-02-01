# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Statistics Info Example
#
# This script computes the statistics of the image and prints it out.

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # or RGB565.
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must be turned off for color tracking
csi0.auto_whitebal(False)  # must be turned off for color tracking

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    print(img.get_statistics())
    print(clock.fps())

# You can also pass get_statistics() an "roi=" to get just the statistics of that area.
# get_statistics() allows you to quickly determine the color channel information of
# any any area in the image.
