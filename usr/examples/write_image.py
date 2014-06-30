import sensor, time

sensor.set_pixformat(sensor.JPEG)
sensor.set_framesize(sensor.QVGA)

image = sensor.snapshot()
f = open("1:/test.jpeg", "wb")
f.write(image)
f.close()