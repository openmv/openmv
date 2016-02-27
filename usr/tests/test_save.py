import pyb, sensor, image, os, time
sensor.reset()
sensor.set_framesize(sensor.QVGA)
if not "test" in os.listdir(): os.mkdir("test")
while(True):
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(2):
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d.bmp" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d.pgm" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d.bmp" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d.pgm" % num)
        #
    sensor.set_pixformat(sensor.RGB565)
    for i in range(2):
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d.bmp" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d.ppm" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d.bmp" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d.ppm" % num)
        #
    sensor.set_pixformat(sensor.JPEG)
    for i in range(2):
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d.jpg" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("test/image-%d.jpeg" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d.jpg" % num)
        #
        img = sensor.snapshot()
        num = pyb.rng()
        print("Saving %d" % num)
        img.save("/test/image-%d.jpeg" % num)
        #
    print("Sleeping 5...")
    time.sleep(1000)
    print("Sleeping 4...")
    time.sleep(1000)
    print("Sleeping 3...")
    time.sleep(1000)
    print("Sleeping 2...")
    time.sleep(1000)
    print("Sleeping 1...")
    time.sleep(1000)
