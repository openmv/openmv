# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This examples shows how to use the Himax Motion Detection feature
# to wake up from low-power Stop Mode on motion detection interrupts.

import csi
import pyb
import machine
from pyb import Pin, ExtInt

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize(csi.QVGA)
csi0.framerate(15)

csi0.ioctl(csi.IOCTL_HIMAX_MD_THRESHOLD, 10)
csi0.ioctl(csi.IOCTL_HIMAX_MD_WINDOW, (0, 0, 320, 240))
csi0.ioctl(csi.IOCTL_HIMAX_MD_CLEAR)
csi0.ioctl(csi.IOCTL_HIMAX_MD_ENABLE, True)


def on_motion(line):
    pass


led = pyb.LED(3)
ext = ExtInt(Pin("PC15"), ExtInt.IRQ_RISING, Pin.PULL_DOWN, on_motion)

while True:
    led.off()
    csi0.ioctl(csi.IOCTL_HIMAX_OSC_ENABLE, True)  # Switch to internal OSC
    csi0.ioctl(csi.IOCTL_HIMAX_MD_CLEAR)  # Clear MD flag
    machine.sleep()  # Enter low-power mode, will wake up on MD interrupt.
    csi0.ioctl(csi.IOCTL_HIMAX_OSC_ENABLE, False)  # Switch back to MCLK
    led.on()
    for i in range(0, 60):  # Capture a few frames
        img = csi0.snapshot()
