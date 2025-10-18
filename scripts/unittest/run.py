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
    tests = sorted(os.listdir(TEST_PATH))

    if not "temp" in os.listdir():
        os.mkdir("temp")  # Make a temp directory

    for test in tests:
        result = "PASSED"
        path = "/".join((TEST_PATH, test))
        start_time = time.ticks_ms()
        try:
            with open(path) as f:
                buf = f.read()
            exec(buf)
            if unittest(DATA_PATH, TEMP_PATH) is False:
                raise Exception()
        except Exception as e:
            if "unavailable" in str(e):
                result = "DISABLED"
            else:
                result = "FAILED"

        # Update counters
        if result == "PASSED":
            passed_count += 1
        elif result == "FAILED":
            failed_count += 1
        else:  # DISABLED
            skipped_count += 1

        time_ms = time.ticks_diff(time.ticks_ms(), start_time)
        print_result(test, result, time_ms)
        gc.collect()

    # Print summary
    total_tests = passed_count + failed_count + skipped_count
    print("\n" + "=" * 60)
    print("Total: %d | " % total_tests +
          COLOR_GREEN + "Passed: %d" % passed_count + COLOR_RESET + " | " +
          COLOR_RED + "Failed: %d" % failed_count + COLOR_RESET + " | " +
          COLOR_YELLOW + "Skipped: %d" % skipped_count + COLOR_RESET)
    print("=" * 60)

    if failed_count > 0:
        print(COLOR_RED + "Some tests FAILED." + COLOR_RESET)
    else:
        print(COLOR_GREEN + "All tests PASSED." + COLOR_RESET)

if __name__ == "__main__":
    main()
