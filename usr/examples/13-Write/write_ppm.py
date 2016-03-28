import sensor, pyb, time

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(16)
sensor.set_contrast(1)
sensor.set_framesize(sensor.QVGA)

# Set sensor to RGB565
sensor.set_pixformat(sensor.RGB565)

# Red LED
led = pyb.LED(1)

# Skip a few frames to allow the sensor settle down
# Note: This takes more time when exec from the IDE.
for i in range(0, 10):
    sensor.snapshot()

# Turn on red LED and wait for a second
led.on()
time.sleep(1000)

# Write image to file
img = sensor.snapshot()
img.save("/test.ppm")

led.off()
print("Reset the camera to see the saved image.")