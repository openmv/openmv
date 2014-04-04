import sensor, time
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.snapshot().median(size=3)
