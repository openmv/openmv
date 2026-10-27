# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# VL53L9CX Distance Example
#
# This example reads the raw distance of every zone, in millimeters, as a
# ulab array and prints the distance at the center of the sensor.

import csi
import time

csi0 = csi.CSI(cid=csi.VL53L9CX, stream=True)
csi0.reset(hard=False)
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize(csi.QQVGA)

# The precision mode has the best accuracy, the range mode sees farther.
csi0.ioctl(csi.IOCTL_VL53L9CX_SET_MODE, csi.VL53L9CX_MODE_PRECISION)

width, height = csi0.ioctl(csi.IOCTL_VL53L9CX_GET_RESOLUTION)
print("Resolution: %dx%d" % (width, height))

clock = time.clock()

while True:
    clock.tick()
    csi0.snapshot()

    # Distances of the last snapshot, a zero means no valid return.
    depth = csi0.ioctl(csi.IOCTL_VL53L9CX_GET_DEPTH)
    print("Center: %d mm, %.1f FPS" % (depth[height // 2, width // 2], clock.fps()))
