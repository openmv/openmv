# Linkage for the HaLow driver's prebuilt morselib and transceiver blobs.
# morselib drags in libgcc's ARM unwinder, which references the exception-index
# table bounds that these linker scripts do not define, so provide empty ones.
CFLAGS += -DMICROPY_PY_NETWORK_HALOW=1
HALOW_TOP := $(TOP_DIR)/$(MICROPY_DIR)
include $(HALOW_TOP)/drivers/halow/halow.mk
LDFLAGS += -Wl,--defsym=__exidx_start=0 -Wl,--defsym=__exidx_end=0
