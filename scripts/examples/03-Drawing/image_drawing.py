# Draw Image Example
#
# This example shows off how to draw images in the frame buffer.

import sensor, image, time, pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA) # or QQVGA...
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()

    img = sensor.snapshot()
    w = img.width()
    h = img.height()
    # Draws an image in the frame buffer. In this case we're
    # drawing the image we're currently drawing which causes
    # graphical glitches but is cool. Pass an optional mask
    # image to control what pixels are drawn.
    img.draw_image(img, w//4, h//4, x_scale=0.5, y_scale=0.5)

    print(clock.fps())
