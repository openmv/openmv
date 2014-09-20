import sensor, time
sensor.set_framesize(sensor.QCIF)
sensor.set_pixformat(sensor.GRAYSCALE)
clock = time.clock()
while (True):
    image = sensor.snapshot()
    clock.tick()
    image.median(size = 3)
    print(clock.fps())
