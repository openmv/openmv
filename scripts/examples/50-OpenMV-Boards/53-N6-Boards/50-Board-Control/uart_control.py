# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# UART Control
#
# This example shows how to use the serial port on your OpenMV Cam. Attach pin
# P4 to the serial input of a serial LCD screen to see "Hello World!" printed
# on the serial LCD display.

import time
from machine import UART

# UART 3 has TX on P4 and RX on P5 on the OpenMV N6.
# The second argument is the UART baud rate.
uart = UART(3, 19200, timeout_char=200)

while True:
    uart.write("Hello World!\r")
    time.sleep_ms(1000)
