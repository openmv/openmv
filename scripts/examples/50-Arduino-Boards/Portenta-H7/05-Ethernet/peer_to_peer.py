# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Ethernet LAN Peer to Peer example.
# On the PC try the following:
#
# $> sudo ifconfig eth0 192.168.1.100 up
# $> ping 192.168.1.102

import network
import time

lan = network.LAN()
lan.active(True)
lan.ifconfig(("192.168.1.102", "255.255.255.0", "192.168.1.1", "192.168.1.1"))

while True:
    # Nothing else to do.
    time.sleep(1.0)
