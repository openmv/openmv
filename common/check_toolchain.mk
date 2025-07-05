ifeq ($(CPU),cortex-m55)
# Check if GCC version is less than 14.3
GCC_VERSION := $(shell arm-none-eabi-gcc -dumpversion | cut -d. -f1-2)
GCC_MAJOR := $(shell echo $(GCC_VERSION) | cut -d. -f1)
GCC_MINOR := $(shell echo $(GCC_VERSION) | cut -d. -f2)

# Convert to comparable number (14.3 becomes 1403)
GCC_VERSION_NUM := $(shell echo $$(($(GCC_MAJOR) * 100 + $(GCC_MINOR))))

# Only add the flag if version < 14.3 (1403)
ifeq ($(shell test $(GCC_VERSION_NUM) -lt 1403 && echo yes),yes)
$(warning *** ERROR ***)
$(warning GCC $(GCC_VERSION) has known issues with Cortex-M55)
$(warning Upgrade to GCC 14.3+ for proper CM55 support)
$(error )
endif
endif
