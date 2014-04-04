import sensor, time
# Set sensor gainceiling
sensor.set_gainceiling(16)
# Set sensor brightness
sensor.set_brightness(-2)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
cascade = HaarCascade("0:/frontalface_default.cascade")
print(cascade)
clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    objects = image.find_features(cascade)
    for r in objects:
        image.draw_rectangle(r)
    print (clock.fps())
