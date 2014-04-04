import sensor, time
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.snapshot().save("0:/test.ppm", subimage=(0, 0, 10,10))
