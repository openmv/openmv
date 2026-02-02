# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Structural Similarity (SSIM) Example
#
# This example shows off how to use the SSIM algorithm on your OpenMV Cam
# to detect differences between two images. The SSIM algorithm compares
# 8x8 blocks of pixels between two images to determine a similarity
# score between two images.

import csi
import image
import time

# The image has likely changed if the sim.min() is lower than this.
MIN_TRIGGER_THRESHOLD = -0.4

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # or csi.RGB565
csi0.framesize(csi.QVGA)  # or csi.QQVGA (or others)
csi0.snapshot(time=2000)  # Let new settings take affect.
csi0.auto_whitebal(False)  # Turn off white balance.
clock = time.clock()  # Tracks FPS.

# Create a second frame buffer on the heap.
extra_fb = image.Image(csi0.width(), csi0.height(), csi0.pixformat())

print("About to save background image...")
csi0.snapshot(time=2000)  # Give the user time to get ready.
extra_fb.draw_image(csi0.snapshot())
print("Saved background image!")

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.
    sim = img.get_similarity(extra_fb)
    change = "- Change -" if sim.min() < MIN_TRIGGER_THRESHOLD else "- No Change -"

    print(clock.fps(), change, sim)
