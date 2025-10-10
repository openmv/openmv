# OpenMV library
add_library("openmv-lib", "$(OMV_LIB_DIR)")

# Libraries
require("ml", library="openmv-lib")
