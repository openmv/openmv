# SPDX-License-Identifier: MIT
#
# Copyright (C) 2024 OpenMV, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import sensor,pyb,time
from machine import UART

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
# Configuration
pyb.freq(400000000)
GREEN_LED_PIN = 2
BLUE_LED_PIN = 3
uart = UART(3, 115200)
uart.init(115200, bits=8, parity=None, stop=1)
counter = 1

def capture_image(counter):
    pyb.LED(BLUE_LED_PIN).on()
    img = sensor.snapshot()
    picture = f"IMG_{counter}.jpg"
    img.save(picture)
    print(f"Captured and saved image as: {picture}")
    pyb.LED(BLUE_LED_PIN).off()
    return img, picture

while True:
    pyb.LED(GREEN_LED_PIN).on()
    if uart.any():
     command=uart.read(1)
     print(command)
     while (command == None):
        command=uart.read(1)
        if(command != None):
            print(command)
            pyb.LED(GREEN_LED_PIN).off()
            img, picture = capture_image(counter)
            break



