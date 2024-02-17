# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# CAN Shield Example
#
# This example demonstrates CAN communications between two cameras.
# NOTE: you need two CAN transceiver shields and DB9 cable to run this example.

import time
from machine import CAN

# NOTE: Set to False on receiving node.
TRANSMITTER = True

can = CAN(0, CAN.NORMAL, baudrate=1000000, auto_restart=True)

if TRANSMITTER:
    while True:
        # Send message with id 1
        can.send("Hello", 1, timeout=100, extframe=False)
        time.sleep_ms(1000)

else:
    # Runs on the receiving node.
    # Set a filter to receive messages with id=1 and 2
    # Filter index, mode (DUAL, etc..), FIFO (0), params
    can.setfilter(0, CAN.DUAL, 0, [1, 2])

    while True:
        # Receive messages on FIFO 0 (there's only one fifo)
        print(can.recv(0, timeout=10000))
