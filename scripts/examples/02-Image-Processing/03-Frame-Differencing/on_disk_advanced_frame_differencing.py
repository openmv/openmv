# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Advanced Frame Differencing Example
#
# Note: You will need an SD card to run this example.
#
# This example demonstrates using frame differencing with your OpenMV Cam. This
# example is advanced because it performs a background update to deal with the
# background image changing overtime.

import sensor
import os
import time

TRIGGER_THRESHOLD = 5

BG_UPDATE_FRAMES = 50  # How many frames before blending.
BG_UPDATE_BLEND = 128  # How much to blend by... ([0-256]==[0.0-1.0]).

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565)  # or sensor.RGB565
sensor.set_framesize(sensor.QVGA)  # or sensor.QQVGA (or others)
sensor.skip_frames(time=2000)  # Let new settings take affect.
sensor.set_auto_whitebal(False)  # Turn off white balance.
clock = time.clock()  # Tracks FPS.

if not "temp" in os.listdir():
    os.mkdir("temp")  # Make a temp directory

print("About to save background image...")
sensor.skip_frames(time=2000)  # Give the user time to get ready.
sensor.snapshot().save("temp/bg.bmp")
print("Saved background image - Now frame differencing!")

triggered = False

frame_count = 0
while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()  # Take a picture and return the image.

    frame_count += 1
    if frame_count > BG_UPDATE_FRAMES:
        frame_count = 0
        # Blend in new frame. We're doing 256-alpha here because we want to
        # blend the new frame into the background. Not the background into the
        # new frame which would be just alpha. Blend replaces each pixel by
        # ((NEW*(alpha))+(OLD*(256-alpha)))/256. So, a low alpha results in
        # low blending of the new image while a high alpha results in high
        # blending of the new image. We need to reverse that for this update.
        img.blend("temp/bg.bmp", alpha=(256 - BG_UPDATE_BLEND))
        img.save("temp/bg.bmp")

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
