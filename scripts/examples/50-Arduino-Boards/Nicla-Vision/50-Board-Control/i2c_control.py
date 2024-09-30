# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# I2C Example.
from pyb import I2C


i2c = I2C(2, I2C.MASTER)
if 0x29 not in i2c.scan():
    raise RuntimeError("Failed to detect ToF")

# Read ToF Model ID.
mid = i2c.mem_read(1, 0x29, 0x010f, addr_size=16)
# Should print 0xEA
print(f"ToF Model ID: 0x{mid[0]:02X}")
