# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Find Small Apriltags
#
# This script shows off how to use blob tracking as a pre-filter to
# finding Apriltags in the image using blob tracking to find the
# area of where the tag is first and then calling find_apriltags
# on that blob.

# Note, this script works well assuming most parts of the image do not
# pass the thresholding test... otherwise, you don't get a distance
# benefit.

# Please use the TAG36H11 tag family for this script - it's the recommended tag family to use.

import sensor
import time
import omv

# Set the thresholds to find a white object (i.e. tag border)
thresholds = (150, 255)

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
if omv.board_type() == "H7":
    sensor.set_framesize(sensor.VGA)
elif omv.board_type() == "M7":
    sensor.set_framesize(sensor.QVGA)
else:
    raise Exception("You need a more powerful OpenMV Cam to run this script")
sensor.skip_frames(time=200)  # increase this to let the auto methods run for longer
sensor.set_auto_gain(False)  # must be turned off for color tracking
sensor.set_auto_whitebal(False)  # must be turned off for color tracking
clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()

    # First, we find blobs that may be candidates for tags.
    box_list = []

    # AprilTags may fail due to not having enough ram given the image size being passed.
    tag_list = []

    for blob in img.find_blobs(
        [thresholds], pixels_threshold=100, area_threshold=100, merge=True
    ):
        # Next we look for a tag in an ROI that's bigger than the blob.
        w = min(max(int(blob.w() * 1.2), 10), 160)  # Not too small, not too big.
        h = min(max(int(blob.h() * 1.2), 10), 160)  # Not too small, not too big.
        x = min(max(int(blob.x() + (blob.w() / 4) - (w * 0.1)), 0), img.width() - 1)
        y = min(max(int(blob.y() + (blob.h() / 4) - (h * 0.1)), 0), img.height() - 1)

        box_list.append((x, y, w, h))  # We'll draw these later.

        # Since we constrict the roi size apriltags shouldn't run out of ram.
        # But, if it does we handle it...
        try:
            tag_list.extend(img.find_apriltags(roi=(x, y, w, h)))
        except (
            MemoryError
        ):  # Don't catch all exceptions otherwise you can't stop the script.
            pass

    for b in box_list:
        img.draw_rectangle(b)
    # Now print out the found tags
    for tag in tag_list:
        img.draw_rectangle(tag.rect())
        img.draw_cross(tag.cx(), tag.cy())
        for c in tag.corners():
            img.draw_circle(c[0], c[1], 5)
        print("Tag:", tag.cx(), tag.cy(), tag.rotation(), tag.id())
