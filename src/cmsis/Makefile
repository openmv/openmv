# Sources
SRCS   = src/st/startup_stm32f407xx.s
#SRCS += $(wildcard src/CommonTables/*.c)
#SRCS += $(wildcard src/ComplexMathFunctions/*.c)
#SRCS += $(wildcard src/ControllerFunctions/*.c)
#SRCS += $(wildcard src/FastMathFunctions/*.c)
#SRCS += $(wildcard src/FilteringFunctions/*.c)
#SRCS += $(wildcard src/MatrixFunctions/*.c)
#SRCS += $(wildcard src/StatisticsFunctions/*.c)
#SRCS += $(wildcard src/SupportFunctions/*.c)
#SRCS += $(wildcard src/TransformFunctions/*.c)

OBJS  = $(addprefix $(BUILD)/, $(SRCS:.s=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(AS) $(AFLAGS) $< -o $@
