# Copy image to framebuffer. 
#
# This example shows how to load and copy an image to framebuffer for testing.

import sensor, image

# Still need to init sensor
sensor.reset()
# Set sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)

# Set sensor pixel format
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load image
img = image.Image("/image.pgm")

# Copy image to framebuffer
img.copy_to_fb()

# Update drawing
sensor.snapshot()
