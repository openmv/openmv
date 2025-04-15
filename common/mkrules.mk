# SPDX-License-Identifier: MIT
#
# Copyright (C) 2025 OpenMV, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Rules for building firmware object files

# Ensure all necessary directories exist
FIRM_DIRS:
	$(MKDIR) -p $(sort $(BUILD) $(FW_DIR) $(dir $(OMV_FIRM_OBJ)))

# Compile C source files to object files
$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

# Compile C++ source files to object files
$(BUILD)/%.o : %.cc
	$(ECHO) "CXX $<"
	$(CC) $(CXXFLAGS) -MMD -c -o $@ $<

# Assemble raw assembly files (.s)
$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

# Assemble preprocessed assembly files (.S) using the C compiler
$(BUILD)/%.o: %.S
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

# Special rule for compiling certain files with Clang
$(OMV_CLANG_OBJ): $(BUILD)/%.o : %.c
	$(ECHO) "CL $<"
	$(CLANG) $(CLANG_FLAGS) -c -o $@ $<

-include $(MPY_FIRM_OBJ:%.o=%.d) $(OMV_FIRM_OBJ:%.o=%.d)
