# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson Gain Control Example
#
# High gain gives the best sensitivity; low gain extends the scene range for
# very hot targets; auto lets the camera switch between them on its own. The
# 16-bit response differs between the two states (~1.4x), so anything that
# interprets raw counts must track the state in force - which is also why
# pinning a state is useful for repeatable measurements.
#
# This example cycles through the gain modes and shows the effect on the
# image statistics.

import csi
import time

GAIN_DWELL_MS = 5000  # how long to hold each gain mode

GAIN_NAMES = {
    csi.BOSON_GAIN_HIGH: "high",
    csi.BOSON_GAIN_LOW: "low",
    csi.BOSON_GAIN_AUTO: "auto",
}
GAIN_CYCLE = (csi.BOSON_GAIN_HIGH, csi.BOSON_GAIN_LOW, csi.BOSON_GAIN_AUTO)

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.QVGA if csi0.cid() == csi.BOSON320 else csi.VGA)

clock = time.clock()
gain_index = -1
gain_set_ms = -GAIN_DWELL_MS
last_print_ms = 0

while True:
    clock.tick()
    img = csi0.snapshot()
    now = time.ticks_ms()

    if time.ticks_diff(now, gain_set_ms) >= GAIN_DWELL_MS:
        gain_index = (gain_index + 1) % len(GAIN_CYCLE)
        csi0.ioctl(csi.IOCTL_BOSON_SET_GAIN_MODE, GAIN_CYCLE[gain_index])
        gain_set_ms = now
        print("Set gain mode: " + GAIN_NAMES[GAIN_CYCLE[gain_index]])

    if time.ticks_diff(now, last_print_ms) >= 1000:
        # In auto mode the readback reports the MODE, not which state the
        # camera resolved it to.
        mode = csi0.ioctl(csi.IOCTL_BOSON_GET_GAIN_MODE)
        stats = img.get_statistics()
        print("FPS %.2f - gain mode %s, image mean %d, stdev %d" % (
            clock.fps(), GAIN_NAMES.get(mode, mode), stats.mean, stats.stdev))
        last_print_ms = now
