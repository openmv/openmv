# Motion Detection Example:
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take picture.

import os, pyb, sensor, image, time

if not "temp" in os.listdir(): os.mkdir("temp") # Make a temp directory

sensor.reset()
sensor.set_framesize(sensor.QVGA)

while(True):
    sensor.set_pixformat(sensor.GRAYSCALE) # Grayscale is much faster than RGB.

    # Warm up the cam
    for i in range(10):
        sensor.snapshot()

    for i in [5, 4, 3, 2, 1]:
        print("Saving background in... %d" % i)
        time.sleep(1000)

    print("Saving background...")
    sensor.snapshot().save("temp/bg.bmp")

    diff = 30 # wait 30 snapshot before taking picture
    while(diff):
        img = sensor.snapshot()
        img.difference("temp/bg.bmp")
        img.binary([(32, 255)])
        sum, x, y = img.centroid()
        if sum > 100: # 100 pixels detected
            img.draw_cross(x, y, color = 127)
            diff -= 1

    sensor.set_pixformat(sensor.RGB565)
    # Warm up the cam
    for i in range(10):
        sensor.snapshot()
    sensor.snapshot().save("temp/movement-%d" % pyb.rng()) # Save movement
