import sensor, time
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

image = sensor.snapshot()
kp =  image.find_keypoints(upright=False, thresh=0.004, octaves=2)
image.draw_keypoints(kp)
time.sleep(2000)

clock = time.clock()
while (True):
    image = sensor.snapshot()
    clock.tick()
    n = image.find_keypoints_match(kp)
    print (n)
    #print (clock.avg())