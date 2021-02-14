#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script generates FFT tables.
# See: https://www.nayuki.io/page/fast-fourier-transform-in-x86-assembly

import sys, math

sys.stdout.write("const float cos_table[512] = {\n")
for i in range(512):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % math.cos((math.pi * i) / 512))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 511:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("const float sin_table[512] = {\n")
for i in range(512):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % math.sin((math.pi * i) / 512))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 511:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")
