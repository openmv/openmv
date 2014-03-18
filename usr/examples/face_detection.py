import sensor, imlib, time
# Set sensor gainceiling
sensor.set_gainceiling(sensor.X8)
# Set sensor brightness
sensor.set_brightness(-2)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
cascade = imlib.load_cascade("0:/frontalface_default.cascade")
print(cascade)
clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    objects = imlib.detect_objects(image, cascade)
    for r in objects:
        imlib.draw_rectangle(image, r)
    print (clock.fps())
