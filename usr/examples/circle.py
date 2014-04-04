import sensor

# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_brightness(-2)

image = sensor.snapshot()
image.draw_circle((50, 50), 10)