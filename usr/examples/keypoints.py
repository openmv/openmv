import sensor, time

NORMALIZED=False
MATCHING_THRESH=70
KEYPOINTS_THRESH=32

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
while (kpts1 == None):
    img = sensor.snapshot()
    kpts1 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED)

img.draw_keypoints(kpts1)
time.sleep(1000)

clock = time.clock()
while (True):
    clock.tick()
    img = sensor.snapshot()
    kpts2 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED)
    if not kpts2:
        continue

    c=img.match_keypoints(kpts1, kpts2, MATCHING_THRESH)
    if (c):
        img.draw_cross(c[0],  c[1], size = 10)
    print (clock.fps())
