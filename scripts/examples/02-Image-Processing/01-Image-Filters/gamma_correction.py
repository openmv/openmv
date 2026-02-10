# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Gamma Correction
#
# This example shows off gamma correction to make the image brighter. The gamma
# correction method can also fix contrast and brightness too.

import csi
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

clock = time.clock()

while True:
    clock.tick()

    # Gamma, contrast, and brightness correction are applied to each color channel. The
    # values are scaled to the range per color channel per image type...
    img = csi0.snapshot().gamma_corr(gamma=0.5, contrast=1.0, brightness=0.0)

    print(clock.fps())
