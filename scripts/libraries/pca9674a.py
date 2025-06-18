# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# PCA9674A I2C expander driver.

from time import sleep_ms
from machine import Pin
from micropython import const

_DEFAULT_ADDR = const(63)


class PCA9674A:

    def __init__(self, bus, irq_pin, address=_DEFAULT_ADDR, callback=None):
        self.bus = bus
        self.address = address
        self.callback = callback
        self.irq_pin = None
        self.irq_pin_label = irq_pin
        self.state = bytearray(1)
        self.reset()

    def write(self, value):
        self.state[0] = value
        self.bus.writeto(self.address, self.state)

    def read(self):
        self.bus.readfrom_into(self.address, self.state)
        return self.state[0]

    def reset(self):
        if self.irq_pin is not None:
            self.irq_pin.irq(handler=None)

        sleep_ms(10)
        self.bus.writeto(self.address, bytes([0xff]))
        sleep_ms(10)
        self.bus.readfrom(self.address, 1)
        sleep_ms(10)

        self.irq_pin = Pin(self.irq_pin_label, Pin.IN, Pin.PULL_UP)

        if self.callback is not None:
            self.irq_pin.irq(
                handler=self.callback, trigger=Pin.IRQ_FALLING, hard=False
            )
