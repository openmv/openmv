# Sources
SRC_S = src/st/$(STARTUP).s
SRC_C = src/st/system_stm32fxxx.c

SRC_C  += $(wildcard src/dsp/CommonTables/*.c)
SRC_C  += $(wildcard src/dsp/FastMathFunctions/*.c)
SRC_C  += $(wildcard src/dsp/MatrixFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/ComplexMathFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/ControllerFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/FilteringFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/StatisticsFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/SupportFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/TransformFunctions/*.c)

OBJS  = $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJS += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@
