# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the genx320 event camera from Prophesee.

import csi
import image
import time

csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize((320, 320))
csi0.brightness(128)  # Leave at 128 generally (this is the default).
csi0.contrast(16)  # Increase to make the image pop.
csi0.color_palette(image.PALETTE_EVT_LIGHT)  # image.PALETTE_EVT_DARK for dark mode.

# The default frame rate is 50 FPS. You can change it between ~20 FPS and ~350 FPS.
csi0.framerate(50)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()
    # img.median(1) # noise cleanup.

    print(clock.fps())
