# Structural Similarity (SSIM) Example
#
# Note: You will need an SD card to run this example.
#
# This example shows off how to use the SSIM algorithm on your OpenMV Cam
# to detect differences between two images. The SSIM algorithm compares
# 8x8 blocks of pixels between two images to determine a similarity
# score between two images.

import sensor, image, pyb, os, time

# The image has likely changed if the sim.min() is lower than this.
MIN_TRIGGER_THRESHOLD = -0.4

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

if not "temp" in os.listdir(): os.mkdir("temp") # Make a temp directory

print("About to save background image...")
sensor.skip_frames(time = 2000) # Give the user time to get ready.
sensor.snapshot().save("temp/bg.bmp")
print("Saved background image!")

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.
    sim = img.get_similarity("temp/bg.bmp")
    change = "- Change -" if sim.min() < MIN_TRIGGER_THRESHOLD else "- No Change -"

    print(clock.fps(), change, sim)
