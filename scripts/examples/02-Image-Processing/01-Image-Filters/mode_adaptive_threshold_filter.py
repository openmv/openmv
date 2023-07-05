# Mode Adaptive Threshold Filter Example
#
# This example shows off mode filtering with adaptive thresholding.
# When mode(threshold=True) the mode() method adaptive thresholds the image
# by comparing the mode of the pixels around a pixel, minus an offset, with that pixel.
# Avoid using the mode filter on RGB565 images. It will cause artifacts on image edges...

import sensor
import time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.RGB565
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    # The only argument to the median filter is the kernel size, it can be
    # either 0, 1, or 2 for a 1x1, 3x3, or 5x5 kernel respectively.
    img.mode(1, threshold=True, offset=5, invert=True)

    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
