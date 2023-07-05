import time
import lsm9ds1
from machine import Pin, I2C

bus = I2C(1, scl=Pin(15), sda=Pin(14))
lsm = lsm9ds1.LSM9DS1(bus)

while True:
    # for g,a in lsm.iter_accel_gyro(): print(g,a)    # using fifo
    print("Accelerometer: x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}".format(*lsm.read_accel()))
    print("Magnetometer:  x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}".format(*lsm.read_magnet()))
    print("Gyroscope:     x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}".format(*lsm.read_gyro()))
    print("")
    time.sleep_ms(500)
