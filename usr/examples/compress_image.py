import sensor, time, led
sensor.reset()
sensor.set_contrast(2)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.RGB565)

clock = time.clock()
# Take snapshot
image = sensor.snapshot()

# Compress Image
clock.tick()
img = image.compress(30)
print(clock.avg())

with open("/test.jpeg", "w") as f:
    f.write(img)
