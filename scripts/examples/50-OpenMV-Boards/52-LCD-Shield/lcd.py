# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# LCD Example
#
# Note: To run this example you will need a LCD Shield for your OpenMV Cam.
#
# The LCD Shield allows you to view your OpenMV Cam's frame buffer on the go.

import sensor
import display

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565)  # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA2)  # Special 128x160 framesize for LCD Shield.
# Initialize the lcd screen.
# Note: A DAC or a PWM backlight controller can be used to control the
# backlight intensity if supported:
#  lcd = display.SPIDisplay(backlight=display.DACBacklight(channel=2))
#  lcd.backlight(25) # 25% intensity
# Otherwise the default GPIO (on/off) controller is used.
lcd = display.SPIDisplay()

while True:
    lcd.write(sensor.snapshot())  # Take a picture and display the image.
