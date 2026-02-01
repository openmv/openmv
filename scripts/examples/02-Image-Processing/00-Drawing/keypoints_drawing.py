# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Keypoints Drawing
#
# This example shows off drawing keypoints on the OpenMV Cam. Usually you call draw_keypoints()
# on a keypoints object but you can also call it on a list of 3-value tuples...

import csi
import time
from random import randint

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)  # or GRAYSCALE...
csi0.framesize(csi.QVGA)  # or QQVGA...
csi0.snapshot(time=2000)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    for i in range(20):
        x = randint(0, 2 * img.width()) - img.width() // 2
        y = randint(0, 2 * img.height()) - img.height() // 2
        rot = randint(0, 360)

        r = randint(0, 127) + 128
        g = randint(0, 127) + 128
        b = randint(0, 127) + 128

        # This method draws a keypoints object or a list of (x, y, rot) tuples...
        img.draw_keypoints(
            [(x, y, rot)], color=(r, g, b), size=20, thickness=2, fill=False
        )

    print(clock.fps())
