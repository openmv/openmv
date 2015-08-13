import sensor, time

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(16)
sensor.set_contrast(1)

# Set sensor to QQVGA/RGB565
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.RGB565)

# Skip a few frames to allow the sensor settle down
# Note: This takes more time when exec from the IDE.
for i in range(0, 10):
    sensor.snapshot()

clock = time.clock()
# Take snapshot
img = sensor.snapshot()

# Compress Image
clock.tick()
img = img.compress(30)
print(clock.avg())

with open("/test.jpeg", "w") as f:
    f.write(img)
