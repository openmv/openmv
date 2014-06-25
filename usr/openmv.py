#!/usr/bin/env python
import sys
import usb.core
import usb.util
import struct
import numpy as np
from PIL import Image
import time
import StringIO

__dev = None
# VID/PID
__VID=0xf055
__PID=0x9800

# Debug __INTERFACE
__INTERFACE     = 0
__ALTSETTING    = 1
__IN_EP         =0x81
__OUT_EP        =0x01
__TIMEOUT       =1000
__FB_HDR_SIZE   =12

# USB Debug commands
__USBDBG_FRAME_SIZE=1
__USBDBG_FRAME_DUMP=2
__USBDBG_FRAME_READY=3
__USBDBG_SCRIPT_EXEC=4
__USBDBG_SCRIPT_STOP=5
__USBDBG_SCRIPT_SAVE=6
__USBDBG_TEMPLATE_SAVE=7
__USBDBG_ATTR_READ=8
__USBDBG_ATTR_WRITE=9

ATTR_CONTRAST=0
ATTR_BRIGHTNESS=1
ATTR_SATURATION=2
ATTR_GAINCEILING=3

def init():
    global __dev
    # find USB __device
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
    buf = __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_SIZE, 0, __INTERFACE, __FB_HDR_SIZE, __TIMEOUT)
    size = struct.unpack("III", buf)
    return size

def fb_ready():
    global __dev
    buf = __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_READY, 0, __INTERFACE, 1, __TIMEOUT)
    return struct.unpack("B", buf)[0]

def fb_dump():
    global __dev

    if (fb_ready() == 0):
        return None

    size = fb_size()
    if (size[2] > 2): #JPEG
        num_bytes = size[2]
    else:
        num_bytes = size[0]*size[1]*size[2]

    # read fb data
    __dev.ctrl_transfer(0xC1, __USBDBG_FRAME_DUMP, num_bytes, __INTERFACE, 0, __TIMEOUT)
    buff = __dev.read(__IN_EP, num_bytes, __INTERFACE, __TIMEOUT)

    #print size, num_bytes
    if (size[2] > 2):
        try:
            #img = Image.frombytes("RGB", (size[0], size[1]), buff, "jpeg", "RGB", "")
            __write_img(buff, "swap.jpeg")
            img = Image.open("swap.jpeg")
            buff = np.fromstring(img.tobytes(), dtype=np.uint8)
        except Exception, e:
            print "JPEG decode error (%s)"%e
            sys.exit(0)
        return (size[0], size[1], buff)
    else:
        return (size[0], size[1], fb_to_arr(buff, size[2]))

def exec_script(buf):
    __dev.ctrl_transfer(0x41, __USBDBG_SCRIPT_EXEC, len(buf), __INTERFACE, None, __TIMEOUT)
    __dev.write(__OUT_EP, buf, __INTERFACE, __TIMEOUT)

def stop_script():
    __dev.ctrl_transfer(0x41, __USBDBG_SCRIPT_STOP, 0, __INTERFACE, None, __TIMEOUT)

def save_template(x, y, w, h, name):
    buf = struct.pack("IIII", x, y, w, h)
    __dev.ctrl_transfer(0x41, __USBDBG_TEMPL_SAVE, len(buf), __INTERFACE, None, __TIMEOUT)
    __dev.write(__OUT_EP, buf, __INTERFACE, __TIMEOUT)

def set_attr(attr, value):
    buf = struct.unpack(">H", struct.pack("bb", attr, value))[0]
    __dev.ctrl_transfer(0x41, __USBDBG_ATTR_WRITE, buf, __INTERFACE, None, __TIMEOUT)

def get_attr(attr):
    return 0

if __name__ == '__main__':
    if len(sys.argv)!= 2:
        print 'usage: openmv.py <script>'
        sys.exit(1)
    with open(sys.argv[1], 'r') as fin:
        buf = fin.read()
        init()
        exec_script(buf)

def __write_img(buff, path):
    with open(path, "wb") as f:
        f.write(buff)
        f.close()
