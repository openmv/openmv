# TV Example
#
# Note: To run this example you will need a tv or wireless tv shield for your OpenMV Cam.
#
# The wireless video tv Shield allows you to view your OpenMV Cam's frame buffer on the go.
#
# The TV Shield's resolution is 352x240 (SIF). By default display output is not buffered.
# You may enable triple buffering at the cost of 372 KB to make display updates non-blocking.

import sensor
import tv
import time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.SIF)
clock = time.clock()

tv.init(triple_buffer=False) # Initialize the tv.
tv.channel(8) # For wireless video transmitter shield

while(True):
    clock.tick()
    tv.display(sensor.snapshot()) # Take a picture and display the image.
    print(clock.fps())
