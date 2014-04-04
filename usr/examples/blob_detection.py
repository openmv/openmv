import sensor, time
sensor.set_pixformat(sensor.RGB565)
clock = time.clock()
while (True):
    clock.tick()
    # take snapshot
    image = sensor.snapshot()

    # detect blobs
    image.threshold((255, 127, 127),  40)
    image.median(3)
    blobs = image.find_blobs()
    
    # draw rectangles around detected blobs
    image = sensor.snapshot()    
    for r in blobs:
        image.draw_rectangle(r)        
        
    print(clock.fps())
