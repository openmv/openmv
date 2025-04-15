# This file is part of the OpenMV project.
#
# Copyright (c) 2019 STMicroelectronics
#
# This work is licensed under the MIT license, see the file LICENSE for details.

# Enable CUBE-AI builtin module
CFLAGS += -DCUBEAI
CFLAGS += -I$(TOP_DIR)/stm32cubeai/
CFLAGS += -I$(TOP_DIR)/stm32cubeai/data/
CFLAGS += -I$(TOP_DIR)/stm32cubeai/AI/Inc/

FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/BasicMathFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/SupportFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/MatrixFunctions/*.o)

FIRM_OBJ += $(addprefix $(BUILD)/stm32cubeai/data/,\
	network.o                       \
	network_data.o                  \
	)

FIRM_OBJ += $(addprefix $(BUILD)/stm32cubeai/,\
	nn_st.o                         \
	py_st_nn.o                      \
	)

LIBS += -l:NetworkRuntime_CM7_GCC.a -Lstm32cubeai/AI/Lib -lc -lm
