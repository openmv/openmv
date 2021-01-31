# Grayscale Light Removal
#
# This example shows off how to remove bright lights from the image.
# You can do this using the binary() method with the "zero=" argument.
#
# Removing bright lights from the image allows you to now use
# histeq() on the image without outliers from oversaturated
# parts of the image breaking the algorithm...

import sensor, image, time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.RGB565
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

thresholds = (220, 255)

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot().binary([thresholds], invert=False, zero=True)

    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
