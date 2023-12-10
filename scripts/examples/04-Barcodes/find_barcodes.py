# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Barcode Example
#
# This example shows off how easy it is to detect bar codes using the
# OpenMV Cam M7. Barcode detection does not work on the M4 Camera.

import sensor
import image
import time
import math

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.VGA)  # High Res!
sensor.set_windowing((640, 80))  # V Res of 80 == less work (40 for 2X the speed).
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
sensor.set_auto_whitebal(False)  # must turn this off to prevent image washout...
clock = time.clock()

# Barcode detection can run at the full 640x480 resolution of your OpenMV Cam's
# OV7725 camera module. Barcode detection will also work in RGB565 mode but at
# a lower resolution. That said, barcode detection requires a higher resolution
# to work well so it should always be run at 640x480 in grayscale...


def barcode_name(code):
    if code.type() == image.EAN2:
        return "EAN2"
    if code.type() == image.EAN5:
        return "EAN5"
    if code.type() == image.EAN8:
        return "EAN8"
    if code.type() == image.UPCE:
        return "UPCE"
    if code.type() == image.ISBN10:
        return "ISBN10"
    if code.type() == image.UPCA:
        return "UPCA"
    if code.type() == image.EAN13:
        return "EAN13"
    if code.type() == image.ISBN13:
        return "ISBN13"
    if code.type() == image.I25:
        return "I25"
    if code.type() == image.DATABAR:
        return "DATABAR"
    if code.type() == image.DATABAR_EXP:
        return "DATABAR_EXP"
    if code.type() == image.CODABAR:
        return "CODABAR"
    if code.type() == image.CODE39:
        return "CODE39"
    if code.type() == image.PDF417:
        return "PDF417"
    if code.type() == image.CODE93:
        return "CODE93"
    if code.type() == image.CODE128:
        return "CODE128"


while True:
    clock.tick()
    img = sensor.snapshot()
    codes = img.find_barcodes()
    for code in codes:
        img.draw_rectangle(code.rect())
        print_args = (
            barcode_name(code),
            code.payload(),
            (180 * code.rotation()) / math.pi,
            code.quality(),
            clock.fps(),
        )
        print(
            'Barcode %s, Payload "%s", rotation %f (degrees), quality %d, FPS %f'
            % print_args
        )
    if not codes:
        print("FPS %f" % clock.fps())
