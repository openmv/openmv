# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Face Eye Detection Example
#
# This script uses the built-in frontalface detector to find a face and then
# the eyes within the face. If you want to determine the eye gaze please see the
# iris_detection script for an example on how to do that.

import csi
import time
import image

csi0 = csi.CSI()
csi0.reset()
csi0.contrast(1)
csi0.gainceiling(16)
csi0.framesize(csi.HQVGA)
csi0.pixformat(csi.GRAYSCALE)

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
face_cascade = image.HaarCascade("/rom/haarcascade_frontalface.cascade", stages=25)
eyes_cascade = image.HaarCascade("/rom/haarcascade_eye.cascade", stages=24)
print(face_cascade, eyes_cascade)

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    # Capture snapshot
    img = csi0.snapshot()

    # Find a face !
    # Note: Lower scale factor scales-down the image more and detects smaller objects.
    # Higher threshold results in a higher detection rate, with more false positives.
    objects = img.find_features(face_cascade, threshold=0.5, scale_factor=1.5)

    # Draw faces
    for face in objects:
        img.draw_rectangle(face)
        # Now find eyes within each face.
        # Note: Use a higher threshold here (more detections) and lower scale (to find small objects)
        eyes = img.find_features(
            eyes_cascade, threshold=0.5, scale_factor=1.2, roi=face
        )
        for e in eyes:
            img.draw_rectangle(e)

    # Print FPS.
    # Note: Actual FPS is higher, streaming the FB makes it slower.
    print(clock.fps())
