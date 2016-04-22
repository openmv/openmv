# Template Matching Example - Normalized Cross Correlation (NCC)
#
# This example shows off how to use the NCC feature of your OpenMV Cam to match
# image patches to parts of an image... expect for extremely controlled enviorments
# NCC is not all to useful.
#
# WARNING: NCC supports needs to be reworked! As of right now this feature needs
# a lot of work to be made into somethin useful. This script will reamin to show
# that the functionality exists, but, in its current state is inadequate.

import time, sensor, image

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(16)
sensor.set_contrast(2)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load template
template = image.Image("/template.bmp") # Image should be like 32x32 grayscale.

# Run template matching
while (True):
    img = sensor.snapshot()

    r = img.find_template(template, 0.75)
    if r:
        img.draw_rectangle(r)
