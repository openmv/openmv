#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# A simple script to visualize GC

from PIL import Image
import sys, os,time, serial

DEFAULT_PORT = "/dev/ttyACM0"
DEFAULT_BAUDRATE = 115200
RAW_CHR = serial.to_bytes([0x0A])   # CTRL+A
RST_CHR = serial.to_bytes([0x04])   # CTRL+D

ser = serial.Serial(
    port=DEFAULT_PORT,\
    baudrate=DEFAULT_BAUDRATE,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
    timeout=3.0)

#ser.write(RST_CHR)      # soft reset
#ser.write(RAW_CHR)      # raw REPL
ser.write("\r\nimport pyb\r\n")
ser.write("\r\npyb.info(1)\r\n")

time.sleep(0.1)
data = ''
while (ser.inWaiting()):
    data += ser.readline()

data = data[data.index('0000:') : ]
heap = [l[6:] for l in data.split('\r\n')[0:-1]]

for l in heap:
    print l

SCALE_X=6
SCALE_Y=6
LINE_WIDTH=64

w = LINE_WIDTH*SCALE_X
h = len(heap) *SCALE_Y

#padding/last line
line =heap[-1]
if (len(line)<LINE_WIDTH):
    heap[-1] = line + '*'*(LINE_WIDTH-len(line))

im = Image.new('RGB', (w, h))

for y in range(0, h):
    line = heap[y/SCALE_Y]
    for x in range(0, w):
        if line[x/SCALE_X]=='*':
            col = (0, 0, 0)
        elif line[x/SCALE_X]=='.':
            col = (0, 255, 0)
        else:
            col = (255, 0, 0)
        im.putpixel((x, y), col)

im.show()
