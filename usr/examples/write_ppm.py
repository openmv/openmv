import sensor
#sensor.reset()
sensor.set_contrast(1)
sensor.set_gainceiling(8)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.RGB565)

image = sensor.snapshot()
image.save("/test.ppm")
