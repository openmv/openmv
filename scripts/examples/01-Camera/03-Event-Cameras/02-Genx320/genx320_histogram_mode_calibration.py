# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off hot pixel calibration
# using the genx320 event camera from Prophesee.

import csi
import time

csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize((320, 320))
csi0.brightness(128)  # Leave at 128 generally (this is the default).
csi0.contrast(16)  # Increase to make the image pop.

# The default frame rate is 50 FPS. You can change it between ~20 FPS and ~350 FPS.
csi0.framerate(50)

# Show uncalibrated image first.
csi0.snapshot(time=5000)

CAL_EVENT_COUNT = 10000  # Number of events to collect for calibration.
CAL_SIGMA = 0.5  # Standard deviation for hot pixel detection.
disabled_pixels = csi0.ioctl(csi.IOCTL_GENX320_CALIBRATE, CAL_EVENT_COUNT, CAL_SIGMA)
print(f'Disabled {disabled_pixels} hot pixels.')

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()
    # img.median(1) # noise cleanup.

    print(clock.fps())
