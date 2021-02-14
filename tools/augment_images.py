#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script augments an image dataset.

import os, sys
import argparse
import random
import cv2
import numpy as np
import imgaug as ia
from imgaug import augmenters as iaa
from tqdm import tqdm

def main():
    # CMD args parser
    parser = argparse.ArgumentParser(description='Augment image datasets')
    parser.add_argument("--input",   action = "store", help = "Input images dir")
    parser.add_argument("--output",  action = "store", help = "Output images dir")
    parser.add_argument("--count",   action = "store", help = "Number of augmented sets to make", type=int, default=1)

    # Parse CMD args
    args = parser.parse_args()
    if (args.input == None or args.output == None):
        parser.print_help()
        sys.exit(1)

    ia.seed(1)
    paths = os.listdir(args.input)

    for x in range(args.count):
        seq = iaa.Sequential([
            iaa.Fliplr(0.5), # horizontal flips

            # Small gaussian blur with random sigma between 0 and 0.5.
            # But we only blur about 50% of all images.
            iaa.Sometimes(0.5,
                iaa.GaussianBlur(sigma=(0, 0.2))
            ),

            # Add gaussian noise.
            # For 50% of all images, we sample the noise once per pixel.
            # For the other 50% of all images, we sample the noise per pixel AND
            # channel. This can change the color (not only brightness) of the pixels.
            iaa.Sometimes(0.5,
                iaa.AdditiveGaussianNoise(
                    loc=0, scale=(0.0, 0.005*255), per_channel=0.5
                )
            ),

            # Make some images brighter and some darker.
            # In 20% of all cases, we sample the multiplier once per channel,
            # which can end up changing the color of the images.
            iaa.Sometimes(0.5,
                iaa.Multiply((0.8, 1.2), per_channel=0.0),
            ),

            # Apply affine transformations to each image.
            # Scale/zoom images.
            iaa.Sometimes(0.5,
                iaa.Affine(
                     rotate=(-20, 20),
                ),
            ),

            # Translate/move images.
            iaa.Sometimes(0.5,
                iaa.Affine(
                     scale={"x": (0.8, 1.2), "y": (0.8, 1.2)},
                ),
            ),

            # Rotate images.
            iaa.Sometimes(0.5,
                iaa.Affine(
                    translate_percent={"x": (-0.1, 0.1), "y": (-0.1, 0.1)},
                ),
            ),
        ], random_order=True) # apply augmenters in random order
        
        print("Augmenting images set %d/%d"%(x+1, args.count))
        for i in tqdm(xrange(len(paths))):
            img = cv2.imread(args.input+'/'+paths[i], cv2.IMREAD_GRAYSCALE)
            img = seq.augment_image(img)
            f = os.path.splitext(paths[i])
            cv2.imwrite(args.output+'/'+f[0] + '_aug%d'%(x) + f[1], img)

    print('Finished processing all images\n')

if __name__ == '__main__':
    main()
