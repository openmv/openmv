# Histogram of Oriented Gradients (HoG) Example
#
# This example demonstrates HoG visualization.
#
# Note: Due to JPEG artifacts, the HoG visualization looks blurry. To see the
# image without JPEG artifacts, uncomment the lines that save the image to uSD.

import sensor, image, time

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.skip_frames(time = 2000)

clock = time.clock() # Tracks FPS.
while (True):
    clock.tick()
    img = sensor.snapshot()
    img.find_hog()

    # Uncomment to save raw FB to file and exit the loop
    #img.save("/hog.pgm")
    #break

    print(clock.fps())
