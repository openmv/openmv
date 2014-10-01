import sensor, time
sensor.reset()
# Set sensor brightness
sensor.set_contrast(1)
# Set sensor gainceiling
sensor.set_gainceiling(16)
# Set framesize
sensor.set_framesize(sensor.QCIF)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
face_cascade = HaarCascade("/frontalface.cascade")
print(face_cascade)
clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    objects = image.find_features(face_cascade, threshold=0.65, scale=1.65)
    for r in objects:
        image.draw_rectangle(r)
    #Add delay to see drawing on FB        
    time.sleep(10)
    print (clock.fps())
