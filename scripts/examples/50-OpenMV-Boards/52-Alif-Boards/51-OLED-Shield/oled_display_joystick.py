# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OLED Display with Joystick Example
#
# Note: To run this example you will need a OLED Breakout Board for your OpenMV AE3
#
# The OLED Breakout Board allows you to view your OpenMV AE3's frame buffer on the go.

import csi
import time
import display
from pca9674a import PCA9674A
from machine import I2C

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

lcd = display.SPIDisplay(width=128, height=128, controller=display.SSD1351())
clock = time.clock()


def read_expander(pin):
    global exp, state
    state = exp.read() ^ 0xFF


x_scale_def = 128 / 400
y_scale_def = 128 / 400
state = 0
cursor_x = 0
cursor_y = 0
exp = PCA9674A(I2C(1), irq_pin="P9", callback=read_expander)


def update_cursor():
    global cursor_x, cursor_y
    if state & 0x01:  # Right
        cursor_x += 2
    if state & 0x02:  # Up
        cursor_y -= 2
    if state & 0x04:  # Left
        cursor_x -= 2
    if state & 0x08:  # Down
        cursor_y += 2
    if state & 0x10:  # Center
        cursor_x = 0
        cursor_y = 0


while True:
    clock.tick()
    update_cursor()
    lcd.write(csi0.snapshot(), x=cursor_x, y=cursor_y,
              x_scale=x_scale_def, y_scale=y_scale_def)

    print(clock.fps())
