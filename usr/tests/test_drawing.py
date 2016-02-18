import pyb, sensor, image
sensor.reset()
sensor.set_framesize(sensor.QVGA)
while(True):
    # Test Draw Line
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x0 = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y0 = (pyb.rng() % (2*img.height())) - (img.height()//2)
            x1 = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y1 = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.draw_line([x0, y0, x1, y1])
    sensor.set_pixformat(sensor.RGB565)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x0 = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y0 = (pyb.rng() % (2*img.height())) - (img.height()//2)
            x1 = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y1 = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.draw_line([x0, y0, x1, y1])
