import sensor, time
#sensor.reset()
# Set framesize
sensor.set_framesize(sensor.QQVGA)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)
# Set sensor contrast
sensor.set_contrast(1)
# Set sensor gainceiling
sensor.set_gainceiling(16)

def find_face():
    global sensor, time
    # Load Haar Cascade
    face_cascade = HaarCascade("/frontalface.cascade")
    while (True):
        image = sensor.snapshot()
        objects = image.find_features(face_cascade, threshold=0.65, scale=1.85)
        if objects:
            print (objects[0])
            image.draw_rectangle(objects[0])
            try:
                kpts1 = image.find_keypoints(threshold=32, normalized=False, roi=objects[0])
            except:
                continue
            if kpts1:
                image.draw_keypoints(kpts1)
                time.sleep(1000)
                return kpts1

kpts1 = find_face()

clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    try:
        kpts2 = image.find_keypoints(threshold=32)
    except:
        continue
    if (kpts2==None):
        continue
    c=image.match_keypoints(kpts1, kpts2, 70)
    if (c):
        l=10
        image.draw_line((c[0]-l,  c[1],  c[0]+l, c[1]))
        image.draw_line((c[0],  c[1]-l,  c[0], c[1]+l))
        time.sleep(10)
    print (clock.fps())
