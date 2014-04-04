import sensor, time
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

clock = time.clock()
while (True):
    image = sensor.snapshot()
    clock.tick()
    kp =  image.find_keypoints(upright=False, thresh=0.0004, octaves=2)
    image.draw_keypoints(kp)
    print (clock.avg())
