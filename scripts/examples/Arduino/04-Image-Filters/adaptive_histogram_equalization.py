# Adaptive Histogram Equalization
#
# This example shows off how to use adaptive histogram equalization to improve
# the contrast in the image. Adaptive histogram equalization splits the image
# into regions and then equalizes the histogram in those regions to improve
# the image contrast versus a global histogram equalization. Additionally,
# you may specify a clip limit to prevent the contrast from going wild.

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()

    # A clip_limit of < 0 gives you normal adaptive histogram equalization
    # which may result in huge amounts of contrast noise...

    # A clip_limit of 1 does nothing. For best results go slightly higher
    # than 1 like below. The higher you go the closer you get back to
    # standard adaptive histogram equalization with huge contrast swings.

    img = sensor.snapshot().histeq(adaptive=True, clip_limit=3)

    print(clock.fps())
