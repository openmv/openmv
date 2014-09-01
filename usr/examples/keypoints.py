import sensor, time
#sensor.reset()
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

image = sensor.snapshot()
kpts = image.find_keypoints(threshold=50, normalized=False)
print (kpts)
time.sleep(500)

clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    try:
        image.match_keypoints(kpts, 32)
    except:
        pass
    print (clock.avg())
