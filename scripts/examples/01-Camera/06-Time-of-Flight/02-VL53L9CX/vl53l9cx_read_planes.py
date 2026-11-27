# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# to read the planes of each zone: the distance, signal, ambient light and
# number of SPADs.

import csi
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

# Initialize the sensor.
csi0 = csi.CSI(cid=csi.VL53L9CX)
csi0.reset()
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi0.framesize((54, 42))

# The precision mode has the best accuracy, the range mode sees farther.
csi0.ioctl(csi.IOCTL_TOF_SET_MODE, csi.TOF_MODE_PRECISION)

# The planes have the same units on every time-of-flight sensor, and each
# sensor supports some of them:
#   TOF_PLANE_DEPTH    Distance in mm, 0 when there is no valid return
#   TOF_PLANE_SIGNAL   Signal per SPAD
#   TOF_PLANE_AMBIENT  Ambient light per SPAD
#   TOF_PLANE_SPADS    Number of SPADs
#   TOF_PLANE_SNR      Signal to noise ratio
#   TOF_PLANE_SIGMA    Distance standard deviation in mm
#   TOF_PLANE_STATUS   Sensor specific status code
supported = csi0.ioctl(csi.IOCTL_TOF_GET_PLANES)

# Enable the planes to read (all the supported ones are enabled by default).
# Reading fewer planes raises the frame rate.
enabled = supported & (csi.TOF_PLANE_DEPTH | csi.TOF_PLANE_SIGNAL | csi.TOF_PLANE_AMBIENT | csi.TOF_PLANE_SPADS)
csi0.ioctl(csi.IOCTL_TOF_SET_PLANES, enabled)

# Stores the enabled planes of each zone, in the order above.
# Shape: (height, width, enabled planes), the same as the image, so windowed
# and transposed too.
count = bin(enabled).count("1")
planes = np.zeros((csi0.height(), csi0.width(), count), dtype=np.float)

clock = time.clock()

while True:
    clock.tick()

    # Reads the enabled planes of a new frame.
    csi0.ioctl(csi.IOCTL_TOF_READ_PLANES, planes)

    # Print the planes of the center zone.
    print(planes[csi0.height() // 2, csi0.width() // 2], clock.fps())
