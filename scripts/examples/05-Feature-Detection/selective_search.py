# Selective Search Example

import sensor
import time
from random import randint

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
sensor.set_auto_gain(False)
sensor.set_auto_exposure(False, exposure_us=10000)
clock = time.clock()                # Create a clock object to track the FPS.


while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    rois = img.selective_search(threshold = 200, size = 20, a1=0.5, a2=1.0, a3=1.0)
    for r in rois:
        img.draw_rectangle(r, color=(255, 0, 0))
        #img.draw_rectangle(r, color=(randint(100, 255), randint(100, 255), randint(100, 255)))
    print(clock.fps())
