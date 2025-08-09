# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the genx320 event camera from Prophesee
# using event streaming mode.
#
# Adds filtering of events based on the event type (PIX_ON_EVENT or PIX_OFF_EVENT).

import csi
import image
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

TARGET_EVENT_TYPE = csi.PIX_ON_EVENT  # Change to PIX_OFF_EVENT to filter low events.

# Surface to draw the histogram image on.
img = image.Image(320, 320, image.GRAYSCALE)

# Stores camera events
# Shape: (EVT_res, 6) where EVT_res is the event resolution
# EVT_res: must be a power of two between 1024 and 65536.
# Columns:
#   [0]  Event type
#   [1]  Seconds timestamp
#   [2]  Milliseconds timestamp
#   [3]  Microseconds timestamp
#   [4]  X coordinate 0 to 319 for GENX320
#   [5]  Y coordinate 0 to 319 for GENX320
events = np.zeros((2048, 6), dtype=np.uint16)

# Initialize the sensor.
csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.ioctl(csi.IOCTL_GENX320_SET_MODE, csi.GENX320_MODE_EVENT, events.shape[0])

clock = time.clock()

while True:
    clock.tick()

    # Reads up to 2048 events from the camera.
    # Returns the number of valid events (0-2048) or a negative error code.
    # Note that old events in the buffer are not cleared to save CPU time.
    event_count = csi0.ioctl(csi.IOCTL_GENX320_READ_EVENTS, events)
    events_slice = events[:event_count]
    indices = np.nonzero(events_slice[:, 0] == TARGET_EVENT_TYPE)[0]
    if len(indices):
        target_events = np.take(events_slice, indices, axis=0)

        # Render events into a histogram image.
        # If clear=True, the image is reset to "brightness" before drawing.
        # For each PIX_ON_EVENT, add "contrast" to the bin value;
        # for each PIX_OFF_EVENT, subtract it and clamp to [0, 255].
        # If clear=False, histogram accumulates over multiple calls.
        img.draw_event_histogram(target_events, clear=True, brightness=128, contrast=64)

        # Push the image to the jpeg buffer for the IDE to pull and display.
        # The IDE pulls frames off the camera at a much lower rate than the
        # onboard camera frame rate printed below.
        img.flush()

        print(len(target_events), clock.fps())
