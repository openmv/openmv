#!/usr/bin/env python
import sys
import usb.core
import usb.util
import numpy as np
import pygame
from time import sleep

interface = 2;
altsetting= 1;

def rgb_to_surface(buff):
    arr = np.fromstring(buff, dtype=np.uint16).newbyteorder('S')
    r = (((arr & 0xF800) >>11)*255.0/31.0).astype(np.uint8)
    g = (((arr & 0x07E0) >>5) *255.0/63.0).astype(np.uint8)
    b = (((arr & 0x001F) >>0) *255.0/31.0).astype(np.uint8)
    arr = np.column_stack((r,g,b)).flat[0:]
    return pygame.image.frombuffer(arr, (160, 120), 'RGB')

# find USB device 
dev = usb.core.find(idVendor=0x0483, idProduct=0x5740)
if dev is None:
    raise ValueError('Device not found')

# detach kernel driver    
if dev.is_kernel_driver_active(interface):    
    dev.detach_kernel_driver(interface)    

# claim interface
usb.util.claim_interface(dev, interface)

# set FB debug alt setting
dev.set_interface_altsetting(interface, altsetting)

# init screen
screen = pygame.display.set_mode((160, 120), pygame.DOUBLEBUF, 32)
img_size = 160*120*2

while True:
    # request snapshot
    dev.ctrl_transfer(0xC1, 8, 0, 2, None, 2000)

    # read framebuffer
    buf = dev.read(0x83, img_size, interface, 5000)
    
    if len(buf) <img_size:
        print(len(buf))
        continue

    # convert to RGB888 and blit
    image = rgb_to_surface(buf)
    screen.blit( image, ( 0, 0 ) )

    # update display
    pygame.display.flip()
