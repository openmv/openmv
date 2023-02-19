# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# HTS221 driver based on public domain driver.

import time
import struct


class HTS221:
    def __init__(self, i2c, data_rate=1, dev_addr=0x5F):
        self.bus = i2c
        self.odr = data_rate
        self.slv_addr = dev_addr

        # Set configuration register
        # Humidity and temperature average configuration
        self.bus.writeto_mem(self.slv_addr, 0x10, b"\x1B")

        # Set control register
        # PD | BDU | ODR
        cfg = 0x80 | 0x04 | (self.odr & 0x3)
        self.bus.writeto_mem(self.slv_addr, 0x20, bytes([cfg]))

        # TODO needed ?
        time.sleep_ms(100)

        # Read Calibration values from non-volatile memory of the device
        # Humidity Calibration values
        self.H0 = self.read_reg(0x30, 1) / 2
        self.H1 = self.read_reg(0x31, 1) / 2
        self.H2 = self.read_reg(0x36, 2)
        self.H3 = self.read_reg(0x3A, 2)

        # Temperature Calibration values
        raw = self.read_reg(0x35, 1)
        self.T0 = ((raw & 0x03) * 256) + self.read_reg(0x32, 1)
        self.T1 = ((raw & 0x0C) * 64) + self.read_reg(0x33, 1)
        self.T2 = self.read_reg(0x3C, 2)
        self.T3 = self.read_reg(0x3E, 2)

    def read_reg(self, reg_addr, size):
        fmt = "B" if size == 1 else "H"
        reg_addr = reg_addr if size == 1 else reg_addr | 0x80
        return struct.unpack(fmt, self.bus.readfrom_mem(self.slv_addr, reg_addr, size))[0]

    def humidity(self):
        rH = self.read_reg(0x28, 2)
        return (self.H1 - self.H0) * (rH - self.H2) / (self.H3 - self.H2) + self.H0

    def temperature(self):
        temp = self.read_reg(0x2A, 2)
        if temp > 32767:
            temp -= 65536
        return ((self.T1 - self.T0) / 8.0) * (temp - self.T2) / (self.T3 - self.T2) + (
            self.T0 / 8.0
        )
