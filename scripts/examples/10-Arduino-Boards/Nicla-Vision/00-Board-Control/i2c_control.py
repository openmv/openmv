# I2C Control
#
# This example shows how to use the i2c bus on your OpenMV Cam by dumping the
# contents on a standard EEPROM. To run this example either connect the
# Thermopile Shield to your OpenMV Cam or an I2C EEPROM to your OpenMV Cam.

from pyb import I2C

i2c = I2C(1, I2C.MASTER)
mem = i2c.mem_read(256, 0x50, 0)  # The eeprom slave address is 0x50.

print("\n[")
for i in range(16):
    print("\t[", end="")
    for j in range(16):
        print("%03d" % mem[(i * 16) + j], end="")
        if j != 15:
            print(", ", end="")
    print("]," if i != 15 else "]")
print("]")
