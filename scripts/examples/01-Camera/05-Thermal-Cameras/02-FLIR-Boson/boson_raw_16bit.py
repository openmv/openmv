# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson Raw 16-bit Example
#
# This example shows off how to read the FLIR Boson's full 16-bit video into a ndarray at
# 60 FPS. Radiometric cores output T-Linear data, which is the absolute temperature of each
# pixel in 0.01 Kelvin units. Non-radiometric cores output raw 16-bit counts.
#
# The frame buffer keeps an auto-ranged 8-bit copy of each frame for the IDE preview. In raw
# mode csi.width() reports the capture width (2x), so get the image size before enabling it.

import csi
import image
import time
from ulab import numpy as np

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.QVGA if csi0.cid() == csi.BOSON320 else csi.VGA)
csi0.color_palette(image.PALETTE_IRONBOW)
w, h = csi0.width(), csi0.height()

radiometric = csi0.ioctl(csi.IOCTL_BOSON_GET_RADIOMETRY_CAPABLE)
print("Radiometry: " + ("Yes" if radiometric else "No"))

csi0.ioctl(csi.IOCTL_BOSON_SET_MODE, csi.BOSON_MODE_RAW)

frame = np.zeros((h, w), dtype=np.uint16)
clock = time.clock()

while True:
    clock.tick()
    csi0.ioctl(csi.IOCTL_BOSON_READ_FRAME, frame)
    center = frame[h // 2, w // 2]
    if radiometric:
        print("FPS %.2f - Center: %.2f C" % (clock.fps(), (center / 100.0) - 273.15))
    else:
        print("FPS %.2f - Center: %d counts" % (clock.fps(), center))
