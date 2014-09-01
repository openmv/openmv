import sensor, time, led
#sensor.reset()
sensor.set_contrast(2)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.RGB565)

clock = time.clock()
while (True):
    clock.tick()
    # Take snapshot
    image = sensor.snapshot()

    # Threshold image with RGB
    binary  = image.threshold([(80, 100, 0),
                               (80, -80,  30),
                               (60, 0, -100)], 65)

    # Image closing
    binary.dilate(3)
    binary.erode(3)

    # Detect blobs in image
    blobs = binary.find_blobs()

    led.off(led.RED)
    led.off(led.GREEN)
    led.off(led.BLUE)

    # Draw rectangles around detected blobs
    for r in blobs:
        if r[5]==1:
            led.on(led.RED)
        if r[5]==2:
            led.on(led.GREEN)
        if r[5]==3:
            led.on(led.BLUE)
        image.draw_rectangle(r[0:4])

    print(clock.fps())
