import time
import lsm6dsox
from machine import Pin, I2C

bus = I2C(0, scl=Pin(13), sda=Pin(12))
lsm = lsm6dsox.LSM6DSOX(bus)

while (True):
    #for g,a in lsm.iter_accel_gyro(): print(g,a)    # using fifo
    print('Accelerometer: x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(*lsm.read_accel()))
    print('Gyroscope:     x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(*lsm.read_gyro()))
    print("")
    time.sleep_ms(500)
