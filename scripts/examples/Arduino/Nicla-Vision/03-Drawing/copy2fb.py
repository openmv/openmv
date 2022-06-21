# Copy image to framebuffer. 
#
# This example shows how to load and copy an image to framebuffer for testing.

import sensor, image, time

sensor.reset()
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load image
img = image.Image("/example.bmp", copy_to_fb=True)

# Add a small delay to allow the IDE to read the loaded image.
time.sleep_ms(500)
