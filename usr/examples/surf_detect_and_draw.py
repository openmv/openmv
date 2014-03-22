import sensor, imlib, time
clock = time.clock()
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

while (True):
    image = sensor.snapshot()
    clock.tick()    
    surf1 =  imlib.surf_detector(image, False, 0.0008)
    print (clock.avg())
    imlib.surf_draw_ipts(image, surf1)