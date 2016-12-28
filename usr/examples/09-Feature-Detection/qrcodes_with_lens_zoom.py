# QRCode Example
#
# This example shows the power of the OpenMV Cam to detect QR Codes.
#
# On the new M7 OpenMV Cam you can detect QR codes at up to 320x240 in Grayscale or RGB565.
# We may be able to enable 640x480 operation in the future if we re-write the QR code library's front end code.
#
# On the M4 OpenMV Cam QR code detection should be done strictly at a maximum of 160x120 for Grayscale or RGB565.
#
# Lastly, reading QRCodes requires lens correction to improve the detection rate.
# Additionally, histogram equalization could be used to increase the contrast but is not required.

import sensor, image, time

# For the new M7 OpenMV Cam...
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.VGA)
sensor.set_windowing((640//2 - 200//2, 480//2 - 200//2, 200, 200))
sensor.skip_frames(10)
sensor.set_gain_ctrl(False)
clock = time.clock()

while(True):
    clock.tick()
    img = sensor.snapshot()
    for code in img.find_qrcodes():
        print(code)
    print(clock.fps())
