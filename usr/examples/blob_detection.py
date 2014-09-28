import sensor, time, led
#sensor.reset()
sensor.set_contrast(2)
sensor.set_framesize(sensor.QCIF)
sensor.set_pixformat(sensor.RGB565)

clock = time.clock()
while (True):
    clock.tick()
    # Take snapshot
    image = sensor.snapshot()

    # Threshold image with RGB
    binary  = image.threshold([(255, 0, 0),
                               (0, 255,  0),
                               (0, 0, 255)], 80)

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
