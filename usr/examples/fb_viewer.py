import sensor, time, led

# Set sensor contrast
sensor.set_contrast(1)
# Set sensor brightness
sensor.set_brightness(-1)
# Set sensor pixel format
sensor.set_pixformat(sensor.RGB565)

while (True):
  image = sensor.snapshot()
