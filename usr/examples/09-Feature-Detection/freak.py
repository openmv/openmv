# Freak Example
#
# This script shows off keypoint tracking by itself. Put an object in front of
# your OpenMV Cam without anything else in the image (i.e. camera should be
# facing a smooth wall) and the camera will learn the keypoints for an track
# whatever object is in the image. You can save keypoints to disk either via
# the OpenMV IDE or from in your script.
#
# Matching keypoints works by first extracting keypoints from an ROI. Once those
# are extracted then the OpenMV Cam compares the extracted keypoints against all
# the keypoints in an image. It tries to find the center matching point between
# the two sets of keypoints.
#
# Keep in mind that keypoint matching with just one training example isn't very
# robust. If you want professional quality results then stick with getting
# professionally generated Haar Cascades like the frontalface or eye cascade.
# That said, if you're in a very controlled enviroment then keypoint tracking
# allows your OpenMV Cam to learn objects on the fly.
#
# If... you want really good keypoint matching results we suggest you gather
# keypoints from all faces of an object and with multiple rotations and scales.
# Comparing against all theses sets of keypoints helps versus just one.
#
# NOTE: LOTS OF KEYPOINTS MAY CAUSE THE SYSTEM TO RUN OUT OF MEMORY!

import sensor, time, image

# Normalized keypoints are not rotation invariant...
NORMALIZED=False
# Keypoint extractor threshold, range from 0 to any number.
# This threshold is used when extracting keypoints, the lower
# the threshold the higher the number of keypoints extracted.
KEYPOINTS_THRESH=30
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
for i in range(0, 30):
    img = sensor.snapshot()
    img.draw_string(0, 0, "Please wait...")

kpts1 = None
# Uncomment to load keypoints from file
#kpts1 = image.load_descriptor(image.FREAK, "/desc.freak")
clock = time.clock()

while (True):
    clock.tick()
    img = sensor.snapshot()
    kpts2 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED)

    if (kpts1==None):
        kpts1 = kpts2
        print(kpts1)
    elif kpts2:
        c = image.match_descriptor(image.FREAK, kpts1, kpts2, threshold=MATCHING_THRESH)
        # C[3] contains the percentage of matching keypoints.
        # If more than 25% of the keypoints match, draw stuff.
        if (c[2]>25):
            img.draw_cross(c[0], c[1], size=15)
            img.draw_string(0, 10, "Match %d%%"%(c[2]))

    # Draw FPS
    img.draw_string(0, 0, "FPS:%.2f"%(clock.fps()))
