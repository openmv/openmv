# I2C scanner examples
#
# 7-bit addresses for NANO RP2040 on I2C0 bus:
#
# ATECC608A  0x60
# LSM6DSOX   0x6A

from machine import Pin
from machine import I2C

i2c_list = [None, None]
i2c_list[0] = I2C(0, scl=Pin(13), sda=Pin(12), freq=100_000)
i2c_list[1] = I2C(1, scl=Pin(7), sda=Pin(6), freq=100_000)

for bus in range(0, 2):
    print("\nScanning bus %d..." % (bus))
    for addr in i2c_list[bus].scan():
        print("Found device at addres %d:0x%x" % (bus, addr))
