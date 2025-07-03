# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the genx320 event camera from Prophesee
# using event streaming mode and calibrating the camera.

import csi
import image
import time

# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

CAL_EVENT_COUNT = 10000  # Number of events to collect for calibration.
CAL_SIGMA = 0.5  # Standard deviation for hot pixel detection.

# Surface to draw the histogram image on.
img = image.Image(320, 320, image.GRAYSCALE)

# Stores camera events
# Shape: (EVT_res, 6) where EVT_res is the event resolution
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
csi0.ioctl(csi.IOCTL_GENX320_SET_MODE, csi.GENX320_MODE_EVENT)

clock = time.clock()

t = time.ticks_ms()
calibrated = False
while True:
    clock.tick()

    # Reads 2048 events from the camera.
    # Returns the number of valid events (0-2048) or a negative error code.
    event_count = csi0.ioctl(csi.IOCTL_GENX320_READ_EVENTS, events)

    # Render events into a histogram image.
    # If clear=True, the image is reset to "brightness" before drawing.
    # For each PIX_ON_EVENT, add "contrast" to the bin value;
    # for each PIX_OFF_EVENT, subtract it and clamp to [0, 255].
    # If clear=False, histogram accumulates over multiple calls.
    img.draw_event_histogram(events[:event_count], clear=True, brightness=128, contrast=64)

    # Push the image to the jpeg buffer for the IDE to pull and display.
    # The IDE pulls frames off the camera at a much lower rate than the
    # onboard camera frame rate printed below.
    img.flush()

    # Show uncalibrated image first.
    if not calibrated and time.ticks_diff(time.ticks_ms(), t) > 5000:
        disabled_pixels = csi0.ioctl(csi.IOCTL_GENX320_CALIBRATE, CAL_EVENT_COUNT, CAL_SIGMA)
        print(f'Disabled {disabled_pixels} hot pixels.')
        calibrated = True

    print(event_count, clock.fps())
