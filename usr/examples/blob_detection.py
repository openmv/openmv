import sensor, imlib, time
clock = time.clock()
while (True):
    # take snapshot
    image = sensor.snapshot()

    # detect blobs
    imlib.threshold(image, (255, 127, 127),  48)
    clock.tick()
    imlib.median(image, 3)
    blobs = imlib.count_blobs(image)
    image = sensor.snapshot()
    for r in blobs:
        imlib.draw_rectangle(image, r)
    break
