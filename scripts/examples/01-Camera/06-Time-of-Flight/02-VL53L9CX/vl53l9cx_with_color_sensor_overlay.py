# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# overlayed on top of a color sensor.

import csi
import image
import time

# Hide the zones without a valid return, which are 0.
alpha_pal = image.Image(256, 1, image.GRAYSCALE)
for i in range(256):
    alpha_pal[i] = 0 if i == 0 else 128

# Setup the color camera sensor.
csi0 = csi.CSI()
csi0.reset(hard=True)  # force hardware reset.
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)

csi1 = csi.CSI(cid=csi.VL53L9CX)
csi1.reset(hard=False)  # no hardware reset - just configure the VL53L9CX
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi1.framesize((54, 42))

# Optional distance range control for the VL53L9CX.
# csi1.ioctl(csi.IOCTL_VL53L9CX_SET_RANGE, 0, 2000)

clock = time.clock()

img1 = image.Image(csi1.width(), csi1.height(), csi1.pixformat())

while True:
    clock.tick()
    img0 = csi0.snapshot()
    csi1.snapshot(blocking=False, image=img1)
    img0.draw_image(img1, 0, 0,
                    x_scale=img0.width() / img1.width(),
                    y_scale=img0.height() / img1.height(),
                    color_palette=image.PALETTE_DEPTH,
                    alpha_palette=alpha_pal,
                    hint=image.BILINEAR)
    print(clock.fps())
