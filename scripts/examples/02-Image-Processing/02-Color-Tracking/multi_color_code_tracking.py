# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Multi Color Code Tracking Example
#
# This example shows off multi color code tracking using the OpenMV Cam.
#
# A color code is a blob composed of two or more colors. The example below will
# only track colored objects which have two or more the colors below in them.

import csi
import time

# Color Tracking Thresholds (L Min, L Max, A Min, A Max, B Min, B Max)
# The below thresholds track in general red/green things. You may wish to tune them...
thresholds = [
    # generic_red_thresholds -> index is 0 so code == (1 << 0)
    (30, 100, 15, 127, 15, 127,),
    # generic_green_thresholds -> index is 1 so code == (1 << 1)
    (30, 100, -64, -8, -32, 32,),
    # generic_blue_thresholds -> index is 2 so code == (1 << 2)
    (0, 15, 0, 40, -80, -20),
]
# Codes are or'ed together when "merge=True" for "find_blobs".

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must be turned off for color tracking
csi0.auto_whitebal(False)  # must be turned off for color tracking
clock = time.clock()

# Only blobs that with more pixels than "pixel_threshold" and more area than "area_threshold" are
# returned by "find_blobs" below. Change "pixels_threshold" and "area_threshold" if you change the
# camera resolution. "merge=True" must be set to merge overlapping color blobs for color codes.

while True:
    clock.tick()
    img = csi0.snapshot()
    for blob in img.find_blobs(
        thresholds, pixels_threshold=100, area_threshold=100, merge=True
    ):
        if blob.code() == 3:  # r/g code
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            img.draw_string(blob.x() + 2, blob.y() + 2, "r/g")
        if blob.code() == 5:  # r/b code
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            img.draw_string(blob.x() + 2, blob.y() + 2, "r/b")
        if blob.code() == 6:  # g/b code
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            img.draw_string(blob.x() + 2, blob.y() + 2, "g/b")
        if blob.code() == 7:  # r/g/b code
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            img.draw_string(blob.x() + 2, blob.y() + 2, "r/g/b")
    print(clock.fps())
