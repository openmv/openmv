# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# to stream a depth map.

import csi
import image
import time

# Initialize the sensor. It's an auxiliary sensor, so stream it to the IDE.
csi0 = csi.CSI(cid=csi.VL53L9CX, stream=True)
csi0.reset()
csi0.pixformat(csi.RGB565)
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi0.framesize((54, 42))
csi0.color_palette(image.PALETTE_DEPTH)

# Distances from 0 to 4000 mm are mapped to the palette (this is the default).
csi0.ioctl(csi.IOCTL_VL53L9CX_SET_RANGE, 0, 4000)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    print(clock.fps())
