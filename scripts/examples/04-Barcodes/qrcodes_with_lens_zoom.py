# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# QRCode Example
#
# This example shows the power of the OpenMV Cam to detect QR Codes
# without needing lens correction.

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize(csi.VGA)
csi0.window((240, 240))  # look at center 240x240 pixels of the VGA resolution.
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must turn this off to prevent image washout...

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color=127)
        print(code)
    print(clock.fps())
