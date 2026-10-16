# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OpenMV Unit Tests.
#
# Runs on hardware via mpremote (`mount scripts/unittest/ run run.py`),
# and on the Unix port via `run_tests.py`. The wrapper passes paths
# through sys.argv so the same script drives both targets.
#
# Skip policy (a test is reported SKIPPED if it):
#   1. returns the string "skip" from `unittest()`,
#   2. raises ImportError on a known optional module
#      (sensor, csi, ml, fir, audio, display, tv, imu, tof, omv),
#   3. raises Exception with "SKIPPED" or "unavailable" in the message
#      text (covers tests using the explicit "X unavailable" idiom and
#      C-level OSError("This function is unavailable on your OpenMV Cam.")
#      raised by modules/py_helper.c:py_func_unavailable).
# Anything else is FAILED.
#
import os
import gc
import sys
import time
import umalloc

# Default to the on-device mpremote-mount layout.
TEST_PATH = "/remote/tests"
DATA_PATH = "/rom"
TEMP_PATH = "/remote/temp"

# Optional positional overrides: run.py [TEST_PATH [DATA_PATH [TEMP_PATH]]].
# Used by the host-side run_tests.py wrapper to point at the workspace.
HOSTED = len(sys.argv) > 1
if len(sys.argv) > 1:
    TEST_PATH = sys.argv[1]
if len(sys.argv) > 2:
    DATA_PATH = sys.argv[2]
if len(sys.argv) > 3:
    TEMP_PATH = sys.argv[3]

# Optional substring filter on test name (matches scripts/unittest/run_tests.py).
TEST_FILTER = sys.argv[4] if len(sys.argv) > 4 else None

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
]

# ImportError on these module names is treated as SKIPPED (the test
# depends on a hardware feature that's disabled on this build).
OPTIONAL_MODULES = {
    "audio", "csi", "display", "fir", "imu", "ml", "omv",
    "sensor", "tof", "tv",
}


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

    if TEST_FILTER:
        tests = [t for t in tests if TEST_FILTER in t[1]]

    if "temp" not in os.listdir():
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
        except ImportError as e:
            # Skip on a missing optional module; treat any other
            # ImportError as a real test failure so a regression
            # breaking `import image` etc. does not silently SKIP.
            # MicroPython's ImportError lacks the `name` attribute, so
            # parse the module name from the canonical message text:
            # "no module named 'X'".
            msg = str(e)
            mod_name = msg.split("'")[1] if msg.count("'") >= 2 else ""
            if mod_name in OPTIONAL_MODULES:
                result = "SKIPPED"
                skipped_count += 1
            else:
                result = "FAILED"
                failed_count += 1
        except Exception as e:
            msg = str(e)
            if "SKIPPED" in msg or "unavailable" in msg:
                result = "SKIPPED"
                skipped_count += 1
            elif "LEAKED" in msg:
                result = "LEAKED"
                failed_count += 1
            else:
                result = "FAILED"
                failed_count += 1

        time_ms = time.ticks_diff(time.ticks_ms(), start_ms)
        gc.collect()

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
        # Only propagate failure as a process exit when invoked from
        # the host wrapper (mpremote `run` doesn't expect SystemExit).
        if HOSTED:
            sys.exit(1)
    else:
        print(COLOR_GREEN + "All tests PASSED." + COLOR_RESET)

if __name__ == "__main__":
    main()
