# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Ulab is a numpy-like module for micropython, meant to simplify and speed up common
# mathematical operations on arrays. This basic example shows mean/std on an image.
#
# NOTE: ndarrays cause the heap to be fragmented easily. If you run out of memory,
# there's not much that can be done about it, lowering the resolution might help.

import csi
import time
from ulab import numpy as np

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.GRAYSCALE)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QQVGA)  # Set frame size to QVGA (320x240)

clock = time.clock()  # Create a clock object to track the FPS.

while True:
    img = csi0.snapshot()  # Take a picture and return the image.
    a = np.array(img, dtype=np.uint8)
    print("mean: %d std:%d" % (np.mean(a), np.std(a)))
