#!/usr/bin/env python
# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
import os
import sys
import zipfile
import subprocess
from operator import itemgetter
from tabulate import tabulate


def extract_zip_files(directory):
    for item in os.listdir(directory):
        if item.endswith(".zip"):
            with zipfile.ZipFile(os.path.join(directory, item), "r") as zip_ref:
                name = os.path.splitext(item.strip())[0].split("firmware_")[1]
                extract_directory = os.path.join(directory, name)
                if not os.path.exists(extract_directory):
                    os.makedirs(extract_directory)
                zip_ref.extractall(extract_directory)


def find_elf_files(directory):
    elf_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".elf"):
                elf_files.append(os.path.join(root, file))
    return elf_files


def get_relative_path(full_path, base_dir):
    return os.path.relpath(full_path, base_dir)


def get_firmware_sizes(path):
    try:
        output = subprocess.check_output(["arm-none-eabi-size", path], text=True)
    except subprocess.CalledProcessError as e:
        print(e.stderr, file=sys.stderr)
        raise e
    return [int(s) for s in output.split() if s.isdigit()][0:3]


def main():
    new_firmware_dir = "new_firmware"
    old_firmware_dir = "old_firmware"

    # Extract zip files
    extract_zip_files(old_firmware_dir)
    extract_zip_files(new_firmware_dir)

    # Find ELF files and their relative paths
    new_elf_files = find_elf_files(new_firmware_dir)
    old_elf_files = find_elf_files(old_firmware_dir)

    new_elf_paths = { get_relative_path(path, new_firmware_dir): path for path in new_elf_files }
    old_elf_paths = { get_relative_path(path, old_firmware_dir): path for path in old_elf_files }

    # Find common ELF files based on relative paths
    common_paths = set(new_elf_paths.keys()).intersection(old_elf_paths.keys())
    # List of common ELF files with their paths
    common_elf_files = [(new_elf_paths[rel_path], old_elf_paths[rel_path]) for rel_path in common_paths]
    common_elf_files = sorted(common_elf_files, key=lambda x: x[0].split(os.sep)[1])

    # Prepare the header
    headers = ["Firmware", "Text Diff", "Data Diff", "BSS Diff"]
    
    # Prepare the data
    data = []
    for new_path, old_path in common_elf_files:
        new_sizes = get_firmware_sizes(new_path)
        old_sizes = get_firmware_sizes(old_path)
    
        text_diff = new_sizes[0] - old_sizes[0]
        data_diff = new_sizes[1] - old_sizes[1]
        bss_diff = new_sizes[2] - old_sizes[2]
    
        if not any([text_diff, data_diff, bss_diff]):
            continue

        text_percent = (((new_sizes[0] - old_sizes[0]) / old_sizes[0]) * 100 if old_sizes[0] != 0 else 0)
        data_percent = (((new_sizes[1] - old_sizes[1]) / old_sizes[1]) * 100 if old_sizes[1] != 0 else 0)
        bss_percent = (((new_sizes[2] - old_sizes[2]) / old_sizes[2]) * 100 if old_sizes[2] != 0 else 0)
    
        text_emoji = "🔺" if text_percent > 0 else "🔻" if text_percent < 0 else "➖"
        data_emoji = "🔺" if data_percent > 0 else "🔻" if data_percent < 0 else "➖"
        bss_emoji = "🔺" if bss_percent > 0 else "🔻" if bss_percent < 0 else "➖"
    
        text_diff_str = f"{text_emoji}{text_percent:.2f}% ({text_diff:+})"
        data_diff_str = f"{data_emoji}{data_percent:.2f}% ({data_diff:+})"
        bss_diff_str =  f"{bss_emoji}{bss_percent:.2f}% ({bss_diff:+})"
   
        board_name = "_".join(old_path.split(os.sep)[1].split("_")[0:3])
        file_name = os.sep.join([board_name, old_path.split(os.sep)[-1]])
        data.append([file_name, text_diff_str, data_diff_str, bss_diff_str])
    
    if data:
        # Print the table
        print("Code Size Report:")
        print(tabulate(data, headers=headers, tablefmt="github", colalign=("left", "left", "left", "left")))

if __name__ == "__main__":
    main()
