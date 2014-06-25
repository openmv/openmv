#!/usr/bin/env python
import sys
import usb.core
import usb.util
import numpy as np
import pygame
import openmv
from time import sleep

# init pygame
pygame.init()

# init openmv
openmv.init()

# init screen
running = True
Clock = pygame.time.Clock()
font = pygame.font.SysFont("monospace", 15)
while running:
    Clock.tick(60)

    # read framebuffer
    fb = openmv.fb_dump()

    if fb == None:
        continue

    # create image from RGB888
    image = pygame.image.frombuffer(fb[2].flat[0:], (fb[0], fb[1]), 'RGB')
    # TODO check if res changed
    screen = pygame.display.set_mode((fb[0], fb[1]), pygame.DOUBLEBUF, 32)

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
        if event.key == pygame.K_c:
            pygame.image.save(image, "capture.png")


pygame.quit()
openmv.release()

