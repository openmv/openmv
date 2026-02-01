# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Find Data Matrices Example
#
# This example shows off how easy it is to detect data matrices using the
# OpenMV Cam M7. Data matrices detection does not work on the M4 Camera.

import csi
import time
import math

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must turn this off to prevent image washout...
csi0.auto_whitebal(False)  # must turn this off to prevent image washout...

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    img.lens_corr(1.8)  # strength of 1.8 is good for the 2.8mm lens.

    matrices = img.find_datamatrices()
    for matrix in matrices:
        img.draw_rectangle(matrix.rect(), color=(255, 0, 0))
        print_args = (
            matrix.rows(),
            matrix.columns(),
            matrix.payload(),
            (180 * matrix.rotation()) / math.pi,
            clock.fps(),
        )
        print(
            'Matrix [%d:%d], Payload "%s", rotation %f (degrees), FPS %f' % print_args
        )
    if not matrices:
        print("FPS %f" % clock.fps())
