# OpenMV Unit Tests.
#
import os, sensor, gc

TEST_DIR    = "unittest"
TEMP_DIR    = "unittest/temp"
DATA_DIR    = "unittest/data"
SCRIPT_DIR  = "unittest/script"

if not (TEST_DIR in os.listdir("")):
    raise Exception('Unittest dir not found!')

print("")
test_failed = False

def print_result(test, passed):
    s = "Unittest (%s)"%(test)
    padding = "."*(60-len(s))
    print(s + padding + ("PASSED" if passed == True else "FAILED"))

for test in sorted(os.listdir(SCRIPT_DIR)):
    if test.endswith(".py"):
        test_passed = True
        test_path = "/".join((SCRIPT_DIR, test))
        try:
            exec(open(test_path).read())
            gc.collect()
            if unittest(DATA_DIR, TEMP_DIR) == False:
                raise Exception()
        except Exception as e:
            test_failed = True
            test_passed = False
        print_result(test, test_passed)

if test_failed:
    print("\nSome tests have FAILED!!!\n\n")
else:
    print("\nAll tests PASSED.\n\n")
