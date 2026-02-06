# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Basic Frame Differencing Example
#
# Note: You will need an SD card to run this example.
#
# This example demonstrates using frame differencing with your OpenMV Cam. It's
# called basic frame differencing because there's no background image update.
# So, as time passes the background image may change resulting in issues.

import csi
import os
import time

TRIGGER_THRESHOLD = 5

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # or csi.RGB565
csi0.framesize(csi.QVGA)  # or csi.QQVGA (or others)
csi0.snapshot(time=2000)  # Let new settings take affect.
csi0.auto_whitebal(False)  # Turn off white balance.
clock = time.clock()  # Tracks FPS.

if "temp" not in os.listdir():
    os.mkdir("temp")  # Make a temp directory

print("About to save background image...")
csi0.snapshot(time=2000)  # Give the user time to get ready.
csi0.snapshot().save("temp/bg.bmp")
print("Saved background image - Now frame differencing!")

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.

    # Replace the image with the "abs(NEW-OLD)" frame difference.
    img.difference("temp/bg.bmp")

    hist = img.get_histogram()
    # This code below works by comparing the 99th percentile value (e.g. the
    # non-outlier max value against the 90th percentile value (e.g. a non-max
    # value. The difference between the two values will grow as the difference
    # image seems more pixels change.
    diff = hist.get_percentile(0.99).l_value() - hist.get_percentile(0.90).l_value()
    triggered = diff > TRIGGER_THRESHOLD

    print(clock.fps(), triggered)  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
