# Edge Detection Example:
#
# This example demonstrates using the morph function on an image to do edge
# detection and then thresholding and filtering that image afterwards.

import sensor, image

kernel_size = 1 # kernel width = (size*2)+1, kernel height = (size*2)+1
kernel = [-1, -1, -1,\
          -1, +8, -1,\
          -1, -1, -1]
# This is a high pass filter kernel. ee here for more kernels:
# http://www.fmwconcepts.com/imagemagick/digital_image_filtering.pdf
thresholds = [(100, 255)] # grayscale thresholds

sensor.reset()
sensor.set_framesize(sensor.QQVGA) # smaller resolution to go faster
sensor.set_pixformat(sensor.GRAYSCALE)
while(True):
    img = sensor.snapshot()
    img.morph(kernel_size, kernel)
    img.binary(thresholds)
    img.erode(1, threshold = 2) # erode pixels with less than 2 neighbors
