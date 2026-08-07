# OpenMV library
add_library("openmv-lib", "$(OMV_LIB_DIR)")

# Camera
freeze ("$(OMV_LIB_DIR)/", "sensor.py")

# Libraries
require("ml", library="openmv-lib")
