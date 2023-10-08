import time
from machine import UART
from modbus import ModbusRTU

uart = UART(3, 115200, parity=None, stop=2, timeout=1, timeout_char=4)
modbus = ModbusRTU(uart, register_num=9999)

while True:
    if modbus.any():
        modbus.handle(debug=True)
    else:
        time.sleep_ms(100)
        modbus.REGISTER[0] = 1000
        modbus.REGISTER[1] += 1
        modbus.REGISTER[3] += 3
        # print(modbus.REGISTER[10:15])
        # image processing in there
