# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TV Example
#
# Note: To run this example you will need a tv or wireless tv shield for your OpenMV Cam.
#
# The wireless video tv Shield allows you to view your OpenMV Cam's frame buffer on the go.
#
# The TV Shield's resolution is 352x240 (SIF). By default display output is not buffered.
# You may enable triple buffering at the cost of 372 KB to make display updates non-blocking.

import csi
import tv
import time

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.RGB565)  # or csi.GRAYSCALE
csi0.framesize(csi.SIF)

clock = time.clock()

tv.init(triple_buffer=False)  # Initialize the tv.
tv.channel(8)  # For wireless video transmitter shield

while True:
    clock.tick()
    tv.display(csi0.snapshot())  # Take a picture and display the image.
    print(clock.fps())
