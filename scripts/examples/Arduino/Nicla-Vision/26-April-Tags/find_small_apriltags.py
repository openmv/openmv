# Find Small Apriltags
#
# This script shows off how to use blob tracking as a pre-filter to
# finding Apriltags in the image using blob tracking to find the
# area of where the tag is first and then calling find_apriltags
# on that blob.

# Note, this script works well assuming most parts of the image do not
# pass the thresholding test... otherwise, you don't get a distance
# benefit.

import sensor, image, time, math, omv

# Set the thresholds to find a white object (i.e. tag border)
thresholds = (150, 255)

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 200) # increase this to let the auto methods run for longer
clock = time.clock()

# The apriltag code supports up to 6 tag families which can be processed at the same time.
# Returned tag objects will have their tag family and id within the tag family.
tag_families = 0
tag_families |= image.TAG16H5 # comment out to disable this family
tag_families |= image.TAG25H7 # comment out to disable this family
tag_families |= image.TAG25H9 # comment out to disable this family
tag_families |= image.TAG36H10 # comment out to disable this family
tag_families |= image.TAG36H11 # comment out to disable this family (default family)
tag_families |= image.ARTOOLKIT # comment out to disable this family

while(True):
    clock.tick()
    img = sensor.snapshot()

    # First, we find blobs that may be candidates for tags.
    box_list = []

    # AprilTags may fail due to not having enough ram given the image sie being passed.
    tag_list = []

    for blob in img.find_blobs([thresholds], pixels_threshold=100, area_threshold=100, merge=True):
        # Next we look for a tag in an ROI that's bigger than the blob.
        w = min(max(int(blob.w() * 1.2), 10), 160) # Not too small, not too big.
        h = min(max(int(blob.h() * 1.2), 10), 160) # Not too small, not too big.
        x = min(max(int(blob.x() + (blob.w()/4) - (w * 0.1)), 0), img.width()-1)
        y = min(max(int(blob.y() + (blob.h()/4) - (h * 0.1)), 0), img.height()-1)

        box_list.append((x, y, w, h)) # We'll draw these later.

        # Since we constrict the roi size apriltags shouldn't run out of ram.
        # But, if it does we handle it...
        try:
            tag_list.extend(img.find_apriltags(roi=(x,y,w,h), families=tag_families))
        except (MemoryError): # Don't catch all exceptions otherwise you can't stop the script.
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
