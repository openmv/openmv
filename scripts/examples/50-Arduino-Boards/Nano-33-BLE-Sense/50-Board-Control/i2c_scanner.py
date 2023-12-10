# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# I2C scanner examples
# 7-bit addresses for NANO33 BLE SENSE
# Sensors on I2C 1 bus:
# LBS22HB  0x5C
# HTS221   0x5F
# LSM9DS1  0x1E
# LSM9DS1  0x6B
# APDS9960 0x39
from machine import Pin, I2C

i2c_list = [None, None]
i2c_list[0] = I2C(0, scl=Pin(2), sda=Pin(31))
i2c_list[1] = I2C(1, scl=Pin(15), sda=Pin(14))

for bus in range(0, 2):
    print("\nScanning bus %d..." % (bus))
    for addr in i2c_list[bus].scan():
        print("Found device at address %d:0x%x" % (bus, addr))
