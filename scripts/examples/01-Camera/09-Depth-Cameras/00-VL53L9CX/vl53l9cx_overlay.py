# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# VL53L9CX Overlay Example
#
# This example overlays the VL53L9CX depth map on the PAG7936 image. Near
# objects are drawn more opaque, and pixels with no valid return are clear.
# The two sensors have different fields of view, so the overlay is only an
# approximate alignment.

import csi
import image
import math
import time

# Distances, in millimeters, mapped to the full color range.
min_distance = 100
max_distance = 2000

# Fade out far objects, which map to darker values.
alpha_pal = image.Image(256, 1, image.GRAYSCALE)
for i in range(256):
    alpha_pal[i] = int(math.pow(i / 255, 2) * 255)

csi0 = csi.CSI(cid=csi.PAG7936)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)

# Don't hard-reset the time-of-flight sensor, which would reset the PAG7936.
csi1 = csi.CSI(cid=csi.VL53L9CX)
csi1.reset(hard=False)
csi1.pixformat(csi.GRAYSCALE)
csi1.framesize(csi.QQVGA)
csi1.ioctl(csi.IOCTL_VL53L9CX_SET_RANGE, min_distance, max_distance)

# Depth frames are scaled to the color image.
depth = image.Image(csi0.width(), csi0.height(), image.GRAYSCALE)
clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    # The depth image keeps its last frame until a new one is ready.
    csi1.snapshot(blocking=False, image=depth)
    img.draw_image(depth, 0, 0, color_palette=image.PALETTE_RAINBOW,
                   alpha_palette=alpha_pal, hint=image.BILINEAR)
    print(clock.fps())
