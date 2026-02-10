# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the frogeye2020 event camera with tracking.
#
# The frogeye2020 is a 320x240 event camera. There are two bits per pixel which show no motion,
# motion in one direction, or motion in another direction. The sensor runs at 50 FPS.

import csi
import image
import time

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.GRAYSCALE)  # Set pixel format to GRAYSCALE
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)

palette = image.Image(1, 256, csi.RGB565)

for i in range(64):
    palette.set_pixel(0, i, (0, 0, 0))

for i in range(64, 128):
    palette.set_pixel(0, i, (255, 0, 0))

for i in range(128, 192):
    palette.set_pixel(0, i, (0, 0, 255))

for i in range(192, 256):
    palette.set_pixel(0, i, (0, 255, 0))

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()
    # Make pretty.
    img.to_rainbow(color_palette=palette, hint=image.ROTATE_180)
    # Cleanup noise.
    img.erode(1)

    blobs = img.find_blobs(
        [(0, 0)], invert=True, pixels_threshold=10, area_threshold=10, merge=False
    )

    for blob in blobs:
        img.draw_rectangle(blob.rect(), color=(0, 255, 0))

    print(clock.fps())
