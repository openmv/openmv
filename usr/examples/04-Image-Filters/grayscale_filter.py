# Grayscale Filter Example
#
# The sensor module can preform some basic image processing while it is reading
# the image in. This example shows off how to apply grayscale thresholds.
#
# WARNING - THIS FEATURE NEEDS TO BE RE-WORKED. THE API MAY CHANGE IN THE
# FUTURE! Please use the binary function for image segmentation if possible.

import sensor, image, time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

# Segment the image by following thresholds. This segmentation is done while
# the image is being read in so it does not cost any additional time...
sensor.set_image_filter(sensor.FILTER_BW, lower=128, upper=255)

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.
    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
