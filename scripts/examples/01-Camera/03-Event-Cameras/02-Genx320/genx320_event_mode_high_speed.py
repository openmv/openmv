# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the genx320 event camera from Prophesee
# using event streaming mode.
#
# This example has no frame buffer visualization to run extremely fast.

import csi
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

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

    # Sub-sample the event rate output to not impact performance of event
    # data processing. The overhead of sending stats outputs to the IDE can
    # become significant at high event rates.
    if not i % 10:
        print(f'{event_count} events\t{clock.fps()} fps\t{diff} us')
