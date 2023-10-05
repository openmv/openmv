"""
This file is part of the OpenMV project.

Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>

This work is licensed under the MIT license, see the file LICENSE for details.

GT911 5-Point Capacitive Touch Controller driver for MicroPython.

Basic polling mode example usage:

import time
from gt911 import GT911
from machine import I2C

# Note use pin numbers or names not Pin objects because the
# driver needs to change pin directions to reset the controller.
touch = GT911(I2C(1, freq=400_000), reset_pin="P1", irq_pin="P2", touch_points=5)

while True:
    n, points = touch.read_points()
    for i in range(0, n):
        print(f"id {points[i][3]} x {points[i][0]} y {points[i][1]} size {points[i][2]}")
    time.sleep_ms(100)
"""

from time import sleep_ms
from array import array
from machine import Pin

_DEFAULT_ADDR = const(0x5D)

_COMMAND = const(0x8040)
_REFRESH_RATE = const(0x8056)
_RESOLUTION_X = const(0x8048)
_RESOLUTION_Y = const(0x804A)
_TOUCH_POINTS = const(0x804C)
_MODULE_SWITCH1 = const(0x804D)
_CONFIG_CHKSUM = const(0x80FF)
_CONFIG_FRESH = const(0x8100)
_POINT_DATA_START = const(0x8150)
_DATA_BUFFER = const(0x814E)


class GT911:
    def __init__(
        self,
        bus,
        reset_pin,
        irq_pin,
        address=_DEFAULT_ADDR,
        width=800,
        height=480,
        touch_points=1,
        reverse_x=False,
        reverse_y=False,
        reverse_axis=True,
        sito=True,
        refresh_rate=240,
        touch_callback=None,
    ):
        self.bus = bus
        self.address = address
        self.touch_callback = touch_callback
        self.rst_pin = Pin(reset_pin, Pin.OUT_PP, value=0)
        self.irq_pin = None
        self.irq_pin_label = irq_pin

        # Reset the touch panel controller.
        self.reset()

        # Write and update the config.
        self._write_reg(_RESOLUTION_X, width, 2)
        self._write_reg(_RESOLUTION_Y, height, 2)
        self._write_reg(_TOUCH_POINTS, touch_points)
        self._write_reg(
            _MODULE_SWITCH1,
            (int(reverse_y) << 7)
            | (int(reverse_x) << 6)
            | (int(reverse_axis) << 3)
            | (int(sito) << 2)
            | 0x01,
        )
        self._write_reg(_REFRESH_RATE, (1000 * 1000) // (refresh_rate * 250))
        self._write_reg(_COMMAND, 0x00)
        self._update_config()

        # Allocate scratch buffer.
        self.points_data = [array("H", [0, 0, 0, 0]) for x in range(5)]

    def _read_reg(self, reg, size=1, buf=None):
        if buf is not None:
            self.bus.readfrom_mem_into(self.address, reg, buf, addrsize=16)
        else:
            return self.bus.readfrom_mem(self.address, reg, size, addrsize=16)

    def _write_reg(self, reg, val, size=1):
        buf = bytes([val & 0xFF]) if size == 1 else bytes([val & 0xFF, val >> 8])
        self.bus.writeto_mem(self.address, reg, buf, addrsize=16)

    def _update_config(self):
        # Read current config
        chksum = ~sum(self._read_reg(0x8047, 184)) + 1
        # Calculate checksum
        self._write_reg(_CONFIG_CHKSUM, chksum)
        # Update the config
        self._write_reg(_CONFIG_FRESH, 0x01)

    def read_id(self):
        return self._read_reg(0x8140, 4)

    def read_points(self):
        status = self._read_reg(_DATA_BUFFER)[0]
        n_points = status & 0x0F
        if status & 0x80:
            for i in range(0, n_points):
                self._read_reg(_POINT_DATA_START + i * 8, buf=self.points_data[i])
                # We read an extra reserved byte, shift track ID to fix it.
                self.points_data[i][-1] = self.points_data[i][-1] >> 8
            self._write_reg(_DATA_BUFFER, 0)
        return n_points, self.points_data

    def reset(self):
        if self.irq_pin is not None:
            self.irq_pin.irq(handler=None)
        self.rst_pin(0)
        sleep_ms(10)
        self.irq_pin = Pin(self.irq_pin_label, Pin.OUT_PP, value=0)
        sleep_ms(50)
        self.rst_pin(1)
        # Note must wait for at least 50ms before switching the IRQ pin to input.
        sleep_ms(100)
        self.irq_pin = Pin(self.irq_pin_label, Pin.IN, Pin.PULL_UP)
        if self.touch_callback is not None:
            self.irq_pin.irq(
                handler=self.touch_callback, trigger=Pin.IRQ_FALLING, hard=False
            )
