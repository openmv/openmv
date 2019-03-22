import struct
import pyb

CONFIG_MODE = 0x00
ACCONLY_MODE = 0x01
MAGONLY_MODE = 0x02
GYRONLY_MODE = 0x03
ACCMAG_MODE = 0x04
ACCGYRO_MODE = 0x05
MAGGYRO_MODE = 0x06
AMG_MODE = 0x07
IMUPLUS_MODE = 0x08
COMPASS_MODE = 0x09
M4G_MODE = 0x0a
NDOF_FMC_OFF_MODE = 0x0b
NDOF_MODE = 0x0c

AXIS_P0 = bytes([0x21, 0x04])
AXIS_P1 = bytes([0x24, 0x00])
AXIS_P2 = bytes([0x24, 0x06])
AXIS_P3 = bytes([0x21, 0x02])
AXIS_P4 = bytes([0x24, 0x03])
AXIS_P5 = bytes([0x21, 0x01])
AXIS_P6 = bytes([0x21, 0x07])
AXIS_P7 = bytes([0x24, 0x05])

_MODE_REGISTER = 0x3d
_POWER_REGISTER = 0x3e
_AXIS_MAP_CONFIG = 0x41

class BNO055:
    def __init__(self, i2c, address=0x28, mode = NDOF_MODE, axis = AXIS_P4):
        self.i2c = i2c
        self.address = address
        if self.read_id() != bytes([0xA0, 0xFB, 0x32, 0x0F]):
            raise RuntimeError('Failed to find expected ID register values. Check wiring!')
        self.operation_mode(CONFIG_MODE)
        self.system_trigger(0x20)# reset
        pyb.delay(700)
        self.power_mode(0x00)#POWER_NORMAL
        self.axis(axis)
        self.page(0)
        pyb.delay(10)
        self.operation_mode(mode)
        self.system_trigger(0x80) # external oscillator
        pyb.delay(200)

    def read_registers(self, register, size=1):
        return(self.i2c.readfrom_mem(self.address, register, size))
    def write_registers(self, register, data):
        self.i2c.writeto_mem(self.address, register, data)
    def operation_mode(self, mode=None):
        if mode:
            self.write_registers(_MODE_REGISTER, bytes([mode]))
        else:
            return(self.read_registers(_MODE_REGISTER, 1)[0])
    def system_trigger(self, data):
        self.write_registers(0x3f, bytes([data]))
    def power_mode(self, mode=None):
        if mode:
            self.write_registers(_POWER_REGISTER, bytes([mode]))
        else:
            return(self.read_registers(_POWER_REGISTER, 1))
    def page(self, num=None):
        if num:
            self.write_registers(0x3f, bytes([num]))
        else:
            self.read_registers(0x3f)
    def temperature(self):
        return(self.read_registers(0x34, 1)[0])
    def read_id(self):
        return(self.read_registers(0x00, 4))
    def axis(self, placement=None):
        if placement:
            self.write_registers(_AXIS_MAP_CONFIG, placement)
        else:
            return(self.read_registers(_AXIS_MAP_CONFIG, 2))
    def quaternion(self):
        data = struct.unpack("<hhhh", self.read_registers(0x20, 8))
        return [d/(1<<14) for d in data] #[w, x, y, z]
    def euler(self):
        data = struct.unpack("<hhh", self.read_registers(0x1A, 6))
        return [d/16 for d in data] # [yaw, roll, pitch]
    def accelerometer(self):
        data = struct.unpack("<hhh", self.read_registers(0x08, 6))
        return [d/100 for d in data] #[x, y, z]
    def magnetometer(self):
        data = struct.unpack("<hhh", self.read_registers(0x0E, 6))
        return [d/16 for d in data] # [x, y, z]
    def gyroscope(self):
        data = struct.unpack("<hhh", self.read_registers(0x14, 6))
        return [d/900 for d in data] #[x, y, z]
    def linear_acceleration(self):
        data = struct.unpack("<hhh", self.read_registers(0x28, 6))
        return [d/100 for d in data] #[x, y, z]
    def gravity(self):
        data = struct.unpack("<hhh", self.read_registers(0x2e, 6))
        return [d/100 for d in data] #[x, y, z]
