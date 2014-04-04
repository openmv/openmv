import sensor, time
sensor.set_pixformat(sensor.GRAYSCALE)
fb = sensor.snapshot()
img = Image("minion.pgm")
fb.blit((0, 0), img)
