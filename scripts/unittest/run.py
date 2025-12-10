# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OpenMV Unit Tests.
#
import os
import gc
import time

TEMP_PATH = "/remote/temp"
DATA_PATH = "/remote/data"
TEST_PATH = "/remote/tests"

# Terminal color codes
COLOR_GREEN = "\033[92m"  # Bright green
COLOR_RED = "\033[91m"    # Bright red
COLOR_YELLOW = "\033[33m"
COLOR_RESET = "\033[0m"


def print_result(test, result, time_ms):
    s = "Unittest (%s)" % (test)
    padding = "." * (60 - len(s))

    # Color the result based on status
    if result == "PASSED":
        colored_result = COLOR_GREEN + result + COLOR_RESET
    elif result == "FAILED":
        colored_result = COLOR_RED + result + COLOR_RESET
    else:  # DISABLED
        colored_result = COLOR_YELLOW + result + COLOR_RESET

    print(s + padding + colored_result + " (%dms)" % time_ms)


def main():
    passed_count = 0
    failed_count = 0
    skipped_count = 0
    tests = os.listdir(TEST_PATH)

    if not "temp" in os.listdir():
        os.mkdir("temp")  # Make a temp directory

    total_start_time = time.ticks_ms()

    try:
        import unittest as ut

        # Add unit tests from C module.
        tests.extend([f.split("test_")[1] for f in dir(ut) if f.startswith("test_")])
    except ImportError:
        ut = None

    for test in sorted(tests):
        start_ms = time.ticks_ms()
        try:
            if test.endswith(".py"):
                with open("/".join((TEST_PATH, test))) as f:
                    buf = f.read()
                exec(buf)
                if unittest(DATA_PATH, TEMP_PATH) is False:
                    raise Exception()
            elif getattr(ut, "test_" + test)() is False:
                    raise Exception()
            result = "PASSED"
            passed_count += 1
        except Exception as e:
            if "unavailable" in str(e):
                result = "DISABLED"
                skipped_count += 1
            else:
                result = "FAILED"
                failed_count += 1

        time_ms = time.ticks_diff(time.ticks_ms(), start_ms)
        print_result(test, result, time_ms)
        gc.collect()

    total_time_ms = time.ticks_diff(time.ticks_ms(), total_start_time)

    # Print summary
    total_tests = passed_count + failed_count + skipped_count
    stats_line = ("Total: %d | Passed: %d | Failed: %d | Skipped: %d | Time: %dms" %
                  (total_tests, passed_count, failed_count, skipped_count, total_time_ms))
    separator = "=" * len(stats_line)

    print("\n" + separator)
    print("Total: %d | " % total_tests +
          COLOR_GREEN + "Passed: %d" % passed_count + COLOR_RESET + " | " +
          COLOR_RED + "Failed: %d" % failed_count + COLOR_RESET + " | " +
          COLOR_YELLOW + "Skipped: %d" % skipped_count + COLOR_RESET + " | " +
          "Time: %dms" % total_time_ms)
    print(separator)

    if failed_count > 0:
        print(COLOR_RED + "Some tests FAILED." + COLOR_RESET)
    else:
        print(COLOR_GREEN + "All tests PASSED." + COLOR_RESET)

if __name__ == "__main__":
    main()
