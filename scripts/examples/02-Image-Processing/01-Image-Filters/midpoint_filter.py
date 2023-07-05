# Midpoint Filter Example
#
# This example shows off midpoint filtering. Midpoint filtering replaces each
# pixel by the average of the min and max pixel values for a NxN neighborhood.

import sensor
import time

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565)  # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA)  # or sensor.QVGA (or others)
sensor.skip_frames(time=2000)  # Let new settings take affect.
clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()  # Take a picture and return the image.

    # The first argument is the kernel size. N coresponds to a ((N*2)+1)^2
    # kernel size. E.g. 1 == 3x3 kernel, 2 == 5x5 kernel, etc. Note: You
    # shouldn't ever need to use a value bigger than 2. The "bias" argument
    # lets you select between min and max blending. 0.5 == midpoint filter,
    # 0.0 == min filter, and 1.0 == max filter. Note that the min filter
    # makes images darker while the max filter makes images lighter.
    img.midpoint(1, bias=0.5)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
