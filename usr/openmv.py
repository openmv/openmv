#!/usr/bin/env python
import sys
import usb.core
import usb.util

# USB Debug commands
__USBDBG_DUMP_FB=1
__USBDBG_EXEC_SCRIPT=2
__USBDBG_READ_SCRIPT=3
__USBDBG_WRITE_SCRIPT=4

# Debug __INTERFACE
__INTERFACE = 2;
__ALTSETTING= 1;

__dev = None

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
    __dev.attach_kernel_driver(__INTERFACE)


def dump_fb():
    global __dev
    # request snapshot
    __dev.ctrl_transfer(0xC1, __USBDBG_DUMP_FB, 0, __INTERFACE, None, 2000)

    # read framebuffer
    return __dev.read(0x83, 160*120*2, __INTERFACE, 5000)

def exec_script(buf):
    __dev.ctrl_transfer(0x41, __USBDBG_EXEC_SCRIPT, len(buf), __INTERFACE, None, 2000)
    __dev.write(0x03, buf, __INTERFACE, 10000)

