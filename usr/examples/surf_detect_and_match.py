import sensor, time
# Set sensor gainceiling
sensor.set_gainceiling(16)
# Set sensor brightness
sensor.set_brightness(-2)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

image = sensor.snapshot()
kp =  image.find_keypoints(upright=False, thresh=0.0004, octaves=2)
image.draw_keypoints(kp)
time.sleep(2000)

clock = time.clock()
while (True):
    image = sensor.snapshot()
    clock.tick()
    ipts2 = image.find_keypoints_match(kp)
    print (clock.avg())