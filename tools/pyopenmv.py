#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Openmv module.

import struct
import sys,time
import serial
import platform
import numpy as np
from PIL import Image

__serial = None
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
__USBDBG_SYS_RESET_TO_BL= 0x0E
__USBDBG_FB_ENABLE      = 0x0D
__USBDBG_TX_BUF_LEN     = 0x8E
__USBDBG_TX_BUF         = 0x8F
__USBDBG_GET_STATE      = 0x93

__USBDBG_STATE_FLAGS_SCRIPT = (1 << 0)
__USBDBG_STATE_FLAGS_TEXT   = (1 << 1)
__USBDBG_STATE_FLAGS_FRAME  = (1 << 2)

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
    # open CDC port
    __serial =  serial.Serial(port, baudrate=baudrate, timeout=timeout)

def disconnect():
    global __serial
    try:
        if (__serial):
            __serial.close()
            __serial = None
    except:
        pass

def set_timeout(timeout):
    __serial.timeout = timeout

def fb_size():
    # read fb header
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_SIZE, __FB_HDR_SIZE))
    return struct.unpack("III", __serial.read(12))

def read_state():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_GET_STATE, 64))
    flags, w, h, size, text_buf = struct.unpack("IIII48s", __serial.read(64))

    text = None
    if flags & __USBDBG_STATE_FLAGS_TEXT:
        text = text_buf.split(b'\0', 1)[0].decode()

    if flags & __USBDBG_STATE_FLAGS_FRAME == 0:
        return 0, 0, None, 0, text

    num_bytes = size if size > 2 else (w * h * size)

    # read fb data
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FRAME_DUMP, num_bytes))
    buff = __serial.read(num_bytes)

    if size == 1:  # Grayscale
        y = np.fromstring(buff, dtype=np.uint8)
        buff = np.column_stack((y, y, y))
        size = w * h
    elif size == 2: # RGB565
        arr = np.fromstring(buff, dtype=np.uint16)
        r = (((arr & 0xF800) >>11)*255.0/31.0).astype(np.uint8)
        g = (((arr & 0x07E0) >>5) *255.0/63.0).astype(np.uint8)
        b = (((arr & 0x001F) >>0) *255.0/31.0).astype(np.uint8)
        buff = np.column_stack((r,g,b))
        size = w * h * 2
    else: # JPEG
        try:
            buff = np.asarray(Image.frombuffer("RGB", (w, h), buff, "jpeg", "RGB", ""))
        except Exception as e:
            raise ValueError(f"JPEG decode error (%e)")

    if (buff.size != (w*h*3)):
        raise ValueError(f"Unexpected frame size. Expected: {w*h*3} received: {buff.size}")

    return w, h, buff.reshape((h, w, 3)), size, text


def fb_dump():
    size = fb_size()

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
        y = np.fromstring(buff, dtype=np.uint8)
        buff = np.column_stack((y, y, y))
    elif size[2] == 2: # RGB565
        arr = np.fromstring(buff, dtype=np.uint16)
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

def exec_script(buf):
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_EXEC, len(buf)))
    __serial.write(buf.encode())

def stop_script():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_STOP, 0))

def script_running():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SCRIPT_RUNNING, 4))
    return struct.unpack("I", __serial.read(4))[0]

def save_template(x, y, w, h, path):
    buf = struct.pack("IIII", x, y, w, h) + path
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TEMPLATE_SAVE, len(buf)))
    __serial.write(buf)

def save_descriptor(x, y, w, h, path):
    buf = struct.pack("HHHH", x, y, w, h) + path
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_DESCRIPTOR_SAVE, len(buf)))
    __serial.write(buf)

def set_attr(attr, value):
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_ATTR_WRITE, 8))
    __serial.write(struct.pack("<II", attr, value))

def get_attr(attr):
    __serial.write(struct.pack("<BBIh", __USBDBG_CMD, __USBDBG_ATTR_READ, 1, attr))
    return __serial.read(1)

def reset():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SYS_RESET, 0))

def reset_to_bl():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_SYS_RESET_TO_BL, 0))

def bootloader_start():
    __serial.write(struct.pack("<I", __BOOTLDR_START))
    return struct.unpack("I", __serial.read(4))[0] == __BOOTLDR_START

def bootloader_reset():
    __serial.write(struct.pack("<I", __BOOTLDR_RESET))

def flash_erase(sector):
    __serial.write(struct.pack("<II", __BOOTLDR_ERASE, sector))

def flash_write(buf):
    __serial.write(struct.pack("<I", __BOOTLDR_WRITE) + buf)

def tx_buf_len():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TX_BUF_LEN, 4))
    return struct.unpack("I", __serial.read(4))[0]

def tx_buf(bytes):
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_TX_BUF, bytes))
    return __serial.read(bytes)

def fw_version():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FW_VERSION, 12))
    return struct.unpack("III", __serial.read(12))

def enable_fb(enable):
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_FB_ENABLE, 4))
    __serial.write(struct.pack("<I", enable))

def arch_str():
    __serial.write(struct.pack("<BBI", __USBDBG_CMD, __USBDBG_ARCH_STR, 64))
    return __serial.read(64).split(b'\0', 1)[0]

if __name__ == '__main__':
    if len(sys.argv)!= 3:
        print ('usage: pyopenmv.py <port> <script>')
        sys.exit(1)

    with open(sys.argv[2], 'r') as fin:
        buf = fin.read()

    disconnect()
    init(sys.argv[1])
    stop_script()
    exec_script(buf)
    tx_len = tx_buf_len()
    time.sleep(0.250)
    if (tx_len):
        print(tx_buf(tx_len).decode())
    disconnect()
