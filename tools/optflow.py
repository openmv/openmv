#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# pygame + sockets util that receives optical flow data from the camera and draws a path.

import time
import select
import socket
import pygame
from math import sqrt, isnan

RED     = (255,0,0)
GREEN   = (0,255,0)
BLUE    = (0,0,255)
WHITE   = (255,255,255)
BLACK   = (0,0,0)
PINK    = (255,200,200)
ADDR    =('192.168.1.101', 8080)
WIDTH   = 640
HEIGHT  = 480

points = []
clock = pygame.time.Clock() 
screen = pygame.display.set_mode((WIDTH, HEIGHT))

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(ADDR)

lx = WIDTH//2
ly = HEIGHT//2

while True:
    msElapsed = clock.tick(30) 
    event = pygame.event.poll()
    if event.type == pygame.QUIT:
        break
    elif event.type == pygame.KEYDOWN:
        if event.key == pygame.K_SPACE:
            # Reset points
            points = []
            lx = WIDTH//2
            ly = HEIGHT//2

    data = s.recv(20)
    print(data)
    dx = float(data.split(',')[0])
    dy = float(data.split(',')[1])
    r  = float(data.split(',')[2])

    if dx and dy and r > 0.15: 
        m = sqrt(dx*dx + dy*dy)
        x2 = lx + int((dx/m)*5)
        y2 = ly - int((dy/m)*5)
        lx = x2; ly = y2
        points.append((x2, y2))

    if len(points)>1:
        screen.fill(WHITE)
        pygame.draw.lines(screen, RED, False, points, 3)

    pygame.display.update()

s.close()
pygame.quit()
