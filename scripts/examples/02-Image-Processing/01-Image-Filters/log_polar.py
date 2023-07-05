# Log Polar Mapping Example
#
# This example shows off re-projecting the image using a log polar
# transformation. Log polar images are useful in that rotations
# become translations in the X direction and exponential changes
# in scale (x2, x4, etc.) become linear translations in the Y direction.

import sensor
import time

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565)  # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA)  # or sensor.QVGA (or others)
sensor.skip_frames(time=2000)  # Let new settings take affect.
clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot().logpolar(reverse=False)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
