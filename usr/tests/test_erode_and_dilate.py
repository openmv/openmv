import pyb, sensor, image, math
sensor.reset()
sensor.set_framesize(sensor.QVGA)
grayscale_thres = (170, 255)
rgb565_thres = (70, 100, -128, 127, -128, 127)
while(True):
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(100):
        img = sensor.snapshot()
        img.binary([grayscale_thres])
        img.erode(2)
    for i in range(100):
        img = sensor.snapshot()
        img.binary([grayscale_thres])
        img.dilate(2)
    sensor.set_pixformat(sensor.RGB565)
    for i in range(100):
        img = sensor.snapshot()
        img.binary([rgb565_thres])
        img.erode(2)
    for i in range(100):
        img = sensor.snapshot()
        img.binary([rgb565_thres])
        img.dilate(2)
