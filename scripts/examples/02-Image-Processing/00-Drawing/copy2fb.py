# Copy image to framebuffer.
#
# This example shows how to load and display an image.

import image
import time

# Load image
img = image.Image("/example.bmp", copy_to_fb=True)

# Add a small delay to allow the IDE to read the loaded image.
time.sleep_ms(1000)
