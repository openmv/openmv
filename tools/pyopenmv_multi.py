#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Openmv module with support for multiple cams.

import struct
import sys,time
import serial
import platform
import numpy as np
from PIL import Image

__serial = []
__port = []

__FB_HDR_SIZE   =12

# USB Debug commands
__USBDBG_CMD            = 48
__USBDBG_FW_VERSION     = 0x80
__USBDBG_FRAME_SIZE     = 0x81
__USBDBG_FRAME_DUMP     = 0x82
__USBDBG_ARCH_STR       = 0x83
__USBDBG_SCRIPT_EXEC    = 0x05
__USBDBG_SCRIPT_STOP    = 0x06
__USBDBG_SCRIPT_SAVE    = 0x07
__USBDBG_SCRIPT_RUNNING = 0x87
__USBDBG_TEMPLATE_SAVE  = 0x08
__USBDBG_DESCRIPTOR_SAVE= 0x09
__USBDBG_ATTR_READ      = 0x8A
__USBDBG_ATTR_WRITE     = 0x0B
__USBDBG_SYS_RESET      = 0x0C
__USBDBG_FB_ENABLE      = 0x0D
__USBDBG_TX_BUF_LEN     = 0x8E
__USBDBG_TX_BUF         = 0x8F

ATTR_CONTRAST   =0
ATTR_BRIGHTNESS =1
ATTR_SATURATION =2
ATTR_GAINCEILING=3

__BOOTLDR_START         = 0xABCD0001
__BOOTLDR_RESET         = 0xABCD0002
__BOOTLDR_ERASE         = 0xABCD0004
__BOOTLDR_WRITE         = 0xABCD0008

def init(port, baudrate=921600, timeout=0.3):
    global __serial
    global __port
    # open CDC port
    __serial.append(serial.Serial(port, baudrate=baudrate, timeout=timeout))
    __port.append(port)

def disconnect(port):
    global __serial
    global __port

    try:
        idx = __port.index(port)
        __serial[idx].close()
        __serial.pop(idx)
        __port.pop(idx)
    except:
        pass

def set_timeout(port, timeout):
    try:
        idx = __port.index(port)
        __serial[idx].timeout = timeout
    except:
        pass

def fb_size(port):

    # read fb header
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_SIZE, __FB_HDR_SIZE))
        return struct.unpack("III", __serial[idx].read(12))
    except:
        return None

def fb_dump(port):
    try:
        idx = __port.index(port)
        size = fb_size(port)

        if (not size[0]):
            # frame not ready
            return None

        if (size[2] > 2): #JPEG
            num_bytes = size[2]
        else:
            num_bytes = size[0]*size[1]*size[2]

        # read fb data
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_DUMP, num_bytes))
        buff = __serial[idx].read(num_bytes)

        if size[2] == 1:  # Grayscale
            y = np.fromstring(buff, dtype=np.uint8)
            buff = np.column_stack((y, y, y))
        elif size[2] == 2: # RGB565
            arr = np.fromstring(buff, dtype=np.uint16).newbyteorder('S')
            r = (((arr & 0xF800) >>11)*255.0/31.0).astype(np.uint8)
            g = (((arr & 0x07E0) >>5) *255.0/63.0).astype(np.uint8)
            b = (((arr & 0x001F) >>0) *255.0/31.0).astype(np.uint8)
            buff = np.column_stack((r,g,b))
        else: # JPEG
            try:
                buff = np.asarray(Image.frombuffer("RGB", size[0:2], buff, "jpeg", "RGB", ""))
            except Exception as e:
                print ("JPEG decode error (%s)"%(e))
                return None

        if (buff.size != (size[0]*size[1]*3)):
            return None

        return (size[0], size[1], buff.reshape((size[1], size[0], 3)))
    except:
        return None

def exec_script(port, buf):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_EXEC, len(buf)))
        __serial[idx].write(buf.encode())
    except:
        pass

def stop_script(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_STOP, 0))
    except:
        pass

def script_running(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_RUNNING, 4))
        return struct.unpack("I", __serial[idx].read(4))[0]
    except:
        return None

def save_template(port, x, y, w, h, path):
    try:
        idx = __port.index(port)
        buf = struct.pack("IIII", x, y, w, h) + path
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TEMPLATE_SAVE, len(buf)))
        __serial[idx].write(buf)
    except:
        pass

def save_descriptor(port, x, y, w, h, path):
    try:
        idx = __port.index(port)
        buf = struct.pack("HHHH", x, y, w, h) + path
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_DESCRIPTOR_SAVE, len(buf)))
        __serial[idx].write(buf)
    except:
        pass

def set_attr(port, attr, value):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_ATTR_WRITE, 8))
        __serial[idx].write(struct.pack("<II", attr, value))
    except:
        pass

def get_attr(port, attr):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBIh", __USBDBG_CMD, __USBDBG_ATTR_READ, 1, attr))
        return __serial[idx].read(1)
    except:
        return None


def reset(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SYS_RESET, 0))
    except:
        pass

def bootloader_start(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<I", __BOOTLDR_START))
        return struct.unpack("I", __serial[idx].read(4))[0] == __BOOTLDR_START
    except:
        pass

def bootloader_reset(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<I", __BOOTLDR_RESET))
    except:
        pass

def flash_erase(port, sector):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<II", __BOOTLDR_ERASE, sector))
    except:
        pass

def flash_write(port, buf):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<I", __BOOTLDR_WRITE) + buf)
    except:
        pass

def tx_buf_len(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TX_BUF_LEN, 4))
        return struct.unpack("I", __serial[idx].read(4))[0]
    except:
        return None

def tx_buf(port, bytes):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TX_BUF, bytes))
        return __serial[idx].read(bytes)
    except:
        return None

def fw_version(port):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FW_VERSION, 12))
        return struct.unpack("III", __serial[idx].read(12))
    except:
        return None

def enable_fb(port, enable):
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FB_ENABLE, 4))
        __serial[idx].write(struct.pack("<I", enable))
    except:
        pass

def arch_str():
    try:
        idx = __port.index(port)
        __serial[idx].write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_ARCH_STR, 64))
        return __serial[idx].read(64).split('\0', 1)[0]
    except:
        return None

if __name__ == '__main__':
    if len(sys.argv)!= 3:
        print ('usage: pyopenmv.py <port> <script>')
        sys.exit(1)

    with open(sys.argv[2], 'r') as fin:
        buf = fin.read()

    portname = sys.argv[1]

    disconnect(portname)
    init(portname)
    stop_script(portname)
    exec_script(portname, buf)
    tx_len = tx_buf_len(portname)
    time.sleep(0.250)
    if (tx_len):
        print(tx_buf(portname, tx_len).decode())
    disconnect(portname)
