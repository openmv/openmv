#!/usr/bin/env python3
# This work is licensed under the MIT license.
# Copyright (c) 2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Host-side wrapper that runs scripts/unittest/run.py against the
# unix-port micropython binary. The same run.py drives both targets;
# this wrapper just supplies the test/data/temp paths to it.

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.absolute()
WORKSPACE_ROOT = SCRIPT_DIR.parents[1]


def safe_temp_dir(value):
    """Reject temp paths that escape the workspace or system tempdir."""
    p = Path(value).resolve()
    sys_tmp = Path(tempfile.gettempdir()).resolve()
    if not (p == WORKSPACE_ROOT or p.is_relative_to(WORKSPACE_ROOT) or
            p.is_relative_to(sys_tmp)):
        raise argparse.ArgumentTypeError(
            f"--temp must be inside {WORKSPACE_ROOT} or {sys_tmp}; got {p}"
        )
    return p


def main():
    parser = argparse.ArgumentParser(description="OpenMV Unit Test Runner (Unix port)")
    parser.add_argument(
        "--micropython-bin",
        default=str(SCRIPT_DIR / "../../lib/micropython/ports/unix/build-openmv/micropython"),
        help="Path to the unix-port MicroPython binary",
    )
    parser.add_argument("--filter", default=None,
                        help="Substring filter on test name (e.g. --filter blob)")
    parser.add_argument("--tests", default=str(SCRIPT_DIR / "tests"))
    parser.add_argument("--data", default=str(SCRIPT_DIR / "data"))
    parser.add_argument("--temp", type=safe_temp_dir, default=str(SCRIPT_DIR / "temp"))
    args = parser.parse_args()

    binary = Path(args.micropython_bin).resolve()
    if not binary.exists():
        print(f"MicroPython binary not found: {binary}", file=sys.stderr)
        print("Build the Unix port first: `make TARGET=UNIX` from the repo root.",
              file=sys.stderr)
        return 1

    Path(args.temp).mkdir(parents=True, exist_ok=True)

    # run.py reads paths from sys.argv: TEST_PATH DATA_PATH TEMP_PATH [FILTER]
    cmd = [str(binary), str(SCRIPT_DIR / "run.py"),
           args.tests, args.data, str(args.temp)]
    if args.filter:
        cmd.append(args.filter)

    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())
