"""
LSM6DSOX STMicro driver for MicroPython based on LSM9DS1:
Source repo: https://github.com/hoihu/projects/tree/master/raspi-hat

The MIT License (MIT)

Copyright (c) 2013, 2014 Damien P. George

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Example usage:
import time
from lsm6dsox import LSM6DSOX

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
    CTRL8_XL     = const(0x17)

    CTRL2_G      = const(0x11)
    CTRL7_G      = const(0x16)

    OUTX_L_G     = const(0x22)
    OUTX_L_XL    = const(0x28)

    def __init__(self, i2c, address=DEFAULT_ADDR):
        self.i2c = i2c
        self.address = address

        # check the id of the Accelerometer/Gyro
        if (self.__read_reg(WHO_AM_I_REG, 1) != b'l'):
            raise OSError("No LSM6DS device was found at address 0x%x"%(self.address))

        # allocate scratch buffer for efficient conversions and memread op's
        self.scratch_int = array.array('h',[0, 0, 0])
        self.init_gyro_accel()

    def __read_reg(self, reg, size):
        return self.i2c.readfrom_mem(self.address, reg, size)

    def __write_reg(self, reg, val):
        self.i2c.writeto_mem(self.address, reg, bytes([val]))

    def init_gyro_accel(self, fs_gyro=104, fs_accel=104, scale_gyro=2000, scale_accel=4):
        """ Initalizes Gyro and Accelerator.
        fs_accel: (0, 1.6Hz, 3.33Hz, 6.66Hz, 12.5Hz, 26Hz, 52Hz, 104Hz, 208Hz, 416Hz, 888Hz)
        fs_gyro:  (0, 1.6Hz, 3.33Hz, 6.66Hz, 12.5Hz, 26Hz, 52Hz, 104Hz, 208Hz, 416Hz, 888Hz)
        scale_gyro:  (245dps, 500dps, 1000dps, 2000dps)
        scale_accel: (+/-2g, +/-4g, +/-8g, +-16g)
        """
        # XL_HM_MODE = 0 by default.
        # G_HM_MODE = 0 by default.
        ODR = {
            0:    0x00,
            1.6:  0x08,
            3.33: 0x09,
            6.66: 0x0A,
            12.5: 0x01,
            26:   0x02,
            52:   0x03,
            104:  0x04,
            208:  0x05,
            416:  0x06,
            888:  0x07
        }

        SCALE_GYRO  =  {250:0, 500:1, 1000:2, 2000:3}
        SCALE_ACCEL =  {2:0, 4:2, 8:3, 16:1}

        fs_gyro  = round(fs_gyro, 2)
        fs_accel = round(fs_accel, 2)
        assert fs_gyro in ODR, "Invalid sampling rate: %d" % fs_accel
        assert scale_gyro  in SCALE_GYRO, "invalid gyro scaling: %d" % scale_gyro

        assert fs_accel in ODR, "Invalid sampling rate: %d" % fs_accel
        assert scale_accel in SCALE_ACCEL, "invalid accelerometer scaling: %d" % scale_accel

        # Set Gyroscope datarate and scale.
        # Note output from LPF2 second filtering stage is selected. See Figure 18.
        self.__write_reg(CTRL1_XL, (ODR[fs_accel] << 4) | (SCALE_ACCEL[scale_accel] << 2) | 2);
        # Enable LPF2 and HPF fast-settling mode, ODR/4
        self.__write_reg(CTRL8_XL, 0x09);

        # Set Gyroscope datarate and scale.
        self.__write_reg(CTRL2_G, (ODR[fs_gyro] << 4) | (SCALE_GYRO[scale_gyro] << 2) | 0);

        self.scale_gyro = 32768 / scale_gyro
        self.scale_accel = 32768 / scale_accel

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
