# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson FFC Control Example
#
# The Boson periodically re-baselines itself against its internal shutter with
# a flat-field correction (FFC) - the click you hear. Readings drift between
# FFCs as the core heats and cools, so the FFC controls are the knobs behind
# measurement stability. This example shows configuring automatic FFC,
# triggering a manual one, and watching the FFC status.

import csi
import image
import time

FFC_TEMP_THRESHOLD = 10  # FPA drift that triggers an FFC, in C x10 (1.0 C)
FFC_PERIOD_S = 300  # seconds between automatic FFCs
FFC_NUM_FRAMES = 8  # frames averaged while the shutter is closed
MANUAL_FFC_PERIOD_MS = 30000  # this example also triggers one every 30s

STATUS_NAMES = {
    csi.BOSON_FFC_STATUS_NONE: "never run",
    csi.BOSON_FFC_STATUS_IMMINENT: "imminent",
    csi.BOSON_FFC_STATUS_RUNNING: "running",
    csi.BOSON_FFC_STATUS_COMPLETE: "complete",
}

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.QVGA if csi0.cid() == csi.BOSON320 else csi.VGA)
csi0.color_palette(image.PALETTE_IRONBOW)

print("FFC defaults: mode %d, temp threshold %d (C x10), period %d s, %d frames" % (
    csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_MODE),
    csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_TEMP_THRESHOLD),
    csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_FRAME_THRESHOLD),
    csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_NUM_FRAMES)))

# Automatic FFC: the camera fires one whenever the core drifts by the
# temperature threshold or the period elapses, whichever comes first.
csi0.ioctl(csi.IOCTL_BOSON_SET_FFC_TEMP_THRESHOLD, FFC_TEMP_THRESHOLD)
csi0.ioctl(csi.IOCTL_BOSON_SET_FFC_FRAME_THRESHOLD, FFC_PERIOD_S)
csi0.ioctl(csi.IOCTL_BOSON_SET_FFC_NUM_FRAMES, FFC_NUM_FRAMES)
csi0.ioctl(csi.IOCTL_BOSON_SET_FFC_MODE, csi.BOSON_FFC_AUTO)

clock = time.clock()
last_print_ms = 0
last_ffc_ms = time.ticks_ms()

while True:
    clock.tick()
    img = csi0.snapshot()
    now = time.ticks_ms()

    # Trigger a manual FFC now and then - e.g. right before a measurement
    # that matters, or on an external signal.
    if time.ticks_diff(now, last_ffc_ms) >= MANUAL_FFC_PERIOD_MS:
        csi0.ioctl(csi.IOCTL_BOSON_RUN_FFC)
        start = time.ticks_ms()
        while csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_STATUS) != csi.BOSON_FFC_STATUS_COMPLETE:
            if time.ticks_diff(time.ticks_ms(), start) > 5000:
                break
            time.sleep_ms(50)
        print("Manual FFC finished in %d ms" % time.ticks_diff(time.ticks_ms(), start))
        last_ffc_ms = now

    if time.ticks_diff(now, last_print_ms) >= 1000:
        status = csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_STATUS)
        print("FPS %.2f - FFC %s, FPA %.1f C" % (
            clock.fps(), STATUS_NAMES.get(status, status),
            csi0.ioctl(csi.IOCTL_BOSON_GET_FPA_TEMP)))
        last_print_ms = now
