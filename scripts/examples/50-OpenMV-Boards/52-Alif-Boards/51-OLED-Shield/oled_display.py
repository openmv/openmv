# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OLED Display Example
#
# Note: To run this example you will need a OLED Breakout Board for your OpenMV AE3
#
# The OLED Breakout Board allows you to view your OpenMV AE3's frame buffer on the go.

import sensor
import time
import display
import image

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.VGA)
sensor.set_windowing((400, 400))

lcd = display.SPIDisplay(width=128, height=128, controller=display.SSD1351())
clock = time.clock()

while True:
    clock.tick()
    lcd.write(sensor.snapshot(), hint=image.CENTER | image.SCALE_ASPECT_KEEP)
    print(clock.fps())
