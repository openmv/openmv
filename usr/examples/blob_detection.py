import sensor, imlib, time
sensor.set_pixformat(sensor.RGB565)
clock = time.clock()
while (True):
    clock.tick()
    # take snapshot
    image = sensor.snapshot()

    # detect blobs
    imlib.threshold(image, (255, 127, 127),  40)
    imlib.median(image, 3)
    blobs = imlib.count_blobs(image)
    
    # draw rectangles around detected blobs
    image = sensor.snapshot()    
    for r in blobs:
        imlib.draw_rectangle(image, r)        
        
    print(clock.fps())
    break
