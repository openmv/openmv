import sensor
sensor.set_contrast(1)
sensor.set_gainceiling(8)

sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.JPEG)
sensor.set_quality (98)

with open("/test.jpeg", "w") as f:
    f.write(sensor.snapshot())

