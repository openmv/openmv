import sensor, time

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
face_cascade = HaarCascade("frontalface")
print(face_cascade)

# FPS clock
clock = time.clock()
while (True):
    clock.tick()
    # Capture snapshot
    image = sensor.snapshot()
    # Find objects
    objects = image.find_features(face_cascade, threshold=0.65, scale=1.85)
    # Draw objects
    for r in objects:
        image.draw_rectangle(r)
        #Add delay to see drawing on FB
        time.sleep(100)

    print (clock.fps())
