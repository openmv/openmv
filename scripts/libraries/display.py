# This file is part of the OpenMV project.
#
# Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This is an extension to the display C user-module. Add or import any display-related
# drivers here, and freeze this module in the board's manifest, and those drivers will
# be importable from display.

from udisplay import *  # noqa

try:
    from st7701 import *  # noqa
except (ImportError, AttributeError):
    pass


class DACBacklight:
    def __init__(self, channel, bits=8):
        from pyb import DAC

        self.bits = bits
        self.dac = DAC(channel, bits=bits, buffering=True)
        self.backlight(100)

    def deinit(self):
        self.dac.deinit()

    def backlight(self, value):
        self.dac.write(int(value / 100 * (2**self.bits - 1)))


class PWMBacklight:
    def __init__(self, pin, timer=3, channel=3, frequency=200):
        from machine import Pin
        from pyb import Timer

        self.timer = Timer(timer, freq=frequency)
        self.channel = self.timer.channel(channel, Timer.PWM, pin=Pin(pin))
        self.backlight(100)

    def deinit(self):
        self.timer.deinit()

    def backlight(self, value):
        self.channel.pulse_width_percent(value * 100 // 100)
