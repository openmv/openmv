import sensor, time, image

NORMALIZED=False
MATCHING_THRESH=70
KEYPOINTS_THRESH=32

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.HQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Skip a few frames to allow the sensor settle down
# Note: This takes more time when exec from the IDE.
for i in range(0, 30):
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
        # Draw a rectangle around the first face
        img.draw_rectangle(objects[0])
        # Extract keypoints using the detect face size as the ROI
        kpts1 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED, roi=objects[0])
        if kpts1:
            img.draw_keypoints(kpts1)
            time.sleep(1000)

# FPS clock
clock = time.clock()

while (True):
    clock.tick()
    img = sensor.snapshot()

    # Extract keypoints using the whole image.
    kpts2 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED)

    # Match the first set of keypoints with the second one
    if (kpts2):
        c=img.match_keypoints(kpts1, kpts2, MATCHING_THRESH)

    # If a match was found, draw the matching keypoints
    if (c):
        img.draw_cross(c[0], c[1], size=5)
        img.draw_string(0, 0, "Tracking face...")
    print (clock.fps())
