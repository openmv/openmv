# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OpenMV Unit Tests.
#
import os
import gc
import time
import umalloc

DATA_PATH = "/rom"
TEMP_PATH = "/remote/temp"
TEST_PATH = "/remote/tests"

# Terminal color codes
COLOR_GREEN = "\033[92m"  # Bright green
COLOR_RED = "\033[91m"    # Bright red
COLOR_YELLOW = "\033[33m"
COLOR_RESET = "\033[0m"

UNITTEST_MODULES = [
    "unittest_fmath",
    "unittest_collections",
    "unittest_imlib",
    "unittest_fb",
    "unittest_simd",
    "unittest_umalloc",
    "unittest_ccm",
    "unittest_awb",
]


def print_result(test, result, time_ms, stats):
    s = "Unittest (%s)" % (test)
    padding = "." * (60 - len(s))

    # Color the result based on status
    if result == "PASSED":
        colored_result = COLOR_GREEN + result + COLOR_RESET
    elif result == "FAILED" or result == "LEAKED":
        colored_result = COLOR_RED + result + COLOR_RESET
    else:  # SKIPPED
        colored_result = COLOR_YELLOW + result + COLOR_RESET

    print(s + padding + colored_result + " (%dms)" % time_ms)

    if (result == "FAILED" or result == "LEAKED") and stats is not None:
        used, free, persist, used_bytes, free_bytes, persist_bytes, peak_bytes = stats
        print(COLOR_RED + "used: %d (%d B)  free: %d (%d B)  persist: %d (%d B)  peak: %d B" %
              (used, used_bytes, free, free_bytes, persist, persist_bytes, peak_bytes) + COLOR_RESET)

def main():
    passed_count = 0
    failed_count = 0
    skipped_count = 0

    # Python tests from TEST_PATH
    tests = [(None, t) for t in os.listdir(TEST_PATH)]

    # C unit test modules
    for mod_name in UNITTEST_MODULES:
        try:
            mod = __import__(mod_name)
            for name in dir(mod):
                if name.startswith("test_"):
                    tests.append((mod, name))
        except ImportError:
            pass

    if not "temp" in os.listdir():
        os.mkdir("temp")  # Make a temp directory

    total_start_time = time.ticks_ms()

    for mod, test in sorted(tests, key=lambda x: x[1]):
        stats = None
        start_ms = time.ticks_ms()
        try:
            if mod is None:
                # Python test file
                with open("/".join((TEST_PATH, test))) as f:
                    buf = f.read()
                exec(buf)
                ret = unittest(DATA_PATH, TEMP_PATH)
            else:
                # C unit test function
                ret = getattr(mod, test)()
                # Use shorter name for display (strip test_ prefix)
                test = test.split("test_")[1]

            stats = umalloc.stats()
            if ret is False:
                raise Exception()
            if ret == "skip":
                raise Exception("SKIPPED")
            if stats[0] > 0:
                raise Exception("LEAKED")

            result = "PASSED"
            passed_count += 1
        except Exception as e:
            if "SKIPPED" in str(e):
                result = "SKIPPED"
                skipped_count += 1
            elif "LEAKED" in str(e):
                result = "LEAKED"
                failed_count += 1
            else:
                result = "FAILED"
                failed_count += 1

        time_ms = time.ticks_diff(time.ticks_ms(), start_ms)
        gc.collect()

        if False: #stats is not None:
            used, free, persist, used_bytes, free_bytes, persist_bytes = stats
            print(COLOR_RED + "used: %d (%d B)  free: %d (%d B)  persist: %d (%d B)" %
                  (used, used_bytes, free, free_bytes, persist, persist_bytes) + COLOR_RESET)

        umalloc.collect()
        print_result(test, result, time_ms, stats)

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
