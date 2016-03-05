import time, sensor, image, gif

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(1)
sensor.set_saturation(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

for i in range(30):
    img = sensor.snapshot()

# FPS clock
clock = time.clock()
gif = gif.Gif("/test.gif", loop=True)

for i in range(30):
    clock.tick()
    img = sensor.snapshot()
    gif.add_frame(img, delay=10)
    # Print FPS.
    # Note: Actual FPS is higher, the IDE slows down streaming.
    print(clock.fps())

gif.close()
print("done")
