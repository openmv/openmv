#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script creates a romfs image.

import sys
import os
import json
import argparse
import struct
from tflite2c import vela_compile
from haar2c import cascade_binary_universal


def main():
    # Set up argument parser
    parser = argparse.ArgumentParser(description='Create a romfs image from json file.')
    parser.add_argument('--input', action = 'store', help = 'Input romfs json config file.', required=True)
    parser.add_argument('--top-dir', action = 'store', help='Top directory', required=True)
    parser.add_argument('--build-dir', action = 'store', help='Build directory', required=True)
    parser.add_argument('--vela-args', action = 'store', help='Vela compiler args', default='')
    parser.add_argument('-v', '--verbose', action='store_true', help='Print additional details about the parsed data')

    # Parse arguments
    args = parser.parse_args()

    # Load romfs config from the JSON file
    try:
        with open(args.input, 'r') as file:
            romfs_data = json.loads(file.read())["romfs"]
    except Exception as e:
        print(f"Error: Unable to read the file '{args.input}'. {str(e)}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format in '{args.input}'. {str(e)}")
        sys.exit(1)

    # print(json.dumps(romfs_data, indent=2))

    # To replace variables in the path
    variables = {
        "TOP" : args.top_dir,
        "BUILD" : args.build_dir
    }

    # Build/convert files if needed.
    for entry in romfs_data:
        file_path = entry['path'].format(**variables)
        file_name = os.path.basename(os.path.splitext(file_path)[0])
        if entry['type'] == 'haar':
            # Convert Haar cascade
            output_path = os.path.join(args.build_dir, file_name)
            cascade_binary_universal(file_path, entry["stages"], output_path)
            file_path = os.path.join(args.build_dir, file_name + ".cascade")
        elif entry['type'] == 'tflite' and args.vela_args:
            # Compile the model using Vela.
            vela_args = args.vela_args + " --optimise " + entry["optimize"]
            vela_compile(file_path, args.build_dir, vela_args.split())
            file_path = os.path.join(args.build_dir, file_name + ".tflite")
        entry['path'] = file_path

    romfs_offset = 0
    with open(os.path.join(args.build_dir, "romfs.bin"), "wb") as romfs:
        romfs_offset += romfs.write(b"MF")
        for entry in romfs_data:
            file_path = entry['path']
            file_name = os.path.basename(os.path.splitext(file_path)[0])
            file_size = os.path.getsize(file_path)
            file_align = entry["alignment"]
            print(f"Adding {file_name} path: {file_path} size: {file_size} align: {file_align}");
            with open(file_path, "rb") as file:
                romfs_offset += romfs.write(struct.pack("<HI", len(file_name), file_size))
                romfs_offset += romfs.write(bytes(file_name, "ascii"))
                # How to handle alignment \../ ?
                # if romfs_offset % file_align:
                #     padding = file_align - (romfs_offset % file_align)
                #     romfs_offset += romfs.write(b"\0" * padding)
                romfs_offset += romfs.write(file.read())
        romfs.write(b"\x00\x00")

if __name__ == '__main__':
    main()
