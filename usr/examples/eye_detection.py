import sensor, time
#sensor.reset()
# Set framesize
sensor.set_framesize(sensor.QCIF)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)
# Set sensor contrast
sensor.set_contrast(1)
# Set sensor gainceiling
sensor.set_gainceiling(16)

face_cascade = HaarCascade("/frontalface.cascade")

while (True):
    image = sensor.snapshot()
    objects = image.find_features(face_cascade, threshold=0.45, scale=1.25)

    if (len(objects)==0):
        continue

    roi = objects[0]
    image.draw_rectangle(roi)

    image.histeq()
    eyes = image.find_eyes(roi)

    l = 5
    c = (eyes[0], eyes[1])
    image.draw_line((c[0]-l,  c[1],  c[0]+l, c[1]))
    image.draw_line((c[0],  c[1]-l,  c[0], c[1]+l))

    c = (eyes[2], eyes[3])
    image.draw_line((c[0]-l,  c[1],  c[0]+l, c[1]))
    image.draw_line((c[0],  c[1]-l,  c[0], c[1]+l))

    time.sleep(500)
