#!/usr/bin/env python
# This file is part of the OpenMV project.
# Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# DFU util.
# See app note AN3156.

import struct
import sys,time
import usb.core
import usb.util
import argparse

# VID/PID
__VID=0x0483
__PID=0xdf11

# USB request __TIMEOUT
__TIMEOUT = 4000

# DFU commands
__DFU_DETACH    = 0
__DFU_DNLOAD    = 1
__DFU_UPLOAD    = 2
__DFU_GETSTATUS = 3
__DFU_CLRSTATUS = 4
__DFU_GETSTATE  = 5
__DFU_ABORT     = 6

# DFU status
__DFU_STATE_APP_IDLE                 = 0x00
__DFU_STATE_APP_DETACH               = 0x01
__DFU_STATE_DFU_IDLE                 = 0x02
__DFU_STATE_DFU_DOWNLOAD_SYNC        = 0x03
__DFU_STATE_DFU_DOWNLOAD_BUSY        = 0x04
__DFU_STATE_DFU_DOWNLOAD_IDLE        = 0x05
__DFU_STATE_DFU_MANIFEST_SYNC        = 0x06
__DFU_STATE_DFU_MANIFEST             = 0x07
__DFU_STATE_DFU_MANIFEST_WAIT_RESET  = 0x08
__DFU_STATE_DFU_UPLOAD_IDLE          = 0x09
__DFU_STATE_DFU_ERROR                = 0x0a


# USB device handle
__dev = None

__verbose = None

# USB DFU interface
__DFU_INTERFACE = 0

def init():
    global __dev
    __dev = usb.core.find(idVendor=__VID, idProduct=__PID)
    if __dev is None:
        raise ValueError('No DFU device found')

    # Claim DFU interface
    usb.util.claim_interface(__dev, __DFU_INTERFACE)

    # Clear status
    clr_status()

def clr_status():
    __dev.ctrl_transfer(0x21, __DFU_CLRSTATUS, 0, __DFU_INTERFACE, None, __TIMEOUT)

def get_status():
    stat =__dev.ctrl_transfer(0xA1, __DFU_GETSTATUS, 0, __DFU_INTERFACE, 6, 20000)
    # print (__DFU_STAT[stat[4]], stat)
    return stat[4]

def mass_erase():
    # Send DNLOAD with first byte=0x41
    __dev.ctrl_transfer(0x21, __DFU_DNLOAD, 0, __DFU_INTERFACE, "\x41", __TIMEOUT)

    # Execute last command
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_BUSY):
        raise Exception("DFU: erase failed")

    # Check command state
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_IDLE):
        raise Exception("DFU: erase failed")

def page_erase(addr):
    if __verbose:
        print ("Erasing page: 0x%x..."%(addr))

    # Send DNLOAD with first byte=0x41 and page address
    buf = struct.pack("<BI", 0x41, addr)
    __dev.ctrl_transfer(0x21, __DFU_DNLOAD, 0, __DFU_INTERFACE, buf, __TIMEOUT)

    # Execute last command
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_BUSY):
        raise Exception("DFU: erase failed")

    # Check command state
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_IDLE):
        raise Exception("DFU: erase failed")

def set_address(addr):
    # Send DNLOAD with first byte=0x21 and page address
    buf = struct.pack("<BI", 0x21, addr)
    __dev.ctrl_transfer(0x21, __DFU_DNLOAD, 0, __DFU_INTERFACE, buf, __TIMEOUT)

    # Execute last command
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_BUSY):
        raise Exception("DFU: set address failed")

    # Check command state
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_IDLE):
        raise Exception("DFU: set address failed")

def write_memory(buf):
    xfer_count = 0
    xfer_bytes = 0
    xfer_total = len(buf)
    xfer_base = 0x08000000

    while (xfer_bytes < xfer_total):
        if (__verbose and xfer_count%512==0):
            print ("Addr 0x%x %dKBs/%dKBs..."%(xfer_base+xfer_bytes, xfer_bytes/1024, xfer_total/1024))

        # Set mem write address
        set_address(xfer_base+xfer_bytes)

        # Send DNLOAD with fw data
        chunk = min (64, xfer_total-xfer_bytes)
        __dev.ctrl_transfer(0x21, __DFU_DNLOAD, 2, __DFU_INTERFACE, buf[xfer_bytes:xfer_bytes+chunk], __TIMEOUT)

        # Execute last command
        if (get_status() != __DFU_STATE_DFU_DOWNLOAD_BUSY):
            raise Exception("DFU: write memory failed")

        # Check command state
        if (get_status() != __DFU_STATE_DFU_DOWNLOAD_IDLE):
            raise Exception("DFU: write memory failed")

        xfer_count +=1
        xfer_bytes += chunk

def write_page(buf, xfer_offset):
    xfer_base = 0x08000000

    # Set mem write address
    set_address(xfer_base+xfer_offset)

    # Send DNLOAD with fw data
    __dev.ctrl_transfer(0x21, __DFU_DNLOAD, 2, __DFU_INTERFACE, buf, __TIMEOUT)

    # Execute last command
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_BUSY):
        raise Exception("DFU: write memory failed")

    # Check command state
    if (get_status() != __DFU_STATE_DFU_DOWNLOAD_IDLE):
        raise Exception("DFU: write memory failed")

    if __verbose:
        print ("Write: 0x%x "%(xfer_base+xfer_offset))

def exit_dfu():
    # Send DNLOAD with 0 length to exit DFU
    __dev.ctrl_transfer(0x21, __DFU_DNLOAD, 0, __DFU_INTERFACE, None, __TIMEOUT)

    # Execute last command
    get_status()

    # Release device
    usb.util.dispose_resources(__dev)


if __name__ == '__main__':
    # Parse CMD args
    parser = argparse.ArgumentParser(description='DFU Python Util')
    parser.add_argument("path", help="file path")
    parser.add_argument("-u", "--upload",   help="read file from DFU device", action="store_true", default=False)
    parser.add_argument("-v", "--verbose",  help="increase output verbosity", action="store_true", default=False)
    args = parser.parse_args()

    __verbose = args.verbose

    with open(args.path, 'r') as fin:
        buf= fin.read()


    print("Init DFU...")
    init()
    #to erase pages:
    # erase_page(0x08004000)
    # erase_page(0x08020000)
    # erase_page(...)

    print ("Mass erase...")
    mass_erase()

    print("Writing memory...")
    write_memory(buf)

    print("Exiting DFU...")
    exit_dfu()
