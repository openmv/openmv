# This file is part of the OpenMV project.
#
# Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This is an extension to the machine module. Add any machine-related
# modules here, and freeze this module in the board's manifest, and those
# modules will be importable from machine.

from umachine import *
import os


class LED:
    def __init__(self, pin_name):
        self.pin = Pin(pin_name, Pin.OUT)
        board = self.boardname()
        if board in []:
            # All boards have inverted LEDs, add ones that don't here.
            self.inverted = False
        else:
            self.inverted = True

    def boardname(self):
        # Currently the most portable way to get a board name.
        return os.uname().machine.split("with")[0]

    def on(self):
        self.pin(not self.inverted)

    def off(self):
        self.pin(self.inverted)

    def toggle(self):
        self.pin(not self.pin())

    def value(self, v=None):
        if v is None:
            return self.pin()
        self.pin(v ^ self.inverted)
