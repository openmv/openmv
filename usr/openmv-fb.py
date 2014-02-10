#!/usr/bin/env python
import sys
import usb.core
import usb.util
import numpy as np
import pygame
import openmv
from time import sleep

def rgb_to_surface(buff):
    arr = np.fromstring(buff, dtype=np.uint16).newbyteorder('S')
    r = (((arr & 0xF800) >>11)*255.0/31.0).astype(np.uint8)
    g = (((arr & 0x07E0) >>5) *255.0/63.0).astype(np.uint8)
    b = (((arr & 0x001F) >>0) *255.0/31.0).astype(np.uint8)
    arr = np.column_stack((r,g,b)).flat[0:]
    return pygame.image.frombuffer(arr, (160, 120), 'RGB')

# init pygame
pygame.init()

# init openmv
openmv.init()

# init screen
screen = pygame.display.set_mode((160, 120), pygame.DOUBLEBUF, 32)

running = True
img_size = 160*120*2
Clock = pygame.time.Clock()
font = pygame.font.SysFont("monospace", 15)
while running:
    Clock.tick(60)

    # read framebuffer
    buf = openmv.dump_fb()
    
    if len(buf) <img_size:
        print(len(buf))
        continue

    # convert to RGB888 and blit
    image = rgb_to_surface(buf)

    # blit stuff 
    screen.blit(image, (0, 0))
    screen.blit(font.render("FPS %.2f"%(Clock.get_fps()), 1, (255, 0, 0)), (0, 0))

    # update display
    pygame.display.flip()

    event = pygame.event.poll()
    if event.type == pygame.QUIT:
         running = False
    elif event.type == pygame.KEYDOWN:
        if event.key == pygame.K_ESCAPE:
            running = False

pygame.quit()
openmv.release()

