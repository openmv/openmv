import sensor, time
sensor.set_pixformat(sensor.RGB565)
clock = time.clock()
while (True):
    clock.tick()
    # take snapshot
    image = sensor.snapshot()
    #get a binary image
    binary  = image.threshold((255, 127, 127),  25)
    # run median filter
    binary.median(3)
    # detect blobs in image
    blobs = binary.find_blobs()
    # draw rectangles around detected blobs
    for r in blobs:
        image.draw_rectangle(r)
    print(clock.fps())
