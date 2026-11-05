# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# VL53L9CX Triggered Example
#
# This example pairs each PAG7936 frame with a VL53L9CX depth frame. In
# triggered mode, every snapshot of either sensor captures one new frame.

import csi
import time

csi0 = csi.CSI(cid=csi.PAG7936)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.ioctl(csi.IOCTL_SET_TRIGGERED_MODE, True)

# Don't hard-reset the time-of-flight sensor, which would reset the PAG7936.
csi1 = csi.CSI(cid=csi.VL53L9CX)
csi1.reset(hard=False)
csi1.pixformat(csi.GRAYSCALE)
csi1.framesize(csi.QQVGA)
csi1.ioctl(csi.IOCTL_SET_TRIGGERED_MODE, True)

width, height = csi1.ioctl(csi.IOCTL_VL53L9CX_GET_RESOLUTION)
clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()
    csi1.snapshot()

    depth = csi1.ioctl(csi.IOCTL_VL53L9CX_GET_DEPTH)
    print("Center: %d mm, %.1f FPS" % (depth[height // 2, width // 2], clock.fps()))
