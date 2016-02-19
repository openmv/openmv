import sensor, time, image

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
face_cascade = image.HaarCascade("frontalface", stages=16)
print(face_cascade)

def find_face():
    for i in range(0, 100):
        img = sensor.snapshot()
    while (True):
        img = sensor.snapshot()
        objects = img.find_features(face_cascade, threshold=0.65, scale=1.65)
        if objects:
            print (objects[0])
            img.draw_rectangle(objects[0])
            try:
                kpts1 = img.find_keypoints(threshold=32, normalized=False, roi=objects[0])
            except:
                continue
            if kpts1:
                img.draw_keypoints(kpts1)
                time.sleep(1000)
                return kpts1

kpts1 = find_face()

clock = time.clock()
while (True):
    clock.tick()
    img = sensor.snapshot()
    try:
        kpts2 = img.find_keypoints(threshold=32, normalized=False)
    except:
        continue

    if (kpts2==None):
        continue

    c=img.match_keypoints(kpts1, kpts2, 70)
    if (c):
        l=10
        img.draw_cross(c[0], c[1])
        time.sleep(10)
    print (clock.fps())
