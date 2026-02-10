# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Automatic Grayscale Color Tracking Example
#
# This example shows off single color automatic grayscale color tracking using the OpenMV Cam.

import csi
import time

print("Letting auto algorithms run. Don't put anything in front of the camera!")

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)
csi0.auto_gain(False)  # must be turned off for color tracking
csi0.auto_whitebal(False)  # must be turned off for color tracking
clock = time.clock()

# Capture the color thresholds for whatever was in the center of the image.
r = [(320 // 2) - (50 // 2), (240 // 2) - (50 // 2), 50, 50]  # 50x50 center of QVGA.

print(
    "Auto algorithms done. Hold the object you want to track in front of the camera in the box."
)
print(
    "MAKE SURE THE COLOR OF THE OBJECT YOU WANT TO TRACK IS FULLY ENCLOSED BY THE BOX!"
)
for i in range(60):
    img = csi0.snapshot()
    img.draw_rectangle(r)

print("Learning thresholds...")
threshold = [128, 128]  # Middle grayscale values.
for i in range(60):
    img = csi0.snapshot()
    hist = img.get_histogram(roi=r)
    lo = hist.get_percentile(
        0.01
    )  # Get the CDF of the histogram at the 1% range (ADJUST AS NECESSARY)!
    hi = hist.get_percentile(
        0.99
    )  # Get the CDF of the histogram at the 99% range (ADJUST AS NECESSARY)!
    # Average in percentile values.
    threshold[0] = (threshold[0] + lo.value()) // 2
    threshold[1] = (threshold[1] + hi.value()) // 2
    for blob in img.find_blobs(
        [threshold], pixels_threshold=100, area_threshold=100, merge=True, margin=10
    ):
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())
        img.draw_rectangle(r)

print("Thresholds learned...")
print("Tracking colors...")

while True:
    clock.tick()
    img = csi0.snapshot()
    for blob in img.find_blobs(
        [threshold], pixels_threshold=100, area_threshold=100, merge=True, margin=10
    ):
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())
    print(clock.fps())
