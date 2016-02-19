import sensor, time

KEYPOINTS_THRESH=32
MATCHING_THRESH=70
NORMALIZED=False

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

kpts1 = None
while (kpts1 == None):
    img = sensor.snapshot()
    kpts1 = img.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=NORMALIZED)

print (kpts1)
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
        time.sleep(10)
    print (clock.fps())
