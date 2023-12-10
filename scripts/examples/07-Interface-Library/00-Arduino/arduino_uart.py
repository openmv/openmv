# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Basic UART communications between OpenMV and Arduino Uno.

# 1) Wire up your OpenMV Cam to your Arduino Uno like this:
#
# OpenMV Cam Ground Pin   ----> Arduino Ground
# OpenMV Cam UART3_TX(P4) ----> Arduino Uno UART_RX(0)
# OpenMV Cam UART3_RX(P5) ----> Arduino Uno UART_TX(1)

# 2) Uncomment and upload the following sketch to Arduino:
#
# void setup() {
#   // put your setup code here, to run once:
#   Serial.begin(19200);
# }
#
# void loop() {
#   // put your main code here, to run repeatedly:
#   if (Serial.available()) {
#     // Read the most recent byte
#     byte byteRead = Serial.read();
#     // ECHO the value that was read
#     Serial.write(byteRead);
#   }
# }

# 3) Run the following script in OpenMV IDE:

import time
from pyb import UART

# UART 3, and baudrate.
uart = UART(3, 19200, timeout_char=200)

while True:
    uart.write("Hello World!\n")
    if uart.any():
        print(uart.read())
    time.sleep_ms(1000)
