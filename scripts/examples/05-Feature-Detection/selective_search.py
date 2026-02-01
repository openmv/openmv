# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Selective Search Example

import csi
import time

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)
csi0.snapshot(time=2000)  # Wait for settings take effect.
csi0.auto_gain(False)
csi0.auto_exposure(False, exposure_us=10000)

clock = time.clock()  # Create a clock object to track the FPS.


while True:
    clock.tick()  # Update the FPS clock.
    img = csi0.snapshot()  # Take a picture and return the image.
    rois = img.selective_search(threshold=200, size=20, a1=0.5, a2=1.0, a3=1.0)
    for r in rois:
        img.draw_rectangle(r, color=(255, 0, 0))
        # from random import randint
        # img.draw_rectangle(r, color=(randint(100, 255), randint(100, 255), randint(100, 255)))
    print(clock.fps())
