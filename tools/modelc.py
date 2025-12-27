#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (C) 2025 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# AI models converter.

import sys
import os
import csv
import glob
import argparse
import binascii
import subprocess
import re

C_GREEN = '\033[92m'
C_RED = '\033[91m'
C_BLUE = '\033[94m'
C_RESET = '\033[0m'


def vela_compile(model_path, build_dir, vela_args):
    vela_ini = os.path.dirname(os.path.abspath(__file__))
    model = os.path.basename(os.path.splitext(model_path)[0])

    # Construct the command
    command = [
        'vela',
        *vela_args,
        '--output-dir', build_dir,
        '--config', f'{vela_ini}/vela.ini',
        model_path
    ]

    # Call the command and capture the output
    try:
        result = subprocess.run(command, check=True, text=True, capture_output=True)
    except subprocess.CalledProcessError as e:
        print(e.stderr, file=sys.stderr)
        print(vela_args, file=sys.stderr)

    csv_file_path = glob.glob(f"{build_dir}/{model}_summary_*.csv")[0]
    with open(csv_file_path, mode='r') as file:
        row = next(csv.DictReader(file))
        stoi = lambda x, d=1: str(int(float(x) / d))
        color = lambda c,x: c + x + C_RESET

        summary = {
            C_BLUE + "Network:": C_BLUE + row["network"],
            C_BLUE + "Accelerator Configuration:": C_GREEN + row["accelerator_configuration"],
            C_BLUE + "System Configuration:": C_BLUE + row["system_config"],
            C_BLUE + "Memory Mode:": C_BLUE + row["memory_mode"],
            C_BLUE + "Compiler Mode: ": C_RED + vela_args[-1],
            C_BLUE + "Accelerator Clock:": C_BLUE + stoi(row["core_clock"], 10**6) + " MHz",
            C_BLUE + "SRAM Usage:": C_RED + stoi(row["sram_memory_used"]) + " KiB",
            C_BLUE + "Flash Usage:": C_RED + stoi(row["off_chip_flash_memory_used"]) + " KiB",
            C_BLUE + "Inference Time:": C_GREEN + "%.2f ms, %.2f inferences/s"%
                (float(row["inference_time"]) * 1000, float(row["inferences_per_second"])),
        }
        for key, value in summary.items():
            print(f"{key:<{35}} {value:<{50}}", file=sys.stderr)
        print(C_RESET, file=sys.stderr)
    os.rename(f"{build_dir}/{model}_vela.tflite", f"{build_dir}/{model}.tflite")

def stedge_compile(model_path, build_dir, profile, stedge_args=None):
    core_dir = os.path.realpath(glob.glob("tools/st/stedgeai/[0-9]*")[0])
    config = os.path.realpath("tools/st/scripts/neuralart.json")
    model_name = os.path.basename(os.path.splitext(model_path)[0])
    model_ext = os.path.splitext(model_path)[1]
    output_dir = os.path.join(build_dir, model_name)

    # Remove any Make-related variables that could leak
    env = os.environ.copy()
    env["STEDGEAI_CORE_DIR"] = core_dir
    for var in ["RM", "CFLAGS", "CPPFLAGS", "CXXFLAGS", "LDFLAGS", 'MAKEFLAGS']:
        env.pop(var, None)

    # Step 1: stedgeai generate
    generate_command = [
        os.path.join(core_dir, "Utilities/linux/stedgeai"),
        "generate",
        *stedge_args,
        "--model", model_path,
        "--relocatable",
        "--st-neural-art", f"{profile}@{config}",
        "--workspace", os.path.join(output_dir, "workspace"),
        "--output", os.path.join(output_dir, "gen"),
        "--verbosity", "1",
    ]

    try:
        result = subprocess.run(generate_command, check=True, text=True, env=env,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print(f"stedgeai command failed with exit code {e.returncode}", file=sys.stderr)
        print(" ".join(generate_command), file=sys.stderr)
        raise(e)

    # Step 2: Python relocation script
    reloc_command = [
        sys.executable,  # Uses current Python interpreter
        os.path.join(core_dir, "scripts/N6_reloc/npu_driver.py"),
        "--input", os.path.join(output_dir, "gen", "network.c"),
        "--output", output_dir,
        "--verbosity", "1",
    ]

    try:
        result = subprocess.run(reloc_command, check=True, text=True, env=env,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print(f"Relocation script failed with exit code {e.returncode}", file=sys.stderr)
        print(" ".join(reloc_command), file=sys.stderr)
        raise(e)

    match = re.search(
        r"([ \t]+XIP size.*?Table: mempool.*?\n)", result.stdout, re.DOTALL | re.MULTILINE
    )
    if match:
        output = "\n".join(l.strip() for l in match.group(1).split("\n"))
        print(f"{C_GREEN}Model: {model_name}{C_RESET}")
        print(C_BLUE + output + C_RESET + "\n")

    os.rename(f"{output_dir}/network_rel.bin", f"{build_dir}/{model_name}{model_ext}")

if __name__ == '__main__':
    # python tools/tflite2c.py --input lib/models/fomo_face_detection.tflite --build-dir /tmp/build_st --stedge-args "--target stm32n6"
    # python tools/tflite2c.py --input lib/models/fomo_face_detection.tflite --build-dir /tmp/build --vela-args "--system-config RTSS_HP_DTCM_MRAM --accelerator-config ethos-u55-256 --memory-mode Shared_Sram"
    parser = argparse.ArgumentParser(description='AI models converter.')
    parser.add_argument('--input', action = 'store', help = 'Input model.', required=True)
    parser.add_argument('--build-dir', action = 'store', help='Build directory.', default='build')
    parser.add_argument('--vela-args', action = 'store', help='Vela compiler args.')
    parser.add_argument('--stedge-args', action = 'store', help='STEdge AI tools args.')
    parser.add_argument('--stedge-profile', action = 'store', help='STEdge AI tools profile.', default="default")
    args = parser.parse_args()

    if args.vela_args:
        # Compile the model using Vela.
        vela_compile(args.input, args.build_dir, args.vela_args.split())
    elif args.stedge_args:
        # Compile the model using STEdge AI tools.
        stedge_compile(args.input, args.build_dir, args.stedge_profile, args.stedge_args.split())
    else:
        parser.print_help(sys.stderr)
