# Crazy Drawing Example
#
# This example shows off your OpenMV Cam's built-in drawing capabilities. This
# example was originally a test but serves as good reference code. Please put
# your IDE into non-JPEG mode to see the best drawing quality.

import pyb, sensor, image, math

sensor.reset()
sensor.set_framesize(sensor.QVGA)

while(True):

    # Test Set Pixel
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.set_pixel(x, y, 255)
    sensor.set_pixformat(sensor.RGB565)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.set_pixel(x, y, (255, 255, 255))

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

    # Test Draw Rectangle
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            w = (pyb.rng() % img.width())
            h = (pyb.rng() % img.height())
            img.draw_rectangle([x, y, w, h])
    sensor.set_pixformat(sensor.RGB565)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            w = (pyb.rng() % img.width())
            h = (pyb.rng() % img.height())
            img.draw_rectangle([x, y, w, h])

    # Test Draw Circle
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            r = (pyb.rng() % (img.width() if (img.width() > img.height()) else img.height()))
            img.draw_circle(x, y, r)
    sensor.set_pixformat(sensor.RGB565)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            r = (pyb.rng() % (img.width() if (img.width() > img.height()) else img.height()))
            img.draw_circle(x, y, r)

    # Test Draw String
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.draw_string(x, y, "Hello\nWorld!")
    sensor.set_pixformat(sensor.RGB565)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.draw_string(x, y, "Hello\nWorld!")

    # Test Draw Cross
    sensor.set_pixformat(sensor.GRAYSCALE)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.draw_cross(x, y)
    sensor.set_pixformat(sensor.RGB565)
    for i in range(10):
        img = sensor.snapshot()
        for j in range(100):
            x = (pyb.rng() % (2*img.width())) - (img.width()//2)
            y = (pyb.rng() % (2*img.height())) - (img.height()//2)
            img.draw_cross(x, y)
