# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Erode and Dilate Example
#
# This example shows off the erode and dilate functions which you can run on
# a binary image to remove noise. This example was originally a test but its
# useful for showing off how these functions work.

import csi

csi0 = csi.CSI()
csi0.reset()
csi0.framesize(csi.QVGA)

grayscale_thres = (170, 255)
rgb565_thres = (70, 100, -128, 127, -128, 127)

while True:
    csi0.pixformat(csi.GRAYSCALE)
    for i in range(20):
        img = csi0.snapshot()
        img.binary([grayscale_thres])
        img.erode(2)
    for i in range(20):
        img = csi0.snapshot()
        img.binary([grayscale_thres])
        img.dilate(2)

    csi0.pixformat(csi.RGB565)
    for i in range(20):
        img = csi0.snapshot()
        img.binary([rgb565_thres])
        img.erode(2)
    for i in range(20):
        img = csi0.snapshot()
        img.binary([rgb565_thres])
        img.dilate(2)
