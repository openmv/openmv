#!/usr/bin/env python2.7
import sys, serial, struct
port = '/dev/ttyACM0'
sp = serial.Serial(port, baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
            xonxoff=False, rtscts=False, stopbits=serial.STOPBITS_ONE, timeout=None, dsrdtr=True)
sp.setDTR(True) # dsrdtr is ignored on Windows.
sp.write("snap")
sp.flush()
size = struct.unpack('<L', sp.read(4))[0]
img = sp.read(size)
sp.close()

with open("img.jpg", "w") as f:
    f.write(img)
