# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the GenX320 event sensor from Prophesee for tracking an active marker
# with LEDs. The sensor settings (biases) are tuned for seeing only high-contrast LEDs modulated at
# high frequency. With these settings, the sensor does not detect any low-contrast changes in
# the scene except of LEDs thus allowing highly efficient and robust LEDs tracking.
# Prophesee active marker board with LEDs and the corresponding event-based data are shown in
# https://youtu.be/j-LpkDpCxUU?si=jA3B4xZg9RHlyoW3.

import csi
import time

csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize((320, 320))
csi0.brightness(128)  # Leave at 128 generally (this is the default).
csi0.contrast(16)  # Increase to make the image pop.
csi0.framerate(200)
# Applies sensor biases tuned for tracking of an active marker with LEDs
csi0.ioctl(csi.IOCTL_GENX320_SET_BIASES, csi.GENX320_BIASES_ACTIVE_MARKER)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    blobs = img.find_blobs(
        [(120, 140)], invert=True, pixels_threshold=2, area_threshold=4, merge=True
    )

    for blob in blobs:
        img.draw_rectangle(blob.rect(), color=(255, 255, 255))
        img.draw_cross(blob.cx(), blob.cy(), color=(0, 0, 0))

    print(clock.fps())
