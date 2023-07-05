# Image Writer Example
#
# USE THIS EXAMPLE WITH A USD CARD! Reset the camera after recording to see the file.
#
# This example shows how to use the Image Writer object to record snapshots of what your
# OpenMV Cam sees for later analysis using the Image Reader object. Images written to disk
# by the Image Writer object are stored in a simple file format readable by your OpenMV Cam.

import sensor
import image
import pyb
import time

record_time = 10000  # 10 seconds in milliseconds

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

stream = image.ImageIO("/stream.bin", "w")

# Red LED on means we are capturing frames.
pyb.LED(1).on()

start = pyb.millis()
while pyb.elapsed_millis(start) < record_time:
    clock.tick()
    img = sensor.snapshot()
    # Modify the image if you feel like here...
    stream.write(img)
    print(clock.fps())

stream.close()

# Blue LED on means we are done.
pyb.LED(1).off()
pyb.LED(3).on()
