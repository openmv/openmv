import time
from pyb import UART
from modbus import ModbusRTU
uart = UART(3)
modbus = ModbusRTU(uart)

while(True):
    if modbus.any():
        modbus.handle()
    else:
        time.sleep(100)
        # image processing in there
