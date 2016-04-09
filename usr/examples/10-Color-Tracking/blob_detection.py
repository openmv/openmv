import sensor, time, pyb

led_r = pyb.LED(1)

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.RGB565)

# Finds a red blob.
COLOR1 = (  50,   55,   73,   82,   47,   63)
# Select an aera of the image and click copy color to get
# new color tracking parameters for something in the image.

clock = time.clock()
while (True):
    clock.tick()
    # Take snapshot
    image = sensor.snapshot()

    # Detect blobs in image
    blob_l = image.find_blobs([COLOR1])

    led_r.off()

    # Draw rectangles around detected blobs
    for blobs in blob_l:
        for r in blobs:
            if r[8]==1:
                led_r.on()
            image.draw_rectangle(r[0:4])
    print(clock.fps())
