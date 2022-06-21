# LSM6DSOX Gyro example.
import time
from lsm6dsox import LSM6DSOX
from machine import I2C, SPI, Pin

lsm = LSM6DSOX(SPI(5), cs_pin=Pin("PF6", Pin.OUT_PP, Pin.PULL_UP))

while (True):
    print('Accelerometer: x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(*lsm.read_accel()))
    print('Gyroscope:     x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(*lsm.read_gyro()))
    print("")
    time.sleep_ms(100)
