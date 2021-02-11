#!/usr/bin/env python
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# An example TCP client for testing WiFi modules.

import time
import select
import socket

UPLOAD_LEN   =  5*1024
DOWNLOAD_LEN = 10*1024
ADDR=('192.168.1.103', 8080)

def recvall(sock, n):
    # Helper function to recv n bytes or return None if EOF is hit
    data = bytearray()
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data.extend(packet)
    return data

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(ADDR)

upload = 0
download = 0
while (True):
    s.sendall(b'0' * UPLOAD_LEN)
    buf = recvall(s, DOWNLOAD_LEN)
    upload += UPLOAD_LEN
    download += DOWNLOAD_LEN
    print("Upload: %.3f MBytes Download: %.3f MBytes" %(upload/(1024*1024), download/(1024*1024)))
s.close()
