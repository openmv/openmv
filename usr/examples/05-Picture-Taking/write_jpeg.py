import sensor, pyb, time

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(16)
sensor.set_contrast(1)
sensor.set_framesize(sensor.QVGA)

# Enable JPEG and set quality
sensor.set_pixformat(sensor.JPEG)
sensor.set_quality(98)

# Red LED
led = pyb.LED(1)

# Skip a few frames to allow the sensor settle down
# Note: This takes more time when exec from the IDE.
for i in range(0, 30):
    sensor.snapshot()

# Turn on red LED and wait for a second
led.on()
time.sleep(1000)

# Write JPEG image to file
with open("/test.jpeg", "w") as f:
    f.write(sensor.snapshot())

led.off()
print("Reset the camera to see the saved image.")