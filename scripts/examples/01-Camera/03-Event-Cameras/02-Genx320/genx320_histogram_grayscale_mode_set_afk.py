# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the GenX320 event sensor from Prophesee and controlling
# AntiFlicKer (AFK) filter bloc in the GenX320 digital pipeline.
# AFK allows to detect periodic changes of given frequencies in the scene and filter them out.

import csi
import time

csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize((320, 320))
csi0.brightness(128)  # Leave at 128 generally (this is the default).
csi0.contrast(16)  # Increase to make the image pop.
csi0.framerate(100)

# Enables AFK filter to remove periodic data in the frequency range from 130Hz to 160Hz
csi0.ioctl(csi.IOCTL_GENX320_SET_AFK, 1, 130, 160)
# Disables AFK filter
# csi0.ioctl(csi.IOCTL_GENX320_SET_AFK, 0)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    print(clock.fps())
