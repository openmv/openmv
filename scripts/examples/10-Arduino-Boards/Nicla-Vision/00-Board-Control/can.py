# CAN Example
#
# This example demonstrates CAN communications between two cameras.
# NOTE: you need two CAN transceiver shields and DB9 cable to run this example.

import time
import omv
from pyb import CAN

# NOTE: Set to False on receiving node.
TRANSMITTER = True

can = CAN(1, CAN.NORMAL, baudrate=125_000, sample_point=75)
# NOTE: uncomment to set bit timing manually, for example:
# can.init(CAN.NORMAL, prescaler=32, sjw=1, bs1=8, bs2=3)
can.restart()

if TRANSMITTER:
    while True:
        # Send message with id 1
        can.send("Hello", 1)
        time.sleep_ms(1000)

else:
    # Runs on the receiving node.
    if omv.board_type() == "H7":  # FDCAN
        # Set a filter to receive messages with id=1 -> 4
        # Filter index, mode (RANGE, DUAL or MASK), FIFO (0 or 1), params
        can.setfilter(0, CAN.RANGE, 0, (1, 4))
    else:
        # Set a filter to receive messages with id=1, 2, 3 and 4
        # Filter index, mode (LIST16, etc..), FIFO (0 or 1), params
        can.setfilter(0, CAN.LIST16, 0, (1, 2, 3, 4))

    while True:
        # Receive messages on FIFO 0
        print(can.recv(0, timeout=10000))
