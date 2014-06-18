import sensor, time
sensor.set_framesize(sensor.QCIF)
sensor.set_pixformat(sensor.RGB565)
sensor.snapshot().save("1:/test.ppm")
