"""
LSM6DSOX STMicro driver for MicroPython.
Base on: LSM9DS1 driver and https://github.com/arduino-libraries/Arduino_LSM6DSOX

Example usage:
import time
import lsm6dsox

from machine import Pin, I2C
lsm = LSM6DSOX(I2C(0, scl=Pin(13), sda=Pin(12)))

while (True):
    print('Accelerometer: x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(*lsm.read_accel()))
    print('Gyroscope:     x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(*lsm.read_gyro()))
    print("")
    time.sleep_ms(100)
"""
import array

class LSM6DSOX:
    DEFAULT_ADDR = const(0x6A)
    
    WHO_AM_I_REG = const(0x0F)
    CTRL1_XL     = const(0x10)
    CTRL2_G      = const(0x11)
    
    STATUS_REG   = const(0x1E)
    
    CTRL6_C      = const(0x15)
    CTRL7_G      = const(0x16)
    CTRL8_XL     = const(0x17)
    
    OUTX_L_G     = const(0x22)
    OUTX_H_G     = const(0x23)
    OUTY_L_G     = const(0x24)
    OUTY_H_G     = const(0x25)
    OUTZ_L_G     = const(0x26)
    OUTZ_H_G     = const(0x27)
    
    OUTX_L_XL    = const(0x28)
    OUTX_H_XL    = const(0x29)
    OUTY_L_XL    = const(0x2A)
    OUTY_H_XL    = const(0x2B)
    OUTZ_L_XL    = const(0x2C)
    OUTZ_H_XL    = const(0x2D)

    def __init__(self, i2c, address=DEFAULT_ADDR):
        self.i2c = i2c
        self.address = address

        # check the id of the Accelerometer/Gyro
        if (self.__read_reg(WHO_AM_I_REG, 1) != b'l'):
            raise OSError("No LSM6DS device was found at address 0x%x"%(self.address))

        # Set the gyroscope control register to work at 104 Hz, 2000 dps and in bypass mode
        self.__write_reg(CTRL2_G, 0x4C);

        # Set the Accelerometer control register to work at 104 Hz, 4g, and in bypass mode and enable ODR/4
        # low pass filter (check figure9 of LSM6DSOX's datasheet)
        self.__write_reg(CTRL1_XL, 0x4A);

        # set gyroscope power mode to high performance and bandwidth to 16 MHz
        self.__write_reg(CTRL7_G, 0x00);

        # Set the ODR config register to ODR/4
        self.__write_reg(CTRL8_XL, 0x09);

        self.scale_gyro  = 32768 / 2000
        self.scale_accel = 32768 / 4
        self.scratch_int = array.array('h',[0, 0, 0])

    def __read_reg(self, reg, size):
        return self.i2c.readfrom_mem(self.address, reg, size)

    def __write_reg(self, reg):
        self.i2c.writeto_mem(self.address, reg)

    def read_gyro(self):
        """Returns gyroscope vector in degrees/sec."""
        mv = memoryview(self.scratch_int)
        f = self.scale_gyro
        self.i2c.readfrom_mem_into(self.address, OUTX_L_G, mv)
        return (mv[0]/f, mv[1]/f, mv[2]/f)

    def read_accel(self):
        """Returns acceleration vector in gravity units (9.81m/s^2)."""
        mv = memoryview(self.scratch_int)
        f = self.scale_accel
        self.i2c.readfrom_mem_into(self.address, OUTX_L_XL, mv)
        return (mv[0]/f, mv[1]/f, mv[2]/f)
