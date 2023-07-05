# Structural Similarity (SSIM) Example
#
# This example shows off how to use the SSIM algorithm on your OpenMV Cam
# to detect differences between two images. The SSIM algorithm compares
# 8x8 blocks of pixels between two images to determine a similarity
# score between two images.

import sensor
import pyb
import os
import time

# The image has likely changed if the sim.min() is lower than this.
MIN_TRIGGER_THRESHOLD = -0.4

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
sensor.set_auto_whitebal(False) # Turn off white balance.
clock = time.clock() # Tracks FPS.

# Take from the main frame buffer's RAM to allocate a second frame buffer.
# There's a lot more RAM in the frame buffer than in the MicroPython heap.
# However, after doing this you have a lot less RAM for some algorithms...
# So, be aware that it's a lot easier to get out of RAM issues now. However,
# frame differencing doesn't use a lot of the extra space in the frame buffer.
# But, things like AprilTags do and won't work if you do this...
extra_fb = sensor.alloc_extra_fb(sensor.width(), sensor.height(), sensor.RGB565)

print("About to save background image...")
sensor.skip_frames(time = 2000) # Give the user time to get ready.
extra_fb.replace(sensor.snapshot())
print("Saved background image!")

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.
    sim = img.get_similarity(extra_fb)
    change = "- Change -" if sim.min() < MIN_TRIGGER_THRESHOLD else "- No Change -"

    print(clock.fps(), change, sim)
