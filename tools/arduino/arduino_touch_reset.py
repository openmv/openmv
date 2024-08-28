#!/usr/bin/env python
import sys, serial, struct
port = "/dev/ttyACM0"
serial.Serial(port, baudrate=1200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE).close()
