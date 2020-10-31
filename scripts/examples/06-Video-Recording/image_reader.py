# Image Reader Example
#
# USE THIS EXAMPLE WITH A USD CARD!
#
# This example shows how to use the Image Reader object to replay snapshots of what your
# OpenMV Cam saw saved by the Image Writer object for testing machine vision algorithms.

# Altered to allow full speed reading from SD card for extraction of sequences to the network etc. 
# Set the new pause parameter to false

import sensor, image, time

snapshot_source = False # Set to true once finished to pull data from sensor.

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

img_reader = None if snapshot_source else image.ImageReader("/stream.bin")

while(True):
    clock.tick()
    img = sensor.snapshot() if snapshot_source else img_reader.next_frame(copy_to_fb=True, loop=True, pause=True)
    # Do machine vision algorithms on the image here.

    print(clock.fps())
