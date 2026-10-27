# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# next to a color sensor, by drawing both onto one canvas.

import csi
import image
import time

# Setup the color camera sensor.
csi0 = csi.CSI()
csi0.reset(hard=True)  # force hardware reset.
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)

csi1 = csi.CSI(cid=csi.VL53L9CX)
csi1.reset(hard=False)  # no hardware reset - just configure the VL53L9CX
csi1.pixformat(csi.RGB565)
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi1.framesize((54, 42))
csi1.color_palette(image.PALETTE_DEPTH)

clock = time.clock()

img1 = image.Image(csi1.width(), csi1.height(), csi1.pixformat())
canvas = image.Image(csi0.width() * 2, csi0.height(), image.RGB565)

while True:
    clock.tick()
    img0 = csi0.snapshot()
    csi1.snapshot(blocking=False, image=img1)
    canvas.draw_image(img0, 0, 0)
    canvas.draw_image(img1, csi0.width(), 0,
                      x_scale=csi0.width() / csi1.width(),
                      y_scale=csi0.height() / csi1.height(),
                      hint=image.BILINEAR)

    # Push the canvas to the jpeg buffer for the IDE to pull and display.
    canvas.flush()

    print(clock.fps())
