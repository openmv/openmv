# Copyright (C) 2023-2024 OpenMV, LLC.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Any redistribution, use, or modification in source or binary form
#    is done solely for personal benefit and not for any commercial
#    purpose or for monetary gain. For commercial licensing options,
#    please contact openmv@openmv.io
#
# THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
# OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
all: $(FW_DIR)/firmware.toc

ifeq ($(MCU_CORE),M55_HP)
ALIF_TOC_APPS = $(FW_DIR)/firmware_M55_HP.bin
else ifeq ($(MCU_CORE),M55_HE)
ALIF_TOC_APPS = $(FW_DIR)/firmware_M55_HE.bin
else
ALIF_TOC_APPS = $(FW_DIR)/bootloader.bin $(FW_DIR)/firmware_M55_HP.bin $(FW_DIR)/firmware_M55_HE.bin
endif

# Force make commands to run the targets every time regardless of whether
# firmware.toc already exists to detect changes in the source files and rebuild.
.PHONY: $(FW_DIR)/bootloader.bin
.PHONY: $(FW_DIR)/firmware_M55_HE.bin
.PHONY: $(FW_DIR)/firmware_M55_HP.bin

ALIF_TOC_CONFIG = $(OMV_PORT_DIR)/alif_cfg.json

$(FW_DIR):
	$(MKDIR) -p $@

$(FW_DIR)/bootloader.bin: | $(FW_DIR)
	make -C $(TOP_DIR)/$(BOOT_DIR) MCU_CORE=M55_HP
	$(Q)cp $(FW_DIR)/bootloader.bin $(TOOLS)/alif/build/images/bootloader.bin

$(FW_DIR)/firmware_M55_HP.bin: | $(FW_DIR)
	make -f $(OMV_PORT_DIR)/alif.mk MCU_CORE=M55_HP MICROPY_PY_OPENAMP_MODE=0
	$(Q)cp $(FW_DIR)/firmware_M55_HP.bin $(TOOLS)/alif/build/images/firmware_M55_HP.bin

$(FW_DIR)/firmware_M55_HE.bin: | $(FW_DIR)
	make -f $(OMV_PORT_DIR)/alif.mk MCU_CORE=M55_HE MICROPY_PY_OPENAMP_MODE=1
	$(Q)cp $(FW_DIR)/firmware_M55_HE.bin $(TOOLS)/alif/build/images/firmware_M55_HE.bin

$(FW_DIR)/firmware.toc: $(ALIF_TOC_APPS)
	$(Q)$(MKDIR) -p $(TOOLS)/alif/build/images/
	$(Q)$(MKDIR) -p $(TOOLS)/alif/build/logs/
	$(Q)cd $(TOOLS)/alif && python app-gen-toc.py -f $(abspath $(ALIF_TOC_CONFIG))
	$(Q)cp $(TOOLS)/alif/build/AppTocPackage.bin $(FW_DIR)/firmware.toc

deploy: $(FW_DIR)/firmware.toc
	$(ECHO) "Writing $< to the board"
	$(Q)cd $(TOOLS)/alif && python app-write-mram.py -p
