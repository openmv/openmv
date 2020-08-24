# Ulab is a numpy-like module for micropython, meant to simplify and speed up common
# mathematical operations on arrays. This basic example shows mean/std on an image.
#
# NOTE: ndarrays cause the heap to be fragmented easily. If you run out of memory,
# there's not much that can be done about it, lowering the resolution might help.

import sensor, image, time, ulab as np
from ulab import numerical

sensor.reset()                          # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)      # Set frame size to QVGA (320x240)
clock = time.clock()                    # Create a clock object to track the FPS.

while (True):
    img = sensor.snapshot()         # Take a picture and return the image.
    a = np.array(img, dtype=np.uint8)
    print("mean: %d std:%d"%(numerical.mean(a), numerical.std(a)))
