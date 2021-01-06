# Gamma Correction
#
# This example shows off gamma correction to make the image brighter. The gamma
# correction method can also fix contrast and brightness too.

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

while(True):
    clock.tick()

    # Gamma, contrast, and brightness correction are applied to each color channel. The
    # values are scaled to the range per color channel per image type...
    img = sensor.snapshot().gamma_corr(gamma = 0.5, contrast = 1.0, brightness = 0.0)

    print(clock.fps())
