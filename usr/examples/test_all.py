import sensor, time
# Set sensor gainceiling
sensor.set_gainceiling(16)
# Set sensor brightness
sensor.set_brightness(-2)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

def test_surf(sensor):
    clock = time.clock()
    for x in range(100):
        image = sensor.snapshot()
        clock.tick()
        kp =  image.find_keypoints(upright=False, thresh=0.0004, octaves=2)
        image.draw_keypoints(kp)
        print (clock.avg())

def test_haar(sensor):
    # Load Haar Cascade
    cascade = HaarCascade("0:/frontalface_default.cascade")
    print(cascade)
    clock = time.clock()
    for x in range(100):
        clock.tick()
        image = sensor.snapshot()
        objects = image.find_features(cascade)
        for r in objects:
            image.draw_rectangle(r)
        print (clock.fps())

while (True):
    test_surf(sensor)
    test_haar(sensor)
