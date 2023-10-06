import sensor
import time
from machine import UART
from modbus import ModbusRTU

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)

uart = UART(3, 115200, parity=None, stop=2, timeout=1, timeout_char=4)
modbus = ModbusRTU(uart, register_num=9999)

sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    if modbus.any():
        modbus.handle(debug=True)
    else:
        clock.tick()
        img = sensor.snapshot()
        tags = img.find_apriltags()  # defaults to TAG36H11 without "families".
        modbus.clear()
        modbus.REGISTER[0] = len(tags)
        if tags:
            print(tags)
            i = 1
            for tag in tags:
                img.draw_rectangle(tag.rect(), color=127)
                modbus.REGISTER[i] = tag.family()
                i += 1
                modbus.REGISTER[i] = tag.id()
                i += 1
                modbus.REGISTER[i] = tag.cx()
                i += 1
                modbus.REGISTER[i] = tag.cy()
                i += 1
        # print(modbus.REGISTER[0:15])
        # print(clock.fps())
