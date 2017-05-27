# Edge Detection Example:
#
# This example demonstrates using the morph function on an image to do edge
# detection and then thresholding and filtering that image afterwards.

import sensor, image, time

kernel_size = 1 # kernel width = (size*2)+1, kernel height = (size*2)+1
kernel = [-1, -1, -1,\
          -1, +8, -1,\
          -1, -1, -1]
# This is a high pass filter kernel. see here for more kernels:
# http://www.fmwconcepts.com/imagemagick/digital_image_filtering.pdf
thresholds = [(100, 255)] # grayscale thresholds

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.RGB565
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

# On the OV7725 sensor, edge detection can be enhanced
# significantly by setting the sharpness/edge registers.
# Note: This will be implemented as a function later.
if (sensor.get_id() == sensor.OV7725):
    sensor.__write_reg(0xAC, 0xDF)
    sensor.__write_reg(0x8F, 0xFF)

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    img.morph(kernel_size, kernel)
    img.binary(thresholds)

    # Erode pixels with less than 2 neighbors using a 3x3 image kernel
    img.erode(1, threshold = 2)

    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
