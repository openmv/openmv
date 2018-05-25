# Copy image to framebuffer. 
#
# This example shows how to load and copy an image to framebuffer for testing.

import sensor, image, time

# Still need to init sensor
sensor.reset()
# Set sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)

# Set sensor pixel format
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load image
img = image.Image("/example.bmp", copy_to_fb=True)

# Add drawing code here.
# img.draw_line(...)

# Flush FB
sensor.flush()

# Add a small delay to allow the IDE to read the flushed image.
time.sleep(100)
