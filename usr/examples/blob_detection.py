import sensor, time, pyb

led_r = pyb.LED(1)
led_g = pyb.LED(2)
led_b = pyb.LED(3)

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

    led_r.off()
    led_g.off()
    led_b.off()

    # Draw rectangles around detected blobs
    for r in blobs:
        if r[5]==1:
            led_r.on()
        if r[5]==2:
            led_g.on()
        if r[5]==3:
            led_b.on()
        image.draw_rectangle(r[0:4])
        time.sleep(50)

    print(clock.fps())
