from openmv import sensor, imlib
while (True):
  image = sensor.snapshot()
  r= imlib.detect_color(image, (340, 50, 50), 10)
  imlib.draw_rectangle(image, r)

