# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# QRCode Example
#
# This example shows the power of the OpenMV Cam to detect QR Codes
# using lens correction (see the qrcodes_with_lens_corr.py script for higher performance).

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must turn this off to prevent image washout...

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    img.lens_corr(1.8)  # strength of 1.8 is good for the 2.8mm lens.
    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color=(255, 0, 0))
        print(code)
    print(clock.fps())
