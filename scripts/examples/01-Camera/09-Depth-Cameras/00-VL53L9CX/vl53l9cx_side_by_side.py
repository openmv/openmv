# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# VL53L9CX Side by Side Example
#
# This example shows the PAG7936 image and the VL53L9CX depth map side by
# side, by drawing both onto one canvas that is sent to the IDE.

import csi
import image
import time

# Distances, in millimeters, mapped to the full color range.
min_distance = 100
max_distance = 2000

csi0 = csi.CSI(cid=csi.PAG7936)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)

# Don't hard-reset the time-of-flight sensor, which would reset the PAG7936.
csi1 = csi.CSI(cid=csi.VL53L9CX)
csi1.reset(hard=False)
csi1.pixformat(csi.RGB565)
csi1.framesize(csi.QQVGA)
csi1.ioctl(csi.IOCTL_VL53L9CX_SET_RANGE, min_distance, max_distance)

width = csi0.width()
height = csi0.height()
canvas = image.Image(width * 2, height, image.RGB565)
depth = image.Image(width, height, image.RGB565)
clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    # The depth image keeps its last frame until a new one is ready.
    csi1.snapshot(blocking=False, image=depth)
    canvas.draw_image(img, 0, 0)
    canvas.draw_image(depth, width, 0)
    canvas.flush()
    print(clock.fps())
