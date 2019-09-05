#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# An example sockets client.

import time
import select
import socket
ADDR=('192.168.1.101', 8000)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(ADDR)
time.sleep(0.500)
s.send("HelloWorld")
time.sleep(0.500)
print (s.recv(10))
time.sleep(3)
print ("closing")
s.close()
