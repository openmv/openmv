# Welcome to the OpenMV IDE.
# Click on the gears button to run this script!
import time, sensor

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(1)
sensor.set_saturation(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.RGB565)

# FPS clock
clock = time.clock()

while (True):
	clock.tick()
  	img = sensor.snapshot()
    # Print FPS.
    # Note: Actual FPS is higher, the IDE slows down streaming.
  	print(clock.fps())
