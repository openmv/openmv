# ble.py - ASM001/ASM002 BLE Driver
#
# Note: You should copy this module to your OpenMV Cam's SD card or internal
# file system.
#
# This is a driver for the ASM001/ASM002 BLE modules. It takes care of parsing
# commands for you. Please see the http://truconnect.ack.me/2.0/general_overview
# reference guide for how to use the ASM001/ASM002 BLE modules.

from time import sleep
from pyb import Pin, UART


class BLE:
    BLE_NONE = 0
    BLE_SHIELD = 1

    def command(self, cmd):
        if self.type == self.BLE_SHIELD:
            self.uart.write(cmd)
            self.uart.write("\r\n")
            r = self.uart.read(9)
            if r[0] != 82:
                raise OSError("Response corrupted!")
            if r[1] == 49:
                raise OSError("Command failed!")
            if r[1] == 50:
                raise OSError("Parse error!")
            if r[1] == 51:
                raise OSError("Unknown command!")
            if r[1] == 52:
                raise OSError("Too few args!")
            if r[1] == 53:
                raise OSError("Too many args!")
            if r[1] == 54:
                raise OSError("Unknown variable or option!")
            if r[1] == 55:
                raise OSError("Invalid argument!")
            if r[1] == 56:
                raise OSError("Timeout!")
            if r[1] == 57:
                raise OSError("Security mismatch!")
            if r[1] != 48:
                raise OSError("Response corrupted!")
            for i in range(2, 6):
                if r[i] < 48 or 57 < r[i]:
                    raise OSError("Response corrupted!")
            if r[7] != 13 or r[8] != 10:
                raise OSError("Response corrupted!")
            l = (
                ((r[2] - 48) * 10000)
                + ((r[3] - 48) * 1000)
                + ((r[4] - 48) * 100)
                + ((r[5] - 48) * 10)
                + ((r[6] - 48) * 1)
            )
            if not l:
                return None
            if l == 1 or l == 2:
                raise OSError("Response corrupted!")
            response = self.uart.read(l - 2)
            if self.uart.readchar() != 13:
                raise OSError("Response corrupted!")
            if self.uart.readchar() != 10:
                raise OSError("Response corrupted!")
            return response

    def deinit(self):
        if self.type == self.BLE_SHIELD:
            self.uart.deinit()
            self.rst = None
            self.uart = None
            self.type = self.BLE_NONE

    def init(self, type=BLE_SHIELD):
        self.deinit()
        if type == self.BLE_SHIELD:
            self.rst = Pin("P7", Pin.OUT_OD, Pin.PULL_NONE)
            self.uart = UART(3, 115200, timeout_char=1000)
            self.type = self.BLE_SHIELD
            self.rst.low()
            sleep(100)
            self.rst.high()
            sleep(100)
            self.uart.write("set sy c m machine\r\nsave\r\nreboot\r\n")
            sleep(1000)
            self.uart.readall()  # clear

    def uart(self):
        if self.type == self.BLE_SHIELD:
            return self.uart

    def type(self):
        if self.type == self.BLE_SHIELD:
            return self.BLE_SHIELD

    def __init__(self):
        self.rst = None
        self.uart = None
        self.type = self.BLE_NONE


if __name__ == "__main__":
    ble = BLE()
    ble.init()
    print(ble.command("ver"))
