# Unix port suite: runs the board test files and the C test modules under
# unittest, run as "micropython -m unittest scripts/unittest/unix_tests.py".
import gc
import os
import umalloc
import unittest

# The C test modules are imported unconditionally so that a missing module fails the run.
import unittest_collections
import unittest_fb
import unittest_fmath
import unittest_imlib
import unittest_simd
import unittest_umalloc

ROOT = __file__.rpartition("/")[0] or "."
DATA_PATH = ROOT + "/data"
TEST_PATH = ROOT + "/tests"
TEMP_PATH = "/tmp/openmv-unittest"

# The Unix port provides neither module.
UNAVAILABLE = ("no module named 'csi'", "no module named 'ml'")

try:
    os.mkdir(TEMP_PATH)
except OSError:
    pass


def clean_temp():
    for name in os.listdir(TEMP_PATH):
        os.remove(TEMP_PATH + "/" + name)


def run(test):
    # Same result convention as run.py: False fails, "skip" skips.
    # A test that leaves non-persistent allocations behind fails.
    clean_temp()
    try:
        ret = test()
        leaked = umalloc.stats()[0]
    except ImportError as exc:
        if str(exc) in UNAVAILABLE:
            raise unittest.SkipTest(str(exc))
        raise
    finally:
        gc.collect()
        umalloc.collect()
    if ret == "skip":
        raise unittest.SkipTest("skipped by test")
    assert ret is not False, "test returned False"
    assert leaked == 0, "%d non-persistent allocations remain" % leaked


class OpenMV(unittest.TestCase):
    pass


def add(name, test):
    setattr(OpenMV, "test_" + name, lambda self: run(test))


def python_test(filename):
    def test():
        with open(TEST_PATH + "/" + filename) as f:
            namespace = {}
            exec(f.read(), namespace)
        return namespace["unittest"](DATA_PATH, TEMP_PATH)

    return test


for filename in os.listdir(TEST_PATH):
    if filename.endswith(".py"):
        add(filename[:-3], python_test(filename))

for module in (unittest_collections, unittest_fb, unittest_fmath,
               unittest_imlib, unittest_simd, unittest_umalloc):
    for name in dir(module):
        if name.startswith("test_"):
            add(module.__name__[len("unittest_"):] + "_" + name[len("test_"):],
                getattr(module, name))
