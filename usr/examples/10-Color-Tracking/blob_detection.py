import sensor, time, pyb

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
    blobs = image.find_blobs([COLOR1])

    # Draw rectangles around detected blobs
    for blob in blobs:
        image.draw_rectangle(blob[0:4])

    print(clock.fps())
