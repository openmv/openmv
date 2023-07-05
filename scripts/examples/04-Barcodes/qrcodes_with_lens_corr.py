# QRCode Example
#
# This example shows the power of the OpenMV Cam to detect QR Codes
# using lens correction (see the qrcodes_with_lens_corr.py script for higher performance).

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()
    img.lens_corr(1.8)  # strength of 1.8 is good for the 2.8mm lens.
    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color=(255, 0, 0))
        print(code)
    print(clock.fps())
