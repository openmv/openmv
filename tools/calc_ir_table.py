#!/usr/bin/env python2
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This scripts generates IR table.

import sys

it = iter(l)

for i,j in zip(it, it):
    aij.append(i/2**39+j/2**32),

for i in range(0, len(aij)):
    sys.stdout.write("{0:.10e}f, ".format(aij[i]))
    if (i+1)%8==0:
        print()
