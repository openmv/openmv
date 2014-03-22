import sensor, imlib, time
clock = time.clock()
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_brightness(-2)
image = sensor.snapshot()
surf1 =  imlib.surf_detector(image, False, 0.0004)
imlib.surf_draw_ipts(image, surf1)
time.sleep(2000)
while (True):
    image = sensor.snapshot()
    clock.tick()
    surf2 = imlib.surf_detector(image, False, 0.0004)
    imlib.surf_match(image, surf1,  surf2)
    print (clock.avg())