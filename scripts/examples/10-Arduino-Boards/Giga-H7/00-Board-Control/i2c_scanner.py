# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# I2C scanner examples
#
from machine import I2C

i2c = I2C(1, freq=400_000)
for addr in i2c.scan():
    print("Found device at address %d:0x%x" % (bus, addr))
