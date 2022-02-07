#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script converts tflite models and labels to a C structs.

import sys, os
import glob
import argparse
import binascii

def main():
    parser = argparse.ArgumentParser(description='Converts TFLite models to C file.')
    parser.add_argument('--input', action = 'store', help = 'Input tflite models directory.', required=True)
    parser.add_argument('--header', action = 'store_true', help = 'Generate header file.', required=False, default=False)
    args = parser.parse_args()

    libtf_builtin_models = []
    print('/* NOTE: This file is auto-generated. */\n')

    models_list = glob.glob(os.path.join(args.input, "*tflite"))
    if (args.header):
        # Generate the header file
        print('// Built-in TFLite Models.')
        print('typedef struct {')
        print('    const char *name;')
        print('    const unsigned int n_labels;')
        print('    const char **labels;')
        print('    const unsigned int size;')
        print('    const unsigned char *data;')
        print('}libtf_builtin_model_t;\n')
        print('extern const libtf_builtin_model_t libtf_builtin_models[{:d}];'.format(len(models_list)))
    else:
        # Generate the C file
        print('#include "libtf_builtin_models.h"')
        for model_file in models_list:
            model_size = os.path.getsize(model_file)
            model_name = os.path.basename(os.path.splitext(model_file)[0])
            labels_file = os.path.splitext(model_file)[0]+'.txt'

            # Generate model labels.
            labels = []
            n_labels = 0
            with open(labels_file, 'r') as f:
                labels = ['"{:s}"'.format(l.strip()) for l in f.readlines()]
                n_labels = len(labels)
            print('const char *libtf_{:s}_labels[] = {{{:s}}};'.format(model_name, ', '.join(labels)))

            # Generate model data.
            print('const unsigned char libtf_{:s}_data[] = {{'.format(model_name))
            with open(model_file, 'rb') as f:
                for chunk in iter(lambda: f.read(12), b''):
                    print('  ', end='')
                    print(' '.join(['0x{:02x},'.format(x) for x in chunk]))
            print('};')

            # Store model info in builtin models table.
            libtf_builtin_models.append([
                    model_name,
                    n_labels,
                    'libtf_{:s}_labels'.format(model_name),
                    model_size,
                    'libtf_{:s}_data'.format(model_name)]
            )

        # Generate built-in models table.
        print('const libtf_builtin_model_t libtf_builtin_models[{:d}] = {{'.format(len(models_list)))
        for model in libtf_builtin_models:
            print('    {{"{:s}", {:d}, {:s}, {:d}, {:s}}},'.format(*model))
        print('};')

if __name__ == '__main__':
    main()
