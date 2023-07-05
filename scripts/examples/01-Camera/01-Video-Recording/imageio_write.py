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

record_time = 10000 # 10 seconds in milliseconds

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

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
