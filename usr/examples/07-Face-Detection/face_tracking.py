# Face Tracking Example
#
# This example shows off using the keypoints feature of your OpenMV Cam to track
# a face after it has been detected by a Haar Cascade. The first part of this
# script finds a face in the image using the frontalface Haar Cascade.
# After which the script uses the keypoints feature to automatically learn your
# face and track it. Keypoints can be used to automatically track anything.
#
# NOTE: LOTS OF KEYPOINTS MAY CAUSE THE SYSTEM TO RUN OUT OF MEMORY!

import sensor, time, image

# Normalized keypoints are not rotation invariant...
NORMALIZED=False
# Keypoint extractor threshold, range from 0 to any number.
# This threshold is used when extracting keypoints, the lower
# the threshold the higher the number of keypoints extracted.
KEYPOINTS_THRESH=32
# Keypoint-level threshold, range from 0 to 100.
# This threshold is used when matching two keypoint descriptors, it's the
# percentage of the distance between two descriptors to the max distance.
# In other words, the minimum matching percentage between 2 keypoints.
MATCHING_THRESH=80

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Skip a few frames to allow the sensor settle down
# Note: This takes more time when exec from the IDE.
for i in range(0, 10):
    img = sensor.snapshot()
    img.draw_string(0, 0, "Please wait...")

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
face_cascade = image.HaarCascade("frontalface", stages=25)
print(face_cascade)

# First set of keypoints
kpts1 = None

# Find a face!
while (kpts1 == None):
    img = sensor.snapshot()
    img.draw_string(0, 0, "Looking for a face...")
    # Find faces
    objects = img.find_features(face_cascade, threshold=0.5, scale=1.5)
    if objects:
        # Expand the ROI by 11 pixels in each direction (half the pattern scale)
        face = (objects[0][0]-22, objects[0][1]-22,objects[0][2]+22*2, objects[0][3]+22*2)
        # Extract keypoints using the detect face size as the ROI
        kpts1 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED, roi=face)
        # Draw a rectangle around the first face
        img.draw_rectangle(objects[0])

# Draw keypoints
print(kpts1)
img.draw_keypoints(kpts1, size=12)
time.sleep(1000)

# FPS clock
clock = time.clock()

while (True):
    clock.tick()
    img = sensor.snapshot()
    # Extract keypoints using the detect face size as the ROI
    kpts2 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED)

    if (kpts2):
        # Match the first set of keypoints with the second one
        c=image.match_descriptor(image.FREAK, kpts1, kpts2, threshold=MATCHING_THRESH)
        # If more than 10% of the keypoints match draw the matching set
        if (c[2]>25):
            img.draw_cross(c[0], c[1], size=5)
            img.draw_string(0, 10, "Match %d%%"%(c[2]))

    # Draw FPS
    img.draw_string(0, 0, "FPS:%.2f"%(clock.fps()))
