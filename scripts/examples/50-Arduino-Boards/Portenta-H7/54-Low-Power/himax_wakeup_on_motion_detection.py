# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This examples shows how to use the Himax Motion Detection feature
# to wake up from low-power Stop Mode on motion detection interrupts.

import sensor
import pyb
import machine
from pyb import Pin, ExtInt

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.set_framerate(15)

sensor.ioctl(sensor.IOCTL_HIMAX_MD_THRESHOLD, 10)
sensor.ioctl(sensor.IOCTL_HIMAX_MD_WINDOW, (0, 0, 320, 240))
sensor.ioctl(sensor.IOCTL_HIMAX_MD_CLEAR)
sensor.ioctl(sensor.IOCTL_HIMAX_MD_ENABLE, True)


def on_motion(line):
    pass


led = pyb.LED(3)
ext = ExtInt(Pin("PC15"), ExtInt.IRQ_RISING, Pin.PULL_DOWN, on_motion)

while True:
    led.off()
    sensor.ioctl(sensor.IOCTL_HIMAX_OSC_ENABLE, True)  # Switch to internal OSC
    sensor.ioctl(sensor.IOCTL_HIMAX_MD_CLEAR)  # Clear MD flag
    machine.sleep()  # Enter low-power mode, will wake up on MD interrupt.
    sensor.ioctl(sensor.IOCTL_HIMAX_OSC_ENABLE, False)  # Switch back to MCLK
    led.on()
    for i in range(0, 60):  # Capture a few frames
        img = sensor.snapshot()
