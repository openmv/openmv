# Find Rects Example
#
# This example shows off how to find rectangles in the image using the quad threshold
# detection code from our April Tags code. The quad threshold detection algorithm
# detects rectangles in an extremely robust way and is much better than Hough
# Transform based methods. For example, it can still detect rectangles even when lens
# distortion causes those rectangles to look bent. Rounded rectangles are no problem!
# (But, given this the code will also detect small radius circles too)...

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # grayscale is faster (160x120 max on OpenMV-M7)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()

    # `threshold` below should be set to a high enough value to filter out noise
    # rectangles detected in the image which have low edge magnitudes. Rectangles
    # have larger edge magnitudes the larger and more contrasty they are...

    for r in img.find_rects(threshold=10000):
        img.draw_rectangle(r.rect(), color=(255, 0, 0))
        for p in r.corners():
            img.draw_circle(p[0], p[1], 5, color=(0, 255, 0))
        print(r)

    print("FPS %f" % clock.fps())
