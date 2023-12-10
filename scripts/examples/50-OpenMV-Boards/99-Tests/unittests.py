# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# OpenMV Unit Tests.
#
import os
import gc

TEST_DIR = "unittest"
TEMP_DIR = "unittest/temp"
DATA_DIR = "unittest/data"
SCRIPT_DIR = "unittest/script"

if not (TEST_DIR in os.listdir("")):
    raise Exception("Unittest dir not found!")

print("")
test_failed = False


def print_result(test, result):
    s = "Unittest (%s)" % (test)
    padding = "." * (60 - len(s))
    print(s + padding + result)


for test in sorted(os.listdir(SCRIPT_DIR)):
    if test.endswith(".py"):
        test_result = "PASSED"
        test_path = "/".join((SCRIPT_DIR, test))
        try:
            exec(open(test_path).read())
            gc.collect()
            if unittest(DATA_DIR, TEMP_DIR) is False:
                raise Exception()
        except Exception as e:
            if "unavailable" in str(e):
                test_result = "DISABLED"
            else:
                test_failed = True
                test_result = "FAILED"
        print_result(test, test_result)

if test_failed:
    print("\nSome tests have FAILED!!!\n\n")
else:
    print("\nAll tests PASSED.\n\n")
