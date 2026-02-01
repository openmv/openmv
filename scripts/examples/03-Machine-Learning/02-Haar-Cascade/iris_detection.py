# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Iris Detection 2 Example
#
# This example shows how to find the eye gaze (pupil detection) after finding
# the eyes in an image. This script uses the find_eyes function which determines
# the center point of roi that should contain a pupil. It does this by basically
# finding the center of the darkest area in the eye roi which is the pupil center.
#
# Note: This script does not detect a face first, use it with the telephoto lens.

import csi
import time
import image

csi0 = csi.CSI()
csi0.reset()
csi0.contrast(3)
csi0.gainceiling(16)
csi0.framesize(csi.VGA)
# Bin/Crop image to 200x100, which gives more details with less data to process
csi0.window((220, 190, 200, 100))
csi0.pixformat(csi.GRAYSCALE)

# Load Haar Cascade
# By default this will use all stages, lower stages is faster but less accurate.
eyes_cascade = image.HaarCascade("/rom/haarcascade_eye.cascade", stages=24)
print(eyes_cascade)

# FPS clock
clock = time.clock()

while True:
    clock.tick()
    # Capture snapshot
    img = csi0.snapshot()
    # Find eyes !
    # Note: Lower scale factor scales-down the image more and detects smaller objects.
    # Higher threshold results in a higher detection rate, with more false positives.
    eyes = img.find_features(eyes_cascade, threshold=0.5, scale_factor=1.5)

    # Find iris
    for e in eyes:
        iris = img.find_eye(e)
        img.draw_rectangle(e)
        img.draw_cross(iris[0], iris[1])

    # Print FPS.
    # Note: Actual FPS is higher, streaming the FB makes it slower.
    print(clock.fps())
