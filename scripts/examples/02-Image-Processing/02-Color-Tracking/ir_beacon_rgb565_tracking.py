# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# IR Beacon RGB565 Tracking Example
#
# This example shows off IR beacon RGB565 tracking using the OpenMV Cam.

import csi
import time

thresholds = (100, 100, 0, 0, 0, 0)  # thresholds for bright white light from IR.

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((240, 240))  # 240x240 center pixels of VGA
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must be turned off for color tracking
csi0.auto_whitebal(False)  # must be turned off for color tracking

clock = time.clock()

# Only blobs that with more pixels than "pixel_threshold" and more area than "area_threshold" are
# returned by "find_blobs" below. Change "pixels_threshold" and "area_threshold" if you change the
# camera resolution. "merge=True" merges all overlapping blobs in the image.

while True:
    clock.tick()
    img = csi0.snapshot()
    for blob in img.find_blobs(
        [thresholds], pixels_threshold=200, area_threshold=200, merge=True
    ):
        ratio = blob.w() / blob.h()
        if (ratio >= 0.5) and (ratio <= 1.5):  # filter out non-squarish blobs
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
    print(clock.fps())
