#!/usr/bin/env python
# This file is part of the OpenMV project.
# Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Openmv module.

import struct
import sys,time
import serial
import pydfu
import platform
from array import array
from PIL import Image

__serial = None

__FB_HDR_SIZE   =12

# USB Debug commands
__USBDBG_CMD            = 48 
__USBDBG_FRAME_SIZE     = 0x81
__USBDBG_FRAME_DUMP     = 0x82
__USBDBG_FRAME_LOCK     = 0x83
__USBDBG_FRAME_UPDATE   = 0x04
__USBDBG_SCRIPT_EXEC    = 0x05
__USBDBG_SCRIPT_STOP    = 0x06
__USBDBG_SCRIPT_SAVE    = 0x07
__USBDBG_TEMPLATE_SAVE  = 0x08
__USBDBG_DESCRIPTOR_SAVE= 0x09
__USBDBG_ATTR_READ      = 0x8A
__USBDBG_ATTR_WRITE     = 0x0B
__USBDBG_SYS_RESET      = 0x0C
__USBDBG_SYS_BOOT       = 0x0D
__USBDBG_TX_BUF_LEN     = 0x8E
__USBDBG_TX_BUF         = 0x8F

ATTR_CONTRAST   =0
ATTR_BRIGHTNESS =1
ATTR_SATURATION =2
ATTR_GAINCEILING=3

def init(serial):
    global __serial
    __serial = serial

def _rgb(rgb):
    return struct.pack("BBB", ((rgb & 0xF800)>>11)*255/31, ((rgb & 0x07E0)>>5)*255/63, (rgb & 0x001F)*255/31)

def fb_size():
    # read fb header
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_SIZE, __FB_HDR_SIZE))
    return struct.unpack("III", __serial.read(12))

def fb_lock():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_LOCK, __FB_HDR_SIZE))
    return struct.unpack("III", __serial.read(12))

def fb_dump():
    size = fb_lock()

    if (not size[0]):
        # frame not ready
        return None

    if (size[2] > 2): #JPEG
        num_bytes = size[2]
    else:
        num_bytes = size[0]*size[1]*size[2]

    # read fb data
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_DUMP, num_bytes))
    buff = __serial.read(num_bytes)

    if size[2] == 1:  # Grayscale
        s = buff 
        buff = ''.join([y for yyy in zip(s, s, s) for y in yyy])
    elif size[2] == 2: #RGB565
        arr = array('H', buff)
        arr.byteswap()
        buff = ''.join(map(_rgb, arr))
    else: # JPEG
        try:
            buff = Image.frombuffer("RGB", (size[0], size[1]), buff, "jpeg", "RGB", "").tostring()
        except Exception as e:
            #print ("JPEG decode error (%s)"%(e))
            return None

        if (len(buff) != (size[0]*size[1]*3)):
            return None

    return (size[0], size[1], buff)

def fb_update():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_UPDATE, 0))

def exec_script(buf):
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_EXEC, len(buf)))
    __serial.write(buf)

def stop_script():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_STOP, 0))

def save_template(x, y, w, h, path):
    buf = struct.pack("IIII", x, y, w, h) + path
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TEMPLATE_SAVE, len(buf)))
    __serial.write(buf)

def save_descriptor(x, y, w, h, path):
    buf = struct.pack("IIII", x, y, w, h) + path
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_DESCRIPTOR_SAVE, len(buf)))
    __serial.write(buf)

def set_attr(attr, value):
    __serial.write(struct.pack("<BBIhh", __USBDBG_CMD, __USBDBG_ATTR_WRITE, 0, attr, value))

def get_attr(attr):
    __serial.write(struct.pack("<BBIh", __USBDBG_CMD, __USBDBG_ATTR_READ, 1, attr))
    return __serial.read(1)

def enter_dfu():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SYS_BOOT, 0))

def reset():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SYS_RESET, 0))

def tx_buf_len():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TX_BUF_LEN, 4))
    return struct.unpack("I", __serial.read(4))[0]

def tx_buf(bytes):
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TX_BUF, bytes))
    return __serial.read(bytes)

if __name__ == '__main__':
    if len(sys.argv)!= 2:
        print ('usage: openmv.py <script>')
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin:
        buf = fin.read()

    s = serial.Serial("/dev/openmvcam", 12000000, timeout=0.1)
    init(s)
    exec_script(buf)
    tx_len = tx_buf_len()
    if (tx_len):
        print(tx_buf(tx_len))
    s.close()
