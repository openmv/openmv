include("$(PORT_DIR)/variants/manifest.py")

include("$(MPY_DIR)/extmod/asyncio")

c_module("$(OMV_DIR)/modules")

# Test runner for scripts/unittest/unix_tests.py ("micropython -m unittest").
require("unittest-discover")
