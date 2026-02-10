# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OLED Display Example
#
# Note: To run this example you will need a OLED Breakout Board for your OpenMV AE3
#
# The OLED Breakout Board allows you to view your OpenMV AE3's frame buffer on the go.

import csi
import time
import display
import image

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

lcd = display.SPIDisplay(width=128, height=128, controller=display.SSD1351())
clock = time.clock()

while True:
    clock.tick()
    lcd.write(csi0.snapshot(), hint=image.CENTER | image.SCALE_ASPECT_KEEP)
    print(clock.fps())
