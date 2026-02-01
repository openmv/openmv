# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Object tracking with keypoints example.
# Show the camera an object and then run the script. A set of keypoints will be extracted
# once and then tracked in the following frames. If you want a new set of keypoints re-run
# the script. NOTE: see the docs for arguments to tune find_keypoints and match_keypoints.
import csi
import time
import image

csi0 = csi.CSI()
csi0.reset()
csi0.contrast(3)
csi0.gainceiling(16)
csi0.framesize(csi.VGA)
csi0.window((320, 240))
csi0.pixformat(csi.GRAYSCALE)

csi0.snapshot(time=2000)
csi0.auto_gain(False, gain_db=100)


def draw_keypoints(img, kpts):
    if kpts:
        print(kpts)
        img.draw_keypoints(kpts)
        img = csi0.snapshot()
        time.sleep_ms(1000)


kpts1 = None
# NOTE: uncomment to load a keypoints descriptor from file
# kpts1 = image.load_descriptor("desc.orb")
# img = csi0.snapshot()
# draw_keypoints(img, kpts1)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()
    if kpts1 is None:
        # NOTE: By default find_keypoints returns multi-scale keypoints extracted from an image pyramid.
        kpts1 = img.find_keypoints(max_keypoints=150, threshold=10, scale_factor=1.2)
        draw_keypoints(img, kpts1)
    else:
        # NOTE: When extracting keypoints to match the first descriptor, we use normalized=True to extract
        # keypoints from the first scale only, which will match one of the scales in the first descriptor.
        kpts2 = img.find_keypoints(max_keypoints=150, threshold=10, normalized=True)
        if kpts2:
            match = image.match_descriptor(kpts1, kpts2, threshold=85)
            if match.count() > 10:
                # If we have at least n "good matches"
                # Draw bounding rectangle and cross.
                img.draw_rectangle(match.rect())
                img.draw_cross(match.cx(), match.cy(), size=10)

            print(kpts2, "matched:%d dt:%d" % (match.count(), match.theta()))
            # NOTE: uncomment if you want to draw the keypoints
            # img.draw_keypoints(kpts2, size=KEYPOINTS_SIZE, matched=True)

    # Draw FPS
    img.draw_string(0, 0, "FPS:%.2f" % (clock.fps()))
