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
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.VGA)
sensor.set_binning(((640-80)//2, (480-60)//2, 80, 60))
sensor.set_pixformat(sensor.GRAYSCALE)

# Load template.
# Template should be a small (eg. 32x32 pixels) grayscale image.
template = image.Image("/template2.pgm")

clock = time.clock()

# Run template matching
while (True):
    clock.tick()
    img = sensor.snapshot()

    # find_template(template, threshold, [roi, step])
    # ROI: The region of interest tuple (x, y, w, h).
    # Step: The loop step used (y+=step, x+=step) use a bigger step to make it faster.
    # Note: ROI has to be smaller than the image and bigger than the template.
    r = img.find_template(template, 0.70, step=3) #, roi=(10, 0, 60, 60))
    if r:
        img.draw_rectangle(r)
    print(clock.fps())