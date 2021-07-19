freeze ("$(PORT_DIR)/modules")
include("$(MPY_DIR)/extmod/uasyncio/manifest.py")
freeze ("$(MPY_LIB_DIR)/", "lsm6dsox.py")
freeze ("$(MPY_LIB_DIR)/", "ble_advertising.py")
