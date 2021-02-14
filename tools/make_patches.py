#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script creates smaller patches from images.

import os, sys
import argparse
import random
import numpy as np
from skimage import io
from skimage import exposure
from sklearn.feature_extraction import image

def main():
    # CMD args parser
    parser = argparse.ArgumentParser(description='Generate smaller patches from images')
    parser.add_argument("--input",   action = "store", help = "Input images dir")
    parser.add_argument("--output",  action = "store", help = "Output images dir")
    parser.add_argument("--width",   action = "store", help = "Patch width",       type=int, default = 32)
    parser.add_argument("--height",  action = "store", help = "Patch height",      type=int, default = 32)
    parser.add_argument("--patches", action = "store", help = "Number of patches", type=int, default = 10)

    # Parse CMD args
    args = parser.parse_args()
    if (args.input == None or args.output == None):
        parser.print_help()
        sys.exit(1)

    count = 0
    images = os.listdir(args.input)
    while (count < args.patches):
        random.shuffle(images)
        for i in xrange(len(images)):
            img = io.imread(args.input+'/'+images[i])
            patches = image.extract_patches_2d(img,
                            patch_size=(args.width, args.height),
                            max_patches=100, random_state=np.random.RandomState(0))
            random.shuffle(patches)
            for p in patches:
                # Save low contrast patches only
                if (exposure.is_low_contrast(p) == False):
                    io.imsave(args.output+'/patch_%.4d.ppm'%(count), p)
                    count += 1
                    break
            if (count == args.patches):
                break

if __name__ == '__main__':
    main()
