#!/usr/bin/env python
import sys
import usb.core
import usb.util
import struct
import numpy as np

# USB Debug commands
__USBDBG_FB_SIZE=1
__USBDBG_DUMP_FB=2
__USBDBG_EXEC_SCRIPT=3
__USBDBG_READ_SCRIPT=4
__USBDBG_WRITE_SCRIPT=5
__USBDBG_STOP_SCRIPT=6

# Debug __INTERFACE
__INTERFACE = 2;
__ALTSETTING= 1;

__dev = None

__TIMEOUT  = 1000

def init():
    global __dev
    # find USB __device
    __dev = usb.core.find(idVendor=0x0483, idProduct=0x5740)
    if __dev is None:
        raise ValueError('__device not found')

    # detach kernel driver
    if __dev.is_kernel_driver_active(__INTERFACE):
        __dev.detach_kernel_driver(__INTERFACE)

    # claim __INTERFACE
    usb.util.claim_interface(__dev, __INTERFACE)

    # set FB debug alt setting
    __dev.set_interface_altsetting(__INTERFACE, __ALTSETTING)

def release():
    global __dev
    # release __INTERFACE
    usb.util.release_interface(__dev, __INTERFACE)

    # reattach kernel driver
    #__dev.attach_kernel_driver(__INTERFACE)

def fb_to_arr(buff, bpp):
    if bpp == 1:
        r = np.fromstring(buff, dtype=np.uint8)
        g = np.fromstring(buff, dtype=np.uint8)
        b = np.fromstring(buff, dtype=np.uint8)
        return np.column_stack((r,g,b))
    else:
        arr = np.fromstring(buff, dtype=np.uint16).newbyteorder('S')
        r = (((arr & 0xF800) >>11)*255.0/31.0).astype(np.uint8)
        g = (((arr & 0x07E0) >>5) *255.0/63.0).astype(np.uint8)
        b = (((arr & 0x001F) >>0) *255.0/31.0).astype(np.uint8)
        return np.column_stack((r,g,b))

def fb_size():
    global __dev
    # read fb header
    __dev.ctrl_transfer(0xC1, __USBDBG_FB_SIZE, 0, __INTERFACE, None, __TIMEOUT)
    size = struct.unpack("III", __dev.read(0x83, 64, __INTERFACE, __TIMEOUT)[0:12])
    return size

def dump_fb():
    global __dev
    size = fb_size()
    num_bytes = size[0]*size[1]*size[2]

    # read fb data
    __dev.ctrl_transfer(0xC1, __USBDBG_DUMP_FB, 0, __INTERFACE, None, __TIMEOUT)
    buff = __dev.read(0x83, num_bytes, __INTERFACE, __TIMEOUT)

    return (size[0], size[1], fb_to_arr(buff, size[2]))

def exec_script(buf):
    __dev.ctrl_transfer(0x41, __USBDBG_EXEC_SCRIPT, len(buf), __INTERFACE, None, __TIMEOUT)
    __dev.write(0x03, buf, __INTERFACE, __TIMEOUT)

def stop_script():
    __dev.ctrl_transfer(0x41, __USBDBG_STOP_SCRIPT, 0, __INTERFACE, None, __TIMEOUT)

if __name__ == '__main__':
    if len(sys.argv)!= 2:
        print 'usage: openmv.py <script>'
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin:
        buf = fin.read()
        init()
        exec_script(buf)
