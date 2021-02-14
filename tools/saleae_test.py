#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This scripts uses Saleae Logic to capture openmv I/O data and compare it to known values.

import sys
import saleae
import argparse

parser = argparse.ArgumentParser(description='Saleae OpenMV test')
parser.add_argument('--test-data', metavar='PATH', help='Capture and compare test data')
parser.add_argument('--export-data', metavar='PATH', help='Capture and export test data')

args = parser.parse_args()

if (args.test_data == None and args.export_data == None):
    parser.print_help()
    sys.exit(1)

# Init device.
s = saleae.Saleae()

# Capture samples.
s.set_num_samples(10e6)

# Set falling trigger on channel 0.
s.set_trigger_one_channel(0, saleae.Trigger.Negedge)

# Start capture.
s.capture_start_and_wait_until_finished()

# Export and exit.
if args.export_data:
    s.export_data2(args.export_data)
    sys.exit(0)
    
if args.test_data:
    data = []
    with open(args.test_data) as f:
        for l in f.read().splitlines()[1:]:
            data.append(int(l.split(',')[1], 16))
    print(len(data))
    for v in data:
        print("0x%x"%(v))
