import sensor, imlib, time
clock = time.clock()
while (True):
    clock.tick()

    # take snapshot
    image = sensor.snapshot()

    print (clock.fps())
