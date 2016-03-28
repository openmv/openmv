import sensor, time
# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QCIF)
sensor.set_pixformat(sensor.GRAYSCALE)

# FPS clock
clock = time.clock()
while (True):
    clock.tick()
    # Capture snapshot
    img = sensor.snapshot()

    # Run median filter (r=3)
    img.median(size = 3)

    # Note: Actual FPS is higher, streaming the FB makes it slower.
    print(clock.fps())
