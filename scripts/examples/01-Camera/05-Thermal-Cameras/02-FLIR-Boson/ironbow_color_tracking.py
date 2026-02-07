# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Single Color RGB565 Blob Tracking Example
#
# This example shows off single color RGB565 tracking using the OpenMV Cam using the FLIR BOSON.

import csi
import time
import image

# Color Tracking Thresholds (L Min, L Max, A Min, A Max, B Min, B Max)
threshold_list = [(70, 100, -30, 40, 20, 100)]

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize(csi.VGA)
csi0.color_palette(image.PALETTE_IRONBOW)
csi0.snapshot(time=5000)

clock = time.clock()

# Only blobs that with more pixels than "pixel_threshold" and more area than "area_threshold" are
# returned by "find_blobs" below. Change "pixels_threshold" and "area_threshold" if you change the
# camera resolution. "merge=True" merges all overlapping blobs in the image.

while True:
    clock.tick()
    img = csi0.snapshot()
    for blob in img.find_blobs(
        threshold_list, pixels_threshold=200, area_threshold=200, merge=True
    ):
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())
    print(clock.fps())
