#!/usr/bin/env python
# This file is part of the OpenMV project.
# Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Openmv module.
import struct
import sys
import usb.core
import usb.util
from usb.core import USBError

__dev = None

# VID/PID
__VID = 0xf055
__PID = 0x9800

# Debug __INTERFACE
__INTERFACE = 0
__ALTSETTING = 1
__IN_EP = 0x81
__OUT_EP = 0x01
__TIMEOUT = 3000
__FB_HDR_SIZE = 12

# USB Debug commands
__USBDBG_FRAME_SIZE = 1
__USBDBG_FRAME_DUMP = 2
__USBDBG_FRAME_LOCK = 3
__USBDBG_FRAME_UPDATE = 4
__USBDBG_SCRIPT_EXEC = 5
__USBDBG_SCRIPT_STOP = 6
__USBDBG_SCRIPT_SAVE = 7
__USBDBG_TEMPLATE_SAVE = 8
__USBDBG_DESCRIPTOR_SAVE = 9
__USBDBG_ATTR_READ = 10
__USBDBG_ATTR_WRITE = 11
__USBDBG_SYS_RESET = 12
__USBDBG_SYS_BOOT = 13

ATTR_CONTRAST = 0
ATTR_BRIGHTNESS = 1
ATTR_SATURATION = 2
ATTR_GAINCEILING = 3

FORMAT_UNKNOWN = 0
FORMAT_RGB565 = 1
FORMAT_JPEG = 2
FORMAT_GRAY = 3


def init():
    global __dev
    # find USB __device
    print('OpenMV Cam init()')
    try:
        __dev = usb.core.find(idVendor=__VID, idProduct=__PID)

        if __dev is None:
            raise ValueError('__device not found')

        # detach kernel driver
        if __dev.is_kernel_driver_active(__INTERFACE):
            __dev.detach_kernel_driver(__INTERFACE)

        # claim __INTERFACE
        usb.util.claim_interface(__dev, __INTERFACE)

        # set FB debug alt setting
        __dev.set_interface_altsetting(__INTERFACE, __ALTSETTING)
    except Exception as e:
        pass
        print('Error while searching for OpenMV Cam: %s' % e.message)


def find():
    found = False
    try:
        dev = usb.core.find(idVendor=__VID, idProduct=__PID)
    except USBError:
        pass
        found = False
    else:
        if dev:
            found = True
    finally:
        return found


def release():
    try:
        # Release device resources, interface, etc.
        usb.util.dispose_resources(__dev)

        # reattach kernel driver
        #__dev.attach_kernel_driver(__INTERFACE)
    except USBError as e:
        print('error disposing resources %s' % e)
        pass


def fb_size():
    # read fb header
    buf = __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_SIZE, 0, __INTERFACE, __FB_HDR_SIZE, __TIMEOUT)
    size = struct.unpack("III", buf)
    return size


def fb_lock():
    try:
        buf = __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_LOCK, 0, __INTERFACE, 1, __TIMEOUT)
    except USBError:
        pass
        return 0
    else:
        return struct.unpack("B", buf)[0]


def fb_get():
    if fb_lock() == 0:
        return None

    buf = __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_SIZE, 0, __INTERFACE, __FB_HDR_SIZE, __TIMEOUT)

    size = struct.unpack("III", buf)

    w = size[0]
    h = size[1]
    bpp = size[2]

    if bpp > 2:
        # bpp is actually image size and data is in JPEG format
        num_bytes = bpp
        fmt = FORMAT_JPEG
    else:
        num_bytes = w * h * bpp
        if bpp == 1:
            fmt = FORMAT_GRAY
        elif bpp == 2:
            fmt = FORMAT_RGB565
        else:
            fmt = None
            w = None
            h = None

    if fmt:
        # read fb data
        try:
            __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_DUMP, num_bytes/4, __INTERFACE, 0, __TIMEOUT)
        except USBError:
            pass
        else:
            buff = __dev.read(__IN_EP, num_bytes, __INTERFACE, __TIMEOUT)
    else:
        buff = None

    return fmt, w, h, buff


def fb_update():
    __dev.ctrl_transfer(0x41, __USBDBG_FRAME_UPDATE, 0, __INTERFACE, None, __TIMEOUT)


def exec_script(buf):
    __dev.ctrl_transfer(0x41, __USBDBG_SCRIPT_EXEC, len(buf), __INTERFACE, None, __TIMEOUT)
    __dev.write(__OUT_EP, buf, __INTERFACE, __TIMEOUT)


def stop_script():
    try:
        __dev.ctrl_transfer(0x41, __USBDBG_SCRIPT_STOP, 0, __INTERFACE, None, __TIMEOUT)
    except USBError:
        pass

def save_template(x, y, w, h, path):
    buf = struct.pack("IIII", x, y, w, h) + path
    __dev.ctrl_transfer(0x41, __USBDBG_TEMPLATE_SAVE, len(buf), __INTERFACE, None, __TIMEOUT)
    __dev.write(__OUT_EP, buf, __INTERFACE, __TIMEOUT)


def save_descriptor(x, y, w, h, path):
    buf = struct.pack("IIII", x, y, w, h) + path
    __dev.ctrl_transfer(0x41, __USBDBG_DESCRIPTOR_SAVE, len(buf), __INTERFACE, None, __TIMEOUT)
    __dev.write(__OUT_EP, buf, __INTERFACE, __TIMEOUT)


def set_attr(attr, value):
    buf = struct.unpack(">H", struct.pack("bb", attr, value))[0]
    __dev.ctrl_transfer(0x41, __USBDBG_ATTR_WRITE, buf, __INTERFACE, None, __TIMEOUT)


def get_attr(attr):
    return 0


def enter_dfu():
    try:
        # This will timeout.
        __dev.ctrl_transfer(0x41, __USBDBG_SYS_BOOT, 0, __INTERFACE, None, __TIMEOUT)
    except USBError as e:
        pass


def reset():
    try:
        # This will timeout.
        __dev.ctrl_transfer(0x41, __USBDBG_SYS_RESET, 0, __INTERFACE, None, __TIMEOUT)
    except USBError:
        pass


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print ('usage: openmv.py <script>')
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin:
        init()
        exec_script(fin.read())


def __write_img(buff, path):
    with open(path, "wb") as f:
        f.write(buff)
        f.close()
