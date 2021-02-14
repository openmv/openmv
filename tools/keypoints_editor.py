#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Simple keypoints descriptor editor.

import time
import struct
import pygame
import sys, os
from math import sin, cos, radians

SCALE = 3
RED   = (255,0,0)
GREEN = (0,255,0)
WHITE = (255,255,255)
GRAY  = (127, 127, 127)
KEYPOINTS_SIZE = 24

def load_keypoints(path):
    kpts = []
    max_octave = 0
    with open(path, "rb") as f:
        desc = struct.unpack("<I", f.read(4))[0]
        size = struct.unpack("<I", f.read(4))[0]
        for i in range(0, size):
            x, y, score, octave, angle = struct.unpack("<HHHHH", f.read(10))
            kpts.append([x, y, score, octave, angle, False, f.read(32)])
            if (octave > max_octave):
                max_octave = octave
    print("size: %d max octave: %d" %(len(kpts), max_octave))
    return (kpts, max_octave)

if __name__ == '__main__':
    if len(sys.argv)!= 2:
        print "usage: kpts_editor.py [descriptor]"
        sys.exit(1)

    selection = False
    save_selected = False
    rect = pygame.Rect(0, 0, 0, 0)
    clock = pygame.time.Clock()

    desc_path = sys.argv[1]
    img_path = os.path.splitext(desc_path)[0] + '.pgm'
    desc_out = os.path.splitext(desc_path)[0] + '_out.orb'

    kpts, max_octave = load_keypoints(desc_path)
    curr_octave = max_octave
    img = pygame.image.load(img_path)
    img = pygame.transform.scale(img, (img.get_width()*SCALE, img.get_height()*SCALE))
    screen = pygame.display.set_mode((img.get_width(), img.get_height()))
    pygame.mouse.set_visible(True)
    
    while True:
        msElapsed = clock.tick(60)
        event = pygame.event.poll()
        if event.type == pygame.QUIT:
            break
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_s:
                save_selected = True
            elif event.key == pygame.K_1:
                if (curr_octave > 1):
                    curr_octave = curr_octave - 1
            elif event.key == pygame.K_2:
                if (curr_octave < max_octave):
                    curr_octave = curr_octave + 1

        elif event.type == pygame.MOUSEBUTTONDOWN:
            selection = True
            rect = pygame.Rect(0, 0, 0, 0)
            rect.x, rect.y = pygame.mouse.get_pos()
        elif event.type == pygame.MOUSEMOTION:
            if (selection):
                rect.w, rect.h = pygame.mouse.get_pos()
                rect.w -= rect.x
                rect.h -= rect.y
        elif event.type == pygame.MOUSEBUTTONUP:
            if (selection):
                selection = False
                rect.w, rect.h = pygame.mouse.get_pos()
                rect.w -= rect.x
                rect.h -= rect.y
    
        screen.blit(img,(0,0))

        if selection == True:
            pygame.draw.rect(screen, RED, rect, 1)
    
        for kp in kpts:
            x, y, score, octave, angle, selected, desc = kp
            if (octave > curr_octave):
                continue

            x1 = x*SCALE
            y1 = y*SCALE
            angle = radians(angle)

            size = KEYPOINTS_SIZE/octave
            x2 = x1 + sin(angle) * size
            y2 = y1 + cos(angle) * size

            kp[5] = selected = rect.collidepoint((x1, y1))
            color = GREEN if selected else WHITE
            pygame.draw.circle(screen, color, (x1, y1), size, 1)
            pygame.draw.line(screen, color, (x1, y1), (x2, y2))
    
        if save_selected == True:
            kpts_sel = []
            for kp in kpts:
                if (kp[5]): kpts_sel.append(kp)
    
            with open(desc_out, "wb") as f:
                f.write(struct.pack("<I", 1))
                f.write(struct.pack("<I", len(kpts_sel)))
                for kp in kpts_sel:
                    x, y, score, octave, angle, selected, desc = kp
                    f.write(struct.pack("<HHHHH", x, y, score, octave, angle))
                    f.write(desc)
            break
    
        pygame.display.update()
    
    pygame.quit()
