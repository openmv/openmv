# LPS22HB/HH pressure seneor micropython drive
# ver: 2.0
# License: MIT
# Author: shaoziyang (shaoziyang@micropython.org.cn)
# v1.0 2016.4
# v2.0 2019.7


class LPS22H:
    LPS22_CTRL_REG1 = const(0x10)
    LPS22_CTRL_REG2 = const(0x11)
    LPS22_STATUS = const(0x27)
    LPS22_TEMP_OUT_L = const(0x2B)
    LPS22_PRESS_OUT_XL = const(0x28)
    LPS22_PRESS_OUT_L = const(0x29)

    def __init__(self, i2c, addr=0x5C):
        self.i2c = i2c
        self.addr = addr
        self.tb = bytearray(1)
        self.rb = bytearray(1)
        self.oneshot = False
        self.irq_v = [0, 0]
        # ODR=1 EN_LPFP=1 BDU=1
        self.setreg(LPS22_CTRL_REG1, 0x1A)
        self.oneshot_mode(False)

    def oneshot_mode(self, oneshot=None):
        if oneshot is None:
            return self.oneshot
        else:
            self.getreg(LPS22_CTRL_REG1)
            self.oneshot = oneshot
            if oneshot:
                self.rb[0] &= 0x0F
            else:
                self.rb[0] |= 0x10
            self.setreg(LPS22_CTRL_REG1, self.rb[0])

    def int16(self, d):
        return d if d < 0x8000 else d - 0x10000

    def setreg(self, reg, dat):
        self.tb[0] = dat
        self.i2c.writeto_mem(self.addr, reg, self.tb)

    def getreg(self, reg):
        self.i2c.readfrom_mem_into(self.addr, reg, self.rb)
        return self.rb[0]

    def get2reg(self, reg):
        return self.getreg(reg) + self.getreg(reg + 1) * 256

    def ONE_SHOT(self, b):
        if self.oneshot:
            self.setreg(LPS22_CTRL_REG2, self.getreg(LPS22_CTRL_REG2) | 0x01)
            self.getreg(0x28 + b * 2)
            while 1:
                if self.getreg(LPS22_STATUS) & b:
                    return

    def temperature(self):
        self.ONE_SHOT(2)
        try:
            return self.int16(self.get2reg(LPS22_TEMP_OUT_L)) / 100
        except MemoryError:
            return self.temperature_irq()

    def pressure(self):
        self.ONE_SHOT(1)
        try:
            return (self.getreg(LPS22_PRESS_OUT_XL) + self.get2reg(LPS22_PRESS_OUT_L) * 256) / 4096
        except MemoryError:
            return self.pressure_irq()

    def altitude(self):
        return (
            (((1013.25 / self.pressure()) ** (1 / 5.257)) - 1.0)
            * (self.temperature() + 273.15)
            / 0.0065
        )

    def temperature_irq(self):
        self.ONE_SHOT(2)
        return self.int16(self.get2reg(LPS22_TEMP_OUT_L)) // 100

    def pressure_irq(self):
        self.ONE_SHOT(1)
        return self.get2reg(LPS22_PRESS_OUT_L) >> 4

    def get_irq(self):
        self.irq_v[0] = self.temperature_irq()
        self.irq_v[1] = self.pressure_irq()
        return self.irq_v
