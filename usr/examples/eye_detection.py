import sensor, time, image

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QCIF)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
face_cascade = image.HaarCascade("frontalface", stages=16)
print(face_cascade)

# FPS clock
clock = time.clock()
while (True):
    clock.tick()

    # Capture snapshot
    img = sensor.snapshot()

    # Find objects.
    # Note: Lower scale factor scales-down the img more and detects smaller objects.
    # Higher threshold results in a higher detection rate, with more false positives.
    objects = img.find_features(face_cascade, threshold=0.65, scale=1.65)

    # Draw objects
    for roi in objects:
        #img.histeq()
        eyes = img.find_eyes(roi)
        img.draw_cross(eyes[0], eyes[1])
        img.draw_cross(eyes[2], eyes[3])
        img.draw_rectangle(roi)

    if (len(objects)):
        # Add a small delay to see the drawing on the FB
        time.sleep(100)

    # Print FPS.
    # Note: Actual FPS is higher, streaming the FB makes it slower.
    print(clock.fps())
