import sensor, time
sensor.set_pixformat(sensor.GRAYSCALE)
fb = sensor.snapshot()
img = Image("minion.pgm")
fb.blit(img, (0, 0))
