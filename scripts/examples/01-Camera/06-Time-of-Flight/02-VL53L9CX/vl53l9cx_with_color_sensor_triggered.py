# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# with a color sensor in triggered mode, where each snapshot of either
# sensor captures one new frame, so their frames are paired.

import csi
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

# Setup the color camera sensor.
csi0 = csi.CSI()
csi0.reset(hard=True)  # force hardware reset.
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.ioctl(csi.IOCTL_SET_TRIGGERED_MODE, True)

csi1 = csi.CSI(cid=csi.VL53L9CX)
csi1.reset(hard=False)  # no hardware reset - just configure the VL53L9CX
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi1.framesize((54, 42))
csi1.ioctl(csi.IOCTL_SET_TRIGGERED_MODE, True)

# Read only the distance, which also raises the frame rate.
csi1.ioctl(csi.IOCTL_VL53L9CX_SET_PLANES, csi.VL53L9CX_PLANE_DEPTH)

# Stores the planes of each zone
# Shape: (height, width, 3) where width x height is the frame size.
# Planes:
#   [0]  Distance in mm, 0 when there is no valid return
#   [1]  Signal amplitude per SPAD, x16
#   [2]  Ambient light per SPAD, x16
planes = np.zeros((csi1.height(), csi1.width(), 3), dtype=np.uint16)

clock = time.clock()

while True:
    clock.tick()
    img0 = csi0.snapshot()
    csi1.ioctl(csi.IOCTL_VL53L9CX_READ_PLANES, planes)
    print(planes[csi1.height() // 2, csi1.width() // 2, 0], clock.fps())
