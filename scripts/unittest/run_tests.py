#!/usr/bin/env python3
"""
OpenMV Unit Test Runner (Host-side)

This script runs OpenMV unit tests on different platforms:
- unix: Run tests on Unix port (desktop MicroPython build)
- hardware: Run tests on hardware via mpremote (future)

Usage:
    python3 run_tests.py --platform unix
    python3 run_tests.py --platform unix --micropython-bin path/to/micropython
"""

import os
import sys
import argparse
import subprocess
import time
from pathlib import Path

# Terminal color codes
COLOR_GREEN = "\033[92m"
COLOR_RED = "\033[91m"
COLOR_YELLOW = "\033[33m"
COLOR_RESET = "\033[0m"

# Get script directory
SCRIPT_DIR = Path(__file__).parent.absolute()
UNITTEST_DIR = SCRIPT_DIR
TESTS_DIR = UNITTEST_DIR / "tests"
DATA_DIR = UNITTEST_DIR / "data"
TEMP_DIR = UNITTEST_DIR / "temp"

# Hardware dependency keywords
HARDWARE_KEYWORDS = [
    "import sensor",
    "import csi",
    "from sensor import",
    "from csi import",
]


def has_hardware_dependency(test_file):
    """Check if a test file requires hardware."""
    try:
        with open(test_file, 'r') as f:
            content = f.read()
            for keyword in HARDWARE_KEYWORDS:
                if keyword in content:
                    return True
    except:
        pass
    return False


def print_result(test_name, result, time_ms):
    """Print test result with color coding."""
    s = f"Unittest ({test_name})"
    padding = "." * (60 - len(s))

    if result == "PASSED":
        colored_result = COLOR_GREEN + result + COLOR_RESET
    elif result == "FAILED":
        colored_result = COLOR_RED + result + COLOR_RESET
    else:  # DISABLED
        colored_result = COLOR_YELLOW + result + COLOR_RESET

    print(f"{s}{padding}{colored_result} ({time_ms}ms)")


def run_test_unix(test_file, micropython_bin, data_path, temp_path):
    """Run a single test on Unix port."""
    test_code = f'''
import sys
sys.path.append("{str(TESTS_DIR)}")

DATA_PATH = "{str(data_path)}"
TEMP_PATH = "{str(temp_path)}"

# Load and execute test
with open("{str(test_file)}", "r") as f:
    test_code = f.read()

exec(test_code)

# Run the unittest function
try:
    result = unittest(DATA_PATH, TEMP_PATH)
    if result is False:
        sys.exit(1)
    sys.exit(0)
except Exception as e:
    error_str = str(e).lower()
    if "unavailable" in error_str or "not available" in error_str:
        # Test requires unavailable feature
        sys.exit(2)
    else:
        # Test failed - print error without traceback module
        print("Test failed:", type(e).__name__, "-", str(e))
        sys.exit(1)
'''

    try:
        result = subprocess.run(
            [micropython_bin, '-c', test_code],
            capture_output=True,
            text=True,
            timeout=30
        )

        if result.returncode == 0:
            return "PASSED", None
        elif result.returncode == 2:
            return "DISABLED", "Feature unavailable"
        else:
            error_msg = result.stderr if result.stderr else result.stdout
            return "FAILED", error_msg

    except subprocess.TimeoutExpired:
        return "FAILED", "Test timeout (30s)"
    except Exception as e:
        return "FAILED", str(e)


def run_tests_unix(args):
    """Run all tests on Unix port."""
    # Create temp directory if needed
    TEMP_DIR.mkdir(exist_ok=True)

    # Get list of tests
    test_files = sorted([f for f in TESTS_DIR.glob("*.py") if f.name != "__init__.py"])

    if not test_files:
        print(f"{COLOR_RED}No test files found in {TESTS_DIR}{COLOR_RESET}")
        return 1

    passed_count = 0
    failed_count = 0
    skipped_count = 0

    print(f"Running {len(test_files)} tests on Unix port...")
    print(f"MicroPython: {args.micropython_bin}")
    print(f"Data dir: {DATA_DIR}")
    print(f"Temp dir: {TEMP_DIR}")
    print("=" * 60)

    for test_file in test_files:
        test_name = test_file.name

        # Check for hardware dependencies
        if has_hardware_dependency(test_file):
            result = "DISABLED"
            error_msg = "Hardware required"
            time_ms = 0
            skipped_count += 1
        else:
            start_time = time.time()
            result, error_msg = run_test_unix(
                test_file,
                args.micropython_bin,
                DATA_DIR,
                TEMP_DIR
            )
            time_ms = int((time.time() - start_time) * 1000)

            if result == "PASSED":
                passed_count += 1
            elif result == "FAILED":
                failed_count += 1
                if args.verbose and error_msg:
                    print(f"\n{COLOR_RED}Error output:{COLOR_RESET}")
                    print(error_msg)
            else:  # DISABLED
                skipped_count += 1

        print_result(test_name, result, time_ms)

    # Print summary
    total_tests = passed_count + failed_count + skipped_count
    print("\n" + "=" * 60)
    print(f"Total: {total_tests} | " +
          f"{COLOR_GREEN}Passed: {passed_count}{COLOR_RESET} | " +
          f"{COLOR_RED}Failed: {failed_count}{COLOR_RESET} | " +
          f"{COLOR_YELLOW}Skipped: {skipped_count}{COLOR_RESET}")
    print("=" * 60)

    if failed_count > 0:
        print(f"{COLOR_RED}Some tests FAILED.{COLOR_RESET}")
        return 1
    else:
        print(f"{COLOR_GREEN}All tests PASSED.{COLOR_RESET}")
        return 0


def main():
    parser = argparse.ArgumentParser(description="OpenMV Unit Test Runner")
    parser.add_argument(
        '--platform',
        choices=['unix', 'hardware'],
        default='unix',
        help='Target platform (unix=desktop, hardware=embedded via mpremote)'
    )
    parser.add_argument(
        '--micropython-bin',
        default='../../lib/micropython/ports/unix/build-openmv/micropython',
        help='Path to Unix MicroPython binary (relative to scripts/unittest/)'
    )
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Verbose output (show error messages)'
    )

    args = parser.parse_args()

    # Resolve micropython binary path
    micropython_path = SCRIPT_DIR / args.micropython_bin
    if not micropython_path.exists():
        print(f"{COLOR_RED}Error: MicroPython binary not found: {micropython_path}{COLOR_RESET}")
        print(f"Build Unix port first: make unix")
        return 1

    args.micropython_bin = str(micropython_path.absolute())

    if args.platform == 'unix':
        return run_tests_unix(args)
    else:
        print(f"{COLOR_RED}Hardware platform not yet implemented{COLOR_RESET}")
        print("Use mpremote to run scripts/unittest/run.py on hardware")
        return 1


if __name__ == "__main__":
    sys.exit(main())
