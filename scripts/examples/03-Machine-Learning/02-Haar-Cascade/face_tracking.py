# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Face Tracking Example
#
# This example shows off using the keypoints feature of your OpenMV Cam to track
# a face after it has been detected by a Haar Cascade. The first part of this
# script finds a face in the image using the frontalface Haar Cascade.
# After which the script uses the keypoints feature to automatically learn your
# face and track it. Keypoints can be used to automatically track anything.
import sensor
import time
import image

# Reset sensor
sensor.reset()
sensor.set_contrast(3)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.VGA)
sensor.set_windowing((320, 240))
sensor.set_pixformat(sensor.GRAYSCALE)

# Skip a few frames to allow the sensor settle down
sensor.skip_frames(time=2000)

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
face_cascade = image.HaarCascade("frontalface", stages=25)
print(face_cascade)

# First set of keypoints
kpts1 = None

# Find a face!
while kpts1 is None:
    img = sensor.snapshot()
    img.draw_string(0, 0, "Looking for a face...")
    # Find faces
    objects = img.find_features(face_cascade, threshold=0.5, scale=1.25)
    if objects:
        # Expand the ROI by 31 pixels in every direction
        face = (
            objects[0][0] - 31,
            objects[0][1] - 31,
            objects[0][2] + 31 * 2,
            objects[0][3] + 31 * 2,
        )
        # Extract keypoints using the detect face size as the ROI
        kpts1 = img.find_keypoints(
            threshold=10, scale_factor=1.1, max_keypoints=100, roi=face
        )
        # Draw a rectangle around the first face
        img.draw_rectangle(objects[0])

# Draw keypoints
print(kpts1)
img.draw_keypoints(kpts1, size=24)
img = sensor.snapshot()
time.sleep_ms(2000)

# FPS clock
clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()
    # Extract keypoints from the whole frame
    kpts2 = img.find_keypoints(
        threshold=10, scale_factor=1.1, max_keypoints=100, normalized=True
    )

    if kpts2:
        # Match the first set of keypoints with the second one
        c = image.match_descriptor(kpts1, kpts2, threshold=85)
        match = c[6]  # C[6] contains the number of matches.
        if match > 5:
            img.draw_rectangle(c[2:6])
            img.draw_cross(c[0], c[1], size=10)
            print(kpts2, "matched:%d dt:%d" % (match, c[7]))

    # Draw FPS
    img.draw_string(0, 0, "FPS:%.2f" % (clock.fps()))
