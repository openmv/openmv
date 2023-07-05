# Mean Adaptive Threshold Filter Example
#
# This example shows off mean filtering with adaptive thresholding.
# When mean(threshold=True) the mean() method adaptive thresholds the image
# by comparing the mean of the pixels around a pixel, minus an offset, with that pixel.

import sensor
import time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    # The first argument is the kernel size. N coresponds to a ((N*2)+1)^2
    # kernel size. E.g. 1 == 3x3 kernel, 2 == 5x5 kernel, etc. Note: You
    # shouldn't ever need to use a value bigger than 2.
    img.mean(1, threshold=True, offset=5, invert=True)

    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
