# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Vertical Flip - Horizontal Mirror - Transpose
#
# This example shows off how to vertically flip, horizontally mirror, or
# transpose an image. Note that:
#
# vflip=False, hmirror=False, transpose=False -> 0 degree rotation
# vflip=True,  hmirror=False, transpose=True  -> 90 degree rotation
# vflip=True,  hmirror=True,  transpose=False -> 180 degree rotation
# vflip=False, hmirror=True,  transpose=True  -> 270 degree rotation

import csi
import time
import image

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

clock = time.clock()

ticks = time.ticks_ms()
counter = 0

while True:
    clock.tick()

    # You can also use image.ROTATE_180, image.ROTATE_90, image.ROTATE_270, etc.
    vflip = image.VFLIP if (counter // 2) % 2 else 0
    hmirror = image.HMIRROR if (counter // 4) % 2 else 0
    transpose = image.TRANSPOSE if (counter // 8) % 2 else 0
    img = csi0.snapshot().scale(hint=(vflip | hmirror | transpose))

    if time.ticks_diff(time.ticks_ms(), ticks) > 1000:
        ticks = time.ticks_ms()
        counter += 1

    print(clock.fps())
