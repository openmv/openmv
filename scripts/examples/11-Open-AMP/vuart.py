# This work is licensed under the MIT license.
# Copyright (c) 2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example demonstrates the most basic use of Open-AMP to communicate between two cores
# on dual-core micro-controllers. To run this example, firmware for the secondary core is
# required. For testing purposes, demo firmware for the secondary core is provided here:
# https://github.com/iabdalkader/openamp_vuart
# To configure and build the firmware, please follow the instructions in the README file.
#
# Note that on most micro-controllers it's not possible to reset the secondary core without
# a full reset, so running and then stopping this script may result in a full reset of the
# board, which is completely normal.

import openamp
import time


def ept_recv_callback(src_addr, data):
    print("Received message: ", data.decode())


# Create a new RPMsg endpoint to communicate with the M4.
ept = openamp.Endpoint("vuart-channel", callback=ept_recv_callback)

# Create a remoteproc object, load its firmware and start it. Note that the remote core
# can also boot from an entry point in flash (such as 0x081E0000) if a firmware is there.
# rproc = openamp.RProc(0x08180000)
rproc = openamp.RemoteProc("vuart.elf")
rproc.start()

count = 0
while True:
    if ept.is_ready():
        ept.send("Hello World %d!" % count, timeout=1000)
        count += 1
    time.sleep_ms(1000)
