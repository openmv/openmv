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

# OpenMV Cam boards have one CAN controller, CAN(2), on P2 (TX) and P3 (RX).
can = CAN(2, 125_000)

if TRANSMITTER:
    while True:
        # Send message with id 1
        can.send(1, "Hello")
        time.sleep_ms(1000)

else:
    # Runs on the receiving node.
    # Set filters to receive messages with id=1 and id=2 only.
    # Each filter is (identifier, bit mask, flags).
    can.set_filters([(1, 0x7FF, 0), (2, 0x7FF, 0)])

    while True:
        # recv() returns None if no message is pending.
        msg = can.recv()
        if msg:
            can_id, data, flags, errors = msg
            print(can_id, bytes(data), flags, errors)
        else:
            time.sleep_ms(1)
