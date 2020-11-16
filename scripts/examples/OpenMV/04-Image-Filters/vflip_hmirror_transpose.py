# Vertical Flip - Horizontal Mirror - Transpose
#
# This example shows off how to vertically flip, horizontally mirror, or
# transpose an image. Note that:
#
# vflip=False, hmirror=False, transpose=False -> 0 degree rotation
# vflip=True,  hmirror=False, transpose=True  -> 90 degree rotation
# vflip=True,  hmirror=True,  transpose=False -> 180 degree rotation
# vflip=False, hmirror=True,  transpose=True  -> 270 degree rotation

import sensor, image, time, pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

mills = pyb.millis()
counter = 0

while(True):
    clock.tick()

    img = sensor.snapshot().replace(vflip=(counter//2)%2,
                                    hmirror=(counter//4)%2,
                                    transpose=(counter//8)%2)

    if (pyb.millis() > (mills + 1000)):
        mills = pyb.millis()
        counter += 1

    print(clock.fps())
