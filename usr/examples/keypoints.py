import sensor, time
#sensor.reset()
sensor.set_framesize(sensor.QQVGA)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

image = sensor.snapshot()
kpts = image.find_keypoints(threshold=30, normalized=False)
print (kpts)
time.sleep(500)

clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    try:
        image.match_keypoints(kpts, 64)
    except:
        pass
    print (clock.fps())
