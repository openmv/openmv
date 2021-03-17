# Flood Fill
#
# This example shows off flood filling areas in the image.

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA) # or QQVGA...
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()

    # seed_threshold controls the maximum allowed difference between
    # the initial pixel and any filled pixels. It's important to
    # set this such that flood fill doesn't fill the whole image.

    # floating_threshold controls the maximum allowed difference
    # between any two pixels. This can easily fill the whole image
    # with even a very low threshold.

    # flood_fill will fill pixels that both thresholds.

    # You can invert what gets filled with "invert" and clear
    # everything but the filled area with "clear_background".

    x = sensor.width() // 2
    y = sensor.height() // 2
    img = sensor.snapshot().flood_fill(x, y, \
        seed_threshold=0.05, floating_thresholds=0.05, \
        color=(255, 0, 0), invert=False, clear_background=False)

    print(clock.fps())
