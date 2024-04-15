# !/usr/bin/env python
# This file is part of the OpenMV project.
#
# This scripts convert ttf to .h file .
# 
# Usage: python ttf2.c [--font-file-path] > ttf.h
#
# Example : 
# 1. Generate subset font. (Roboto-Regular.subset.ttf):
#       'pyftsubset Roboto-Regular.ttf --text="\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\`abcdefghijklmnopqrstuvwxyz{|}~!" --no-hinting' 
# 2. Run this command "python ttf2.c --font-file-path=Roboto-Regular.subset.ttf > ttf.h" . 

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--font-file-path', required=True, help='the path of ttf file')
args = parser.parse_args() 

with open(args.font_file_path, 'rb') as ttf:
    file = ttf.read()
    print('const unsigned char builtin_ttf[] = {')
    print(' '.join(['0x{:02x},'.format(x) for x in file]))
    print('};')