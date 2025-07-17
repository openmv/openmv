# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the genx320 event camera from Prophesee
# using event streaming mode.
#
# This example has no frame buffer visualization to run extremely fast.
#
# Adds filtering of events based on the event type (PIX_ON_EVENT or PIX_OFF_EVENT).

import csi
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

TARGET_EVENT_TYPE = csi.PIX_ON_EVENT  # Change to PIX_OFF_EVENT to filter low events.

# ndarray to hold events from the camera
# must be EVT_res events by 6 values
#
# 0: event type
# 1: seconds timestamp
# 2: milliseconds timestamp
# 3: microseconds timestamp
# 4: x coordinate (0-319 for the genx320)
# 5: y coordinate (0-319 for the genx320)
events = np.zeros((2048, 6), dtype=np.uint16)

# Initialize the sensor.
csi0 = csi.CSI(cid=csi.GENX320)
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.EVT_2048)  # Must be EVT_1024/2048/.../65536

clock = time.clock()
t = time.ticks_us()

i = 0
while True:
    clock.tick()
    t1 = time.ticks_us()
    diff = time.ticks_diff(t1, t)
    t = t1
    i += 1

    # Reads up to 2048 events from the camera.
    # Returns the number of valid events (0-2048) or a negative error code.
    # Note that old events in the buffer are not cleared to save CPU time.
    event_count = csi0.ioctl(csi.IOCTL_GENX320_READ_EVENTS, events)
    events_slice = events[:event_count]
    indices = np.nonzero(events_slice[:, 0] == TARGET_EVENT_TYPE)[0]
    if len(indices):
        valid_events = np.take(events_slice, indices, axis=0)

        # Sub-sample the event rate output to not impact performance of event
        # data processing. The overhead of sending stats outputs to the IDE can
        # become significant at high event rates.
        if not i % 10:
            print(f'{len(valid_events)} events\t{clock.fps()} fps\t{diff} us')
