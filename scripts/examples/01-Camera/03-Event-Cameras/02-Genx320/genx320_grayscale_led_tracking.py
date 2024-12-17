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

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)  # Must always be grayscale.
sensor.set_framesize(sensor.B320X320)  # Must always be 320x320.
sensor.set_framerate(200)
# Applies sensor biases tuned for tracking of an active marker with LEDs
sensor.ioctl(sensor.IOCTL_GENX320_SET_BIASES, sensor.GENX320_BIASES_ACTIVE_MARKER)

clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot()

    blobs = img.find_blobs(
        [(120, 140)], invert=True, pixels_threshold=2, area_threshold=4, merge=True
    )

    for blob in blobs:
        img.draw_rectangle(blob.rect(), color=(255, 255, 255))
        img.draw_cross(blob.cx(), blob.cy(), color=(0, 0, 0))

    print(clock.fps())
