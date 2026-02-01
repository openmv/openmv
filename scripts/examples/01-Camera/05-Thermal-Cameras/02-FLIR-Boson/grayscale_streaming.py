# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off streaming the default grayscale 8-bit AGC image from the FLIR Boson.

import csi
import time

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.VGA)  # Must always be VGA or QVGA.

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    print(clock.fps())
