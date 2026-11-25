# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the VL53L9CX time-of-flight sensor from ST
# to read the signal of each zone, which is an infrared image of the scene
# lit by the sensor, and to discard distances with a weak signal.

import csi
import image
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

# Zones with a lower signal per SPAD have noisy distances.
SIGNAL_THRESHOLD = 1.25

# Initialize the sensor.
csi0 = csi.CSI(cid=csi.VL53L9CX)
csi0.reset()
# Supported frame sizes: (54, 42), (24, 20), (18, 14), (12, 10), (8, 6) and (4, 4).
csi0.framesize((54, 42))

# Enable the planes to read, which are stored in the order below.
#   [0]  TOF_PLANE_DEPTH    Distance in mm, 0 when there is no valid return
#   [1]  TOF_PLANE_SIGNAL   Signal per SPAD
#   [2]  TOF_PLANE_AMBIENT  Ambient light per SPAD
csi0.ioctl(csi.IOCTL_TOF_SET_PLANES,
           csi.TOF_PLANE_DEPTH | csi.TOF_PLANE_SIGNAL | csi.TOF_PLANE_AMBIENT)

# Stores the enabled planes of each zone
# Shape: (height, width, 3), the same as the image, so windowed and transposed too.
planes = np.zeros((csi0.height(), csi0.width(), 3), dtype=np.float)

clock = time.clock()

while True:
    clock.tick()

    # Reads the planes of a new frame.
    csi0.ioctl(csi.IOCTL_TOF_READ_PLANES, planes)

    depth = planes[:, :, 0].flatten()
    signal = planes[:, :, 1]

    # Scale the signal to 0-255 and send it to the IDE as an image.
    # Near and reflective objects are bright, it works in the dark too.
    scaled = signal * (255 / max(np.max(signal), 0.001))
    img = image.Image(scaled)
    img.flush()

    # Keep the distances of zones with a strong signal only.
    valid = depth[signal.flatten() >= SIGNAL_THRESHOLD]
    mean = np.mean(valid) if len(valid) else 0

    # The ambient light rises under sunlight, which adds noise.
    ambient = np.mean(planes[:, :, 2])

    print("Valid zones: %d/%d Mean distance: %d mm Ambient: %d FPS: %.1f" %
          (len(valid), len(depth), mean, ambient, clock.fps()))
