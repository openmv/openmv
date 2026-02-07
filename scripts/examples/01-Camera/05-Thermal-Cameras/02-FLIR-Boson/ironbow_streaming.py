# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off streaming an ironbow image from the FLIR Boson.

import csi
import time
import image

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.VGA)  # Must always be VGA or QVGA.
csi0.color_palette(image.PALETTE_IRONBOW)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    print(clock.fps())
