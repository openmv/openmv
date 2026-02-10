# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# LCD Shield Example
#
# Note: To run this example you will need a LCD Shield for your OpenMV Cam.
#
# The LCD shield allows you to view your OpenMV Cam's frame buffer on the go.

import csi
import time
import display
import image

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.LCD)

# Initialize the lcd screen.
# Note: A DAC or a PWM backlight controller can be used to control the
# backlight intensity if supported:
#  lcd = display.SPIDisplay(backlight=display.DACBacklight(channel=2))
#  lcd.backlight(25) # 25% intensity
# Otherwise the default GPIO (on/off) controller is used.
#  OpenMV Cam M4/M7/H7/H7 Plus -> DAC and GPIO Support
#  OpenMV Cam RT1062 -> GPIO Support
#  OpenMV Cam N6 -> PWM and GPIO Support
lcd = display.SPIDisplay(vflip=True, hmirror=True)
clock = time.clock()

while True:
    clock.tick()
    lcd.write(csi0.snapshot(), hint=image.CENTER | image.SCALE_ASPECT_KEEP)
    print(clock.fps())
