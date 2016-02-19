import sensor
from math import sin, cos

sensor.reset()
# Set sensor settings
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(8)
sensor.set_contrast(2)

# Set sensor pixel format
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Skip a few frames to allow the sensor settle down
for i in range(0, 3):
    image = sensor.snapshot()

x = int(320/2)
y = int(240/2)
image.draw_circle(x, y, 50)
image.draw_line((x, y, int(50*sin(45))+x, int(50*cos(45))+y))
image.draw_rectangle((x-60, y-60, 120, 120))
image.draw_string(10, 10, "OpenMV", 0xFF)
image.draw_string(10, 25, "Hello World", 0xFF)
# Flush buffer...
sensor.snapshot()
