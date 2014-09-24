import sensor, time
KEYPOINTS_THRESH=60
MATCHING_THRESH=70

#sensor.reset()
sensor.set_contrast(1)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

image = sensor.snapshot()
kpts1 = image.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=False)
print (kpts1)
time.sleep(1000)

clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    try:
        kpts2 = image.find_keypoints(threshold=KEYPOINTS_THRESH, normalized=False)
        c=image.match_keypoints(kpts1, kpts2, MATCHING_THRESH)
        if (c):
            l=10
            image.draw_line((c[0]-l,  c[1],  c[0]+l, c[1]))
            image.draw_line((c[0],  c[1]-l,  c[0], c[1]+l))
            time.sleep(10)
    except:
        pass
    print (clock.fps())
