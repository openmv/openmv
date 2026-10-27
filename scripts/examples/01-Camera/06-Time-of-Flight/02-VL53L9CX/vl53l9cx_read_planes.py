# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# to read the distance, signal amplitude and ambient light of each zone.

import csi
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

# Initialize the sensor.
csi0 = csi.CSI(cid=csi.VL53L9CX)
csi0.reset()
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi0.framesize((54, 42))

# Stores the planes of each zone
# Shape: (height, width, 3) where width x height is the frame size.
# Planes:
#   [0]  Distance in mm, 0 when there is no valid return
#   [1]  Signal amplitude per SPAD, x16
#   [2]  Ambient light per SPAD, x16
planes = np.zeros((csi0.height(), csi0.width(), 3), dtype=np.uint16)

# The precision mode has the best accuracy, the range mode sees farther.
csi0.ioctl(csi.IOCTL_VL53L9CX_SET_MODE, csi.VL53L9CX_MODE_PRECISION)

# Enable the planes to read (this is the default). Planes not enabled are
# not written to save CPU time, and planes after the last one enabled are
# not read from the sensor, which raises the frame rate.
csi0.ioctl(csi.IOCTL_VL53L9CX_SET_PLANES,
           csi.VL53L9CX_PLANE_DEPTH | csi.VL53L9CX_PLANE_AMPLITUDE | csi.VL53L9CX_PLANE_AMBIENT)

clock = time.clock()

while True:
    clock.tick()

    # Reads the enabled planes of a new frame.
    csi0.ioctl(csi.IOCTL_VL53L9CX_READ_PLANES, planes)

    print(planes[csi0.height() // 2, csi0.width() // 2, 0], clock.fps())
