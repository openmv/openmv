# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# VL53L9CX Depth Example
#
# This example streams the depth map of the VL53L9CX time-of-flight sensor.
# Near objects are bright and far objects are dark, mapped over the range
# set below. Pixels with no valid return are black.

import csi
import time

# Distances, in millimeters, mapped to the full color range.
min_distance = 100
max_distance = 2000

csi0 = csi.CSI(cid=csi.VL53L9CX, stream=True)
csi0.reset(hard=False)
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QQVGA)
csi0.ioctl(csi.IOCTL_VL53L9CX_SET_RANGE, min_distance, max_distance)

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    print(clock.fps())
