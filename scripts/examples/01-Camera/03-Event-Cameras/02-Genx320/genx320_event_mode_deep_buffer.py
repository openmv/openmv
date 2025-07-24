# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the genx320 event camera from Prophesee
# using event streaming mode with a very deep event buffer for capturing events.

import csi
import image
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

# Surface to draw the histogram image on.
img = image.Image(320, 320, image.GRAYSCALE)

# ndarray to hold events from the camera
# must be EVT_res events by 6 values
#
# 0: event type
# 1: seconds timestamp
# 2: milliseconds timestamp
# 3: microseconds timestamp
# 4: x coordinate (0-319 for the genx320)
# 5: y coordinate (0-319 for the genx320)
events = np.zeros((32768, 6), dtype=np.uint16)

# Initialize the sensor.
csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.EVT_32768)  # Must be EVT_1024/2048/.../65536

clock = time.clock()

while True:
    clock.tick()

    # Reads up to 32768 events from the camera.
    # Returns the number of valid events (0-32768) or a negative error code.
    # Note that old events in the buffer are not cleared to save CPU time.
    event_count = csi0.ioctl(csi.IOCTL_GENX320_READ_EVENTS, events)

    # Draws the events on the image. If clear=True then the image is cleared
    # to the default value of "brightness". Event histogram bins have "contrast"
    # added to them for PIX_ON_EVENT events and subtracted from them for
    # PIX_OFF_EVENT events clamped between 0 and 255. Pass clear=False to keep
    # accumulating events in the histogram image.
    img.draw_event_histogram(events[:event_count], clear=True, brightness=128, contrast=16)

    # Push the image to the jpeg buffer for the IDE to pull and display.
    # The IDE pulls frames off the camera at a much lower rate than the
    # onboard camera frame rate printed below.
    img.flush()

    print(event_count, clock.fps())
